/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFactorDlg.cpp $
|    $RCSfile: HMRFactorDlg.cpp,v $
|   $Revision: 1.10 $
|       $Date: 2011/07/18 21:12:29 $
|     $Author: Donald.Morissette $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// HMRFactorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HMRFactor.h"
#include "HMRFactorDlg.h"
#include ".\hmrfactordlg.h"
#include "HMRFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FILE_BUFFER_LIMIT           32*1024       // 32k is the limit on NT4, Win2000, winXP
#define SURVEYFEET_TO_INTFEET       1.000002000004

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
        CAboutDlg();

// Dialog Data
        enum { IDD = IDD_ABOUTBOX };

        protected:
        virtual BOOL OnInitDialog();
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
        DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CEdit* pVersion = (CEdit*)GetDlgItem (IDC_VERSION);
    pVersion->SetWindowText(_T("HMRFactor Version 1.0 - ")_T(__DATE__));

    CEdit* pCopyright = (CEdit*)GetDlgItem (IDC_COPYRIGHT);
    pCopyright->SetWindowText(_T("Copyright (C) 2006 Bentley Systems, Inc"));

    return true;  // return true  unless you set the focus to a control
    }

// HMRFactorDlg dialog



HMRFactorDlg::HMRFactorDlg(CWnd* pParent /*=NULL*/)
        : CDialog(HMRFactorDlg::IDD, pParent)
{
m_hIcon = AfxGetApp()->LoadIcon(IDI_BENTLEY);
}

HMRFactorDlg::~HMRFactorDlg ()
    {
    m_SmallImageList.DeleteImageList ();
    }

void HMRFactorDlg::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Control(pDX, IDC_LIST_FILES, m_ListFiles);
DDX_Control(pDX, IDC_FACTOR, m_CustomFactor);
DDX_Control(pDX, IDC_FROMCOMBO, m_FromComboBox);
DDX_Control(pDX, IDC_TOCOMBO,   m_ToComboBox);
}

BEGIN_MESSAGE_MAP(HMRFactorDlg, CDialog)
        ON_WM_SYSCOMMAND()
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        ON_BN_CLICKED(IDC_ADDFILES, OnBnClickedAddfiles)
        ON_BN_CLICKED(IDC_REMOVEFILES, OnBnClickedRemovefiles)
        ON_BN_CLICKED(IDC_Process, OnBnClickedProcess)
        ON_BN_CLICKED(IDC_FROMBTN, OnBnClickedFrombtn)
        ON_BN_CLICKED(IDC_CUSTOMBTN, OnBnClickedCustombtn)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILES, OnLVNItemChangedListFiles)
        ON_NOTIFY(NM_CLICK, IDC_LIST_FILES, OnNMClickListFiles)
        ON_NOTIFY(NM_KILLFOCUS, IDC_LIST_FILES, OnNMKillfocusListFiles)
        ON_CBN_SETFOCUS(IDC_FROMCOMBO, OnCbnSetfocusFromcombo)
        ON_CBN_KILLFOCUS(IDC_FROMCOMBO, OnCbnKillfocusFromcombo)
        ON_CBN_SETFOCUS(IDC_TOCOMBO, OnCbnSetfocusTocombo)
        ON_CBN_KILLFOCUS(IDC_TOCOMBO, OnCbnKillfocusTocombo)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// HMRFactorDlg message handlers

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL HMRFactorDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        // Add "About..." menu item to system menu.

        // IDM_ABOUTBOX must be in the system command range.
        ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
        ASSERT(IDM_ABOUTBOX < 0xF000);

        CMenu* pSysMenu = GetSystemMenu(false);
        if (pSysMenu != NULL)
            {
            CString strAboutMenu;
            strAboutMenu.LoadString(IDS_ABOUTBOX);
            if (!strAboutMenu.IsEmpty())
                {
                pSysMenu->AppendMenu(MF_SEPARATOR);
                pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
                }
            }

        // Set the icon for this dialog.  The framework does this automatically
        //  when the application's main window is not a dialog
        SetIcon(m_hIcon, true);                 // Set big icon
        SetIcon(m_hIcon, false);                // Set small icon

        // List of Files Initialization
        m_SmallImageList.Create(16, 16, ILC_MASK | ILC_COLORDDB, 1, 1000);
        m_SmallImageList.SetBkColor(ILD_TRANSPARENT);
        m_SmallImageList.Add (AfxGetApp()->LoadIcon(IDI_BLANK));
        m_SmallImageList.Add (AfxGetApp()->LoadIcon(IDI_PROCESSING));
        m_SmallImageList.Add (AfxGetApp()->LoadIcon(IDI_SUCCESS));
        m_SmallImageList.Add (AfxGetApp()->LoadIcon(IDI_FILEERROR));
        m_ListFiles.SetImageList(&m_SmallImageList, LVSIL_SMALL);

        // List of Files Initialization
        // Always show selection even if we lose the focus. Don't show colum header
        m_ListFiles.ModifyStyle(0, LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER,0);

        // Insert the column
        m_ListFiles.InsertColumn( 0, _TEXT("Name"), LVCFMT_LEFT,  -1, -1);

        m_CustomFactor.SetAttributes (12);
        m_CustomFactor.SetNumericValue (SURVEYFEET_TO_INTFEET);
        m_CustomFactor.SetRange (0.0001, MAXDWORD);

        m_FromComboBox.Fill(Feet);
        m_ToComboBox.Fill(SurveyFeet);


        CButton* pFromBtn =    (CButton*)GetDlgItem (IDC_FROMBTN);
        pFromBtn->SetCheck (BST_CHECKED);

        CustomFactorMode (false);

        CButton* pRemoveBtn = (CButton*)GetDlgItem (IDC_REMOVEFILES);
        pRemoveBtn->EnableWindow (false);

        CButton* pProcessBtn = (CButton*)GetDlgItem (IDC_Process);
        pProcessBtn->EnableWindow (false);

        
        return true;  // return true  unless you set the focus to a control
}

void HMRFactorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
        if ((nID & 0xFFF0) == IDM_ABOUTBOX)
        {
                CAboutDlg dlgAbout;
                dlgAbout.DoModal();
        }
        else
        {
                CDialog::OnSysCommand(nID, lParam);
        }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void HMRFactorDlg::OnPaint() 
{
        if (IsIconic())
        {
                CPaintDC dc(this); // device context for painting

                SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

                // Center icon in client rectangle
                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);
                CRect rect;
                GetClientRect(&rect);
                int x = (rect.Width() - cxIcon + 1) / 2;
                int y = (rect.Height() - cyIcon + 1) / 2;

                // Draw the icon
                dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
                CDialog::OnPaint();
        }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR HMRFactorDlg::OnQueryDragIcon()
{
        return static_cast<HCURSOR>(m_hIcon);
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnOK()
    {
    // Don't let the dialog close
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnBnClickedAddfiles()
    {
    TCHAR             szFilters[]= _TEXT("HMR (*.hmr)|*.hmr|All Files (*.*)|*.*||");
    DWORD ofn_flags       = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

    // TODO: Add your control notification handler code here
    CFileDialog fileDlg (true, _TEXT("hmr"), 0, ofn_flags, szFilters, this);
    // Display the file dialog. When user clicks OK, fileDlg.DoModal() 
    // returns IDOK.
    if( fileDlg.DoModal ()==IDOK )
        {
        POSITION pos (fileDlg.GetStartPosition  ());

        while (pos)
            {
            CString filename (fileDlg.GetNextPathName (pos));

            m_ListFiles.AddFilename (filename);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnBnClickedRemovefiles()
    {
    m_ListFiles.RemoveSelected ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnBnClickedProcess()
    {
    UINT  nItemCount = m_ListFiles.GetItemCount();
    UINT  nItem = -1;


    double dFactor (m_CustomFactor.GetNumericValue ());

    CButton* pFromRadioBtn ((CButton*) GetDlgItem (IDC_FROMBTN));

    if (pFromRadioBtn->GetCheck ())
        {
        dFactor = (m_ToComboBox.GetUnit ()->GetDenominator () * m_FromComboBox.GetUnit ()->GetNumerator ()) /
                   (m_ToComboBox.GetUnit ()->GetNumerator () * m_FromComboBox.GetUnit ()->GetDenominator ());
        }

    if (nItemCount && IDYES == AfxMessageBox(IDS_WARNING, MB_YESNO|MB_ICONWARNING))
        {
        HCURSOR cursor (GetCursor ());
        SetCursor(LoadCursor(NULL,IDC_WAIT));

        for (UINT i = 0; i < nItemCount; i++)
            {
            DWORD_PTR nStatus (CONVERTED);
            nItem = m_ListFiles.GetNextItem (nItem, LVNI_ALL);

            // Process only items which haven't been converted
            nStatus = m_ListFiles.GetItemData (nItem);

            if (nStatus == CONVERTED)
                {
                // This file was already converted
                continue;
                }

            CString filename (m_ListFiles.GetItemText (nItem, 0));

            // Convert this filename
            bool bConverted (false);

            m_ListFiles.DeleteItem (nItem);
            m_ListFiles.InsertItem (nItem, filename, 1);
            m_ListFiles.EnsureVisible(nItem, false);
            m_ListFiles.Update (nItem);

                {
                HMRFile file (filename.GetBuffer ());

                bConverted = file.ApplyFactor (dFactor);
                nStatus = file.GetStatus ();
                }

            m_ListFiles.DeleteItem (nItem);
            m_ListFiles.InsertItem (nItem, filename, bConverted ? 2 : 3);
            m_ListFiles.SetItemData (nItem, nStatus);

            m_ListFiles.Update (nItem);
            }

        SetCursor (cursor);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::CustomFactorMode (bool isOn)
    {
    m_CustomFactor.EnableWindow (isOn);

    m_ToComboBox.EnableWindow (!isOn);
    m_FromComboBox.EnableWindow (!isOn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnBnClickedFrombtn()
    {
    CustomFactorMode (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnBnClickedCustombtn()
    {
    CustomFactorMode (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnLVNItemChangedListFiles(NMHDR *pNMHDR, LRESULT *pResult)
    {
    CButton* pRemoveBtn = (CButton*)GetDlgItem (IDC_REMOVEFILES);
    pRemoveBtn->EnableWindow (m_ListFiles.GetSelectedCount () ? true : false);

    CButton* pProcessBtn = (CButton*)GetDlgItem (IDC_Process);
    pProcessBtn->EnableWindow (m_ListFiles.GetItemCount () ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnNMClickListFiles(NMHDR *pNMHDR, LRESULT *pResult)
    {
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    
    if (phdr->iItem != -1)
        {
        UINT64 status = m_ListFiles.GetItemData (phdr->iItem);

        CString cstrStatus;

        switch  (status)
            {
            case NEWFILE:
                cstrStatus.Empty ();
    	        break;
            case CANTBELOCKED:
            case INVALIDACCESSMODE:
                cstrStatus = "File is Read-Only";
                break;
            case INVALIDFILE:
                cstrStatus = "Invalid File Type";
                break;
            case UNKNOWNERROR:
                cstrStatus = "Unknown Error";
                break;
            default:
                cstrStatus = "File Successfully Converted";
                break;
            }

        CEdit* pStatus = (CEdit*)GetDlgItem (IDC_STATUS);
        if (pStatus)
            pStatus->SetWindowText (cstrStatus);
        }

    *pResult = 0; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::SetStatusText (LPCTSTR lpszString)
    {
    CEdit* pStatus = (CEdit*)GetDlgItem (IDC_STATUS);
    if (pStatus)
        pStatus->SetWindowText (lpszString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnNMKillfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult)
    {
    SetStatusText (_TEXT(""));
    *pResult = 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnCbnSetfocusFromcombo()
    {
    SetStatusText (_TEXT("Design Unit used to save the HMR"));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnCbnKillfocusFromcombo()
    {
    SetStatusText (_TEXT(""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnCbnSetfocusTocombo()
    {
    SetStatusText (_TEXT("New Design Unit to use to save the HMR"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void HMRFactorDlg::OnCbnKillfocusTocombo()
    {
    SetStatusText (_TEXT(""));
    }
