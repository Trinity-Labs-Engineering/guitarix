/*
 * Copyright (C) 2026 Trinity Labs Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * DSP adapted from TONE3000 plugin/include/PitchShift.h and
 * plugin/src/PitchShift.cpp at 01a842a6064300e90a61e59505b4bfd52b58047d.
 * Upstream: https://github.com/tone-3000/tone3000-plugin
 * The original DSP is distributed under the following license:
 *
 * MIT License
 *
 * Copyright (c) 2026 TONE3000.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace gx_engine {

// Tuning, all in ms so every rate behaves the same. The values are the ones
// upstream's plugin/docs/pitch-shift.md bench settled on; none is exposed.
namespace tone3000_pitch {
// A drift splice crossfades over kFadeMs when its lag matches well and
// stretches toward kFadeMaxMs as the match worsens (ncc from kFadeNccHi
// down to kFadeNccLo). On a dissonant chord no lag lines every partial up;
// a short fade turns each mismatched partial's phase step into a click,
// a long one spreads it below the chord. kFadeMinMs is the shortest fade
// the geometry plans for: a splice landing at the far end of its range
// has only that much buffer left to fade in.
constexpr double kFadeMs = 30.0;
constexpr double kFadeMaxMs = 120.0;
constexpr double kFadeMinMs = 6.0;
constexpr double kFadeNccHi = 0.95;
constexpr double kFadeNccLo = 0.6;
constexpr double kOnsetFadeMs = 2.0;     // crossfade of an onset re-sync
constexpr double kMaxCorrMs = 25.0;      // correlation window (capped at the buffer)
constexpr double kOnsetSpanMs = 4.0;     // an onset re-sync lands within this of the floor
constexpr double kRefractoryMs = 40.0;   // one re-sync per attack
constexpr double kDetectorHpfHz = 600.0; // attacks are HF-rich, low fundamentals never ripple it
constexpr double kDetectorSmoothMs = 2.0;
constexpr int kHistoryCells = 50;        // 1 ms energy cells kept for the detector
constexpr int kHistorySkipCells = 5;     // the attack itself is not its own reference
constexpr double kOnsetOverMin = 7.94;   // 9 dB over the recent floor ...
constexpr double kOnsetOverMax = 3.98;   // ... and 6 dB over the recent ceiling (a new peak;
                                         // a beating dyad swings its HF energy by less)
// Lag search granularity: 4 samples at 48 kHz, refined to the sample around
// the coarse best. Free in quality on the bench, 3x cheaper than exhaustive.
constexpr double kCoarseStepRate = 12000.0;
// A drift splice is predictable (the tap approaches the buffer end at a
// known rate), so its lag search is spread over the samples of this lead
// instead of running as one burst: at a 16-sample host buffer the burst
// alone (400-800 us at 48 kHz) blew the callback budget and clicked.
constexpr double kSearchLeadMs = 4.0;
// An upshift tap gains on the write head, so every fade and every search
// lead spends buffer at the drift rate (three samples a sample at +24).
// Whatever they leave is the range of jumps a splice may choose from, and
// a splice is only clean at a whole number of periods: the range has to be
// at least a period long for every note to have one. Half the buffer is
// kept for it; where the fades and the lead don't fit beside that (above
// +8 on the 20 ms buffer, +11 on 30 ms) they shrink to make room. Smaller
// shares keep longer fades but leave notes whose period the range misses
// off-pitch (plugin/docs/pitch-shift.md, Two octaves).
constexpr double kUpshiftLandShare = 0.5;
// A pitch change smaller than this (in ratio; ~1.7 st around unity) keeps a
// pending search: the landing moves by at most the change times the lead,
// ~20 samples, which the landing range's margins cover.
constexpr double kSearchKeepRatio = 0.1;
// Prime the delay then blend it in over the same 25 ms as TONE3000.
constexpr double kBlendSeconds = 0.025;
constexpr double kPi = 3.14159265358979323846;
constexpr double kMinDelayMs = 2.0;

struct State {
  // Power-of-two ring indexed by absolute sample number; sample i lives at
  // buf[i & mask]. Reads are 4-point Hermite so a fractional tap is smooth.
  struct Ring {
    std::vector<float> buf;
    uint32_t mask = 0;
    void init(int minSize) {
      int size = 1;
      while (size < minSize) size <<= 1;
      buf.assign(static_cast<size_t>(size), 0.0f);
      mask = static_cast<uint32_t>(size - 1);
    }
    void clear() { std::fill(buf.begin(), buf.end(), 0.0f); }
    void write(int64_t i, float x) { buf[static_cast<uint32_t>(i) & mask] = x; }
    float at(int64_t i) const { return buf[static_cast<uint32_t>(i) & mask]; }
    float read(double pos) const {
      const auto i = static_cast<int64_t>(std::floor(pos));
      const float t = static_cast<float>(pos - static_cast<double>(i));
      const float xm1 = at(i - 1), x0 = at(i), x1 = at(i + 1), x2 = at(i + 2);
      const float c = 0.5f * (x1 - xm1);
      const float v = x0 - x1;
      const float w = c + v;
      const float a = w + v + 0.5f * (x2 - x0);
      const float b = w + a;
      return ((a * t - b) * t + c) * t + x0;
    }
  };

  // The control ring is mirrored (every sample written twice, `size`
  // apart) so any window ending at sample i is contiguous memory: the
  // correlation is then a plain dot product the compiler vectorises.
  struct MirroredRing {
    std::vector<float> buf;
    uint32_t mask = 0;
    int size = 0;
    void init(int minSize) {
      size = 1;
      while (size < minSize) size <<= 1;
      buf.assign(static_cast<size_t>(size) * 2, 0.0f);
      mask = static_cast<uint32_t>(size - 1);
    }
    void clear() { std::fill(buf.begin(), buf.end(), 0.0f); }
    void write(int64_t i, float x) {
      const auto k = static_cast<uint32_t>(i) & mask;
      buf[k] = x;
      buf[k + static_cast<uint32_t>(size)] = x;
    }
    // The n samples before sample i (excluding it), oldest first.
    const float* windowEndingAt(int64_t i, int n) const {
      const auto k = static_cast<uint32_t>(i) & mask;
      return buf.data() + (static_cast<int>(k) + size - n);
    }
  };

  double sampleRate = 0.0;
  double ratio = 1.0;

  // The adapter is mono, as is Guitarix's existing Pitch Shift effect.
  Ring ring;
  MirroredRing control;
  int primeLeft = 0, blendLength = 1, blendLeft = 0;
  float wetMix = 0.0f, blendStep = 1.0f;

  int64_t written = 0;  // samples written so far; the newest is written - 1

  // Geometry at the current rate / window, in samples. fadeLo..fadeHi is
  // the drift fade's range and searchLeadNow the drift search's lead here
  // (the constants above, cut down by the buffer and the ratio, see
  // updateFadeRange).
  int dMin = 0, dMax = 0, corrLen = 0;
  int fadeLen = 0, fadeMaxLen = 0, fadeMinLen = 0, fadeLo = 0, fadeHi = 0, onsetFadeLen = 0;
  int onsetSpan = 0, refractory = 0, cellLen = 1, coarseStep = 1, searchLead = 1, searchLeadNow = 1;

  // Read taps (absolute positions) and the crossfade between them. fadeNcc
  // is how well the two correlate, which sets the fade's gain law.
  double rA = 0.0, rB = 0.0;
  bool fading = false;
  double fade = 0.0, fadeInc = 0.0, fadeNcc = 1.0;

  // Lag search, incremental. The reference is the tap's recent waveform,
  // copied when the search starts; candidates are buffer positions scored
  // as damage per splice (1 - ncc) over the seconds the jump buys, so a
  // long jump with a good match beats a short perfect one (max-ncc alone
  // picks one-period hops and splices constantly on low dyads). The result
  // is a tap jump J (new position = tap + J); a jump found a few ms early
  // stays valid because both segments move together on a sustained note.
  struct Search {
    bool active = false, ready = false;
    int lo = 0, hi = 0, next = 0, perSample = 1;
    double bestScore = -1e9, bestNcc = 0.0;
    int bestDelay = 0;
    int64_t now0 = 0, a0 = 0;
    int64_t resultJump = 0;
  } search;
  std::vector<float> ref;
  double refXX = 0.0;

  // Onset detector: first-order HPF, 2 ms energy smoother, 1 ms min/max
  // history cells.
  double hpA = 0.0, hpZ1 = 0.0, hpZ2 = 0.0;
  double smoothA = 0.0, eHf = 0.0;
  std::array<float, kHistoryCells> minHist{}, maxHist{};
  double minAcc = 1e9, maxAcc = 0.0;
  int64_t lastOnset = 0;

  // The longest drift fade and the search lead this ratio and window
  // allow. An upshift tap gains on the write head, so one cycle spends
  // buffer on the fade that lands the tap, the next search's lead and the
  // fade after it, all at the drift rate (`landLo` in process()). The
  // longest fade is held to a sixth of the range, and where the cycle
  // would leave less than kUpshiftLandShare of the buffer to land in, the
  // fade and the lead scale down together until it does. A downshift tap
  // only runs deeper, which costs ring memory, not range, so it keeps the
  // full span and the full lead.
  void updateFadeRange() {
    const double drift = std::abs(1.0 - ratio);
    double hi = fadeMaxLen;
    double lead = searchLead;
    int floor = fadeMinLen;
    if (ratio > 1.0) {
      hi = std::max<double>(fadeMinLen, std::min(hi, static_cast<double>(dMax - dMin) / (6.0 * drift)));
      const double cost = drift * (2.0 * hi + lead + 4.0) + 6.0;
      const double budget = dMax * (1.0 - kUpshiftLandShare);
      if (cost > budget) {
        const double scale = budget / cost;
        hi *= scale;
        lead *= scale;
        floor = 8;
      }
    }
    fadeHi = std::max(floor, static_cast<int>(hi));
    fadeLo = std::min(fadeLen, fadeHi);
    searchLeadNow = std::max(1, static_cast<int>(lead));
  }

  // Delay floor the tap may approach at the current ratio: an upshift tap
  // gains on the write head during a fade, so the floor moves out to keep
  // the interpolator behind the newest sample.
  int lowGuard() const {
    return std::max(dMin, static_cast<int>(std::max(0.0, ratio - 1.0) * fadeHi) + 4);
  }

  // Fade length for a drift splice whose best lag matched with `ncc`,
  // within `room`, the samples the destination tap can fade before it runs
  // out of buffer.
  int fadeFor(double ncc, double room) const {
    const double t = std::max(0.0, std::min(1.0, (kFadeNccHi - ncc) / (kFadeNccHi - kFadeNccLo)));
    const double len = fadeLo + t * (fadeHi - fadeLo);
    return std::max(8, static_cast<int>(std::min(len, room)));
  }

  void reset() {
    ring.clear();
    control.clear();
    written = 0;
    ratio = 1.0;
    updateFadeRange();
    wetMix = 0.0f;
    blendLeft = 0;
    rA = rB = static_cast<double>(written - dMin);  // start at the floor
    primeLeft = dMin + 8;
    fading = false;
    search = Search{};
    hpZ1 = hpZ2 = 0.0;
    eHf = 0.0;
    minHist.fill(1e9f);
    maxHist.fill(0.0f);
    minAcc = 1e9;
    maxAcc = 0.0;
    lastOnset = -(1 << 30);
  }

  // Normalised cross-correlation of the reference with the corrLen control
  // samples ending at `b`.
  double nccAt(int64_t b) const {
    const float* y = control.windowEndingAt(b, corrLen);
    const float* x = ref.data();
    // Eight independent accumulators: float sums can't be reassociated by
    // the compiler, so this is what lets the loop run as SIMD lanes.
    float xy[8] = {}, yy[8] = {};
    int i = 0;
    for (; i + 8 <= corrLen; i += 8)
      for (int k = 0; k < 8; ++k) {
        xy[k] += x[i + k] * y[i + k];
        yy[k] += y[i + k] * y[i + k];
      }
    for (; i < corrLen; ++i) {
      xy[0] += x[i] * y[i];
      yy[0] += y[i] * y[i];
    }
    double sxy = 0.0, syy = 0.0;
    for (int k = 0; k < 8; ++k) {
      sxy += xy[k];
      syy += yy[k];
    }
    return sxy / (std::sqrt(refXX * syy) + 1e-9);
  }

  // Start scoring candidate delays [lo, hi] against the tap's waveform now,
  // spread over `lead` samples (0: all at once, for the onset re-sync).
  void beginSearch(int lo, int hi, int lead) {
    search.active = true;
    search.ready = false;
    search.now0 = written - 1;
    search.a0 = static_cast<int64_t>(std::floor(rA));
    search.lo = lo;
    search.hi = std::max(lo, hi);
    search.next = lo;
    search.bestScore = -1e9;
    search.bestDelay = lo;
    const float* x = control.windowEndingAt(search.a0, corrLen);
    std::copy(x, x + corrLen, ref.begin());
    refXX = 0.0;
    for (int i = 0; i < corrLen; ++i) refXX += static_cast<double>(x[i]) * x[i];
    const int candidates = (search.hi - search.lo) / coarseStep + 1;
    search.perSample = lead > 0 ? (candidates + lead - 1) / lead : candidates;
  }

  void consider(int d) {
    const double curDelay = static_cast<double>(search.now0 - search.a0);
    const double jump = std::max(1.0, std::abs(curDelay - static_cast<double>(d)));
    const double ncc = nccAt(search.now0 - d);
    const double s = -(1.0 - ncc) / jump;
    if (s > search.bestScore) {
      search.bestScore = s;
      search.bestDelay = d;
      search.bestNcc = ncc;
    }
  }

  // Scores up to `count` coarse candidates; on the last one refines to the
  // sample around the best and publishes the jump.
  void stepSearch(int count) {
    while (count-- > 0 && search.next <= search.hi) {
      consider(search.next);
      search.next += coarseStep;
    }
    if (search.next > search.hi) {
      if (coarseStep > 1) {
        const int c = search.bestDelay;
        for (int d = std::max(search.lo, c - coarseStep + 1); d <= std::min(search.hi, c + coarseStep - 1); ++d)
          if (d != c) consider(d);
      }
      search.resultJump = (search.now0 - search.bestDelay) - search.a0;
      search.active = false;
      search.ready = true;
    }
  }

  // Crossfade to the tap position rA + jump, the one the search just found.
  // A plan kept across a pitch change is checked here: a
  // destination ahead of the write head or off the ring is dropped and the
  // next sample plans afresh.
  void startFade(int64_t jump, int fadeSamples) {
    search.ready = false;
    const double destDelay = static_cast<double>(written - 1) - (rA + static_cast<double>(jump));
    if (destDelay < 2.0 || destDelay > static_cast<double>(ring.buf.size()) - 8.0) return;
    // Keep rA's fraction so the splice is a pure integer lag.
    rB = rA + static_cast<double>(jump);
    fading = true;
    fade = 0.0;
    fadeInc = 1.0 / fadeSamples;
    fadeNcc = std::max(0.0, std::min(1.0, search.bestNcc));
  }

  // Runs the detector on one control sample; true on a pick attack.
  bool detectOnset(double x) {
    const int64_t now = written - 1;
    const double hp = x - hpZ1 + hpA * hpZ2;  // y = x - x[-1] + a * y[-1]
    hpZ1 = x;
    hpZ2 = hp;
    eHf = smoothA * eHf + (1.0 - smoothA) * hp * hp;
    const int64_t cellNow = now / cellLen;
    if (now % cellLen == 0 && now > 0) {
      // Commit the finished cell.
      const auto prev = static_cast<size_t>((cellNow - 1) % kHistoryCells);
      minHist[prev] = static_cast<float>(minAcc);
      maxHist[prev] = static_cast<float>(maxAcc);
      minAcc = 1e9;
      maxAcc = 0.0;
    }
    minAcc = std::min(minAcc, eHf);
    maxAcc = std::max(maxAcc, eHf);
    double recentMin = 1e9, recentMax = 0.0;
    for (int back = kHistorySkipCells; back < kHistoryCells; ++back) {
      const auto c = static_cast<size_t>((cellNow - back + 4 * kHistoryCells) % kHistoryCells);
      recentMin = std::min(recentMin, static_cast<double>(minHist[c]));
      recentMax = std::max(recentMax, static_cast<double>(maxHist[c]));
    }
    return eHf > 1e-7 && eHf > recentMin * kOnsetOverMin && eHf > recentMax * kOnsetOverMax;
  }
};

} // namespace tone3000_pitch

/*
 * TONE3000's correlation-spliced delay line: Hermite read taps, incremental
 * damage-per-splice lag search, correlation-dependent fades and onset re-sync.
 * This mono adapter preserves Guitarix's parameter ABI and independent Wet/Dry
 * gains without linking JUCE. With neutral legacy bands and full Wet it follows
 * the upstream pure-shift path, including its startup blend. At unity the tap
 * stops drifting; nominal latency is the mean delay, not a fixed wet delay.
 *
 * prepare() is the only allocating operation. reset() clears resident scene
 * history without allocation. process() accepts arbitrary blocks and in-place
 * buffers. See documentation/tone3000-pitch-shift.md for the port boundary.
 */
