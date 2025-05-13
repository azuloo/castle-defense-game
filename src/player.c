#include "player.h"
#include "drawable_ops.h"
#include "utils.h"
#include "freetype_renderer.h"
#include "freetype_text.h"

extern int wWidth;
extern int wHeight;

#define DEFAULT_PLAYER_CURRENCY_AMOUNT 250
#define CURRENCY_TEXT_LENGTH 6

static CurrencyDef s_PlayerCurrency;
static char* s_CurrencyIconPath = "/res/static/textures/coin.png";
static int s_CurrencyTextHandles[CURRENCY_TEXT_LENGTH];

static int hide_trailing_zeros_drawables(const char* currency_str)
{
	// If the player has 0 amount - show a single 0 at the beginning
	int i = 0;
	if (s_PlayerCurrency.amount == 0)
	{
		i = 1;
	}

	for (; i < strlen(currency_str); i++)
	{
		char ch = currency_str[i];
		if (ch == '0')
		{
			int char_handle = s_CurrencyTextHandles[i];
			DrawableDef* char_drawable = NULL;
			get_drawable_def(&char_drawable, char_handle);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != char_drawable, "[player]: Failed to get currency char drawable.");
			char_drawable->visible = 0;
			continue;
		}

		break;
	}

	return 0;
}

static int popupale_str_with_trailing_zeros(char* currency_str)
{

	int num_digits = get_num_digits(s_PlayerCurrency.amount);
	int trailing_zeros_num = CURRENCY_TEXT_LENGTH - num_digits - 1;
	for (int i = 0; i < trailing_zeros_num; i++)
	{
		// Filling up with trailing zeros
		currency_str[i] = '0';
	}

	snprintf(&currency_str[trailing_zeros_num], sizeof(currency_str) - trailing_zeros_num, "%d", s_PlayerCurrency.amount);

	return 0;
}

// ! Allocates memory on heap ! 
static int render_currency_amount(int rerender)
{
	// 5 digits max
	char coins_str[CURRENCY_TEXT_LENGTH];
	Vec3 color = { 1.f, 1.f, 1.f };

	popupale_str_with_trailing_zeros(coins_str);

	if (rerender)
	{
		rerender_text(coins_str, CURRENCY_TEXT_LENGTH - 1, FT_DEFAULT_FONT_SIZE, 75.f, wHeight - 60.f, s_CurrencyTextHandles, CURRENCY_TEXT_LENGTH - 1);
	}
	else
	{
		memset(s_CurrencyTextHandles, -1, CURRENCY_TEXT_LENGTH * sizeof *s_CurrencyTextHandles);
		render_text(coins_str, CURRENCY_TEXT_LENGTH - 1, FT_DEFAULT_FONT_SIZE, 75.f, wHeight - 60.f, color, s_CurrencyTextHandles, CURRENCY_TEXT_LENGTH - 1);
	}

	hide_trailing_zeros_drawables(coins_str);

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
	render_currency_amount(0);

	return 0;
}

void change_player_currency_amount(int amount)
{
	s_PlayerCurrency.amount += amount;
	render_currency_amount(1);
}

int get_player_currency_amount()
{
	return s_PlayerCurrency.amount;
}
