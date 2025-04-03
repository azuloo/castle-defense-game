#ifndef _FREETYPE_TEXT_H
#define _FREETYPE_TEXT_H

#include "lin_alg.h"

#define FT_DEFAULT_FONT_SIZE 26

typedef struct CharacterDef
{
	char            ch;
	unsigned int    tex_id;
	unsigned int    advance;
	Vec2i           size;
	Vec2i           bearing;
} CharacterDef;

int init_ft();
int add_font(int font_size);
int ft_free_resources();

int load_ascii_chars();
int find_char_def(char ch, int font_size, CharacterDef** dest);

#endif // _FREETYPE_TEXT_H