class PolyphonicPitchShifter {
private:
    tone3000_pitch::State state;
    float tone_low = 0.0f, tone_low_mid = 0.0f, tone_high_mid = 0.0f;
    float tone_low_coefficient = 0.0f, tone_low_mid_coefficient = 0.0f;
    float tone_high_mid_coefficient = 0.0f;

    float colour(float input, float low, float low_mid,
                 float high_mid, float high)
    {
        const float low_band = tone_low +=
            tone_low_coefficient*(input - tone_low);
        const float low_mid_sum = tone_low_mid +=
            tone_low_mid_coefficient*(input - tone_low_mid);
        const float high_mid_sum = tone_high_mid +=
            tone_high_mid_coefficient*(input - tone_high_mid);
        // This crossover reconstructs input exactly when all four legacy
        // Guitarix band controls are at their default value of one.
        if (low == 1.0f && low_mid == 1.0f && high_mid == 1.0f && high == 1.0f)
            return input;
        return low*low_band
            + low_mid*(low_mid_sum - low_band)
            + high_mid*(high_mid_sum - low_mid_sum)
            + high*(input - high_mid_sum);
    }

    static float one_pole_coefficient(double sample_rate, double frequency)
    {
        const double pi2 = 6.28318530717958647692;
        return static_cast<float>(1.0 - std::exp(-pi2*frequency/sample_rate));
    }

public:
    void prepare(int new_sample_rate, int quality)
    {
        using namespace tone3000_pitch;
        auto& s = state;
        const double sampleRate = std::max(8000, new_sample_rate);
        s.sampleRate = sampleRate;
        // Houston already selects mode 1: use TONE3000's default 30 ms.
        // Preserve the legacy quality setting for existing Guitarix presets.
        const double windowMs = quality <= 0 ? 60.0 : quality >= 2 ? 20.0 : 30.0;
        s.dMin = static_cast<int>(sampleRate * kMinDelayMs * 0.001);
        s.dMax = static_cast<int>(sampleRate * windowMs * 0.001);
        s.corrLen = std::min(s.dMax, static_cast<int>(sampleRate * kMaxCorrMs * 0.001));
        s.fadeLen = std::max(8, static_cast<int>(sampleRate * kFadeMs * 0.001));
        s.fadeMaxLen = std::max(s.fadeLen, static_cast<int>(sampleRate * kFadeMaxMs * 0.001));
        s.fadeMinLen = std::max(8, static_cast<int>(sampleRate * kFadeMinMs * 0.001));
        s.onsetFadeLen = std::max(8, static_cast<int>(sampleRate * kOnsetFadeMs * 0.001));
        s.onsetSpan = static_cast<int>(sampleRate * kOnsetSpanMs * 0.001);
        s.refractory = static_cast<int>(sampleRate * kRefractoryMs * 0.001);
        s.searchLead = std::max(1, static_cast<int>(sampleRate * kSearchLeadMs * 0.001));
        s.cellLen = std::max(1, static_cast<int>(sampleRate * 0.001));
        // Match JUCE roundToInt's ties-to-even rounding at rates such as 30 kHz.
        s.coarseStep = std::max(1, static_cast<int>(std::lrint(sampleRate / kCoarseStepRate)));
        s.hpA = std::exp(-2.0 * kPi * kDetectorHpfHz / sampleRate);
        s.smoothA = std::exp(-1.0 / (sampleRate * kDetectorSmoothMs * 0.001));
        // Rings hold the largest window plus the correlation reach behind the
        // farthest candidate and the overshoot of a late search and of a
        // downshift tap running deeper through the longest fade.
        const int maxCorr = static_cast<int>(sampleRate * kMaxCorrMs * 0.001);
        const int reach = static_cast<int>(sampleRate * 0.060) + maxCorr + s.fadeMaxLen + 2 * s.searchLead + 64;
        s.ring.init(reach);
        s.control.init(reach);
        s.ref.assign(static_cast<size_t>(maxCorr), 0.0f);
        s.blendLength = std::max(1, static_cast<int>(std::floor(sampleRate * kBlendSeconds)));
        s.blendStep = 1.0f / s.blendLength;
        tone_low_coefficient = one_pole_coefficient(sampleRate, 180.0);
        tone_low_mid_coefficient = one_pole_coefficient(sampleRate, 900.0);
        tone_high_mid_coefficient = one_pole_coefficient(sampleRate, 4000.0);
        reset();
    }

