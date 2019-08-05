// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYDATACOLORLOOK_H
#define __GWCPROPERTYDATACOLORLOOK_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyLook.h"

class GWCPROPCTRL_LINKAGE GWCPropertyDataColorLook : public GWCPropertyLook  
{
	// Cosntruction
	public:
		GWCPropertyDataColorLook(bool htmlFormat = false);
		virtual ~GWCPropertyDataColorLook();

	// Attributes
	protected:
		bool m_htmlFormat;

	// Operations
	public:
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CRect GetDisplayStringRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		virtual CString GetDisplayString();
		virtual CString TransformDisplayString(CString str);

	protected:
		CRect GetColorRect(CRect& valueRect);
};

#endif // __GWCPROPERTYHTMLCOLORLOOK_H
