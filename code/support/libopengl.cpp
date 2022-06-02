#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"
#define WGL_WGLEXT_PROTOTYPES
#include "wglext.h"

static PFNWGLCREATECONTEXTATTRIBSARBPROC s_pFn_wglCreateContextAttribsARB = NULL;
HGLRC WINAPI wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int* attribList)
{
	return s_pFn_wglCreateContextAttribsARB(hDC, hShareContext, attribList);
}

static PFNWGLMAKECONTEXTCURRENTARBPROC s_pFn_wglMakeContextCurrentARB = NULL;
BOOL WINAPI wglMakeContextCurrentARB(HDC hDrawDC, HDC hReadDC, HGLRC hglrc)
{
	return s_pFn_wglMakeContextCurrentARB(hDrawDC, hReadDC, hglrc);
}

static PFNWGLSWAPINTERVALEXTPROC s_pFn_wglSwapIntervalEXT = NULL;
BOOL WINAPI wglSwapIntervalEXT(int interval)
{
	return s_pFn_wglSwapIntervalEXT(interval);
}

static PFNGLDEBUGMESSAGECALLBACKPROC s_pFn_glDebugMessageCallback = NULL;
void APIENTRY glDebugMessageCallback(GLDEBUGPROCARB callback, const void* userParam)
{
	return s_pFn_glDebugMessageCallback(callback, userParam);
}

static PFNGLDEBUGMESSAGECONTROLPROC s_pFn_glDebugMessageControl = NULL;
void APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
	return s_pFn_glDebugMessageControl(source, type, severity, count, ids, enabled);
}

static PFNGLGENBUFFERSPROC s_pFn_glGenBuffers = NULL;
void APIENTRY glGenBuffers(GLsizei n, GLuint* buffers)
{
	return s_pFn_glGenBuffers(n, buffers);
}

static PFNGLBINDBUFFERPROC s_pFn_glBindBuffer = NULL;
void APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
	return s_pFn_glBindBuffer(target, buffer);
}

static PFNGLBUFFERDATAPROC s_pFn_glBufferData = NULL;
void APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	return s_pFn_glBufferData(target, size, data, usage);
}

static PFNGLBUFFERSTORAGEPROC s_pFn_glBufferStorage = NULL;
void APIENTRY glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags)
{
	return s_pFn_glBufferStorage(target, size, data, flags);
}

static PFNGLBUFFERSUBDATAPROC s_pFn_glBufferSubData = NULL;
void APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
	return s_pFn_glBufferSubData(target, offset, size, data);
}

static PFNGLACTIVETEXTUREPROC s_pFn_glActiveTexture = NULL;
void APIENTRY glActiveTexture(GLenum texture)
{
	return s_pFn_glActiveTexture(texture);
}

static PFNGLTEXSTORAGE2DPROC s_pFn_glTexStorage2D = NULL;
void APIENTRY glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	return s_pFn_glTexStorage2D(target, levels, internalformat, width, height);
}

static PFNGLTEXSTORAGE2DMULTISAMPLEPROC s_pFn_glTexStorage2DMultisample = NULL;
void APIENTRY glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	return s_pFn_glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

static PFNGLGENSAMPLERSPROC s_pFn_glGenSamplers = NULL;
void APIENTRY glGenSamplers(GLsizei count, GLuint* samplers)
{

	return s_pFn_glGenSamplers(count, samplers);
}

static PFNGLSAMPLERPARAMETERIPROC s_pFn_glSamplerParameteri = NULL;
void APIENTRY glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	return s_pFn_glSamplerParameteri(sampler, pname, param);
}

static PFNGLSAMPLERPARAMETERFVPROC s_pFn_glSamplerParameterfv = NULL;
void APIENTRY glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
	return s_pFn_glSamplerParameterfv(sampler, pname, param);
}

static PFNGLBINDSAMPLERPROC s_pFn_glBindSampler = NULL;
void APIENTRY glBindSampler(GLuint unit, GLuint sampler)
{
	return s_pFn_glBindSampler(unit, sampler);
}

static PFNGLGENFRAMEBUFFERSPROC s_pFn_glGenFramebuffers = NULL;
void APIENTRY glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	return s_pFn_glGenFramebuffers(n, framebuffers);
}

