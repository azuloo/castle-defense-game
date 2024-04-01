#include "ft2build.h"
#include FT_FREETYPE_H 

#include "freetype_text.h"
#include "graphics.h"
#include "utils.h"
#include "file_reader.h"

static FT_Library      s_FTLib;
static FT_Face         s_Face; // TODO: Different faces for different fonts
static CharacterDef    s_Chars[128]; // TODO: Use c map impl here

#define FT_DEFAULT_FONT_SIZE 48

int init_ft()
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		PRINT_ERR("[freetype_text]: Failed to init FreeType lib.");
		return TERMINATE_ERR_CODE;
	}

	s_FTLib = ft;

	// TODO: Move to other func
	static const char* futura_font = "/res/fonts/16020_FUTURAM.ttf";
	char buf[256];
	get_file_path(futura_font, &buf, 256);

	FT_Face face;
	if (FT_New_Face(s_FTLib, buf, 0, &face))
	{
		// TODO: Free s_FTLib here?
		PRINT_ERR("[freetype_text]: Failed to load futura font.");
		return TERMINATE_ERR_CODE;
	}

	s_Face = face;

	// TODO: Add different sized glyphs support
	FT_Set_Pixel_Sizes(s_Face, 0, FT_DEFAULT_FONT_SIZE);

	return 0;
}

int ft_free_resources()
{
	FT_Done_Face(s_Face);
	FT_Done_FreeType(s_FTLib);

	return 0;
}

int load_ascii_chars()
{
	set_unpack_alignment(1);

	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(s_Face, c, FT_LOAD_RENDER))
		{
			// TODO: Free fr resources here?
			PRINT_ERR("[freetype_text]: Failed to load char.");
			return TERMINATE_ERR_CODE;
		}

		unsigned int texture;
		create_texture_2D(s_Face->glyph->bitmap.buffer, s_Face->glyph->bitmap.width, s_Face->glyph->bitmap.rows, &texture, TexType_RED);

		Vec2i size = { { s_Face->glyph->bitmap.width, s_Face->glyph->bitmap.rows } };
		Vec2i bearing = { { s_Face->glyph->bitmap_left, s_Face->glyph->bitmap_top } };
		CharacterDef character = {
			c,
			texture,
			s_Face->glyph->advance.x,
			size,
			bearing
		};

		s_Chars[c] = character;
	}

	return 0;
}

int find_char_def(char ch, CharacterDef** dest)
{
	for (int i = 0; i < 128; i++)
	{
		if (s_Chars[i].ch == ch)
		{
			*dest = &s_Chars[i];
			return 1;
		}
	}

	return 0;
}
