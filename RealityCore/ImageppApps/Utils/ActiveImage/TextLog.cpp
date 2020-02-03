/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/TextLog.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// TextLog.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "TextLog.h"
#include "afxdialogex.h"


// TextLog dialog

IMPLEMENT_DYNAMIC(TextLog, CDialog)

TextLog::TextLog(CWnd* pParent /*=NULL*/)
	: CDialog(TextLog::IDD, pParent)
{
    Create(TextLog::IDD, pParent);
    m_textlimit = 0;
}

TextLog::~TextLog() 
{
}

void TextLog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TEXTLOG, m_TestLog);
}


BEGIN_MESSAGE_MAP(TextLog, CDialog)
    ON_BN_CLICKED(IDTEXTLOG_CLOSE, &TextLog::OnBnClickedClose)
    ON_BN_CLICKED(IDC_CLEAR, &TextLog::OnBnClickedClear)
    ON_WM_SIZING()
END_MESSAGE_MAP()


// TextLog message handlers


BOOL TextLog::DestroyWindow()
{
    // TODO: Add your specialized code here and/or call the base class

    return CDialog::DestroyWindow();
}


BOOL TextLog::OnInitDialog()
{
    CDialog::OnInitDialog(); 

    GetWindowRect(&m_PrevPos);
    m_TestLog.SetReadOnly (true);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL TextLog::Create(UINT nID, CWnd* pParentWnd)
{
    // TODO: Add your specialized code here and/or call the base class

    return CDialog::Create(nID, pParentWnd);
}


void TextLog::OnBnClickedClose()
{
    //DestroyWindow();
    ShowWindow(SW_HIDE);
}


void TextLog::AddString(const CString& pi_String)
{
    CTime CurTime(CTime::GetCurrentTime());
    CString Str = CurTime.Format(_TEXT("%H:%M:%S - "));
    Str += pi_String + _TEXT("\r\n");
    m_textlimit += Str.GetLength();
    m_TestLog.SetLimitText(m_textlimit);

    m_TestLog.SetSel(-1, -1);
    m_TestLog.ReplaceSel(Str);
}


void TextLog::OnBnClickedClear()
{
    if (MessageBox(_TEXT("Ok to Clear the log?"), _TEXT("Info"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        m_TestLog.SetReadOnly (false);
        m_TestLog.SetSel(0, -1);
        m_TestLog.Clear();
        m_TestLog.SetReadOnly (true);
    }
}


void TextLog::OnSizing(UINT fwSide, LPRECT pRect)
{
    CDialog::OnSizing(fwSide, pRect);

    CRect rectVar;
    GetWindowRect(&rectVar);

    int DiffX = rectVar.Width() - m_PrevPos.Width();
    int DiffY = rectVar.Height() - m_PrevPos.Height();
    m_PrevPos = rectVar;

    CWnd* pCtl = GetDlgItem(IDTEXTLOG_CLOSE);
    if (pCtl != 0)
    {
        pCtl->GetWindowRect(&rectVar); 
        ScreenToClient(&rectVar);

        pCtl->MoveWindow(rectVar.left + DiffX, 
                         rectVar.top + DiffY,
                         rectVar.Width(),
                         rectVar.Height(),
                        TRUE);
    }

    pCtl = GetDlgItem(IDC_CLEAR);
    if (pCtl != 0)
    {
        pCtl->GetWindowRect(&rectVar); 
        ScreenToClient(&rectVar);

        pCtl->MoveWindow(rectVar.left + DiffX, 
            rectVar.top + DiffY,
            rectVar.Width(),
            rectVar.Height(),
            TRUE);
    }


    pCtl = GetDlgItem(IDC_TEXTLOG);
    if (pCtl != 0)
    {
        pCtl->GetWindowRect(&rectVar); 
        ScreenToClient(&rectVar);

        pCtl->MoveWindow(rectVar.left, rectVar.top,
                         rectVar.Width() + DiffX,
                         rectVar.Height() + DiffY,
                         TRUE);
    }
}