static PFNGLBINDFRAMEBUFFERPROC s_pFn_glBindFramebuffer = NULL;
void APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	return s_pFn_glBindFramebuffer(target, framebuffer);
}

static PFNGLFRAMEBUFFERTEXTURE2DPROC s_pFn_glFramebufferTexture2D = NULL;
void APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	return s_pFn_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

static PFNGLDRAWBUFFERSPROC s_pFn_glDrawBuffers = NULL;
void APIENTRY glDrawBuffers(GLsizei n, const GLenum* bufs)
{
	return s_pFn_glDrawBuffers(n, bufs);
}

static PFNGLCHECKFRAMEBUFFERSTATUSPROC s_pFn_glCheckFramebufferStatus = NULL;
GLenum APIENTRY glCheckFramebufferStatus(GLenum target)
{
	return s_pFn_glCheckFramebufferStatus(target);
}

static PFNGLCREATESHADERPROC s_pFn_glCreateShader = NULL;
GLuint APIENTRY glCreateShader(GLenum type)
{
	return s_pFn_glCreateShader(type);
}

static PFNGLSHADERBINARYPROC s_pFn_glShaderBinary = NULL;
void APIENTRY glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
	return s_pFn_glShaderBinary(count, shaders, binaryformat, binary, length);
}

static PFNGLSPECIALIZESHADERPROC s_pFn_glSpecializeShader = NULL;
void APIENTRY glSpecializeShader(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue)
{
	return s_pFn_glSpecializeShader(shader, pEntryPoint, numSpecializationConstants, pConstantIndex, pConstantValue);
}

static PFNGLSHADERSOURCEPROC s_pFn_glShaderSource = NULL;
void APIENTRY glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
	return s_pFn_glShaderSource(shader, count, string, length);
}
static PFNGLCOMPILESHADERPROC s_pFn_glCompileShader = NULL;
void APIENTRY glCompileShader(GLuint shader)
{
	return s_pFn_glCompileShader(shader);
}
static PFNGLGETSHADERIVPROC s_pFn_glGetShaderiv = NULL;
void APIENTRY glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	return s_pFn_glGetShaderiv(shader, pname, params);
}

static PFNGLGETSHADERINFOLOGPROC s_pFn_glGetShaderInfoLog = NULL;
void APIENTRY glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	return s_pFn_glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

static PFNGLCREATEPROGRAMPROC s_pFn_glCreateProgram = NULL;
GLuint APIENTRY glCreateProgram(void)
{
	return s_pFn_glCreateProgram();
}

static PFNGLATTACHSHADERPROC s_pFn_glAttachShader = NULL;
void APIENTRY glAttachShader(GLuint program, GLuint shader)
{
	return s_pFn_glAttachShader(program, shader);
}

static PFNGLLINKPROGRAMPROC s_pFn_glLinkProgram = NULL;
void APIENTRY glLinkProgram(GLuint program)
{
	return s_pFn_glLinkProgram(program);
}

static PFNGLGETPROGRAMIVPROC s_pFn_glGetProgramiv = NULL;
void APIENTRY glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	return s_pFn_glGetProgramiv(program, pname, params);
}

static PFNGLGETPROGRAMINFOLOGPROC s_pFn_glGetProgramInfoLog = NULL;
void APIENTRY glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	return s_pFn_glGetProgramInfoLog(program, bufSize, length, infoLog);
}

static PFNGLUSEPROGRAMPROC s_pFn_glUseProgram = NULL;
void APIENTRY glUseProgram(GLuint program)
{
	return s_pFn_glUseProgram(program);
}

static PFNGLBINDBUFFERBASEPROC s_pFn_glBindBufferBase = NULL;
void APIENTRY glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	return s_pFn_glBindBufferBase(target, index, buffer);
}

static PFNGLGENVERTEXARRAYSPROC s_pFn_glGenVertexArrays = NULL;
void APIENTRY glGenVertexArrays(GLsizei n, GLuint* arrays)
{
	return s_pFn_glGenVertexArrays(n, arrays);
}

static PFNGLBINDVERTEXARRAYPROC s_pFn_glBindVertexArray = NULL;
void APIENTRY glBindVertexArray(GLuint array)
{
	return s_pFn_glBindVertexArray(array);
}

