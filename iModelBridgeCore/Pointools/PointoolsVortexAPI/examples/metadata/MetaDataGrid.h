//----------------------------------------------------------------------------
//
// MetaDataGrid.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#pragma once

#include "GWCPropertiesInc.h"
#include "../include/PointoolsVortexAPI_import.h"

class CMetaDataGrid : public GWCPropertiesCtrl
{
// Construction
public:
	CMetaDataGrid();

// Attributes
public:
	int m_id;
	HBITMAP m_hStatusBmp;

// Operations
public:
	void Populate( PThandle h );
	
	void OnPropertyChanged(GWCDeepIterator iter);

	PThandle	m_metaHandle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaDataGrid)
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
	void OnEnableItem(GWCDeepIterator iter, bool enable, bool direct);
public:
	virtual ~CMetaDataGrid();

	// Generated message map functions
protected:

	//{{AFX_MSG(CMetaDataGrid)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.