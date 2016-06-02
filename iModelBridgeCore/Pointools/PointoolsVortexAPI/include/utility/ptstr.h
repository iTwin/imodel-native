#ifndef POINTOOLS_SAFE_STRING_FUNCTIONS
#define POINTOOLS_SAFE_STRING_FUNCTIONS

#include <assert.h>
#include <wchar.h>
#include <pt/unicodeconversion.h>
#include <pt/debugAssert.h>

#define PT_DESCRIPTOR_SIZE 64
#define PT_PATH_SIZE 260

#ifdef NEEDS_WORK_VORTEX_DGNDB
 #ifdef POINTOOLS_API_INCLUDE
     #ifdef BENTLEY_WIN32
         #undef debugAssertM
         #define debugAssertM(a,b) if(!a) OutputDebugStringA("assertion failed!!")
     #endif
 #endif
 #endif

struct ptstr
{
	static bool copy(char *dest, const char *src, int max_length)
	{
		int ls = len(src, max_length);
		int l = ls > max_length ? max_length-1 : ls;

		memcpy(dest, src, l);
		dest[l] = '\0';

		if (ls >= max_length)
		{
			debugAssertM(0, "string copy truncation");
			return false;
		}
		return true;
	}
	static int len(const char *src, int max_length)
	{
		int l =0;
		while (l < max_length && src[l] != '\0' ) l++;
		return l;
	}
	static bool copy(wchar_t *dest, const wchar_t *src, int max_length)
	{
		int ls = len(src, max_length);
		int l = ls > max_length ? max_length : ls;

		memcpy(dest, src, l*sizeof(wchar_t));
		dest[l] = L'\0';

		if (ls >= max_length)
		{
			debugAssertM(0, "string copy truncation");
			return false;
		}
		return true;
	}
	static int len(const wchar_t *src, int max_length)
	{
		int l =0;
		while (l < max_length && src[l] != L'\0' ) l++;
		return l+1;
	}
	static bool copy(wchar_t *dest, const char *src, int max_length)
	{
		return copy(dest, pt::Ascii2Unicode::convert(src).c_str(), max_length);
	}
	static bool copy(char *dest, const wchar_t *src, int max_length)
	{
		return copy(dest, pt::Unicode2Ascii::convert(src).c_str(), max_length);
	}
};
#endif
