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
public:
	void Init();
	void Tick(HDC hDC);
};

#endif