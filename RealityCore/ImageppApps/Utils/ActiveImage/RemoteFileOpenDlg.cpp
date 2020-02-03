/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RemoteFileOpenDlg.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/RemoteFileOpenDlg.cpp,v 1.3 2011/07/18 21:10:33 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class CRemoteFileOpenDlg
//-----------------------------------------------------------------------------


//####################################################
//INCLUDE FILES
//####################################################

#include "stdafx.h"
#include "RemoteFileOpenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//####################################################
//MAPS
//####################################################

BEGIN_MESSAGE_MAP(CRemoteFileOpenDlg, CDialog)
	//{{AFX_MSG_MAP(CRemoteFileOpenDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CRemoteFileOpenDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CRemoteFileOpenDlg)
	ON_EVENT(CRemoteFileOpenDlg, IDC_HMRFILEOPENCTRLCTRL, 1 /* Open */, OnOpenHmrfileopenctrlctrl, VTS_NONE)
	ON_EVENT(CRemoteFileOpenDlg, IDC_HMRFILEOPENCTRLCTRL, 3 /* Cancel */, OnCancelHmrfileopenctrlctrl, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


//----------------------------------------------------
//public
//Constructor
//----------------------------------------------------
CRemoteFileOpenDlg::CRemoteFileOpenDlg(CWnd* pParent)
	: CDialog(CRemoteFileOpenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRemoteFileOpenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    
    m_DefaultServer = 0;
    m_FilePos       = 0;
}

//----------------------------------------------------
//public
//Destructor
//----------------------------------------------------
CRemoteFileOpenDlg::~CRemoteFileOpenDlg()
{
}

//----------------------------------------------------
//public
//DoDataExchange
//----------------------------------------------------
void CRemoteFileOpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemoteFileOpenDlg)
	DDX_Control(pDX, IDC_HMRFILEOPENCTRLCTRL, m_HmrFileOpen);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------
//public
//OnOpenHmrfileopenctrlctrl
//----------------------------------------------------
void CRemoteFileOpenDlg::OnOpenHmrfileopenctrlctrl() 
{
	long Count = m_HmrFileOpen.GetSelCount();

    if( Count > 0 )
    {
        // Get the first sel
        m_FileList.AddTail(CString(m_HmrFileOpen.GetFirstSel()));
        
        // Get all remaining sel
        for(long Index = 1; Index < Count; Index++)
        {
            m_FileList.AddTail(CString(m_HmrFileOpen.GetNextSel()));
        }
    }

    OnOK();	
}

//----------------------------------------------------
//public
//OnCancelHmrfileopenctrlctrl
//----------------------------------------------------
void CRemoteFileOpenDlg::OnCancelHmrfileopenctrlctrl() 
{
    OnCancel();
}

//----------------------------------------------------
// public
// The caller must delete the pointer
//----------------------------------------------------
const WChar* CRemoteFileOpenDlg::GetFirstSel()
{
    CString FileName;
    WChar* FileNameSTR = 0;

    // Get the head position
    m_FilePos = m_FileList.GetHeadPosition();
    if( m_FilePos != 0 )
    {
        // Get the first element
        FileName = m_FileList.GetNext(m_FilePos);

        FileNameSTR = new WChar[FileName.GetLength() + 1];
        _tcscpy(FileNameSTR, (LPCTSTR)FileName);
    }

    return FileNameSTR;
}

//----------------------------------------------------
// public
// The caller must delete the pointer
//----------------------------------------------------
const WChar* CRemoteFileOpenDlg::GetNextSel()
{
    CString FileName;
    WChar* FileNameSTR = 0;

    if( m_FilePos != 0 )
    {
        CString FileName = m_FileList.GetNext(m_FilePos);

        FileNameSTR = new WChar[FileName.GetLength() + 1];
        _tcscpy(FileNameSTR, (LPCTSTR)FileName);
    }

    return FileNameSTR;
}

//----------------------------------------------------
// public
// 
//----------------------------------------------------
long CRemoteFileOpenDlg::GetSelCount() const
{
    return (long)m_FileList.GetCount();
}

//----------------------------------------------------
//public
//OnInitDialog
//----------------------------------------------------
BOOL CRemoteFileOpenDlg::OnInitDialog() 
{
    LONG Index = 0;
    LONG DefaultIndex;
    POSITION OldPos;

    CDialog::OnInitDialog();
	
    // Add all server stored in the list
    POSITION Pos = m_ServerInfoList.GetHeadPosition();
    while( Pos != 0 )
    {
        OldPos = Pos;
        CServerInfo ServerInfo = m_ServerInfoList.GetNext(Pos);
        Index = m_HmrFileOpen.APIAddServer(ServerInfo.GetName().c_str(),
                                           ServerInfo.GetPathExtention().c_str(),
                                           ServerInfo.GetAddress().c_str(),
                                           ServerInfo.GetPort(),
                                           (short)ServerInfo.GetType());
        if( m_DefaultServer == OldPos )
            DefaultIndex = Index;
    }

    // If no default server specified, set the first entry 
    // as the default one
    if( !m_DefaultServer && !m_ServerInfoList.IsEmpty() )
        DefaultIndex = 0;

    m_HmrFileOpen.APISetCurrentServer(DefaultIndex);

    return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------
// public
// Add a server to the server list
//-----------------------------------------------------------------------------
VOID CRemoteFileOpenDlg::AddServer(LPCTSTR pi_Name, 
                                   LPCTSTR pi_PathExtention, 
                                   LPCTSTR pi_Address, 
                                   short pi_Port, 
                                   FILE_OPEN_PROTOCOL pi_Type,
                                   BOOL pi_IsDefault) 
{
    POSITION Pos;
    
    // Insert server
    CServerInfo ServerInfo(pi_Name, 
                           pi_PathExtention, 
                           pi_Address, 
                           pi_Port,
                           (uint32_t)pi_Type,
                           0);

    Pos = m_ServerInfoList.AddTail(ServerInfo);
    
    // If this is the default server
    // must retain is position
    if( pi_IsDefault )
        m_DefaultServer = Pos;
}
