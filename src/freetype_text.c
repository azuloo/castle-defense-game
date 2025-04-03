#include "ft2build.h"
#include FT_FREETYPE_H 

#include "freetype_text.h"
#include "graphics.h"
#include "utils.h"
#include "file_reader.h"

#define FT_FACE_SIZE 16

static FT_Library      s_FTLib;
static FT_Face         s_Face[FT_FACE_SIZE]; // TODO: Different faces for different fonts
static int             s_FaceFontMap[FT_FACE_SIZE];
static int             s_FaceCount = 0;
static CharacterDef    s_Chars[FT_FACE_SIZE][128]; // TODO: Use c map impl here

int init_ft()
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		PRINT_ERR("[freetype_text]: Failed to init FreeType lib.");
		return TERMINATE_ERR_CODE;
	}

	s_FTLib = ft;

	return 0;
}

int add_font(int font_size)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(s_FaceCount < FT_FACE_SIZE, "[freetype_text]: Reached the max font capacity.");

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

	s_Face[s_FaceCount] = face;
	s_FaceFontMap[s_FaceCount] = font_size;

	FT_Set_Pixel_Sizes(s_Face[s_FaceCount], 0, font_size);

	s_FaceCount++;

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
	static const tex_params[8] = {
		GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER,
		GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER,
		GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR,
		GL_TEXTURE_MAG_FILTER, GL_LINEAR
	};
	set_unpack_alignment(1);

	for (int i = 0; i < s_FaceCount; i++)
	{
		for (unsigned char c = 0; c < 128; c++)
		{
			if (FT_Load_Char(s_Face[i], c, FT_LOAD_RENDER))
			{
				// TODO: Free fr resources here?
				PRINT_ERR("[freetype_text]: Failed to load char.");
				return TERMINATE_ERR_CODE;
			}

			unsigned int texture;
			create_texture_2D(s_Face[i]->glyph->bitmap.buffer, s_Face[i]->glyph->bitmap.width, s_Face[i]->glyph->bitmap.rows, &texture, TexType_RED, tex_params, sizeof(tex_params) / sizeof(tex_params[0]));

			Vec2i size = { { s_Face[i]->glyph->bitmap.width, s_Face[i]->glyph->bitmap.rows } };
			Vec2i bearing = { { s_Face[i]->glyph->bitmap_left, s_Face[i]->glyph->bitmap_top } };
			CharacterDef character = {
				c,
				texture,
				s_Face[i]->glyph->advance.x,
				size,
				bearing,
			};

			s_Chars[i][c] = character;
		}
	}

	set_unpack_alignment(DEFAULT_UNPACK_ALIGNMENT);

	return 0;
}

int find_char_def(char ch, int font_size, CharacterDef** dest)
{
	int face_font_idx = -1;
	// TODO: Use map here.
	for (int i = 0; i < FT_FACE_SIZE; i++)
	{
		if (s_FaceFontMap[i] == font_size)
		{
			face_font_idx = i;
			break;
		}
	}
	CHECK_EXPR_FAIL_RET_TERMINATE(-1 != face_font_idx, "[freetype_text]: Failed to find the corresponding face idx for this font.");

	for (int i = 0; i < 128; i++)
	{
		if (s_Chars[face_font_idx][i].ch == ch)
		{
			*dest = &s_Chars[face_font_idx][i];
			return 1;
		}
	}

	return 0;
}
