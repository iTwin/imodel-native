// This is a part of the Smart PropertyGrid Library
// Copyright (C) 2001-2004 VisualHint
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __GWCLISTBOX_H
#define __GWCLISTBOX_H

#include "GWCPropertiesLink.h"
#include "GWCWrappedItemCollection.h"

class GWCPROPCTRL_LINKAGE GWCListBox : public CListBox
{
	// Construction
	public:
		GWCListBox() {}
		virtual ~GWCListBox() {}

	public:
	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(GWCListBox)
		//}}AFX_VIRTUAL

	void SetOwnerItem(GWCDeepIterator& iter);
	virtual UINT GetCustomStyles() { return 0; }

		// Generated message map functions
	protected:
	//{{AFX_MSG(GWCListBox)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	GWCDeepIterator m_iterItem;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __GWCLISTBOX_H
