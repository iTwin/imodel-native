/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageFrame.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageFrame.cpp,v 1.6 2011/07/18 21:10:41 Donald.Morissette Exp $
//
// Class: ActiveImageFrame
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageDoc.h"
#include "ActiveImageSplash.h"
#include "ActiveImageDefaultMode.h"
#include "ActiveImageViewMode.h"
#include <commctrl.h>


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_TOOLBAR 1000

//-----------------------------------------------------------------------------
// Message Map and other MFC Macros
//-----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(CActiveImageFrame, CFrameWnd)
BEGIN_MESSAGE_MAP(CActiveImageFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CActiveImageFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_FILETOOLBAR, OnViewFiletoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILETOOLBAR, OnUpdateViewFiletoolbar)
	ON_COMMAND(ID_VIEW_OBJECTTOOLBAR, OnViewObjecttoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTTOOLBAR, OnUpdateViewObjecttoolbar)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP

    // Notifications from the tree list
    ON_NOTIFY(TVN_SELCHANGED, IDC_OBJECT_TREE, OnSelectedObjectChanged)

	// toolbar "tooltip" notification
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTip)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTip)

END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
   	ID_INDICATOR_DRAWTIME,
	ID_INDICATOR_MOUSEPOSITION,
	ID_INDICATOR_PROGRESSION,
	ID_INDICATOR_EXPORTSIZE //Estimation
};


//Size of the status bar's panes in pixels. The total size should not be greater than some 
//minimal horizontal screen resolution (e.g. : 800 or 1024)
//The size of ID_SEPARATOR is -1 because it isn't set.
static const int statusBarPaneSize[] = {-1, 100, 175, 80, 1500};

//-----------------------------------------------------------------------------
// Private
// Helpers for saving/restoring window state
//-----------------------------------------------------------------------------
static TCHAR BASED_CODE szSection[] = _T("Settings");
static TCHAR BASED_CODE szWindowPos[] = _T("WindowPos");
static TCHAR szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");


//-----------------------------------------------------------------------------
// Private
// 
//-----------------------------------------------------------------------------
static BOOL PASCAL NEAR ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	CString strBuffer = AfxGetApp()->GetProfileString(szSection, szWindowPos);
	if (strBuffer.IsEmpty())
		return false;

	WINDOWPLACEMENT wp;
	int nRead = _stscanf_s(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
		return false;

	wp.length = sizeof wp;
	*pwp = wp;
	return true;
}


//-----------------------------------------------------------------------------
// Private
// 
//-----------------------------------------------------------------------------
static void PASCAL NEAR WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
	// write a window placement to settings section of app's ini file
{
	TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

	wsprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
	AfxGetApp()->WriteProfileString(szSection, szWindowPos, szBuffer);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageFrame::CActiveImageFrame()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageFrame::~CActiveImageFrame()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
int CActiveImageFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    uint32_t m_ToolBarID = IDC_TOOLBAR;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

  	// CG: The following line was added by the Splash Screen component.
	CActiveImageSplashWnd::ShowSplashScreen(this);

    //
    // Setup the main frame
    //
	WINDOWPLACEMENT wp;
	if (ReadWindowPlacement(&wp))
		SetWindowPlacement(&wp);
    
    //
    // Create the Status Bar
    //

	int32_t nbIndicators = sizeof(indicators)/sizeof(UINT);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
									  nbIndicators))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}		

	for (int32_t indicatorInd = 1; indicatorInd < nbIndicators; indicatorInd++)
	{			
		m_wndStatusBar.SetPaneInfo(indicatorInd, indicatorInd, SBPS_NORMAL, statusBarPaneSize[indicatorInd]);		
	}		 	

    // 
    // Create the file tool bar
    //
    if (!m_FileToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP, m_ToolBarID++) ||
        !m_FileToolBar.LoadToolBar(IDR_FILE_TOOLBAR))
    {
        TRACE0("Failed to create File Tool Bar\n");
        return -1;
    }

    EnableDocking(CBRS_ALIGN_ANY);
    m_FileToolBar.SetBarStyle(m_FileToolBar.GetBarStyle() | CBRS_FLYBY | CBRS_TOOLTIPS);
    m_FileToolBar.SetWindowText(_TEXT("File"));
	m_FileToolBar.EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_FileToolBar, AFX_IDW_DOCKBAR_TOP);

    // 
    // Create the Nagigation tool bar
    //
    CToolBar* pNav = CActiveImageViewMode::SetupToolbar(this, m_ToolBarID++);
    HASSERT(pNav != 0);
    pNav->EnableDocking(CBRS_ALIGN_ANY);
    DockControlBarLeftOf(pNav, &m_FileToolBar);


    //
    // Create the image list for the tree list items
    //
    if (!m_TreeListIcons.Create(IDB_TREELISTICONS, 16, 2, RGB(255, 255, 255)))
    {
        TRACE0("Failed to create image list for tree items icons\n");
        return -1;
    }

    //
    // Create the Object Tool Bar
    //
    if (!m_ObjectToolBar.Create(this, IDD_OBJECT_TOOLBAR, CBRS_ALIGN_LEFT, m_ToolBarID++))
    {
        TRACE0("Failed to create Object Tool Bar\n");
        return -1;
    }
 	
    m_ObjectToolBar.SetBarStyle(m_ObjectToolBar.GetBarStyle() | CBRS_FLYBY | CBRS_TOOLTIPS);
    m_ObjectToolBar.SetWindowText(_TEXT("Object"));
	m_ObjectToolBar.EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_ObjectToolBar, AFX_IDW_DOCKBAR_LEFT);
    GetSelector()->SetImageList(&m_TreeListIcons, TVSIL_NORMAL);

