/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/AuthenticationDialog.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "afxwin.h"

// AuthenticationDialog dialog

class AuthenticationDialog : public CDialog
{
	DECLARE_DYNAMIC(AuthenticationDialog)

public:
	AuthenticationDialog(HFCAuthentication* pi_Authentication,
                         CWnd* pParent = NULL);   // standard constructor
	virtual ~AuthenticationDialog();

// Dialog Data
	enum { IDD = IDD_DLG_AUTHENTICATION };

    BOOL        OnInitDialog();

    virtual INT_PTR DoModal();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:

    CEdit               m_AuthenticationString;
    HFCAuthentication*  m_pAuthentication;

public:
    afx_msg void OnEnChangeAuthenticationstring();
};
