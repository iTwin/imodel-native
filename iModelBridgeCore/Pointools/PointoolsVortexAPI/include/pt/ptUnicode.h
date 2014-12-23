#pragma once

#include <tchar.h>

#ifndef tstring
	#ifdef UNICODE
		#define tstring wstring
	#else
		#define tstring string
	#endif
#endif