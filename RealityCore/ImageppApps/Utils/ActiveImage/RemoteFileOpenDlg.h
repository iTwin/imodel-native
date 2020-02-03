/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RemoteFileOpenDlg.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/RemoteFileOpenDlg.h,v 1.3 2011/07/18 21:10:34 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Methods for class CRemoteFileOpenDlg
//-----------------------------------------------------------------------------


//####################################################
//INCLUDE FILES
//####################################################

#include "ServerInfo.h"
#include <afxtempl.h>

//{{AFX_INCLUDES()
#include "hmrfileopenctrl.h"
#include "resource.h"
//}}AFX_INCLUDES
#if !defined(AFX_REMOTEFILEOPENDLG_H__1231790A_6B29_11D1_8C88_0EE5A1000000__INCLUDED_)
#define AFX_REMOTEFILEOPENDLG_H__1231790A_6B29_11D1_8C88_0EE5A1000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//####################################################
//ENUM
//####################################################

enum FILE_OPEN_PROTOCOL
{
    PROTOCOL_SOCKET,
    PROTOCOL_HTTP,
    PROTOCOL_LOCAL
};

enum FILE_OPEN_VIEW_MODE
{
    LIST_VIEW = 0,
    DETAIL_VIEW
};

class CRemoteFileOpenDlg : public CDialog
{
    public:
	    
        //Construction - Destruction
        CRemoteFileOpenDlg(CWnd* pParent = 0);

        virtual ~CRemoteFileOpenDlg();

        // Get - Set methods
        const WChar* GetFirstSel();
        const WChar* GetNextSel();
        long GetSelCount()        const;
                
        // Operation
        VOID AddServer(LPCTSTR pi_Name, 
                       LPCTSTR pi_PathExtention, 
                       LPCTSTR pi_Address, 
                       short pi_Port, 
                       FILE_OPEN_PROTOCOL pi_Type, 
                       BOOL pi_IsDefault = false);

    // Dialog Data
	    //{{AFX_DATA(CRemoteFileOpenDlg)
	    enum { IDD = IDD_DFILE_OPEN };
	    CHMRFileOpenCtrl	m_HmrFileOpen;
	    //}}AFX_DATA


    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CRemoteFileOpenDlg)
	    protected:
	    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	    //}}AFX_VIRTUAL

    // Implementation
    protected:

	    // Generated message map functions
	    //{{AFX_MSG(CRemoteFileOpenDlg)
	    afx_msg void OnOpenHmrfileopenctrlctrl();
	    afx_msg void OnCancelHmrfileopenctrlctrl();
	    virtual BOOL OnInitDialog();
	    DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
        
        CList<CServerInfo, CServerInfo> m_ServerInfoList;
        CList<CString, CString> m_FileList;
        POSITION m_DefaultServer;
        POSITION m_FilePos;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTEFILEOPENDLG_H__1231790A_6B29_11D1_8C88_0EE5A1000000__INCLUDED_)
