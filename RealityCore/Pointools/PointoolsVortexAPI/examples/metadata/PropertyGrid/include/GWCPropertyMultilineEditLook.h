// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYMULTILINEEDITLOOK_H
#define __GWCPROPERTYMULTILINEEDITLOOK_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyLook.h"

class GWCPROPCTRL_LINKAGE GWCPropertyMultilineEditLook : public GWCPropertyLook  
{
	// Construction
	public:
		GWCPropertyMultilineEditLook();
		virtual ~GWCPropertyMultilineEditLook();

	// Operations
	public:
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CRect GetDisplayStringRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		virtual CSize GetTooltipStringSize(CDC* pDC, CRect& valueRect, CPoint& point);
};

#endif // __GWCPROPERTYMULTILINEEDITLOOK_H
