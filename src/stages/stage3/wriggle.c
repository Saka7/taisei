/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "coroutine.h"
#include "taisei.h"

#include "wriggle.h"

#include "common_tasks.h"

void stage3_draw_wriggle_spellbg(Boss *b, int time) {
	r_color4(1,1,1,1);
	fill_viewport(0, 0, 768.0/1024.0, "stage3/wspellbg");
	r_color4(1,1,1,0.5);
	r_blend(r_blend_compose(
		BLENDFACTOR_SRC_ALPHA, BLENDFACTOR_ONE, BLENDOP_SUB,
		BLENDFACTOR_ZERO,      BLENDFACTOR_ONE, BLENDOP_ADD
	));
	fill_viewport(sin(time) * 0.015, time / 50.0, 1, "stage3/wspellclouds");
	r_blend(BLEND_PREMUL_ALPHA);
	r_color4(0.5, 0.5, 0.5, 0.0);
	fill_viewport(0, time / 70.0, 1, "stage3/wspellswarm");
	r_blend(r_blend_compose(
		BLENDFACTOR_SRC_ALPHA, BLENDFACTOR_ONE, BLENDOP_SUB,
		BLENDFACTOR_ZERO,      BLENDFACTOR_ONE, BLENDOP_ADD
	));
	r_color4(1,1,1,0.4);
	fill_viewport(cos(time) * 0.02, time / 30.0, 1, "stage3/wspellclouds");

	r_blend(BLEND_PREMUL_ALPHA);
	r_color4(1, 1, 1, 1);
}

Boss *stage3_spawn_wriggle(cmplx pos) {
	Boss *wriggle = create_boss("Wriggle", "wriggle", pos);
	boss_set_portrait(wriggle, "wriggle", NULL, "proud");
	wriggle->glowcolor = *RGBA_MUL_ALPHA(0.2, 0.4, 0.5, 0.5);
	wriggle->shadowcolor = *RGBA_MUL_ALPHA(0.4, 0.2, 0.6, 0.5);
	return wriggle;
}

static void wriggle_slave_visual(Enemy* e, int time) {

//	if(ARGS.render) {
	r_draw_sprite(&(SpriteParams) {
		.sprite = "fairy_circle",
		.rotation.angle = DEG2RAD * 7 * time,
		.scale.both = 0.7,
		.color = RGBA(0.8, 1.0, 0.4, 0),
		.pos = { creal(e->pos), cimag(e->pos) },
		});
// };
	if (time % 5 == 0) {
		tsrand_fill(2);
		PARTICLE(
			.sprite = "smoothdot",
			.pos = 5*cexp(2*I*M_PI*afrand(0)),
			.color = RGBA(0.6, 0.6, 0.5, 0),
			.draw_rule = Shrink,
			.rule = enemy_flare,
			.timeout = 60,
			.args = {
				0.3*cexp(2*M_PI*I*afrand(1)),
				add_ref(e),
			},
		);
	}
}

void wriggle_nonspell_slave(Boss* b, Enemy* e, int time) {

//	AT(EVENT_DEATH) {
//		free_ref(e->args[0]);
//		return 1;
//	}
//	if(!boss)
//		return ACTION_DESTROY;

		WAIT(1);
		wriggle_slave_visual(e, time);
		int level = e->args[3];
		float angle = e->args[2] * (time / 70.0 + e->args[1]);
		cmplx dir = cexp(I*angle);

		e->move = move_towards(b->pos + (100 + 20 * e->args[2] * sin(time / 100.0)) * dir, 0.03);

		int d = 10 - global.diff;
		if(level > 2)
			d += 4;

		if(!(time % d)) {
			play_sfx("shot1");

			PROJECTILE(
				.proto = pp_rice,
				.pos = e->pos,
				.color = RGB(0.7, 0.2, 0.1),
				.rule = linear,
				.args = { 3 * cexp(I*carg(b->pos - e->pos)) },
			);

			if(!(time % (d*2)) || level > 1) {
				PROJECTILE(
					.proto = pp_thickrice,
					.pos = e->pos,
					.color = RGB(0.7, 0.7, 0.1),
					.rule = linear,
					.args = { 2.5 * cexp(I*carg(b->pos - e->pos)) },
				);
			}

			if(level > 2) {
				PROJECTILE(
					.proto = pp_wave,
					.pos = e->pos,
					.color = RGB(0.3, 0.1 + 0.6 * psin(time / 25.0), 0.7),
					.rule = linear,
					.args = { 2 * cexp(I*carg(b->pos - e->pos)) },
				);
			}
		}
}

DEFINE_EXTERN_TASK(wriggle_nonspell_1) {
	Boss *boss = INIT_BOSS_ATTACK();
	BEGIN_BOSS_ATTACK();

	for(int time = 0;; time++) {
		create_enemy1c();
		wriggle_nonspell_slave(*boss, Enemy *e, int time)
	}


}
