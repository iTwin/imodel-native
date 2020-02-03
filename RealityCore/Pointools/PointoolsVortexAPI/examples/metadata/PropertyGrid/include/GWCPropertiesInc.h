// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPCTRLINC_H
#define __GWCPROPCTRLINC_H

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

#pragma warning(disable:4786)

#include "GWCPropertiesLink.h"

#ifdef _GWCPROPCTRL_IN_OTHER_DLL
GWCPROPCTRL_LINKAGE void GWCPropCtrlDllInitialize ();
#endif // _GWCPROPCTRL_IN_OTHER_DLL

#if defined _AFXDLL && !defined _GWCPROPCTRL_STATIC_
	//----------------------------------------------
	// MFC shared DLL, GWCPropertiesCtrl shared DLL
	//----------------------------------------------
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropUD.lib")
			#pragma message("Automatically linking with GWCPropUD.dll")
		#else
			#pragma comment(lib,"GWCPropD.lib")
			#pragma message("Automatically linking with GWCPropD.dll")
		#endif
	#else
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropU.lib")
			#pragma message("Automatically linking with GWCPropU.dll")
		#else
			#pragma comment(lib,"GWCProp.lib")
			#pragma message("Automatically linking with GWCProp.dll")
		#endif
	#endif
#elif defined _GWCPROPCTRL_STATIC_
	//--------------------------------------------------
	// MFC shared DLL, GWCPropertiesCtrl static library
	//--------------------------------------------------
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropStUDS.lib")
			#pragma message("Automatically linking with GWCPropStUDS.lib")
		#else
			#pragma comment(lib,"GWCPropStDS.lib")
			#pragma message("Automatically linking with GWCPropStDS.lib")
		#endif
	#else
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropStUS.lib")
			#pragma message("Automatically linking with GWCPropStUS.lib")
		#else
			#pragma comment(lib,"GWCPropStS.lib")
			#pragma message("Automatically linking with GWCPropStS.lib")
		#endif
	#endif
#else
	//------------------------------------------------------
	// MFC static library, GWCPropertiesCtrl static library
	//------------------------------------------------------
	#ifdef _DEBUG
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropStUD.lib")
			#pragma message("Automatically linking with GWCPropStUD.lib")
		#else
			#pragma comment(lib,"GWCPropStD.lib")
			#pragma message("Automatically linking with GWCPropStD.lib")
		#endif
	#else
		#ifdef _UNICODE
			#pragma comment(lib,"GWCPropStU.lib")
			#pragma message("Automatically linking with GWCPropStU.lib")
		#else
			#pragma comment(lib,"GWCPropSt.lib")
			#pragma message("Automatically linking with GWCPropSt.lib")
		#endif
	#endif
#endif

	#include "GWCPropertiesCtrl.h"
	#include "GWCPropertyItem.h"
	#include "GWCCategoryItem.h"
	#include "GWCSubCategoryItem.h"
	#include "GWCPropMsgs.h"
	#include "GWCPropertiesFactoryBase.h"
	#include "GWCPropertyFeel.h"
	#include "GWCPropertyValue.h"
	#include "GWCRGBPropertyValue.h"
	#include "GWCFontPropertyValue.h"
	#include "GWCDateTimePropertyValue.h"
	#include "GWCHotKeyPropertyValue.h"
	#include "GWCPropertyLook.h"
	#include "GWCPropertyDataColorLook.h"
	#include "GWCPropertyPasswordLook.h"
	#include "GWCPropertyCheckboxLook.h"
	#include "GWCPropertyRadioButtonLook.h"
	#include "GWCPropertyMultilineEditLook.h"
	#include "GWCPropertyMultiCheckboxLook.h"
	#include "GWCPropertyIconLook.h"
	#include "GWCListBox.h"
	#include "GWCCustomDrawManager.h"
	#include "GWCDotNetDrawManager.h"
	#include "GWCLightColorDrawManager.h"
	#include "GWCDefaultDrawManager.h"
	#include "GWCWrappedItemCollection.h"
	#include "GWCXPTheme.h"

#endif // __GWCPROPCTRLINC_H
