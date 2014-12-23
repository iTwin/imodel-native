// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCRGBPROPERTYVALUE_H
#define __GWCRGBPROPERTYVALUE_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyValue.h"

class GWCPROPCTRL_LINKAGE GWCRGBPropertyValue : public GWCPropertyValue  
{
	// Types
	public:
		static int COLORRGB;

	// Construction
	public:
		GWCRGBPropertyValue(int id, int type);
		virtual ~GWCRGBPropertyValue();

	// Attributes
	protected:
		// A pointer to the color data monitored by this object.
		COLORREF* m_color;

	// Operations
	public:
		COLORREF GetColor();
		void SetColor(COLORREF color);
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CString GetStringValue(int* enumValue = NULL);
		virtual void ChangeDataPointer(void* data, bool managed);
		virtual CRect GetStringValueRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		virtual void _SetStringValue(CString value);
};

#endif // __GWCRGBPROPERTYVALUE_H
