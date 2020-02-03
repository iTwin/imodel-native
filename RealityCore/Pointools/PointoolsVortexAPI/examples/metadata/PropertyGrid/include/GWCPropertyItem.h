// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYITEM_H
#define __GWCPROPERTYITEM_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"

class GWCPropertyFeel;
class GWCCategoryItem;
class GWCPropertyValue;
class GWCPropertiesCtrl;

// Description:
// 
class GWCPROPCTRL_LINKAGE GWCPropertyItem : public CObject
{
	// Construction
	public:
		GWCPropertyItem(int id);
		virtual ~GWCPropertyItem();

		void AddRef();
		void Release();

	DECLARE_DYNAMIC(GWCPropertyItem)

	// Attributes
	protected:
		// Any user data that can be attached to a unique property or category. Use SetUserData and
		// GetUserData to access it.
		DWORD m_userData;

		// True if this property is enabled, false otherwise.
		bool m_enabled;

		// False when the property is not displayed although its ancestors are expanded, true otherwise.
		// Note that it has nothing to do with the visiblity related to vertical scrolling.
		bool m_visible;

		// Displayed label in the label column of the properties control.
		CString m_name;

		// String displayed in the comments area of the properties control when this property is selected.
		CString m_comment;

		// Unique identifier for a property or category.
		int m_id;

		// Parent properties control.
		GWCPropertiesCtrl* m_propCtrl;

		// How this property should behave (button, list, ...).
		GWCPropertyFeel* m_feel;

		// True to display a checkbox near the label, used to manually enable/disable a property.
		bool m_canBeDisabledManually;

		// Pointer to a user variable representing the state of the checkbox (enabled or disabled).
		bool* m_enabledVariable;

		// Variable that hold the "enable" state of the item if NULL is passed to SetManuallyDisabled()
		bool m_managedData;

		// True if this item is selected, false otherwise.
		bool m_selected;

		// A pointer to an inplace control if the in-place control is running on this item, NULL otherwise.
		CWnd* m_inPlaceCtrlInAction;

		// Pointer to the value stored by this item.
		GWCPropertyValue* m_value;

		// Color used for the text if the user wants to override the system one (for unselected state)
		COLORREF m_textColor;

		// A reference counter used to propertly delete this instance when it is not used anymore
		int m_refCounter;

		// Stores the level in the properties hierarchy (0 = root category)
		int m_treeLevel;

		// Multiplier for the height of this item
		int m_heightMultiplier;

		// True if the property is expanded, false otherwise.
		bool m_expanded;

	// Operations
	public:
		GWCPropertiesCtrl* GetPropCtrl();
		void SetManuallyDisabled(bool b, bool* enabledVariable);
		bool CanBeManuallyDisabled();
		bool GetManuallyDisabledVariable();
		void Select(bool selected = true);
		bool IsSelected();
		int GetID();
		bool IsEnabled();
		CRect GetValueRect(CRect itemRect);
		void SetParentCtrl(GWCPropertiesCtrl* propCtrl);
		void SetName(CString& name);
		CString GetName();
		void SetComment(LPCTSTR comment);
		CString& GetComment();
		void SetFeel(GWCPropertyFeel* feel);
		void ShowItem(bool show);
		bool IsVisible();
		GWCPropertyValue* GetValue();
		GWCPropertyFeel* GetFeel();
		void SetValue(GWCPropertyValue* value);
		void SetInPlaceCtrlInAction(CWnd* pInPlaceCtrl);
		CWnd* GetInPlaceCtrlInAction();
		void GWCPropertyItem::SetUserData(DWORD data);
		DWORD GWCPropertyItem::GetUserData();
		int GetType();
		void SetTextColor(COLORREF color);
		COLORREF GetTextColor();
		void Enable(GWCDeepIterator& iter, bool enable, bool ancestorDisabled, bool direct = true);
		void SetTreeLevel(int level);
		int GetTreeLevel();
		int GetHeightMultiplier();
		void SetHeightMultiplier(int heightMultiplier);
		void UpdateValueFromChildren(GWCDeepIterator iter);
		bool IsExpanded();
		void Expand(bool expand);
		CRect GetSignRect(CRect itemRect);

		virtual CRect GetLabelRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual int GetLabelShift(GWCVisibleDeepIterator iterSelf);
		virtual CRect GetManuallyDisableRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual void DrawItem(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator iterSelf);
		virtual BOOL OnSetCursor(CPoint& point, CRect& itemRect, GWCVisibleDeepIterator iterSelf);
		virtual void OnPropertyClicked(GWCVisibleDeepIterator& iterSelf, CPoint& point, CRect& itemRect);

	friend GWCInternalGrid;
};

#endif // __GWCPROPERTYITEM_H
