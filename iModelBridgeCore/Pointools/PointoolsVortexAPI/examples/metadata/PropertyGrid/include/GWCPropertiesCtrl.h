// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions
// of the accompanying license agreement.

#ifndef __GWCPROPERTIESCTRL_H
#define __GWCPROPERTIESCTRL_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyItem.h"
#include "GWCCategoryItem.h"
#include "GWCPropMsgs.h"
#include "GWCPropertiesFactoryBase.h"
#include "GWCPropertyFeel.h"
#include "GWCPropertyValue.h"
#include "GWCRGBPropertyValue.h"
#include "GWCFontPropertyValue.h"
#include "GWCDateTimePropertyValue.h"
#include "GWCWrappedItemCollection.h"
#include <vector>
#include <set>
#include <map>

// Use this as the classname when inserting this control as a custom control
// in the MSVC++ dialog editor.
#define GWCPROPERTIESCTRL_CLASSNAME    _T("PropertiesCtrl")

// Constants used in calls to GWCPropertiesCtrl::GetRegisteredFeel()
// to acquire a GWCPropertyFeel object.
#define GWCFEEL_EDIT			_T("edit")
#define GWCFEEL_EDITSPIN		_T("editspin")
#define GWCFEEL_EDITPASSWORD	_T("editpwd")
#define GWCFEEL_LIST			_T("list")
#define GWCFEEL_EDITLIST		_T("editlist")
#define GWCFEEL_CUSTOMLIST		_T("customlist")
#define GWCFEEL_BUTTON			_T("button")
#define GWCFEEL_EDITBUTTON		_T("editbutton")
#define GWCFEEL_POPUP			_T("popup")
#define GWCFEEL_EDITPOPUP		_T("editpopup")
#define GWCFEEL_FONTBUTTON		_T("fontbutton")
#define GWCFEEL_DATETIME		_T("datetime")
#define GWCFEEL_HOTKEY			_T("hotkey")
#define GWCFEEL_CHECKBOX		_T("checkbox")
#define GWCFEEL_CHECKBOXMULTI	_T("multicheckbox")
#define GWCFEEL_RADIOBUTTON		_T("radiobutton")
#define GWCFEEL_SLIDER			_T("slider")
#define GWCFEEL_SLIDEREDIT		_T("slideredit")
#define GWCFEEL_EDITMULTILINE	_T("multilineedit")

class GWCToolTipCtrl;
class GWCListBox;
class GWCCustomDrawManager;
class GWCInternalGrid;

class GWCPROPCTRL_LINKAGE GWCPropertiesCtrl : public CWnd
{
	// Constants
	public:
		static int DISPLAYMODE_CATEGORIZED;
		static int DISPLAYMODE_FLAT;
		static int DISPLAYMODE_FLATSORTED;

		static int MULTISELECTMODE_NONE;
		static int MULTISELECTMODE_SAMELEVEL;
		static int MULTISELECTMODE_GLOBAL;

	// Construction
	public:
		GWCPropertiesCtrl();
		virtual ~GWCPropertiesCtrl();
		BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// Attributes
	protected:
		// The main grid of the window
		GWCInternalGrid* m_grid;

		// Set to true if the method Create is called
		bool m_initByCreate;

		// Stores the built-in factory as well as the user own factories.
		static std::set<GWCPropertiesFactoryBase*> m_propFactories;

		// Stores all feels (each feel is a singleton).
		std::map<CString,GWCPropertyFeel*> m_registeredFeels;

		// Bitmap in the top area used to switch to categorized mode.
		CBitmap m_catBitmap;

		// Bitmap in the top area used to switch to sorted mode.
		CBitmap m_sortBitmap;

		// True if the comment box is to be shown, false otherwise.
		bool m_showComments;

		// Current height in pixels of the comment box.
		int m_commentsHeight;

		// True if the area with buttons is to be shown at the top of the control,
		// false otherwise.
		bool m_showButtons;

		// Current feel used when the client calls the method AddProperty.
		GWCPropertyFeel* m_defaultFeel;

		// True if the user is currently resizing the comment box, false otherwise.
		bool m_resizingCommentsInProgress;

