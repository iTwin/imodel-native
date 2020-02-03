/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/OpenInternetFileDialog.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// OpenInternetFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "OpenInternetFileDialog.h"
#include "RemoteFileOpenDlg.h"
#include <Imagepp/all/h/HRFVirtualEarthFile.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OpenInternetFileDialog dialog


OpenInternetFileDialog::OpenInternetFileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(OpenInternetFileDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIIPDialog)	
	m_InternetUrl = _T("");
	//}}AFX_DATA_INIT
}


BOOL OpenInternetFileDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

#define CEDIT_MULTILINE_BREAK_LINE L"\r\n"

    CString examples("Examples:" CEDIT_MULTILINE_BREAK_LINE);

    examples+= CEDIT_MULTILINE_BREAK_LINE "http://www.bentley.com/BentleyWebSite/Images/common/bentley-logo.gif";
    examples+= CEDIT_MULTILINE_BREAK_LINE "http://127.0.0.1:8087/?fif=quebec/DG30000.itiff";
    examples+= CEDIT_MULTILINE_BREAK_LINE; 
    examples+= HRFVirtualEarthFile::ComposeURL(BINGMAPS_IMAGERYSET_ROAD).c_str();
    examples+= CEDIT_MULTILINE_BREAK_LINE; 
    examples+= HRFVirtualEarthFile::ComposeURL(BINGMAPS_IMAGERYSET_AERIALWITHLABELS).c_str();

    GetDlgItem(IDC_URL_EXAMPLES)->SetWindowText(examples);
    return true;
    }



void OpenInternetFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OpenInternetFileDialog)
	DDX_Text(pDX, IDC_INTERNET_URL, m_InternetUrl);		
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OpenInternetFileDialog, CDialog)
	//{{AFX_MSG_MAP(OpenInternetFileDialog)	
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOPEN, &OpenInternetFileDialog::OnBnClickedOpen)
	ON_BN_CLICKED(IDCANCEL, &OpenInternetFileDialog::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OpenInternetFileDialog message handlers

void OpenInternetFileDialog::OnBnClickedOpen()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	CDialog::OnOK();
}

void OpenInternetFileDialog::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
