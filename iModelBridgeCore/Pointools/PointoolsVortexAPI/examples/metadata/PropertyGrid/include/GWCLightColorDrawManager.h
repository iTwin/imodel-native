// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions
// of the accompanying license agreement.

#ifndef __GWCLIGHTCOLORDRAWMANAGER_H
#define __GWCLIGHTCOLORDRAWMANAGER_H

#include "GWCPropertiesLink.h"
#include "GWCDefaultDrawManager.h"

class GWCPROPCTRL_LINKAGE GWCLightColorDrawManager : public GWCDefaultDrawManager  
{
	// Construction
	public:
		GWCLightColorDrawManager();
		virtual ~GWCLightColorDrawManager();

	// Attributes
	protected:
		bool m_useBoldFontForCategories;
		COLORREF m_bkgColorCategory1;
		COLORREF m_bkgColorCategory2;
		COLORREF m_bkgColorSubCategory1;
		COLORREF m_bkgColorSubCategory2;

	// Operations
	public:
		virtual void DrawLeftColumnBackground(CDC* pDC, CRect* rect);
		virtual void DrawCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
		virtual void DrawSubCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter);
		virtual bool AllowCategoryVerticalBar();
		virtual void DrawPlusMinusSign(CDC* pDC, CRect& itemRect, GWCVisibleDeepIterator& iter);
		virtual void DrawCategoryLabelText(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);
		virtual void DrawCategoryValue(CDC* pDC, CRect& valueRect, GWCVisibleDeepIterator& iter);
		virtual CRect NcCalcSize(CRect windowRect);
		virtual void DrawGridNonClientArea(CDC* pDC, CRect* gridRect);
		virtual void DrawCommentsBackground(CDC* pDC, CRect* commentRect);
		virtual void DrawCommentsGap(CDC* pDC, CRect rect);
		virtual void DrawButtonsBackground(CDC* pDC, CRect buttonsRect);
		virtual void DrawButtonsGap(CDC* pDC, CRect buttonsRect);
		virtual void DrawSeparationLines(CDC* pDC, CRect& itemRect, CRect& labelRect, GWCVisibleDeepIterator& iterSelf);
		virtual void DrawPropertyLabelBackground(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);

		void SetCategoryBkgColor(COLORREF color1, COLORREF color2 = RGB(255,255,255));
		void SetSubCategoryBkgColor(COLORREF color1, COLORREF color2 = RGB(255,255,255));

	protected:
		void FillGradient(CDC* pDC, CRect& rect, COLORREF colorStart, COLORREF colorFinish);
};

#endif // __GWCLIGHTCOLORDRAWMANAGER_H
