#ifndef _DEMO_H_
#define _DEMO_H_ 1

#include <sdkddkver.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>

#define GL_GLEXT_PROTOTYPES
#include "support/glcorearb.h"
#define WGL_WGLEXT_PROTOTYPES
#include "support/wglext.h"

class Demo
{
	GLuint m_plane_vb_position;
	GLuint m_plane_vb_varying;
	GLuint m_plane_vao;
	GLuint m_plane_vs;
	GLuint m_plane_fs;
	GLuint m_plane_program;
	GLuint m_plane_uniform_buffer_per_frame_binding;

	GLuint m_rect_light_vao;
	GLuint m_rect_light_vs;
	GLuint m_rect_light_fs;
	GLuint m_rect_light_program;
	GLuint m_rect_light_uniform_buffer_per_frame_binding;

	GLuint m_post_process_vs;
	GLuint m_post_process_fs;
	GLuint m_post_process_program;
	GLuint m_post_process_vao;

	GLuint m_clamped_sampler;
	GLuint m_ltc_mat_texture;
	GLuint m_ltc_mag_texture;

	GLuint m_attachment_backup_odd;
	GLuint m_attachment_depth;
	GLuint m_framebuffer_lightpass;

public:
	void Init();
	void Tick(HDC hDC);
};

#endif