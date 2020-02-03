// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCPROPERTYFEEL_H
#define __GWCPROPERTYFEEL_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"

class GWCPropertyItem;
class GWCPropertiesCtrl;

class GWCPROPCTRL_LINKAGE GWCPropertyFeel : public CObject
{
	// Construction
	public:
		GWCPropertyFeel(GWCPropertiesCtrl* pWnd);
		virtual ~GWCPropertyFeel();

	// Attributes
	protected:
		// Pointer to the in-place control used by this feel
		CWnd* m_inPlaceCtrl;

		// Windows child identifier for the in-place control (in the properties control parent)
		UINT m_childId;

		// Pointer to the parent window
		CWnd* m_parentWnd;

		// Pointer to the parent properties control
		GWCPropertiesCtrl* m_pPropCtrl;

		// Identifier used by the properties control as a key to register this feel
		stlstring m_name;

	// Operations
	public:
		void SetChildId(UINT id);
		virtual CWnd* ShowControl(CRect valueRect, GWCDeepIterator& iter);
		virtual void MoveControl(CRect valueRect, GWCDeepIterator& iter);
		void SetName(LPCTSTR name);
		stlstring GetName();
		void UpdateFont();
};

#endif // __GWCPROPERTYFEEL_H
