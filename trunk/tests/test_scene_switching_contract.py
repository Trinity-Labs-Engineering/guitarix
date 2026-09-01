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
        self.assertIn(
            'throw RpcError(-32001, "Scene processing chain did not commit")', rpc
        )

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
        self.assertIn("wait_scene_audio_cycle()", rpc)
        self.assertIn('"gainSmoothersSnapped"', rpc)
        self.assertIn('"gainSmoothersSettled"', rpc)
        self.assertIn("scene_smoother_snap_pending", neural)
        self.assertIn("SceneOutputLevel::request_scene_smoother_snap", internal)
        self.assertIn("pl.add(&scene_outputlevel", engine)
        self.assertNotIn("gx_effects::gx_outputlevel::plugin()", engine)

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
            standalone_compute.index("if (!model_ready) return;"),
        )
        self.assertLess(
            multi_compute.index("scene_smoother_snap_pending"),
            multi_compute.index("if (!modela && !modelb) return;"),
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
            "!(pdef->flags & PGNI_RESIDENT_PRESERVE_STATE)", loader
        )

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
