/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ReprojectDialog.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include "afxwin.h"


class ReprojectDialog : public CDialog
{
	DECLARE_DYNAMIC(ReprojectDialog)

public:
	ReprojectDialog(CWnd* pParent = NULL);
	virtual ~ReprojectDialog();
    CString GetWkt();

// Dialog Data
	enum { IDD = IDD_REPROJECT };

private:
    CString m_WktString;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL DestroyWindow();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedClear();
};
