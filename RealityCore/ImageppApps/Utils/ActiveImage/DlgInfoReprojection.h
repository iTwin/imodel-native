/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/DlgInfoReprojection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "afxwin.h"

class DlgInfoReprojection : public CDialog
{
	DECLARE_DYNAMIC(DlgInfoReprojection)

public:
	DlgInfoReprojection(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgInfoReprojection();

// Dialog Data
	enum { IDD = IDD_DLG_INFO_REPROJECTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL DestroyWindow();
    virtual BOOL OnInitDialog();
    virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
};
