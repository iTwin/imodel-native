/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/AuthenticationDialog.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// AuthenticationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "AuthenticationDialog.h"

#include <Imagepp/all/h/HFCCallback.h>
#include <Imagepp/all/h/HFCCallbacks.h>
#include <Imagepp/all/h/HRFPDFFile.h>
#include <Imagepp/all/h/HRFVirtualEarthFile.h>


// AuthenticationDialog dialog

IMPLEMENT_DYNAMIC(AuthenticationDialog, CDialog)

AuthenticationDialog::AuthenticationDialog(HFCAuthentication* pi_pAuthentication,
                                           CWnd*              pParent /*=NULL*/)
: CDialog(AuthenticationDialog::IDD, pParent),
  m_pAuthentication(pi_pAuthentication)
{
    HPRECONDITION(pi_pAuthentication != 0);
}

AuthenticationDialog::~AuthenticationDialog()
{
}

INT_PTR AuthenticationDialog::DoModal()
    {
    return __super::DoModal();
    }

BOOL AuthenticationDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_pAuthentication == 0)
        return false;

    if (m_pAuthentication->IsCompatibleWith(HFCInternetAuthentication::CLASS_ID))
    {
        SetWindowText(_TEXT("Internet Authentication."));
        GetDlgItem(IDC_AuthenticationFormat)->SetWindowTextW(L"Format: user[:password[@server]]");

        // Special case for Bing Map server / Set a default
        HFCPtr<HFCURL> pServerURL = HFCURL::Instanciate(((HFCInternetAuthentication*)m_pAuthentication)->GetServer());

        if(pServerURL != NULL && HRFVirtualEarthFile::IsURLVirtualEarth(*pServerURL))
        {
            WString BingKey = L"BingMap:AtCA5ym9lN9WSAUbyC1-P4tZFTVbfS2ypJHg8CcAWDR9NCcQOUkk7X7Cw_nvoEBX";        
            GetDlgItem(IDC_AUTHENTICATIONSTRING)->SetWindowText(BingKey.c_str());
        }

    }
    else if (m_pAuthentication->IsCompatibleWith(HFCOracleAuthentication::CLASS_ID))
    {
        SetWindowText(_TEXT("Oracle Authentication."));
        GetDlgItem(IDC_AuthenticationFormat)->SetWindowTextW(L"Format: DataSource=[DataSource];User Id=[UserId];Password=[Password]");
    }
    else if (m_pAuthentication->IsCompatibleWith(HFCPDFAuthentication::CLASS_ID))
    {
        if (((HFCPDFAuthentication*)m_pAuthentication)->GetPasswordType() == HFCPDFAuthentication::OPEN)
            SetWindowText(_TEXT("PDF Open Authentication"));
        else if (((HFCPDFAuthentication*)m_pAuthentication)->GetPasswordType() == HFCPDFAuthentication::PERMISSION)
            SetWindowText(_TEXT("PDF Permission Authentication"));
        else
        {
            HASSERT(0);
            return false;
        }
        GetDlgItem(IDC_AuthenticationFormat)->SetWindowTextW(L"Format: password");
    }
    else if (m_pAuthentication->IsCompatibleWith(HFCProxyAuthentication::CLASS_ID))
    {
        SetWindowText(_TEXT("Proxy Authentication. "));
        GetDlgItem(IDC_AuthenticationFormat)->SetWindowTextW(L"Format: user[:password]");
    }
    else
    {
        HASSERT(0);
        return false;
    }

    return true;
}


void AuthenticationDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_AUTHENTICATIONSTRING, m_AuthenticationString);

    CString AuthenticationString;
    m_AuthenticationString.GetWindowText(AuthenticationString);
    m_pAuthentication->SetByString(Utf8String(AuthenticationString.GetString()));
}


BEGIN_MESSAGE_MAP(AuthenticationDialog, CDialog)
    ON_EN_CHANGE(IDC_AUTHENTICATIONSTRING, &AuthenticationDialog::OnEnChangeAuthenticationstring)
END_MESSAGE_MAP()

// AuthenticationDialog message handlers


void AuthenticationDialog::OnEnChangeAuthenticationstring()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}
