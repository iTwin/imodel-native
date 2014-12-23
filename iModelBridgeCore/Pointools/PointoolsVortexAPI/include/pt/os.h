#ifndef POINTOOLS_OS_HEADERS
#define POINTOOLS_OS_HEADERS 1

#ifdef WIN32
	#ifndef _AFXDLL

		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN 1
		#endif

		#include <windows.h>
	#else
		#include "stdafx.h"
	#endif

	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif

#endif

#endif