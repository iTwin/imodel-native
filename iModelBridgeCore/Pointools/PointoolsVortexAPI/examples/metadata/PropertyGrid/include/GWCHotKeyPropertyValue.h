// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCHOTKEYPROPERTYVALUE_H
#define __GWCHOTKEYPROPERTYVALUE_H


#include "GWCPropertiesLink.h"
#include "GWCPropertyValue.h"

class GWCPROPCTRL_LINKAGE GWCHotKeyPropertyValue : public GWCPropertyValue  
{
	// Types
	public:
		static int HOTKEY;

	// Construction
	public:
		GWCHotKeyPropertyValue(int id, int type);
		virtual ~GWCHotKeyPropertyValue();

	// Attributes
	protected:
		// A pointer to the hotkey data monitored by this object.
		DWORD* m_hotKey;

	// Operations
	public:
		DWORD* GetHotKey();

		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CString GetStringValue(int* enumValue = NULL);
		virtual void ChangeDataPointer(void* data, bool managed);
};

#endif // __GWCHOTKEYPROPERTYVALUE_H
