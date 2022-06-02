#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <windows.h>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"
#define WGL_WGLEXT_PROTOTYPES
#include "wglext.h"

#include "window_main.h"

#include "render_main.h"

#include "libopengl.h"

#include "../demo.h"

#ifndef NDEBUG
static void APIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

unsigned __stdcall render_main(void* pVoid)
{
	HWND hWnd = static_cast<HWND>(pVoid);
	HDC hDC = GetDC(hWnd);

	// WGL choose config
	{
		PIXELFORMATDESCRIPTOR FormatDescriptor = {};
		int PixelFormatCount = DescribePixelFormat(hDC, 1, sizeof(FormatDescriptor), &FormatDescriptor);

		int PixelFormatIndex = 1;

		while (PixelFormatIndex <= PixelFormatCount)
		{
			int res_describe_pixel_format = DescribePixelFormat(hDC, PixelFormatIndex, sizeof(FormatDescriptor), &FormatDescriptor);
			assert(0 != res_describe_pixel_format);

			if (
				(0U != (FormatDescriptor.dwFlags & PFD_DOUBLEBUFFER)) && (0U == (FormatDescriptor.dwFlags & PFD_STEREO)) && (0U != (FormatDescriptor.dwFlags & PFD_DRAW_TO_WINDOW)) && (0U != (FormatDescriptor.dwFlags & PFD_SUPPORT_OPENGL))
				// && (0U != (FormatDescriptor.dwFlags & PFD_SUPPORT_COMPOSITION)) // https://www.opengl.org/pipeline/article/vol003_7
				&& (PFD_TYPE_RGBA == FormatDescriptor.iPixelType) && (0U != FormatDescriptor.cColorBits) && (0U == FormatDescriptor.cAccumBits) && (0U == FormatDescriptor.cDepthBits) && (0U == FormatDescriptor.cStencilBits))
			{
				break;
			}

			++PixelFormatIndex;
		}

		BOOL res_set_pixel_format = SetPixelFormat(hDC, PixelFormatIndex, &FormatDescriptor);
		assert(res_set_pixel_format != FALSE);
	}

	// Context Legacy
	{
		HGLRC tmp_opengl_context = wglCreateContext(hDC);
		assert(NULL != tmp_opengl_context);

		BOOL res_make_current_1 = wglMakeCurrent(hDC, tmp_opengl_context);
		assert(FALSE != res_make_current_1);

		init_libwgl();

		BOOL res_make_current_2 = wglMakeCurrent(hDC, 0);
		assert(FALSE != res_make_current_2);

		BOOL res_delete_context = wglDeleteContext(tmp_opengl_context);
		assert(FALSE != res_delete_context);
	}

	// Context ARB
	HGLRC opengl_context = NULL;
	{
		int attribList[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 6,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifndef NDEBUG
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#else
			WGL_CONTEXT_FLAGS_ARB, 0,

#endif
			0 };
		opengl_context = wglCreateContextAttribsARB(hDC, NULL, attribList);
		assert(0 != opengl_context);
	}

	BOOL res_make_current_1 = wglMakeCurrent(hDC, opengl_context);
	assert(res_make_current_1 != FALSE);

	init_libopengl();

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(opengl_debug_message_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0U, NULL, GL_TRUE);
	glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0U, NULL, GL_FALSE);
#endif

	class Demo demo;

	demo.Init();

	while (!g_window_quit)
	{
		demo.Tick(hDC);
	}

	BOOL res_make_current_2 = wglMakeCurrent(hDC, NULL);
	assert(res_make_current_2 != FALSE);

	BOOL res_delete_context = wglDeleteContext(opengl_context);
	assert(res_delete_context != FALSE);

	return 0U;
}

#ifndef NDEBUG
static void APIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	char* str_source;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		str_source = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		str_source = "Window_System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		str_source = "Shader_Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		str_source = "ThirdParty";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		str_source = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		str_source = "Other";
		break;
	default:
		str_source = "Unknown";
	}

	char* str_type;
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		str_type = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		str_type = "Deprecated_Behavior";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		str_type = "Undefined_Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		str_type = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		str_type = "Peformance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		str_type = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		str_type = "Push_Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		str_type = "Pop_Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		str_type = "Other";
		break;
	default:
		str_type = "Unknown";
	}

	char* str_severity;
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		str_severity = "High";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		str_severity = "Medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		str_severity = "Low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		str_severity = "Notification";
		break;
	default:
		str_severity = "Unknown";
	}

	char OutputString[4096];
	sprintf_s(OutputString, "OpenGL Debug Message \n Source_%s \n Type_%s \n ID 0X%08X \n Severity_%s\n %s \n", str_source, str_type, id, str_severity, message);
	OutputDebugStringA(OutputString);
}
#endif
