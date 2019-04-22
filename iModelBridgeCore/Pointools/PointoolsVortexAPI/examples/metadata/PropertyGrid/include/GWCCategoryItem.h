// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCCATEGORYITEM_H
#define __GWCCATEGORYITEM_H

#include "GWCPropertiesLink.h"
#include "GWCPropertyItem.h"

class GWCSubCategoryItem;

//================================================================================
// Description:
// GWCCategoryItem is used to represent a root category. A category is a property
// without value, under which you can add properties and sub-categories. A text
// can be displayed on the right but it is read-only and doesn't accept an inplace
// control.
// The proper way to create a new category is to use the AddRootCategory method
// of the GWCPropertiesCtrl class.
//================================================================================
class GWCPROPCTRL_LINKAGE GWCCategoryItem : public GWCPropertyItem  
{
	// Construction
	public:
		GWCCategoryItem(int id, CString& catName, int imageIndex = -1);
		virtual ~GWCCategoryItem();

	DECLARE_DYNAMIC(GWCCategoryItem)

	// Attributes
	protected:
		int m_imageIndex;		// An index to an image of an imageList contained in the parent properties control, -1 if none
		stlstring m_valueText;	// A static text that can be displayed ine the values column

	// Operations
	public:
		void SetValueText(LPCTSTR text);
		stlstring GetValueText();

		void SetImageIndex(int imageIndex);

		// Virtual methods
		virtual CRect GetLabelRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual CRect GetManuallyDisableRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual void DrawItem(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator iterSelf);
};

#endif // __GWCCATEGORYITEM_H
