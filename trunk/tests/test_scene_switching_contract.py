from __future__ import annotations

from pathlib import Path
import unittest


TRUNK = Path(__file__).resolve().parents[1]
ENGINE = TRUNK / "src" / "gx_head" / "engine"
HEADERS = TRUNK / "src" / "headers"
LADSPA_HOST = TRUNK / "src" / "ladspa" / "ladspa_guitarix.cpp"


class SceneSwitchingContractTests(unittest.TestCase):
    def test_muted_scene_rpc_is_present_in_template_and_generated_contract(self) -> None:
        template = (ENGINE / "jsonrpc_methods.gperf_tmpl").read_text(encoding="utf-8")
        generated_header = (ENGINE / "jsonrpc_methods-generated.h").read_text(
            encoding="utf-8"
        )
        generated_source = (ENGINE / "jsonrpc_methods-generated.cc").read_text(
            encoding="utf-8"
        )

        self.assertIn('"set_scene_muted", true', template)
        self.assertIn("RPCM_set_scene_muted", generated_header)
        self.assertIn('{"set_scene_muted", RPCM_set_scene_muted}', generated_source)
        self.assertIn('{ "set_scene_muted", true }', generated_source)
        self.assertIn('"prepare_mnam_models", true', template)
        self.assertIn("RPCM_prepare_mnam_models", generated_header)
        self.assertIn('{ "prepare_mnam_models", true }', generated_source)

    def test_scene_residency_is_hidden_and_bypassed_in_the_rt_chain(self) -> None:
        loader = (ENGINE / "gx_pluginloader.cpp").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")

        self.assertIn('s+".scene_resident"', loader)
        self.assertIn("p_scene_resident->setSavable(false);", loader)
        self.assertGreaterEqual(audio.count("!p->owner->get_rt_on_off()"), 2)
        self.assertIn("commit_rt_scene_states()", audio)
        self.assertIn("pdef->clear_state(pdef);", loader)
        self.assertNotIn("update_rt_on_off", loader)

    def test_resident_processing_helpers_do_not_change_public_active_lists(self) -> None:
        loader = (ENGINE / "gx_pluginloader.cpp").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")

        ordered_mono = loader.split(
            "void PluginList::ordered_mono_list", maxsplit=1
        )[1].split("void PluginList::ordered_stereo_list", maxsplit=1)[0]
        processing_mono = loader.split(
            "void PluginList::ordered_processing_mono_list", maxsplit=1
        )[1].split("void PluginList::ordered_processing_stereo_list", maxsplit=1)[0]
        self.assertIn("pl->get_on_off()", ordered_mono)
        self.assertNotIn("get_processing_active", ordered_mono)
        self.assertIn("get_processing_active", processing_mono)
        self.assertIn("ordered_processing_mono_list", audio)

    def test_dynamic_plugin_removal_clears_residency_before_deletion(self) -> None:
        engine = (ENGINE / "gx_engine.cpp").read_text(encoding="utf-8")
        removal = engine.split("if (j == ml.end())", maxsplit=1)[1].split(
            "} else {", maxsplit=1
        )[0]
        self.assertLess(
            removal.index("pl->set_scene_resident(false)"),
            removal.index("pl->set_on_off(false)"),
        )

    def test_guitarix_plugin_hosts_publish_the_deferred_rt_snapshot(self) -> None:
        host = LADSPA_HOST.read_text(encoding="utf-8")

        self.assertIn("ordered_processing_mono_list", host)
        self.assertIn("ordered_processing_stereo_list", host)
        self.assertEqual(host.count("pluginlist.rt_scene_state_changed()"), 4)
        self.assertEqual(host.count("pluginlist.commit_rt_scene_states()"), 2)

    def test_external_mute_suppresses_only_the_chain_ramp(self) -> None:
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_modulesequencer.h").read_text(encoding="utf-8")

        self.assertIn("commit_pending_module_lists(bool externally_muted", header)
        self.assertIn("!already_down && !externally_muted", audio)
        self.assertIn("commit_module_lists(externally_muted);", audio)
        self.assertIn("mono_chain.start_ramp_down();", audio)
        self.assertIn("stereo_chain.start_ramp_down();", audio)

    def test_scene_rpc_rejects_an_uninitialised_audio_engine(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")

        self.assertIn("get_jack_client_activity_status()", rpc)
        self.assertIn("!jack_status.ok || !jack_status.active", rpc)
        self.assertIn('throw RpcError(-32000, "Audio engine is not ready")', rpc)
        commit_pending = audio.split(
            "bool ModuleSequencer::commit_pending_module_lists", maxsplit=1
        )[1].split("void ModuleSequencer::set_rack_changed", maxsplit=1)[0]
        self.assertIn("!scene_commit_ready()", commit_pending)

    def test_scene_rpc_propagates_plugin_activation_failure(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_modulesequencer.h").read_text(encoding="utf-8")

        self.assertIn("bool* commit_ok", header)
        self.assertIn("*commit_ok = ok;", audio)
        self.assertIn(
            "if (!commit_ok || !chain_settled || !gain_smoothers_settled)",
            rpc,
        )
        self.assertIn('throw RpcError(-32001, details.str())', rpc)
        for diagnostic in (
            "commitOk=",
            "chainSettled=",
            "monoAudioFinished=",
            "stereoAudioFinished=",
            "outputGainSettled=",
            "namGainSettled=",
            "snamGainSettled=",
            "mnamGainSettled=",
        ):
            self.assertIn(diagnostic, rpc)

    def test_reverse_delay_exposes_a_tail_reset_for_resident_reenable(self) -> None:
        source = (TRUNK / "src" / "plugins" / "reversedelay.cc").read_text(
            encoding="utf-8"
        )

        self.assertIn("clear_state = clear_state_impl;", source)
        self.assertIn("std::fill(self.buffer", source)
        self.assertIn("self.feedback_buf = 0.0f;", source)

    def test_univibe_exposes_a_complete_resident_state_reset(self) -> None:
        source = (TRUNK / "src" / "plugins" / "vibe.cc").read_text(
            encoding="utf-8"
        )

        self.assertIn("clear_state = clear_state_impl;", source)
        self.assertIn("vibe_mono_lfo_sine::clear_state_f();", source)
        self.assertIn("lfo.reset();", source)
        self.assertIn("vc[i].x1 = vc[i].y1 = 0.0f;", source)
        self.assertIn("fbl = 0.0f;", source)

    def test_pitch_shift_resident_reset_qualifies_the_callback_slot(self) -> None:
        source = (ENGINE / "gx_internal_plugins.cpp").read_text(encoding="utf-8")
        constructor = source.split(
            "smbPitchShift::smbPitchShift", maxsplit=1
        )[1].split("void smbPitchShift::init", maxsplit=1)[0]

        # smbPitchShift also has a clear_state() member function, so an
        # unqualified assignment resolves to that function instead of the
        # PluginDef callback field and fails to compile.
        self.assertIn("PluginDef::clear_state = clear_state_static;", constructor)
        self.assertNotIn("\n    clear_state = clear_state_static;", constructor)

    def test_multi_nam_uses_the_bounded_prepared_model_cache_for_both_slots(
        self,
    ) -> None:
        source = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_neural_plugins.h").read_text(encoding="utf-8")

        self.assertIn("std::vector<CachedNamModel> model_cache;", header)
        self.assertIn(
            "if (model_cache.size() >= kNamModelCacheEntries)", source
        )
        self.assertIn("maSampleRate, 'A');", source)
        self.assertIn("mbSampleRate, 'B');", source)
        self.assertIn("reused prewarmed cached A ", source)
        self.assertIn("reused prewarmed cached B ", source)
        multi_parking = source.split(
            "void NeuralAmpMulti::cache_model", maxsplit=1
        )[1].split("PreparedNamModelResult", maxsplit=1)[0]
        self.assertNotIn("ResetAndPrewarm", multi_parking)
        self.assertNotIn("->Reset(", multi_parking)
        standalone_parking = source.split(
            "void NeuralAmp::cache_current_model", maxsplit=1
        )[1].split("inline void NeuralAmp::clear_state_f", maxsplit=1)[0]
        self.assertNotIn("ResetAndPrewarm", standalone_parking)
        self.assertIn("prepare_song_models", source)
        self.assertIn("load_nam_model(", source)
        self.assertIn("entry.slot == slot", source)
        self.assertIn("prepared_generation", header)
        self.assertIn("entry.host_sample_rate == fSampleRate", source)

    def test_muted_scene_snaps_only_the_bounded_gain_whitelist(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        neural = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")
        internal = (ENGINE / "gx_internal_plugins.cpp").read_text(encoding="utf-8")
        engine = (ENGINE / "gx_engine.cpp").read_text(encoding="utf-8")

        for parameter_id in (
            "amp.out_master",
            "nam.input",
            "nam.output",
            "snam.input",
            "snam.output",
            "mnam.input",
            "mnam.inputb",
            "mnam.output",
            "mnam.mix",
        ):
            self.assertIn(parameter_id, rpc)
        self.assertNotIn('attr == "mnam.cdelay"', rpc)
        self.assertIn(
            "wait_scene_audio_cycle(mono_gain_pending, output_gain_pending)", rpc
        )
        self.assertIn('"gainSmoothersSnapped"', rpc)
        self.assertIn('"gainSmoothersSettled"', rpc)
        self.assertIn("scene_smoother_snap_pending", neural)
        self.assertIn("SceneOutputLevel::request_scene_smoother_snap", internal)
        self.assertIn("pl.add(&scene_outputlevel", engine)
        self.assertNotIn("gx_effects::gx_outputlevel::plugin()", engine)

    def test_model_batches_reuse_the_model_ownership_barrier_for_smoothers(
        self,
    ) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        neural = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_neural_plugins.h").read_text(encoding="utf-8")
        scene = rpc.split("FUNCTION(set_scene)", maxsplit=1)[1].split(
            "FUNCTION(get) {", maxsplit=1
        )[0]
        snap_whitelist = scene.split(
            "if (externally_muted) {", maxsplit=1
        )[1].split("gx_engine::GxEngine& engine", maxsplit=1)[0]

        # File selection and model-size callbacks already call sync() before
        # replacing/resetting DSP state. They must not arm an additional RT
        # snap; real gain/on-off controls still must.
        for model_control in (
            'attr == "nam.loadpath"',
            'attr == "nam.flist"',
            'attr == "nam.size"',
            'attr == "snam.loadpath"',
            'attr == "snam.flist"',
            'attr == "snam.size"',
            'attr == "mnam.loadapath"',
            'attr == "mnam.loadbpath"',
            'attr == "mnam.falist"',
            'attr == "mnam.fblist"',
            'attr == "mnam.sizea"',
            'attr == "mnam.sizeb"',
        ):
            self.assertNotIn(model_control, snap_whitelist)
        for live_control in (
            'attr == "nam.input"',
            'attr == "nam.output"',
            'attr == "nam.on_off"',
            'attr == "mnam.input"',
            'attr == "mnam.inputb"',
            'attr == "mnam.output"',
            'attr == "mnam.mix"',
            'attr == "mnam.on_off"',
        ):
            self.assertIn(live_control, snap_whitelist)

        self.assertEqual(header.count("begin_scene_parameter_batch"), 2)
        self.assertEqual(header.count("finish_scene_parameter_batch"), 2)
        self.assertLess(
            scene.index("begin_scene_parameter_batch(externally_muted)"),
            scene.index("apply_parameter_set(params)"),
        )
        self.assertLess(
            scene.index("apply_parameter_set(params)"),
            scene.index("finish_scene_parameter_batch()"),
        )
        self.assertIn("catch (...) {", scene)
        self.assertIn('"gainSmoothersPreset"', scene)
        for plugin in ("nam", "snam", "mnam"):
            self.assertIn(f'"{plugin}GainSmootherPreset"', scene)
            self.assertIn(f"{plugin}GainPreset=", scene)

        standalone_load = neural.split(
            "void NeuralAmp::load_nam_file()", maxsplit=1
        )[1].split("void NeuralAmp::set_nam_size()", maxsplit=1)[0]
        self.assertLess(
            standalone_load.index("sync();"),
            standalone_load.index("preset_scene_smoother_targets();"),
        )
        self.assertLess(
            standalone_load.index("preset_scene_smoother_targets();"),
            standalone_load.rindex("atomic_set(&ready"),
        )
        for loader, following in (
            ("void NeuralAmpMulti::load_nam_afile()", "void NeuralAmpMulti::load_nam_bfile_impl()"),
            ("void NeuralAmpMulti::load_nam_bfile()", "void NeuralAmpMulti::set_nam_asize()"),
        ):
            model_load = neural.split(loader, maxsplit=1)[1].split(
                following, maxsplit=1
            )[0]
            self.assertLess(
                model_load.index("sync();"),
                model_load.index("preset_scene_smoother_targets();"),
            )
            self.assertLess(
                model_load.index("preset_scene_smoother_targets();"),
                model_load.rindex("atomic_set(&ready"),
            )

        multi_compute = neural.split(
            "void always_inline NeuralAmpMulti::compute", maxsplit=1
        )[1].split("void NeuralAmpMulti::connect", maxsplit=1)[0]
        standalone_compute = neural.split(
            "void always_inline NeuralAmp::compute", maxsplit=1
        )[1].split("void NeuralAmp::connect", maxsplit=1)[0]
        self.assertLess(
            standalone_compute.index("if (!processing_ready && !snap_pending) return;"),
            standalone_compute.index("const bool has_model = model != nullptr;"),
        )
        self.assertLess(
            standalone_compute.index("if (!processing_ready && !snap_pending) return;"),
            standalone_compute.index("preset_scene_smoother_targets();"),
        )
        self.assertLess(
            multi_compute.index("if (!processing_ready && !snap_pending) return;"),
            multi_compute.index("const bool has_model = modela || modelb;"),
        )
        self.assertLess(
            multi_compute.index("if (!processing_ready && !snap_pending) return;"),
            multi_compute.index("preset_scene_smoother_targets();"),
        )

    def test_direct_smoother_preset_does_not_mask_a_pending_rt_snap(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        scene = rpc.split("FUNCTION(set_scene)", maxsplit=1)[1].split(
            "FUNCTION(get) {", maxsplit=1
        )[0]

        # A batch can contain a model replacement followed by a live gain
        # change. The direct preset is diagnostic only: requested RT work must
        # still be consumed before the transaction can succeed.
        for plugin in ("nam", "snam", "mnam"):
            self.assertIn(
                f"bool {plugin}_gain_settled = !snap_{plugin}_gain_rt;", scene
            )
            self.assertIn(
                f"if (snap_{plugin}_gain_rt) {{", scene
            )
        failure_predicate = scene.split("if (!commit_ok", maxsplit=1)[1].split(
            ") {", maxsplit=1
        )[0]
        self.assertIn("gain_smoothers_settled", failure_predicate)
        self.assertNotIn("gain_preset", failure_predicate)

    def test_scene_audio_wait_is_selective_shared_and_hard_capped(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_modulesequencer.h").read_text(encoding="utf-8")

        self.assertIn("mono_gain_pending =", rpc)
        self.assertIn("nam_gain_pending || snam_gain_pending || mnam_gain_pending", rpc)
        self.assertIn("output_gain_pending", rpc)
        self.assertIn("wait_scene_audio_cycle(bool wait_mono", header)
        scene_wait = audio.split(
            "SceneAudioCycleStatus ModuleSequencer::wait_scene_audio_cycle", maxsplit=1
        )[1].split(
            "bool ModuleSequencer::commit_pending_module_lists", maxsplit=1
        )[0]
        self.assertIn("10000ULL", scene_wait)
        self.assertIn("50000ULL", scene_wait)
        self.assertEqual(scene_wait.count("timespec deadline;"), 1)
        self.assertIn("mono_chain.wait_rt_finished_until(deadline)", scene_wait)
        self.assertIn("stereo_chain.wait_rt_finished_until(deadline)", scene_wait)

    def test_consumed_snap_is_authoritative_over_a_missed_wakeup(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")
        scene = rpc.split("FUNCTION(set_scene)", maxsplit=1)[1].split(
            "FUNCTION(get) {", maxsplit=1
        )[0]
        failure_predicate = scene.split("if (!commit_ok", maxsplit=1)[1].split(
            ") {", maxsplit=1
        )[0]

        self.assertNotIn("gain_audio_cycle_finished", failure_predicate)
        self.assertIn("gain_smoothers_settled", failure_predicate)
        for smoother in ("output", "nam", "snam", "mnam"):
            self.assertIn(f"{smoother}_gain_settled", scene)
        self.assertIn("finish_scene_smoother_snap()", scene)

    def test_rt_cycle_latch_uses_generation_as_the_wait_predicate(self) -> None:
        latch = (HEADERS / "gx_rt_cycle_latch.h").read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")

        self.assertIn("generation_counter.fetch_add", latch)
        self.assertIn("generation_counter.load", latch)
        self.assertIn("armed_generation", latch)
        self.assertIn("rt_cycle_latch.arm();", audio)
        self.assertNotIn("sem_getvalue(&sync_sem", audio)

    def test_model_less_nam_wrappers_consume_muted_scene_snaps(self) -> None:
        source = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")

        standalone_compute = source.split(
            "void always_inline NeuralAmp::compute", maxsplit=1
        )[1].split(
            "void always_inline NeuralAmpMulti::compute", maxsplit=1
        )[0]
        multi_compute = source.split(
            "void always_inline NeuralAmpMulti::compute", maxsplit=1
        )[1].split(
            "void NeuralAmpMulti::connect", maxsplit=1
        )[0]

        self.assertLess(
            standalone_compute.index("scene_smoother_snap_pending"),
            standalone_compute.index("if (!processing_ready || !has_model) return;"),
        )
        self.assertLess(
            multi_compute.index("scene_smoother_snap_pending"),
            multi_compute.index("if (!processing_ready || !has_model) return;"),
        )

    def test_song_model_prepare_rpc_uses_the_checked_in_fallback(self) -> None:
        rpc = (ENGINE / "jsonrpc.cpp").read_text(encoding="utf-8")

        self.assertIn("FUNCTION(prepare_mnam_models)", rpc)
        self.assertIn(
            '{ "prepare_mnam_models", RPCM_prepare_mnam_models }',
            rpc,
        )
        self.assertIn("mneural_amp.prepare_song_models", rpc)
        self.assertIn('jw.write_bool_kv("rapidSwitchReady"', rpc)

    def test_resident_nam_preserves_prewarmed_model_and_smoother_state(self) -> None:
        source = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")
        loader = (ENGINE / "gx_pluginloader.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_pluginloader.h").read_text(encoding="utf-8")

        self.assertIn("PGNI_RESIDENT_PRESERVE_STATE = 0x200000", header)
        self.assertEqual(source.count("flags = PGNI_RESIDENT_PRESERVE_STATE;"), 2)
        self.assertIn(
            "PGNI_RESIDENT_PRESERVE_STATE |", loader
        )

    def test_resident_dynamics_track_live_bypass_but_tail_effects_reset(self) -> None:
        engine = (ENGINE / "gx_engine.cpp").read_text(encoding="utf-8")
        ladspa = LADSPA_HOST.read_text(encoding="utf-8")
        audio = (ENGINE / "gx_engine_audio.cpp").read_text(encoding="utf-8")
        loader = (ENGINE / "gx_pluginloader.cpp").read_text(encoding="utf-8")

        def registration(source: str, plugin: str) -> str:
            lines = source.splitlines()
            start = next(
                index for index, line in enumerate(lines) if plugin in line
            )
            statement = lines[start]
            while ");" not in statement:
                start += 1
                statement += " " + lines[start]
            return statement

        tracker = "PGNI_RESIDENT_TRACK_BYPASS"
        self.assertIn(
            tracker,
            registration(engine, "gx_effects::compressor::plugin()"),
        )
        self.assertIn(
            tracker,
            registration(engine, "gx_effects::expander::plugin()"),
        )
        self.assertIn(
            tracker,
            registration(ladspa, "gx_effects::compressor::plugin()"),
        )
        self.assertIn("float resident_bypass_output[count];", audio)
        self.assertIn("memcpy(resident_bypass_output, output", audio)
        self.assertIn("p->plugin->flags & PGNI_RESIDENT_TRACK_BYPASS", audio)
        self.assertIn("PGNI_RESIDENT_TRACK_BYPASS", loader)

        for source in (engine, ladspa):
            for plugin in (
                "gx_effects::echo::plugin()",
                "gx_effects::delay::plugin()",
                "gx_effects::freeverb::plugin()",
                "pluginlib::reversedelay::plugin()",
            ):
                self.assertNotIn(tracker, registration(source, plugin))

    def test_resident_pitch_shift_resets_history_without_rebuilding_fftw(self) -> None:
        source = (ENGINE / "gx_internal_plugins.cpp").read_text(encoding="utf-8")
        header = (HEADERS / "gx_internal_plugins.h").read_text(encoding="utf-8")

        constructor = source.split("smbPitchShift::smbPitchShift", maxsplit=1)[1].split(
            "void smbPitchShift::init", maxsplit=1
        )[0]
        reset = source.split("void smbPitchShift::clear_state_static", maxsplit=1)[1].split(
            "void smbPitchShift::mem_alloc", maxsplit=1
        )[0]
        self.assertIn("clear_state = clear_state_static;", constructor)
        self.assertIn("static void clear_state_static(PluginDef *p);", header)
        self.assertIn("if (self->mem_allocated)", reset)
        self.assertIn("self->clear_state();", reset)
        self.assertNotIn("mem_free", reset)
        self.assertNotIn("self->mem_alloc(", reset)
        self.assertNotIn("fftwf_plan_dft_1d", reset)

    def test_legacy_rtneural_audio_reset_does_not_grow_selector_vectors(self) -> None:
        source = (ENGINE / "gx_neural_plugins.cpp").read_text(encoding="utf-8")

        standalone_clear = source.split(
            "inline void RtNeural::clear_state_f()", maxsplit=1
        )[1].split("void RtNeural::clear_state_f_static", maxsplit=1)[0]
        multi_clear = source.split(
            "inline void RtNeuralMulti::clear_state_f()", maxsplit=1
        )[1].split("void RtNeuralMulti::clear_state_f_static", maxsplit=1)[0]
        self.assertNotIn("file_names.push_back", standalone_clear)
        self.assertNotIn("file_names.push_back", multi_clear)
        self.assertIn('rtneural_file_names.assign(128, "None")', source)
        self.assertIn('rtneural_afile_names.assign(128, "None")', source)
        self.assertIn('rtneural_bfile_names.assign(128, "None")', source)

    def test_resident_mono_convolver_reuses_only_an_exact_prepared_ir(self) -> None:
        source = (ENGINE / "gx_internal_plugins.cpp").read_text(encoding="utf-8")

        self.assertIn("configured_jcset == jcset", source)
        self.assertIn(
            "configured_samplerate == conv.get_samplerate()", source
        )
        self.assertIn(
            "configured_buffersize == conv.get_buffersize()", source
        )
        self.assertIn(
            "configuration_matches() && conv.state() == Convproc::ST_STOP",
            source,
        )
        self.assertIn("retain_configuration_while_bypassed = true;", source)
        self.assertNotIn("processing_state_changed", source)
        loader = (ENGINE / "gx_pluginloader.cpp").read_text(encoding="utf-8")
        self.assertIn("pdef->activate_plugin(false, pdef);", loader)
        self.assertIn("pdef->activate_plugin(true, pdef)", loader)


if __name__ == "__main__":
    unittest.main()