		// True if the mouse cursor is currently above the categorized mode button,
		// false otherwise.
		bool m_mouseOverCategoryButton;

		// True if the mouse cursor is currently above the sorted mode button,
		// false otherwise.
		bool m_mouseOverSortButton;

		// Window ID used for the singleton feels. It is incremented each time a
		// feel is registered.
		UINT m_feelId;

	public:
		static int m_editboxLeftMargin;

	// Operations
	public:
		GWCDeepIterator AddRootCategory(int id, CString catName, int imageIndex = -1);
		GWCDeepIterator AddPropertyUnderCategory(GWCDeepIterator& underParentIter, int id, CString propName, int type, void* data = NULL, LPCTSTR comment = NULL);
		GWCDeepIterator AddSubCategoryUnderCategory(GWCDeepIterator& underParentIter, int id, CString catName, int imageIndex = -1);
		GWCDeepIterator AddHotLinkUnderCategory(GWCDeepIterator& underParentIter, int id, CString propName, LPCTSTR comment = NULL);
		GWCDeepIterator InsertRootCategoryBefore(GWCDeepIterator& beforeRootCategoryIter, int id, CString catName, int imageIndex = -1);
		GWCDeepIterator InsertSubCategoryBeforeProperty(GWCDeepIterator& beforePropertyIter, int id, CString catName, int imageIndex = -1);
		GWCDeepIterator InsertPropertyBeforeProperty(GWCDeepIterator& beforePropertyIter, int id, CString propName, int type, void* data = NULL, LPCTSTR comment = NULL);
		GWCDeepIterator InsertHotLinkBeforeProperty(GWCDeepIterator& beforePropertyIter, int id, CString propName, LPCTSTR comment = NULL);
		void SwapSiblingProperties(GWCDeepIterator& iter1, GWCDeepIterator& iter2);

		int GetVisibleItemCount();
		int GetVisibleLineCount();
		int GetDisplayedItemCount();
		int GetCategoriesCount();
		int GetPropertyChildrenCount(GWCDeepIterator& iter);

		GWCDeepIterator GetFirstItem();
		GWCDeepIterator GetLastItem();
		GWCVisibleDeepIterator GetFirstDisplayedItem();
		GWCVisibleDeepIterator GetFirstVisibleItem();
		GWCVisibleDeepIterator GetLastVisibleItem();
		GWCDeepIterator FindItem(int id);
		GWCDeepIterator dbegin();
		GWCDeepIterator dend();
		GWCVisibleDeepIterator vbegin();
		GWCVisibleDeepIterator vend();
		GWCSiblingIterator sbegin();
		GWCSiblingIterator send();
		GWCVisibleSiblingIterator svbegin();
		GWCVisibleSiblingIterator svend();

		CRect GetItemRect(GWCDeepIterator& iter);
		int GetItemHeight();
		int GetSecondColumnWidth();
		void SetSecondColumnWidth(int width);
		int GetLeftBorder();
		void ShowLeftBorder(bool show = true);
		int GetCommentsHeight();
		int SetCommentsHeight(int height);

		void ShowComments(bool show = true);
		bool DoesShowComments();
		void ShowButtons(bool show = true);
		bool DoesShowButtons();

		void SetDefaultFeel(GWCPropertyFeel* defaultFeel);
		GWCPropertyFeel* GetDefaultFeel();
		GWCPropertyFeel* GetRegisteredFeel(CString feelName);

		GWCPropertyValue* CreatePropertyValue(int id, int type, void* data);

		void SelectIterItem(GWCVisibleDeepIterator iter);
		GWCVisibleDeepIterator GetSelectedIterItem();

		bool DoesShowCategoryLine();
		void ShowCategoryLine(bool show = true);

		GWCVisibleDeepIterator PropertyItemFromPoint(CPoint& point, CRect& itemRect);

		void Clear();

		void DeleteProperty(GWCDeepIterator& iter);

		CFont* GetCtrlFont();
		void SetCtrlFont(HFONT hFont);
		COLORREF GetGridColor();
		void SetRefColor(COLORREF color, bool force = false);
		COLORREF GetRefColor();
		void SetSelectedBkgColor(COLORREF color);
		COLORREF GetSelectedBkgColor();
		void SetBkgColor(COLORREF color);
		COLORREF GetBkgColor();