    void reset()
    {
        state.reset();
        tone_low = tone_low_mid = tone_high_mid = 0.0f;
    }

    int latency_samples() const { return (state.dMin + state.dMax) / 2; }

    void process(const float *input, float *output, int count,
                 double target_ratio, float wet, float dry,
                 bool compensate_dry = false, float low = 1.0f, float low_mid = 1.0f,
                 float high_mid = 1.0f, float high = 1.0f)
    {
        using namespace tone3000_pitch;
        if (count <= 0) return;
        auto& s = state;
        if (s.sampleRate <= 0.0 || s.ring.buf.empty()) {
            if (input != output) std::copy(input, input + count, output);
            return;
        }
        target_ratio = std::isfinite(target_ratio)
            ? std::max(0.25, std::min(4.0, target_ratio)) : 1.0;
        if (target_ratio != s.ratio) {
            // As upstream: small sweeps retain a pending search; large changes
            // and direction changes discard a landing planned for the old drift.
            const auto direction = [](double r) { return r > 1.0 ? 1 : r < 1.0 ? -1 : 0; };
            if (direction(target_ratio) != direction(s.ratio)
                || std::abs(target_ratio - s.ratio) > kSearchKeepRatio)
                s.search = State::Search{};
            s.ratio = target_ratio;
            s.updateFadeRange();
        }
        const double ratio = s.ratio;
        // How far the tap's delay moves over a search lead / a fade. The lead is
        // padded by a few samples so a planned search always completes before
        // the tap reaches the trigger.
        const double drift = std::abs(1.0 - ratio);
        const double leadDrift = drift * (s.searchLeadNow + 4);
        const int leadDriftI = static_cast<int>(std::ceil(leadDrift));
        // An upshift fade always runs fadeHi (fadeLo meets it there); a downshift
        // one is planned for the shortest and gets whatever room the landing has.
        const int fadeDrift = static_cast<int>(std::ceil(drift * (ratio > 1.0 ? s.fadeHi : s.fadeMinLen)));

        for (int i = 0; i < count; ++i) {
          const int64_t now = s.written++;
          const float raw = input[i];
          s.ring.write(now, raw);
          s.control.write(now, raw);

          const double delay = static_cast<double>(now) - s.rA;
          const int guard = s.lowGuard();
          const bool onset = s.detectOnset(raw);
          // Where a splice may land (delay at splice time). Downshift: anywhere
          // from the floor up to where the shortest fade and the next search's
          // lead still fit before dMax. Upshift: the mirror image above the
          // guard. The onset re-sync targets the front of that range.
          const int landLo = ratio > 1.0 ? std::min(guard + fadeDrift + leadDriftI + 2, s.dMax) : s.dMin;
          const int landHi = ratio < 1.0 ? std::max(s.dMin, s.dMax - fadeDrift - leadDriftI - 2) : s.dMax;
          if (!s.fading) {
            if (onset && now - s.lastOnset > s.refractory && delay > landLo + s.onsetSpan) {
              // Onset re-sync: a pick attack brings the tap to the front of the
              // buffer, so attacks arrive with the floor delay wherever the tap
              // was. A small search, run at once.
              s.lastOnset = now;
              s.beginSearch(landLo, std::min(landLo + s.onsetSpan, s.dMax), 0);
              s.stepSearch(s.search.perSample);
              s.startFade(s.search.resultJump, s.onsetFadeLen);
              // The attack becomes the reference: a re-trigger needs a fresh dip.
              s.minHist.fill(static_cast<float>(s.eHf));
              s.maxHist.fill(static_cast<float>(s.eHf));
              s.minAcc = s.maxAcc = s.eHf;
            } else if (ratio < 1.0) {
              // Drift splice, downshift: the tap falls back toward dMax. The
              // search is planned a lead early; a candidate that will have delay
              // dL when the splice comes has delay dL - leadDrift now, so the
              // candidate range is the landing range shifted toward the head.
              // If the plan is late (the ratio or window just changed) the tap
              // simply overshoots dMax by the lead; the rings have the room.
              if (!s.search.active && !s.search.ready && delay >= s.dMax - leadDrift) {
                const int lo = std::max(4, landLo - leadDriftI);
                s.beginSearch(lo, std::max(lo, landHi - leadDriftI), s.searchLeadNow);
              }
              if (s.search.ready && delay >= s.dMax) {
                // The destination keeps drifting deeper while it fades in, so
                // it may fade for as long as it takes to reach the end itself
                // (less the lead the next search needs).
                const double dest = delay - static_cast<double>(s.search.resultJump);
                const double room = (static_cast<double>(s.dMax) - dest - leadDrift - 2.0) / drift;
                s.startFade(s.search.resultJump, s.fadeFor(s.search.bestNcc, room));
              }
            } else if (ratio > 1.0) {
              // Upshift: the tap gains on the write head toward the guard; the
              // candidates sit deeper than where they will land. The guard
              // already leaves room for the longest fade.
              if (!s.search.active && !s.search.ready && delay <= guard + leadDrift)
                s.beginSearch(landLo + leadDriftI, landHi + leadDriftI, s.searchLeadNow);
              if (s.search.ready && delay <= guard) s.startFade(s.search.resultJump, s.fadeFor(s.search.bestNcc, s.fadeHi));
            }
          }
          if (s.search.active) s.stepSearch(s.search.perSample);

          // Read. The fade is a raised cosine; taps that don't correlate add in
          // power rather than amplitude, so the gains are normalised by the taps'
          // correlation (r = 1 leaves the plain complementary fade, r = 0 is the
          // equal-power one), and the level holds through the fade either way.
          float gainA = 1.0f, gainB = 0.0f;
          if (s.fading) {
            gainB = static_cast<float>(0.5 - 0.5 * std::cos(kPi * s.fade));
            gainA = 1.0f - gainB;
            const float r = static_cast<float>(s.fadeNcc);
            const float norm = std::sqrt(gainA * gainA + gainB * gainB + 2.0f * gainA * gainB * r);
            gainA /= norm;
            gainB /= norm;
            s.fade += s.fadeInc;
          }
          if (s.primeLeft > 0 && --s.primeLeft == 0) s.blendLeft = s.blendLength;
          if (s.blendLeft > 0) {
            if (--s.blendLeft > 0) s.wetMix += s.blendStep;
            else s.wetMix = 1.0f;
          }
          float shifted = s.ring.read(s.rA);
          if (s.fading) shifted = shifted * gainA + gainB * s.ring.read(s.rB);
          shifted = raw + s.wetMix * (shifted - raw);
          shifted = colour(shifted, low, low_mid, high_mid, high);
          // Independent Guitarix Wet/Dry gains, including the legacy option to
          // delay the dry by the nominal mean latency. Tonality is off in TONE3000's
          // default pure shift and is deliberately not part of this adapter.
          const float drySample = compensate_dry ? s.ring.at(now - latency_samples()) : raw;
          output[i] = wet * shifted + dry * drySample;
          if (s.fading && s.fade >= 1.0) {
            s.rA = s.rB;
            s.fading = false;
          }
          s.rA += ratio;
          s.rB += ratio;
        }

    }
};

} // namespace gx_engine