////////////////////////////////////////
// Create Function Mode toolbars
////////////////////////////////////////

    CToolBar* pTB = CActiveImageDefaultMode::SetupToolbar(this, m_ToolBarID++);
    HASSERT(pTB != 0);
    pTB->EnableDocking(CBRS_ALIGN_ANY);
    DockControlBarLeftOf(pTB, pNav);


////////////////////////////////////////
// Create Function Mode Menus
////////////////////////////////////////

    CActiveImageDefaultMode::SetupMenu(GetMenu()->GetSubMenu(1), GetMenu()->GetSubMenu(2));


////////////////////////////////////////
// Load the control bar states
////////////////////////////////////////

    //
    // Get the bar states from the INI file
    //
    //LoadBarState("ToolBar");

    return 0;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    BOOL bResult;

    // set the style
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	// call the parent class
    bResult = CFrameWnd::PreCreateWindow(cs);

    // remove the FWS_PREFIXTITLE style that the parent PreCreateWindow assigns
    // to the frame
    if (bResult)
        cs.style &= ~(FWS_PREFIXTITLE);

    return (bResult);
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf)
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::GetMessageString(UINT nID, CString& rMessage) const
{
    const char* pResult;

    ////////////////////////////////////////
    // Get the function mode strings
    ////////////////////////////////////////
    
    // Call GetMessageString on the parent Function object
    // (It keeps a list of all the command and their prompts).
    // Otherwise call the default method (from CFrameWnd);
    if ((pResult = CActiveImageFunctionMode::GetPrompt(nID)) != 0)
        rMessage = pResult;
    else
        CFrameWnd::GetMessageString(nID, rMessage);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageFrame::OnToolTip(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

	// allow top level routing frame to handle the message
	if (GetRoutingFrame() != NULL)
		return false;

	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	UINT_PTR nID = pNMHDR->idFrom;
	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = (UINT)(WORD)::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) // will be zero on a separator
	{
        const char* pResult;

        // Verify if there is a function mode tool tip for the current
        // id.  If not, try to find one in the ressource.
        if ((pResult = CActiveImageFunctionMode::GetToolTip(nID)) != 0)
            strTipText = pResult;
        else
        {
            CString FullText;

            // Get the string from the ressource
		    FullText.LoadString((UINT)nID);

            // Extract the sub-string after the '\n'
            int Pos = FullText.Find('\n');
            strTipText = FullText.Mid(Pos + 1);
        }
	}

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, lstrlen(strTipText) + 1);
	else
		_mbstowcsz(pTTTW->szText, strTipText, lstrlen(strTipText) + 1);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, lstrlen(strTipText) + 1);
	else
		lstrcpyn(pTTTW->szText, strTipText, lstrlen(strTipText) + 1);
#endif
	*pResult = 0;

	// bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);

	return true;    // message was handled
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnSelectedObjectChanged(NMHDR * pNotifyStruct, LRESULT * result)
{
    CActiveImageApp* pApp = (CActiveImageApp*) AfxGetApp();
    CDocTemplate*    pTemplate;
    CActiveImageDoc* pDoc;
    POSITION         pos;

    // Get the document template of the application
    pos = pApp->GetFirstDocTemplatePosition();
    pTemplate = pApp->GetNextDocTemplate(pos);

    // Get the document
    pos = pTemplate->GetFirstDocPosition();
    pDoc = (CActiveImageDoc*)pTemplate->GetNextDoc(pos);

    // Notify that selection has changed
    pDoc->SelectedObjectChanged();   
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnViewFiletoolbar() 
{
    if (m_FileToolBar.IsWindowVisible())
        ShowControlBar(&m_FileToolBar, false, false);
    else
        ShowControlBar(&m_FileToolBar, true, false);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnViewObjecttoolbar() 
{
    if (m_ObjectToolBar.IsWindowVisible())
        ShowControlBar(&m_ObjectToolBar, false, false);
    else
        ShowControlBar(&m_ObjectToolBar, true, false);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnUpdateViewFiletoolbar(CCmdUI* pCmdUI) 
{
        pCmdUI->SetCheck(m_FileToolBar.IsWindowVisible());
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnUpdateViewObjecttoolbar(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(m_ObjectToolBar.IsWindowVisible());
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageFrame::OnClose() 
{
	WINDOWPLACEMENT wp;

    // save the window placement
	wp.length = sizeof wp;
	if (GetWindowPlacement(&wp))
	{
	    wp.flags = 0;
	    if (IsZoomed())
	    	wp.flags |= WPF_RESTORETOMAXIMIZED;
	    // and write it to the .INI file
	    WriteWindowPlacement(&wp);
	}

    SaveBarState(_TEXT("ToolBar"));
    
    CFrameWnd::OnClose();
}
//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CTreeCtrl* CActiveImageFrame::GetSelector()
{
     CTreeCtrl* pTree = (CTreeCtrl*)m_ObjectToolBar.GetDlgItem(IDC_OBJECT_TREE);
     return pTree;
}