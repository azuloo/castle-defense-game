#ifndef _FREETYPE_TEXT_H
#define _FREETYPE_TEXT_H

#include "lin_alg.h"

typedef struct CharacterDef
{
	char            ch;
	unsigned int    tex_id;
	unsigned int    advance;
	Vec2i           size;
	Vec2i           bearing;
} CharacterDef;

int init_ft();
int ft_free_resources();

int load_ascii_chars();
int find_char_def(char ch, CharacterDef** dest);

#endif // _FREETYPE_TEXT_H
