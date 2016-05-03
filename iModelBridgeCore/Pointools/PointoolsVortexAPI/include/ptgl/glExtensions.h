#ifndef GL_EXTENSIONS_HELPER_HEADER
#define GL_EXTENSIONS_HELPER_HEADER

#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include "ptgl.h"

namespace ptgl
{
struct PTGL_API Ext
{
	static bool load_extensions();
	static bool isSupported(const char *extension);
	static PROC getEntryPoint(const char*extension, const char *function);
};
}
#endif