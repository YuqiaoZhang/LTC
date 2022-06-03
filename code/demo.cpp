
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <windows.h>

#include <stdint.h>
#include <assert.h>
#include <cmath>

#include <DirectXMath.h>

#define GL_GLEXT_PROTOTYPES
#include "support/glcorearb.h"
#define WGL_WGLEXT_PROTOTYPES
#include "support/wglext.h"

#include "support/resolution.h"

#include "support/camera_controller.h"

#include "demo.h"

#include "ltc_tables.h"

static uint32_t const plane_vs_bytecode[] = {
#include "../shaders/plane.vert.inl"
};

static uint32_t const plane_fs_bytecode[] = {
#include "../shaders/plane.frag.inl"
};

static uint32_t const rect_light_vs_bytecode[] = {
#include "../shaders/rect_light.vert.inl"
};

static uint32_t const rect_light_fs_bytecode[] = {
#include "../shaders/rect_light.frag.inl"
};

static uint32_t const post_process_vs_bytecode[] = {
#include "../shaders/post_process.vert.inl"
};

static uint32_t const post_process_fs_bytecode[] = {
#include "../shaders/post_process.frag.inl"
};

struct plane_uniform_buffer_per_frame_binding_t
{

	// mesh
	DirectX::XMFLOAT4X4 model_transform;
	DirectX::XMFLOAT3 dcolor;
	float _padding_dcolor;
	DirectX::XMFLOAT3 scolor;
	float _padding_scolor;
	float roughness;
	float _padding_roughness_1;
	float _padding_roughness_2;
	float _padding_roughness_3;

	// camera
	DirectX::XMFLOAT4X4 view_transform;
	DirectX::XMFLOAT4X4 projection_transform;
	DirectX::XMFLOAT3 eye_position;
	float _padding_eye_position;

	// light
	DirectX::XMFLOAT4 rect_light_vetices[4];
	float intensity;
	float twoSided;
};

struct rect_light_uniform_buffer_per_frame_binding_t
{
	// camera
	DirectX::XMFLOAT4X4 view_transform;
	DirectX::XMFLOAT4X4 projection_transform;

	// light
	DirectX::XMFLOAT4 rect_light_vetices[4];
	float intensity;
};

