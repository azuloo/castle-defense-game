#ifndef _GLOBAL_DEFS_H
#define _GLOBAL_DEFS_H

#pragma warning(disable: 6001)

#define MAX_FRAME_TIME 0.1f

// Literals
#define COMMON_MODEL_UNIFORM_NAME         "model"
#define COMMON_PROJECTION_UNIFORM_NAME    "projection"
#define COMMON_COLOR_UNIFORM_NAME         "UColor"

enum DrawLayer
{
	DrawLayer_Background = 0,
	DrawLayer_MapPath,
	DrawLayer_Castle,
	DrawLayer_Entity,
	DrawLayer_Tower,
	DrawLayer_Projectile,
	DrawLayer_TextBackground,

	DrawLayer_Text = 100,
};

#define POS_TEXTURE_ATTRIBUTE_SIZE 4

#endif // _GLOBAL_DEFS_H