static PFNGLVERTEXATTRIBFORMATPROC s_pFn_glVertexAttribFormat = NULL;
void APIENTRY glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	s_pFn_glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

static PFNGLENABLEVERTEXATTRIBARRAYPROC s_pFn_glEnableVertexAttribArray = NULL;
void APIENTRY glEnableVertexAttribArray(GLuint index)
{
	return s_pFn_glEnableVertexAttribArray(index);
}

static PFNGLVERTEXATTRIBBINDINGPROC s_pFn_glVertexAttribBinding = NULL;
void APIENTRY glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
	return s_pFn_glVertexAttribBinding(attribindex, bindingindex);
}

static PFNGLBINDVERTEXBUFFERPROC s_pFn_glBindVertexBuffer = NULL;
void APIENTRY glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
	return s_pFn_glBindVertexBuffer(bindingindex, buffer, offset, stride);
}

static PFNGLPRIMITIVERESTARTINDEXPROC s_pFn_glPrimitiveRestartIndex = NULL;
void APIENTRY glPrimitiveRestartIndex(GLuint index)
{
	return s_pFn_glPrimitiveRestartIndex(index);
}

static PFNGLVIEWPORTARRAYVPROC s_pFn_glViewportArrayv = NULL;
void APIENTRY glViewportArrayv(GLuint first, GLsizei count, const GLfloat* v)
{
	return s_pFn_glViewportArrayv(first, count, v);
}

static PFNGLCLIPCONTROLPROC s_pFn_glClipControl = NULL;
void APIENTRY glClipControl(GLenum origin, GLenum depth)
{
	return s_pFn_glClipControl(origin, depth);
}

static PFNGLPOLYGONOFFSETCLAMPPROC s_pFn_glPolygonOffsetClamp = NULL;
void APIENTRY glPolygonOffsetClamp(GLfloat factor, GLfloat units, GLfloat clamp)
{
	return s_pFn_glPolygonOffsetClamp(factor, units, clamp);
}

static PFNGLCOLORMASKIPROC s_pFn_glColorMaski = NULL;
void APIENTRY glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	return s_pFn_glColorMaski(index, r, g, b, a);
}

static PFNGLCLEARBUFFERFVPROC s_pFn_glClearBufferfv = NULL;
void APIENTRY glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	return s_pFn_glClearBufferfv(buffer, drawbuffer, value);
}

static PFNGLBLITFRAMEBUFFERPROC s_pFn_glBlitFramebuffer = NULL;
void APIENTRY glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	return s_pFn_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void init_libwgl(void)
{
	s_pFn_wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
}

