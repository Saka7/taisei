/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
*/

#include "taisei.h"

#include "stage2.h"
#include "spells/spells.h"
#include "hina.h"
#include "draw.h"
#include "background_anim.h"
#include "timeline.h"

#include "global.h"
#include "portrait.h"

struct stage2_spells_s stage2_spells = {
	.boss = {
		.amulet_of_harm = {
			{ 0,  1,  2,  3}, AT_Spellcard, "Shard “Amulet of Harm”", 50, 50000,
			hina_amulet, stage2_draw_hina_spellbg, BOSS_DEFAULT_GO_POS, 2
		},
		.bad_pick = {
			{ 4,  5,  6,  7}, AT_Spellcard, "Lottery Sign “Bad Pick”", 60, 43200,
			hina_bad_pick, stage2_draw_hina_spellbg, BOSS_DEFAULT_GO_POS, 2
		},
		.wheel_of_fortune = {
			{ 8,  9, 10, 11}, AT_Spellcard, "Lottery Sign “Wheel of Fortune”", 50, 32000,
			hina_wheel, stage2_draw_hina_spellbg, BOSS_DEFAULT_GO_POS, 2
		},
	},

	.extra.monty_hall_danmaku = {
		{ 0,  1,  2,  3}, AT_ExtraSpell, "Lottery Sign “Monty Hall Danmaku”", 60, 60000,
		hina_monty, stage2_draw_hina_spellbg, BOSS_DEFAULT_GO_POS, 2
	},
};

static void stage2_start(void) {
	stage2_drawsys_init();
	stage2_bg_init_fullstage();
}

static void stage2_spellpractice_start(void) {
	stage2_drawsys_init();
	stage2_bg_init_spellpractice();

	Boss *hina = stage2_spawn_hina(BOSS_DEFAULT_SPAWN_POS);
	boss_add_attack_from_info(hina, global.stage->spell, true);
	boss_start_attack(hina, hina->attacks);
	global.boss = hina;

	stage_start_bgm("stage2boss");
}

static void stage2_end(void) {
	stage2_drawsys_shutdown();
}

static void stage2_preload(void) {
	portrait_preload_base_sprite("hina", NULL, RESF_DEFAULT);
	portrait_preload_face_sprite("hina", "normal", RESF_DEFAULT);
	preload_resources(RES_BGM, RESF_OPTIONAL, "stage2", "stage2boss", NULL);
	preload_resources(RES_SPRITE, RESF_DEFAULT,
		"stage2/border",
		"stage2/leaves",
		"stage2/roadgrass",
		"stage2/roadstones",
		"stage2/spellbg1",
		"stage2/spellbg2",
	NULL);
	preload_resources(RES_SHADER_PROGRAM, RESF_DEFAULT,
		"bloom",
		"zbuf_fog",
		"alpha_depth",
	NULL);
	preload_resources(RES_SHADER_PROGRAM, RESF_OPTIONAL,
		"lasers/linear",
	NULL);
	preload_resources(RES_ANIM, RESF_DEFAULT,
		"boss/wriggle",
		"boss/hina",
		"fire",
	NULL);
	preload_resources(RES_SFX, RESF_OPTIONAL,
		"laser1",
	NULL);
}

StageProcs stage2_procs = {
	.begin = stage2_start,
	.preload = stage2_preload,
	.end = stage2_end,
	.draw = stage2_draw,
	.event = stage2_events,
	.shader_rules = stage2_bg_effects,
	.spellpractice_procs = &(StageProcs) {
		.begin = stage2_spellpractice_start,
		.preload = stage2_preload,
		.end = stage2_end,
		.draw = stage2_draw,
		.shader_rules = stage2_bg_effects,
	},
};
