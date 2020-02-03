// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCSUBCATEGORYITEM_H
#define __GWCSUBCATEGORYITEM_H

#include "GWCPropertiesLink.h"
#include "GWCCategoryItem.h"

//==========================================================================
// Description:
// The class GWCSubCategoryItem has been introduced so that it's possible
// to have a hierarchy of properties. With GWCCategoryItem, only a one level
// was possible.
//
// Use:
// Call the method AddSubCategory on a GWCSubCategoryItem instance to create
// a new sub-category.
//==========================================================================
class GWCPROPCTRL_LINKAGE GWCSubCategoryItem : public GWCCategoryItem  
{
	// Construction
	public:
		GWCSubCategoryItem(int id, CString& catName, int imageIndex = -1);
		virtual ~GWCSubCategoryItem();

	DECLARE_DYNAMIC(GWCSubCategoryItem)

	// Attributes
	protected:

	// Operations
	public:
		virtual CRect GetManuallyDisableRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual CRect GetLabelRect(CRect* itemRect, GWCVisibleDeepIterator iterSelf);
		virtual void DrawItem(CDC* pDC, CRect itemRect, GWCVisibleDeepIterator iterSelf);
		virtual int GetLabelShift(GWCVisibleDeepIterator iterSelf);
};

#endif // __GWCSUBCATEGORYITEM_H
