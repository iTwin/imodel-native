// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions
// of the accompanying license agreement.

#ifndef __GWCDOTNETDRAWMANAGER_H
#define __GWCDOTNETDRAWMANAGER_H

#include "GWCPropertiesLink.h"
#include "GWCCustomDrawManager.h"

class GWCPROPCTRL_LINKAGE GWCDotNetDrawManager : public GWCCustomDrawManager
{
	// Construction
	public:
		GWCDotNetDrawManager();
		virtual ~GWCDotNetDrawManager();

	// Attributes
	protected:
		bool m_drawSubCatBkg;

	// Operations
	public:
		void SetDrawSubCategoryBackground(bool draw);

		virtual void SetPropCtrl(GWCPropertiesCtrl* pPropCtrl);
		virtual void DrawGridBackground(CDC* pDC, CRect* gridRect);
		virtual void DrawGridNonClientArea(CDC* pDC, CRect* gridRect);
		virtual void DrawLeftColumnBackground(CDC* pDC, CRect* rect);
		virtual void DrawCommentsBackground(CDC* pDC, CRect* commentRect);
		virtual void DrawCommentText(CDC* pDC, CRect rect, GWCVisibleDeepIterator& iter);
		virtual void DrawSubCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
		virtual CRect GetClientGridRect(CRect& gridRect);
		virtual void DrawCommentsGap(CDC* pDC, CRect rect);
		virtual CRect NcCalcSize(CRect windowRect);
		virtual bool AllowCategoryVerticalBar();
		virtual void DrawButtonsBackground(CDC* pDC, CRect buttonsRect);
		virtual void DrawCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
};

#endif // __GWCDOTNETDRAWMANAGER_H
