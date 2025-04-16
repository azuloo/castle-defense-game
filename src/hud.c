#include "global_decl.h"
#include "global_defs.h"
#include "hud.h"
#include "graphics.h"
#include "drawable_ops.h"
#include "freetype_renderer.h"
#include "freetype_text.h"

static int s_GameOverState = GameOverHUDState_Hidden;

extern int wWidth;
extern int wHeight;

void render_player_controls()
{
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text("Press 1, 2 or 3 to select Towers", FT_DEFAULT_FONT_SIZE, wWidth - 400.f, wHeight - 50.f, color);
}

void render_game_over()
{
	if (GameOverHUDState_Hidden == s_GameOverState)
	{
		DrawableDef* game_over_drawable = NULL;
		const Vec3 pos = { wWidth / 2, wHeight / 2, 0.f };
		const Vec3 scale = { wWidth / 2, wHeight / 2, 1.f };
		const Vec4 color = { 0.388f, 0.388f, 0.356f, 0.3f };
		draw_quad(&game_over_drawable, &pos, &scale, &color, DrawLayer_TextBackground);

		Vec3 text_color = { 1.f, 1.f, 1.f };
		render_text("GAME OVER", 48, wWidth / 2 - 100, wHeight / 2, text_color);

		s_GameOverState = GameOverHUDState_Rendered;
	}
}
