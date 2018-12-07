// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYLOOK_H
#define __GWCPROPERTYLOOK_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"

class GWCPropertyValue;
class GWCCustomDrawManager;

class GWCPROPCTRL_LINKAGE GWCPropertyLook  
{
	// Construction
	public:
		GWCPropertyLook();
		virtual ~GWCPropertyLook() {}

	// Attributes
	protected:
		// The value object this look belongs to
		GWCPropertyValue* m_value;

	// Operations
	public:
		GWCCustomDrawManager* GetCustomDrawManager();
		void SetValue(GWCPropertyValue* value);
		virtual CString GetDisplayString();
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CRect GetDisplayStringRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		virtual CSize GetTooltipStringSize(CDC* pDC, CRect& valueRect, CPoint& point);
		virtual CString TransformDisplayString(CString str);
		virtual CString GetTooltipString(int line);
};

#endif // __GWCPROPERTYLOOK_H
