// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYICONLOOK_H
#define __GWCPROPERTYICONLOOK_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyLook.h"
#include "GWCWrappedItemCollection.h"

class GWCPROPCTRL_LINKAGE GWCPropertyIconLook : public GWCPropertyLook  
{
	// Construction
	public:
		GWCPropertyIconLook();
		GWCPropertyIconLook(HBITMAP hBitmap, COLORREF transparentColor = -1, DWORD userData = 0);
		GWCPropertyIconLook(HICON hIcon, DWORD userData = 0);
		virtual ~GWCPropertyIconLook();

	// Attributes
	protected:
		HBITMAP m_hBitmap;
		HICON m_hIcon;
		DWORD m_linkedData;
		COLORREF m_transparentColor;

	// Operations
	public:
		void SetBitmap(HBITMAP hBitmap, COLORREF transparentColor = -1, DWORD userData = 0);
		void SetIcon(HICON hIcon, DWORD userData = 0);
		DWORD GetLinkedData();
		virtual void OnDraw(CDC* pDC, CRect valueRect, GWCDeepIterator& iter, bool drawString);
		virtual CRect GetDisplayStringRect(CDC* pDC, CFont* font, CRect& valueRect, CPoint& point);
		CRect GetBitmapRect(CRect& valueRect);
};

#endif // __GWCPROPERTYICONLOOK_H
