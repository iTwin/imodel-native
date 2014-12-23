// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCCUSTOMDRAWMANAGER_H
#define __GWCCUSTOMDRAWMANAGER_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"
#include "GWCXPTheme.h"

class GWCPropertiesCtrl;
class GWCPropertyItem;

#define TREEGLYPHWIDTH 9

//==================================================================================
// Description:
// This is the class you have to derive from if you want to customize the appearance
// of the properties control.
// Then call :
//               myPropCtrl->SetCustomDrawManager(new MytDrawManager());
//==================================================================================
class GWCPROPCTRL_LINKAGE GWCCustomDrawManager  
{
	// Construction
	public:
		GWCCustomDrawManager() {}
		virtual ~GWCCustomDrawManager() {}

	// Attributes
	protected:
		GWCPropertiesCtrl* m_pPropCtrl;		// A pointer to the properties control in which drawings are done.
		GWCXPTheme m_xpDrawer;

	// Operations
	public:
		void InitializeXPTheme(HWND hWnd);
		bool IsAppThemed();

		virtual void SetPropCtrl(GWCPropertiesCtrl* pPropCtrl);
		virtual COLORREF GetBkgColor();
		virtual void DrawGridBackground(CDC* pDC, CRect* gridRect) {}
		virtual void DrawGridNonClientArea(CDC* pDC, CRect* gridRect) {}
		virtual void DrawLeftColumnBackground(CDC* pDC, CRect* rect) {}

		virtual void DrawCommentsBackground(CDC* pDC, CRect* commentRect) {}
		virtual void DrawCommentsGap(CDC* pDC, CRect commentRect) {}
		virtual void DrawCommentText(CDC* pDC, CRect rect, GWCVisibleDeepIterator& iter);

		virtual void DrawCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter) {} 
		virtual void DrawCategoryLabelText(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);
		virtual void DrawCategoryValue(CDC* pDC, CRect& valueRect, GWCVisibleDeepIterator& iter);

		virtual void DrawSubCategoryLabelBackground(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator& iter) {} 
		virtual void DrawSubCategoryLabelText(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);
		virtual void DrawSubCategoryValue(CDC* pDC, CRect& valueRect, GWCVisibleDeepIterator& iter);

		virtual void DrawPropertyLabelText(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);
		virtual void DrawPropertyLabelBackground(CDC* pDC, CRect& labelRect, GWCVisibleDeepIterator& iter);

		virtual void DrawPlusMinusSign(CDC* pDC, CRect& itemRect, GWCVisibleDeepIterator& iter);
		virtual void DrawSeparationLines(CDC* pDC, CRect& itemRect, CRect& labelRect, GWCVisibleDeepIterator& iterSelf);
		virtual void DrawCheckBox(CDC* pDC, CRect& frameRect, bool checked ,bool enabled);
		virtual void DrawRadioButton(CDC* pDC, CRect& buttonRect, bool checked ,bool enabled);
		virtual void DrawButton(CDC* pDC, CRect& buttonRect, bool pushed);
		virtual void DrawDropDownArrow(CDC* pDC, CRect& buttonRect, bool pushed);
		virtual CRect NcCalcSize(CRect windowRect);
		virtual CRect GetGridRect();
		virtual CRect GetClientGridRect(CRect& gridRect);
		virtual bool AllowCategoryVerticalBar();
		virtual void DrawButtonsBackground(CDC* pDC, CRect buttonsRect) {}
		virtual void DrawButtonsGap(CDC* pDC, CRect buttonsRect) {}
		virtual TCHAR GetPasswordChar();
};

#endif // __GWCCUSTOMDRAWMANAGER_H
