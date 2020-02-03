/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFileDialog.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFileDialog.cpp,v 1.3 2011/07/18 21:10:39 Donald.Morissette Exp $
//
// Class: ActiveImageFileDialog
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageFileDialog.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_LINKBOX     40000

//-----------------------------------------------------------------------------
// Message Map and other MFC Macros
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CActiveImageFileDialog, CFileDialog)
BEGIN_MESSAGE_MAP(CActiveImageFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CActiveImageFileDialog)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_LINKBOX, OnLinkButtonClicked)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageFileDialog::CActiveImageFileDialog(BOOL bOpenFileDialog, 
                                               BOOL bShowLinkBox, 
                                               LPCTSTR lpszDefExt, 
                                               LPCTSTR lpszFileName,
		                                       DWORD dwFlags, 
                                               LPCTSTR lpszFilter, 
                                               CWnd* pParentWnd) 
    : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
    m_bLink = true;
    m_bShowLinkBox = bShowLinkBox;

    // Alloc Buffer to accept Multi-Selection.
    // Accept a minimun of 512 files
    m_ofn.nMaxFile = _MAX_PATH*512;            
    m_pBufferFileNames = new WChar[m_ofn.nMaxFile];
    m_pBufferFileNames[0] = 0;
    m_pBufferFileNames[1] = 0;
    m_ofn.lpstrFile = m_pBufferFileNames;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageFileDialog::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
    // if the show link box flag is true, show it
    if (m_bShowLinkBox)
    {
        // get the rect of the dialog
        CRect DialogRect;
        GetWindowRect(&DialogRect);

        // Add 20 to the dialog rect to add the new control
        DialogRect.bottom += 20;
        DialogRect.right += 100;
        MoveWindow(DialogRect);

        // Set the clip siblings style to the child dialog
        LONG Style = GetWindowLong(m_hWnd, GWL_STYLE);
        Style |= WS_CLIPSIBLINGS;
        SetWindowLong(m_hWnd, GWL_STYLE, Style);

        // Create the link box button
        m_LinkBox.Create(_TEXT("Insert as link"), WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE,
                         CRect(5, 0, 85, 14), this, IDC_LINKBOX);

        // check the box if m_bLink is true
        if (m_bLink)
            m_LinkBox.SetCheck(1);

        // The dialog object we're in is actually a child of the file dialog.
        // Get the font of the parent dialog to set it to its child dialog
        m_LinkBox.SetFont(GetParent()->GetFont());
    }
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageFileDialog::IsLinkBoxChecked() const
{
    return m_bLink;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFileDialog::OnLinkButtonClicked()
{
    m_bLink = !m_bLink;
}