		void RegisterFactory(GWCPropertiesFactoryBase* factory);
		void UnregisterFactory(GWCPropertiesFactoryBase* factory);
		void RegisterFeel(LPCTSTR feelName, GWCPropertyFeel* feel);

		virtual void OnPropertyExpanded(GWCDeepIterator iter, bool expanded);
		virtual void OnEnableItem(GWCDeepIterator iter, bool enable, bool direct);
		virtual void OnPropertyChanged(GWCDeepIterator iter);
		virtual void OnPropertyBeginEdit(GWCDeepIterator iter);
		virtual bool OnPropertyButtonClicked(GWCDeepIterator iter);
		virtual void OnPropertyValueBkgClicked(GWCDeepIterator iter);
		virtual void OnDisplayModeChanged();
		virtual void OnSelectionChanged(GWCDeepIterator iter);
		virtual void OnShowInPlaceCtrl(CWnd* pWnd, GWCDeepIterator iter);
		virtual void OnHotLinkClicked(GWCDeepIterator iter);
		virtual GWCListBox* OnRequestListbox(GWCDeepIterator iter);
		virtual void OnPropertyUpDown(GWCDeepIterator iter, int delta);

		void SetDisplayMode(int mode);
		int GetDisplayMode();

		void AllowToolTips(bool allow = true);

		void SetUserData(GWCDeepIterator& iter, DWORD data);
		DWORD GetUserData(GWCDeepIterator& iter);

		void SetImageList(CImageList* imageList);
		bool DrawImage(int imageIndex, CDC* pDC, int x, int y);

		bool EnsureVisible(GWCVisibleDeepIterator& iter);

		void LockRefresh();
		void UnlockRefresh();

		void ExpandCategory(GWCWrappedItemCollection::deepIterator iter, bool expand);
		void ExpandProperty(GWCDeepIterator iter, bool expand);

		bool IsDescendantOf(GWCDeepIterator& iterDescendantTested, GWCDeepIterator& iterAncestor);

		void EnableItem(GWCDeepIterator& iter, bool enable = true);
		bool IsItemEnabled(GWCDeepIterator& iter);

		GWCDeepIterator GetParentItem(GWCDeepIterator& iter);

		void ShowItem(GWCDeepIterator iter, bool show = true);

		void SetCustomDrawManager(GWCCustomDrawManager* customDrawManager);
		GWCCustomDrawManager* GetCustomDrawManager();

		void AdjustLabelColumn();

		virtual CRect GetButtonsRect();
		virtual CRect GetGridRect();
		virtual CRect GetCommentsRect();

		CWnd* GetGridWindow();

		void NotifySetStringValue(GWCDeepIterator iter);

		virtual bool WantToProcessKey(DWORD key);

		void SetMultiSelectionMode(int mode);
		int GetMultiSelectionMode();

		void GetMultiSelectedProperties(std::vector<GWCVisibleDeepIterator>& items);

	protected:
		GWCWrappedItemCollection::deepIterator AddPropertyUnder(GWCDeepIterator& underParentIter, GWCPropertyItem* pItem);
		GWCWrappedItemCollection::deepIterator AddPropertyBefore(GWCDeepIterator& beforeSiblingIter, GWCPropertyItem* pItem);

		BOOL RegisterWindowClass();
		virtual void Initialize();

		void DrawButtonsArea(CDC* pDC, CRect rect);
		void DrawCommentsArea(CDC* pDC, CRect commentRect);

		int MeasureWidestLabel();

		UINT HitTest(CPoint& point);

		bool HasOneVisibleChild(GWCDeepIterator iter);

		virtual bool OnValidateMultiSelection(GWCVisibleDeepIterator mainSelectedIter, GWCVisibleDeepIterator selectIter);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(GWCPropertiesCtrl)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

		// Generated message map functions
	protected:
	//{{AFX_MSG(GWCPropertiesCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnRequestListbox(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHotLinkClicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	friend GWCCategoryItem;
	friend GWCSubCategoryItem;
	friend GWCPropertyItem;
	friend GWCPropertyValue;
	friend GWCInternalGrid;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __GWCPROPERTIESCTRL_H
