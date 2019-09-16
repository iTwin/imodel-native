// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCDEFAULTDRAWMANAGER_H
#define __GWCDEFAULTDRAWMANAGER_H

#include "GWCPropertiesLink.h"
#include "GWCCustomDrawManager.h"

class GWCPROPCTRL_LINKAGE GWCDefaultDrawManager : public GWCCustomDrawManager  
{
	// Construction
	public:
		GWCDefaultDrawManager();
		virtual ~GWCDefaultDrawManager();

	// Attributes
	protected:

	// Operations
	public:
		virtual void DrawGridBackground(CDC* pDC, CRect* gridRect);
		virtual void DrawLeftColumnBackground(CDC* pDC, CRect* rect);
		virtual void DrawCommentsBackground(CDC* pDC, CRect* commentRect);
		virtual void DrawCommentsGap(CDC* pDC, CRect rect);
		virtual void DrawButtonsBackground(CDC* pDC, CRect buttonsRect);
		virtual void DrawButtonsGap(CDC* pDC, CRect buttonsRect);
		virtual void DrawCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
		virtual void DrawSubCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
};

#endif // __GWCDEFAULTDRAWMANAGER_H