void init_libopengl(void)
{
	s_pFn_wglMakeContextCurrentARB = reinterpret_cast<PFNWGLMAKECONTEXTCURRENTARBPROC>(wglGetProcAddress("wglMakeContextCurrentARB"));
	s_pFn_wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));

	s_pFn_glDebugMessageCallback = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(wglGetProcAddress("glDebugMessageCallback"));
	s_pFn_glDebugMessageControl = reinterpret_cast<PFNGLDEBUGMESSAGECONTROLPROC>(wglGetProcAddress("glDebugMessageControl"));

	s_pFn_glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>(wglGetProcAddress("glGenBuffers"));
	s_pFn_glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERPROC>(wglGetProcAddress("glBindBuffer"));
	s_pFn_glBufferData = reinterpret_cast<PFNGLBUFFERDATAPROC>(wglGetProcAddress("glBufferData"));
	s_pFn_glBufferStorage = reinterpret_cast<PFNGLBUFFERSTORAGEPROC>(wglGetProcAddress("glBufferStorage"));
	s_pFn_glBufferSubData = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(wglGetProcAddress("glBufferSubData"));

	s_pFn_glActiveTexture = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(wglGetProcAddress("glActiveTexture"));
	s_pFn_glTexStorage2D = reinterpret_cast<PFNGLTEXSTORAGE2DPROC>(wglGetProcAddress("glTexStorage2D"));
	s_pFn_glTexStorage2DMultisample = reinterpret_cast<PFNGLTEXSTORAGE2DMULTISAMPLEPROC>(wglGetProcAddress("glTexStorage2DMultisample"));

	s_pFn_glGenSamplers = reinterpret_cast<PFNGLGENSAMPLERSPROC>(wglGetProcAddress("glGenSamplers"));
	s_pFn_glSamplerParameteri = reinterpret_cast<PFNGLSAMPLERPARAMETERIPROC>(wglGetProcAddress("glSamplerParameteri"));
	s_pFn_glSamplerParameterfv = reinterpret_cast<PFNGLSAMPLERPARAMETERFVPROC>(wglGetProcAddress("glSamplerParameterfv"));
	s_pFn_glBindSampler = reinterpret_cast<PFNGLBINDSAMPLERPROC>(wglGetProcAddress("glBindSampler"));

	s_pFn_glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(wglGetProcAddress("glGenFramebuffers"));
	s_pFn_glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(wglGetProcAddress("glBindFramebuffer"));
	s_pFn_glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(wglGetProcAddress("glFramebufferTexture2D"));
	s_pFn_glDrawBuffers = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(wglGetProcAddress("glDrawBuffers"));
	s_pFn_glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(wglGetProcAddress("glCheckFramebufferStatus"));

	s_pFn_glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(wglGetProcAddress("glCreateShader"));
	s_pFn_glShaderBinary = reinterpret_cast<PFNGLSHADERBINARYPROC>(wglGetProcAddress("glShaderBinary"));
	s_pFn_glSpecializeShader = reinterpret_cast<PFNGLSPECIALIZESHADERPROC>(wglGetProcAddress("glSpecializeShader"));
	s_pFn_glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(wglGetProcAddress("glShaderSource"));
	s_pFn_glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(wglGetProcAddress("glCompileShader"));
	s_pFn_glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(wglGetProcAddress("glGetShaderiv"));
	s_pFn_glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(wglGetProcAddress("glGetShaderInfoLog"));

	s_pFn_glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(wglGetProcAddress("glCreateProgram"));
	s_pFn_glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(wglGetProcAddress("glAttachShader"));
	s_pFn_glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(wglGetProcAddress("glLinkProgram"));
	s_pFn_glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(wglGetProcAddress("glGetProgramiv"));
	s_pFn_glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(wglGetProcAddress("glGetProgramInfoLog"));

	s_pFn_glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(wglGetProcAddress("glUseProgram"));

	s_pFn_glBindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(wglGetProcAddress("glBindBufferBase"));

	s_pFn_glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(wglGetProcAddress("glGenVertexArrays"));
	s_pFn_glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(wglGetProcAddress("glBindVertexArray"));
	s_pFn_glVertexAttribFormat = reinterpret_cast<PFNGLVERTEXATTRIBFORMATPROC>(wglGetProcAddress("glVertexAttribFormat"));
	s_pFn_glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(wglGetProcAddress("glEnableVertexAttribArray"));
	s_pFn_glVertexAttribBinding = reinterpret_cast<PFNGLVERTEXATTRIBBINDINGPROC>(wglGetProcAddress("glVertexAttribBinding"));
	s_pFn_glBindVertexBuffer = reinterpret_cast<PFNGLBINDVERTEXBUFFERPROC>(wglGetProcAddress("glBindVertexBuffer"));

	s_pFn_glPrimitiveRestartIndex = reinterpret_cast<PFNGLPRIMITIVERESTARTINDEXPROC>(wglGetProcAddress("glPrimitiveRestartIndex"));

	s_pFn_glClipControl = reinterpret_cast<PFNGLCLIPCONTROLPROC>(wglGetProcAddress("glClipControl"));
	s_pFn_glViewportArrayv = reinterpret_cast<PFNGLVIEWPORTARRAYVPROC>(wglGetProcAddress("glViewportArrayv"));
	s_pFn_glPolygonOffsetClamp = reinterpret_cast<PFNGLPOLYGONOFFSETCLAMPPROC>(wglGetProcAddress("glPolygonOffsetClamp"));

	s_pFn_glColorMaski = reinterpret_cast<PFNGLCOLORMASKIPROC>(wglGetProcAddress("glColorMaski"));

	s_pFn_glClearBufferfv = reinterpret_cast<PFNGLCLEARBUFFERFVPROC>(wglGetProcAddress("glClearBufferfv"));

	s_pFn_glBlitFramebuffer = reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(wglGetProcAddress("glBlitFramebuffer"));
}