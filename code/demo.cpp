
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <windows.h>

#include <assert.h>

#define GL_GLEXT_PROTOTYPES
#include "support/glcorearb.h"
#define WGL_WGLEXT_PROTOTYPES
#include "support/wglext.h"

#include "demo.h"

void Demo::Init()
{
}

void Demo::Tick(HDC hDC)
{
	BOOL res_swap_buffers = SwapBuffers(hDC);
	assert(TRUE == res_swap_buffers);
}