void Demo::Init()
{
	m_plane_vb_position = 0;
	{
		glGenBuffers(1, &m_plane_vb_position);
		glBindBuffer(GL_ARRAY_BUFFER, m_plane_vb_position);

		float vb_position_data[] = {
			-7777.0, 0.0, 7777.0,
			7777.0, 0.0, 7777.0,
			-7777.0, 0.0, -7777.0,
			7777.0, 0.0, -7777.0 };

		glBufferData(GL_ARRAY_BUFFER, sizeof(vb_position_data), vb_position_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_plane_vb_varying = 0;
	{
		glGenBuffers(1, &m_plane_vb_varying);
		glBindBuffer(GL_ARRAY_BUFFER, m_plane_vb_varying);

		float vb_varying_data[] = {
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0 };

		glBufferData(GL_ARRAY_BUFFER, sizeof(vb_varying_data), vb_varying_data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_plane_vao;
	{
		glGenVertexArrays(1, &m_plane_vao);
		assert(0 != m_plane_vao);

		glBindVertexArray(m_plane_vao);
		glEnableVertexAttribArray(0U);
		glVertexAttribFormat(0U, 3U, GL_FLOAT, GL_FALSE, 0U);
		glVertexAttribBinding(0U, 0U);
		glEnableVertexAttribArray(1U);
		glVertexAttribFormat(1U, 3U, GL_FLOAT, GL_FALSE, 0U);
		glVertexAttribBinding(1U, 1U);
		glBindVertexArray(0U);
	}

	m_plane_vs = 0;
	{
		m_plane_vs = glCreateShader(GL_VERTEX_SHADER);
		assert(0 != m_plane_vs);

		glShaderBinary(1, &m_plane_vs, GL_SHADER_BINARY_FORMAT_SPIR_V, plane_vs_bytecode, sizeof(plane_vs_bytecode));
		glSpecializeShader(m_plane_vs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_plane_vs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_plane_fs = 0;
	{
		m_plane_fs = glCreateShader(GL_FRAGMENT_SHADER);
		assert(0 != m_plane_fs);

		glShaderBinary(1, &m_plane_fs, GL_SHADER_BINARY_FORMAT_SPIR_V, plane_fs_bytecode, sizeof(plane_fs_bytecode));
		glSpecializeShader(m_plane_fs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_plane_fs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_plane_program = 0;
	{
		m_plane_program = glCreateProgram();
		assert(0 != m_plane_program);

		glAttachShader(m_plane_program, m_plane_vs);
		glAttachShader(m_plane_program, m_plane_fs);

		glLinkProgram(m_plane_program);
		GLint link_status;
		glGetProgramiv(m_plane_program, GL_LINK_STATUS, &link_status);
		assert(GL_TRUE == link_status);
	}

	m_plane_uniform_buffer_per_frame_binding = 0;
	{
		glGenBuffers(1, &m_plane_uniform_buffer_per_frame_binding);
		assert(0 != m_plane_uniform_buffer_per_frame_binding);

		glBindBuffer(GL_UNIFORM_BUFFER, m_plane_uniform_buffer_per_frame_binding);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(plane_uniform_buffer_per_frame_binding_t), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0U);
	}

	m_rect_light_vao;
	{
		glGenVertexArrays(1, &m_rect_light_vao);
		assert(0 != m_rect_light_vao);

		glBindVertexArray(m_rect_light_vao);
		glBindVertexArray(0U);
	}

	m_rect_light_vs = 0;
	{
		m_rect_light_vs = glCreateShader(GL_VERTEX_SHADER);
		assert(0 != m_rect_light_vs);

		glShaderBinary(1, &m_rect_light_vs, GL_SHADER_BINARY_FORMAT_SPIR_V, rect_light_vs_bytecode, sizeof(rect_light_vs_bytecode));
		glSpecializeShader(m_rect_light_vs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_rect_light_vs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_rect_light_fs = 0;
	{
		m_rect_light_fs = glCreateShader(GL_FRAGMENT_SHADER);
		assert(0 != m_rect_light_fs);

		glShaderBinary(1, &m_rect_light_fs, GL_SHADER_BINARY_FORMAT_SPIR_V, rect_light_fs_bytecode, sizeof(rect_light_fs_bytecode));
		glSpecializeShader(m_rect_light_fs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_rect_light_fs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_rect_light_program = 0;
	{
		m_rect_light_program = glCreateProgram();
		assert(0 != m_rect_light_program);

		glAttachShader(m_rect_light_program, m_rect_light_vs);
		glAttachShader(m_rect_light_program, m_rect_light_fs);

		glLinkProgram(m_rect_light_program);
		GLint link_status;
		glGetProgramiv(m_rect_light_program, GL_LINK_STATUS, &link_status);
		assert(GL_TRUE == link_status);
	}

	m_rect_light_uniform_buffer_per_frame_binding = 0;
	{
		glGenBuffers(1, &m_rect_light_uniform_buffer_per_frame_binding);
		assert(0 != m_rect_light_uniform_buffer_per_frame_binding);

		glBindBuffer(GL_UNIFORM_BUFFER, m_rect_light_uniform_buffer_per_frame_binding);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(rect_light_uniform_buffer_per_frame_binding_t), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0U);
	}

	m_post_process_vs = 0;
	{
		m_post_process_vs = glCreateShader(GL_VERTEX_SHADER);
		assert(0 != m_post_process_vs);

		glShaderBinary(1, &m_post_process_vs, GL_SHADER_BINARY_FORMAT_SPIR_V, post_process_vs_bytecode, sizeof(post_process_vs_bytecode));
		glSpecializeShader(m_post_process_vs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_post_process_vs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_post_process_fs = 0;
	{
		m_post_process_fs = glCreateShader(GL_FRAGMENT_SHADER);
		assert(0 != m_post_process_fs);

		glShaderBinary(1, &m_post_process_fs, GL_SHADER_BINARY_FORMAT_SPIR_V, post_process_fs_bytecode, sizeof(post_process_fs_bytecode));
		glSpecializeShader(m_post_process_fs, "main", 0, NULL, NULL);

		GLint compile_status;
		glGetShaderiv(m_post_process_fs, GL_COMPILE_STATUS, &compile_status);
		assert(GL_TRUE == compile_status);
	}

	m_post_process_program = 0;
	{
		m_post_process_program = glCreateProgram();
		assert(0 != m_post_process_program);

		glAttachShader(m_post_process_program, m_post_process_vs);
		glAttachShader(m_post_process_program, m_post_process_fs);

		glLinkProgram(m_post_process_program);
		GLint link_status;
		glGetProgramiv(m_post_process_program, GL_LINK_STATUS, &link_status);
		assert(GL_TRUE == link_status);
	}

	m_post_process_vao = 0;
	{
		glGenVertexArrays(1, &m_post_process_vao);
		assert(0 != m_post_process_vao);

		glBindVertexArray(m_post_process_vao);
		glBindVertexArray(0U);
	}

	m_clamped_sampler = 0;
	{
		glGenSamplers(1, &m_clamped_sampler);
		assert(0 != m_clamped_sampler);

		glSamplerParameteri(m_clamped_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_clamped_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_clamped_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_clamped_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glSamplerParameteri(m_clamped_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	m_ltc_mat_texture = 0;
	{
		glGenTextures(1, &m_ltc_mat_texture);
		assert(0 != m_ltc_mat_texture);

		glBindTexture(GL_TEXTURE_2D, m_ltc_mat_texture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 64, 64);
		static_assert((64 * 64 * 4) == (sizeof(g_ltc_1) / sizeof(g_ltc_1[0])), "");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RGBA, GL_FLOAT, g_ltc_1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_ltc_mag_texture = 0;
	{
		glGenTextures(1, &m_ltc_mag_texture);
		assert(0 != m_ltc_mag_texture);

		glBindTexture(GL_TEXTURE_2D, m_ltc_mag_texture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 64, 64);
		static_assert((64 * 64 * 4) == (sizeof(g_ltc_2) / sizeof(g_ltc_2[0])), "");
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RGBA, GL_FLOAT, g_ltc_2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_attachment_backup_odd;
	{
		glGenTextures(1, &m_attachment_backup_odd);
		glBindTexture(GL_TEXTURE_2D, m_attachment_backup_odd);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, g_resolution_width, g_resolution_height);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_attachment_depth;
	{
		glGenTextures(1, &m_attachment_depth);
		glBindTexture(GL_TEXTURE_2D, m_attachment_depth);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, g_resolution_width, g_resolution_height);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_framebuffer_lightpass;
	{
		glGenFramebuffers(1, &m_framebuffer_lightpass);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_lightpass);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_attachment_backup_odd, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_attachment_depth, 0);

		GLenum RenderTarget[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, RenderTarget);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0U);
	}

	BOOL res_swap_interval = wglSwapIntervalEXT(1);
	assert(FALSE != res_swap_interval);

	g_camera_controller.m_eye_position = DirectX::XMFLOAT3(0.00000000, 6.00000000, -0.500000000);
	g_camera_controller.m_eye_direction = DirectX::XMFLOAT3(0.00000000, 0.174311504, 1.99238944);
	g_camera_controller.m_up_direction = DirectX::XMFLOAT3(0.0, 1.0, 0.0);
}

void Demo::Tick(HDC hDC)
{
	// Upload
	plane_uniform_buffer_per_frame_binding_t plane_uniform_buffer_data_per_frame_binding;
	rect_light_uniform_buffer_per_frame_binding_t rect_light_uniform_buffer_data_per_frame_binding;
	{
		// camera
		{
			DirectX::XMFLOAT3 eye_position = g_camera_controller.m_eye_position;
			DirectX::XMFLOAT3 eye_direction = g_camera_controller.m_eye_direction;
			DirectX::XMFLOAT3 up_direction = g_camera_controller.m_up_direction;

			DirectX::XMMATRIX tmp_view_transform = DirectX::XMMatrixLookToRH(DirectX::XMLoadFloat3(&eye_position), DirectX::XMLoadFloat3(&eye_direction), DirectX::XMLoadFloat3(&up_direction));
			DirectX::XMFLOAT4X4 view_transform;
			DirectX::XMStoreFloat4x4(&view_transform, tmp_view_transform);

			float fov_angle_y = 2.0 * atan((1.0 / 2.0));
			DirectX::XMMATRIX tmp_projection_transform = DirectX::XMMatrixPerspectiveFovRH(fov_angle_y, 1.0, 7.0, 7777.0);
			DirectX::XMFLOAT4X4 projection_transform;
			DirectX::XMStoreFloat4x4(&projection_transform, tmp_projection_transform);

			plane_uniform_buffer_data_per_frame_binding.view_transform = view_transform;
			plane_uniform_buffer_data_per_frame_binding.projection_transform = projection_transform;
			plane_uniform_buffer_data_per_frame_binding.eye_position = eye_position;

			rect_light_uniform_buffer_data_per_frame_binding.view_transform = view_transform;
			rect_light_uniform_buffer_data_per_frame_binding.projection_transform = projection_transform;
		}

		// light
		{
			plane_uniform_buffer_data_per_frame_binding.rect_light_vetices[0] = DirectX::XMFLOAT4(-4.0, 2.0, 32.0, 1.0);
			plane_uniform_buffer_data_per_frame_binding.rect_light_vetices[1] = DirectX::XMFLOAT4(4.0, 2.0, 32.0, 1.0);
			plane_uniform_buffer_data_per_frame_binding.rect_light_vetices[2] = DirectX::XMFLOAT4(4.0, 10.0, 32.0, 1.0);
			plane_uniform_buffer_data_per_frame_binding.rect_light_vetices[3] = DirectX::XMFLOAT4(-4.0, 10.0, 32.0, 1.0);
			plane_uniform_buffer_data_per_frame_binding.twoSided = -1.0;
			plane_uniform_buffer_data_per_frame_binding.intensity = 4.0;

			rect_light_uniform_buffer_data_per_frame_binding.rect_light_vetices[0] = DirectX::XMFLOAT4(-4.0, 2.0, 32.0, 1.0);
			rect_light_uniform_buffer_data_per_frame_binding.rect_light_vetices[1] = DirectX::XMFLOAT4(4.0, 2.0, 32.0, 1.0);
			rect_light_uniform_buffer_data_per_frame_binding.rect_light_vetices[2] = DirectX::XMFLOAT4(-4.0, 10.0, 32.0, 1.0);
			rect_light_uniform_buffer_data_per_frame_binding.rect_light_vetices[3] = DirectX::XMFLOAT4(4.0, 10.0, 32.0, 1.0);
			rect_light_uniform_buffer_data_per_frame_binding.intensity = 4.0;
		}

		// mesh
		{
			DirectX::XMFLOAT4X4 model_transform;
			DirectX::XMStoreFloat4x4(&model_transform, DirectX::XMMatrixIdentity());
			plane_uniform_buffer_data_per_frame_binding.model_transform = model_transform;
			plane_uniform_buffer_data_per_frame_binding.dcolor = DirectX::XMFLOAT3A(1.0, 1.0, 1.0);
			plane_uniform_buffer_data_per_frame_binding.scolor = DirectX::XMFLOAT3A(0.23, 0.23, 0.23);
			plane_uniform_buffer_data_per_frame_binding.roughness = 0.25;
		}
	}
	glBindBuffer(GL_UNIFORM_BUFFER, m_plane_uniform_buffer_per_frame_binding);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(plane_uniform_buffer_per_frame_binding_t), &plane_uniform_buffer_data_per_frame_binding);
	glBindBuffer(GL_UNIFORM_BUFFER, 0U);
	glBindBuffer(GL_UNIFORM_BUFFER, m_rect_light_uniform_buffer_per_frame_binding);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(rect_light_uniform_buffer_per_frame_binding_t), &rect_light_uniform_buffer_data_per_frame_binding);
	glBindBuffer(GL_UNIFORM_BUFFER, 0U);

	// Light Pass
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer_lightpass);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer_lightpass);

		GLfloat color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearBufferfv(GL_COLOR, 0, color);
		GLfloat depth = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &depth);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		// glEnable(GL_BLEND);
		// glBlendFunc(GL_ONE, GL_ONE);
		glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		GLfloat Viewport[4] = { 0.0f, 0.0f, g_resolution_width, g_resolution_height };
		glViewportArrayv(0U, 1U, Viewport);
		glDisable(GL_CULL_FACE);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_MULTISAMPLE);

		// Draw Plane
		{
			glUseProgram(m_plane_program);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_plane_uniform_buffer_per_frame_binding);
			glBindSampler(1, m_clamped_sampler);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_ltc_mat_texture);
			glBindSampler(2, m_clamped_sampler);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_ltc_mag_texture);

			glBindVertexArray(m_plane_vao);
			glBindVertexBuffer(0U, m_plane_vb_position, 0U, sizeof(float) * 3);
			glBindVertexBuffer(1U, m_plane_vb_varying, 0U, sizeof(float) * 3);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			// unbind
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
			glBindSampler(1, 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindSampler(2, 0);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Draw Rect Light
		{
			glUseProgram(m_rect_light_program);
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_rect_light_uniform_buffer_per_frame_binding);

			glBindVertexArray(m_rect_light_vao);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	// Post Process Pass
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);

		glUseProgram(m_post_process_program);
		glBindSampler(1, m_clamped_sampler);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_attachment_backup_odd);

		glBindVertexArray(m_post_process_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

		// unbind
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
		glBindSampler(1, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	BOOL res_swap_buffers = SwapBuffers(hDC);
	assert(TRUE == res_swap_buffers);
}