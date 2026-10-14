/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gx_harmoniser_plugin.h"
namespace pluginlib { namespace dual_harmoniser {
PluginDef* plugin() { return new gx_engine::Harmoniser<true>; }
} }
