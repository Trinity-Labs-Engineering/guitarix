/*
 * ampsim.cpp
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: Vladimir Sadovnikov <lsp.plugin@gmail.com>
 *
 * This file is part of tamgamp.lv2 <https://github.com/sadko4u/tamgamp.lv2>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Adapted from the three selected rows of ampsim_norming.
#pragma once

namespace pluginlib { namespace metal_preamp {
constexpr float peavey_5150ii_lead_norm[32] = {
    0.3047826886f, 0.2648236156f, 0.2531969249f, 0.2478669435f,
    0.2448812425f, 0.2430451214f, 0.2418813407f, 0.2411671877f,
    0.2406960726f, 0.2404698133f, 0.2404086292f, 0.2404829264f,
    0.2406484634f, 0.2407827526f, 0.2411351204f, 0.2414602786f,
    0.2418037057f, 0.2422104180f, 0.2426451594f, 0.2431395650f,
    0.2436546385f, 0.2442533225f, 0.2447513044f, 0.2453264743f,
    0.2459146827f, 0.2464866340f, 0.2470885068f, 0.2477051467f,
    0.2482542843f, 0.2490466982f, 0.2498849332f, 0.2503454685f,
};

constexpr float mesa_dual_rect_red_norm[32] = {
    0.5854488015f, 0.4859512746f, 0.4596460760f, 0.4486874640f,
    0.4430651367f, 0.4398461282f, 0.4378585517f, 0.4367038012f,
    0.4360551536f, 0.4359038770f, 0.4357212782f, 0.4357691705f,
    0.4361508191f, 0.4365439415f, 0.4370864034f, 0.4377513230f,
    0.4383253753f, 0.4391932786f, 0.4398071170f, 0.4405093193f,
    0.4412986338f, 0.4419083893f, 0.4427633882f, 0.4434800446f,
    0.4442277253f, 0.4449123740f, 0.4457814097f, 0.4467603862f,
    0.4477423429f, 0.4489898682f, 0.4502777457f, 0.4507282972f,
};

constexpr float mesa_dc3_lead_norm[32] = {
    0.9888177514f, 0.5937648416f, 0.4548042119f, 0.3837075830f,
    0.3402505815f, 0.3107162714f, 0.2892007828f, 0.2727884650f,
    0.2598533630f, 0.2494262755f, 0.2408783287f, 0.2337731421f,
    0.2277993560f, 0.2227144092f, 0.2183327675f, 0.2145205140f,
    0.2111811489f, 0.2082512230f, 0.2056866735f, 0.2034535408f,
    0.2015227079f, 0.1998672932f, 0.1984593719f, 0.1972661614f,
    0.1962527782f, 0.1953949481f, 0.1946728975f, 0.1940659881f,
    0.1935582012f, 0.1931332797f, 0.1927780211f, 0.1924820095f,
};

} }
