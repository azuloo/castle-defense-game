#include "player.h"
#include "drawable_ops.h"
#include "utils.h"
#include "freetype_renderer.h"
#include "freetype_text.h"

extern int wWidth;
extern int wHeight;

static CurrencyDef s_PlayerCurrency;
static char* s_CurrencyIconPath = "/res/static/textures/coin.png";

static int render_currency_amount()
{
	// 5 digits max
	char coins_str[6];
	itoa(s_PlayerCurrency.amount, coins_str, 10);
	Vec3 color = { 1.f, 1.f, 1.f };
	render_text(coins_str, FT_DEFAULT_FONT_SIZE, 75.f, wHeight - 60.f, color);

	return 0;
}

static int render_currency_icon()
{
	DrawableDef* currency_drawable = NULL;
	Vec3 currency_icon_pos = { { 50.f, wHeight - 50.f, 0.f } };
	Vec3 currency_icon_scale = { { (float)wHeight * 0.02f, (float)wHeight * 0.02f, 1.f } };
	Vec4 currency_icon_color = COLOR_VEC_WHITE;
	draw_quad(&currency_drawable, &currency_icon_pos, &currency_icon_scale, &currency_icon_color, 60);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != currency_drawable, "[player]: Failed to create currency icon quad.");

	add_texture_2D(currency_drawable, s_CurrencyIconPath, TexType_RGBA, default_texture_params, DEFAULT_TEXTURE_PARAMS_COUNT);

	return 0;
}

int init_player_currency()
{
	s_PlayerCurrency.currency_drawable = NULL;
	s_PlayerCurrency.amount = DEFAULT_PLAYER_CURRENCY_AMOUNT;

	render_currency_icon();
	render_currency_amount();

	return 0;
}

void change_player_currency_amount(int amount)
{
	s_PlayerCurrency.amount += amount;
	render_currency_amount();
}
