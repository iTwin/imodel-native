// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCFONTPROPERTYVALUE_H
#define __GWCFONTPROPERTYVALUE_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyValue.h"

class GWCPROPCTRL_LINKAGE GWCFontPropertyValue : public GWCPropertyValue  
{
	// Types
	public:
		static int FONT;
		static int FONTCOLOR;

	// Construction
	public:
		GWCFontPropertyValue(int id, int type, bool displayColor);
		virtual ~GWCFontPropertyValue();

	// Operations
	public:
		CFont* GetFont();
		COLORREF GetFontColor();
		void SetFontColor(COLORREF color);

		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CString GetStringValue(int* enumValue = NULL);
		virtual void ChangeDataPointer(void* data, bool managed);

	// Attributes
	protected:
		CFont* m_font;			// A pointer to the CFont object monitored by this object.
		COLORREF m_fontColor;	// Color of the font.
		bool m_displayColor;
};

#endif // __GWCFONTPROPERTYVALUE_H
