// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPCTRLLINK_H
#define __GWCPROPCTRLLINK_H


#if defined _AFXDLL && !defined _GWCPROPCTRL_STATIC_
	#ifdef __GWCPROPCTRL_DLL
	   #define GWCPROPCTRL_LINKAGE  _declspec(dllexport)
	#else
	   #define GWCPROPCTRL_LINKAGE  _declspec(dllimport)
	#endif
#else
	#define GWCPROPCTRL_LINKAGE
#endif

#include <string>

#ifdef _UNICODE
#define stlstring std::wstring
#else
#define stlstring std::string
#endif

#endif // __GWCPROPCTRLLINK_H
