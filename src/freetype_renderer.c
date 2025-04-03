#include "freetype_renderer.h"
#include "graphics.h"
#include "freetype_text.h"
#include "utils.h"
#include "global_defs.h"

#define TEXT_QUAD_VERTICES_COUNT   24
#define TEXT_DEFUALT_SCALE         1.f

static int create_freetype_buffers(unsigned int* vao, unsigned int* vbo)
{
	create_vao(vao);
	create_vbo(vbo, NULL, TEXT_QUAD_VERTICES_COUNT, DRAW_MODE_DYNAMIC);

	// TODO: Check for errors

	return 0;
}

static int build_ft_shaders(char** vertex_shader_dest, char** frag_shader_dest)
{
	static const char* vertex_shader_path = "/res/static/shaders/freetype_text_vert.txt";
	static const char* frag_shader_path = "/res/static/shaders/freetype_text_frag.txt";

	char* vertex_shader_src = get_shader_source(vertex_shader_path);
	char* fragment_shader_src = get_shader_source(frag_shader_path);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != vertex_shader_src && NULL != fragment_shader_src, "[freetype_renderer]: Failed to get vertex and/or fragment shader(s) sources.");

	*vertex_shader_dest = vertex_shader_src;
	*frag_shader_dest = fragment_shader_src;

	return 0;
}

int render_text(const char* text, int font_size, float x, float y, Vec3 color)
{
	ASSERT_GRAPHICS_INITIALIZED

	Mat4 ortho_mat = COMMON_ORTHO_MAT;

	unsigned int vao;
	unsigned int vbo;

	create_freetype_buffers(&vao, &vbo);

	char* vertex_shader_src = NULL;
	char* fragment_shader_src = NULL;

	build_ft_shaders(&vertex_shader_src, &fragment_shader_src);

	char* glyph = text;
	CharacterDef* char_def = NULL;
	for (int i = 0; i < strlen(text); i++)
	{
		if (find_char_def(*glyph, font_size, &char_def))
		{
			DrawableDef* drawable = NULL;
			create_drawable(&drawable);
			CHECK_EXPR_FAIL_RET_TERMINATE(NULL != drawable, "[freetype_renderer]: Drawable is not defined.");

			drawable->buffers.vao = vao;
			drawable->buffers.vbo = vbo;

			drawable->texture = char_def->tex_id;
			drawable->draw_mode = DRAW_MODE_DYNAMIC;

			drawable->draw_layer = DrawLayer_Text;

			compile_shaders(&drawable->shader_prog, vertex_shader_src, fragment_shader_src);

			add_uniform_vec3f(drawable->shader_prog, "TextColor", &color);
			add_uniform_mat4f(drawable->shader_prog, "projection", &ortho_mat);

			float xpos = x + char_def->bearing.x * TEXT_DEFUALT_SCALE;
			float ypos = y - (char_def->size.y - char_def->bearing.y) * TEXT_DEFUALT_SCALE;
			float w = char_def->size.x * TEXT_DEFUALT_SCALE;
			float h = char_def->size.y * TEXT_DEFUALT_SCALE;

			float vertices[] = {
				xpos,     ypos + h,   0.0f, 0.0f,
				xpos,     ypos,       0.0f, 1.0f,
				xpos + w, ypos,       1.0f, 1.0f,

				xpos,     ypos + h,   0.0f, 0.0f,
				xpos + w, ypos,       1.0f, 1.0f,
				xpos + w, ypos + h,   1.0f, 0.0f
			};

			register_drawable_attribute(drawable, 4);
			process_drawable_attributes(drawable);

			DrawBufferData buf_data;
			buf_data.vertices = vertices;
			buf_data.vertices_len = 24;
			buf_data.indices = NULL;
			buf_data.indices_len = 0;

			create_drawable_buffer_data(drawable, &buf_data);

			x += (char_def->advance >> 6) * TEXT_DEFUALT_SCALE; // Bitshift by 6 to get value in pixels (2^6 = 64)

			glyph++;
		}
	}

	free(vertex_shader_src);
	free(fragment_shader_src);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return 0;
}
