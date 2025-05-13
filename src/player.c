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

static int hide_redundant_zeros_drawables(const char* currency_str)
{
	int num_digits = get_num_digits(s_PlayerCurrency.amount);

	for (int i = num_digits; i < strlen(currency_str); i++)
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

static int append_zeros_to_currency(char* currency_str)
{
	int num_digits = get_num_digits(s_PlayerCurrency.amount);
	snprintf(currency_str, CURRENCY_TEXT_LENGTH, "%d", s_PlayerCurrency.amount);

	for (int i = num_digits; i < CURRENCY_TEXT_LENGTH - 1; i++)
	{
		// Appdening zeros
		currency_str[i] = '0';
	}

	return 0;
}

// ! Allocates memory on heap ! 
static int render_currency_amount(int rerender)
{
	char coins_str[CURRENCY_TEXT_LENGTH];
	coins_str[CURRENCY_TEXT_LENGTH - 1] = '\0';
	Vec3 color = { 1.f, 1.f, 1.f };

	append_zeros_to_currency(coins_str);

	if (rerender)
	{
		rerender_text(coins_str, CURRENCY_TEXT_LENGTH - 1, FT_DEFAULT_FONT_SIZE, 75.f, wHeight - 60.f, s_CurrencyTextHandles, CURRENCY_TEXT_LENGTH - 1);
	}
	else
	{
		memset(s_CurrencyTextHandles, -1, CURRENCY_TEXT_LENGTH * sizeof *s_CurrencyTextHandles);
		render_text(coins_str, CURRENCY_TEXT_LENGTH - 1, FT_DEFAULT_FONT_SIZE, 75.f, wHeight - 60.f, color, s_CurrencyTextHandles, CURRENCY_TEXT_LENGTH - 1);
	}

	hide_redundant_zeros_drawables(coins_str);

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
