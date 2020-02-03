/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/TextLog.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "afxwin.h"


// TextLog dialog

class TextLog : public CDialog
{
	DECLARE_DYNAMIC(TextLog)

public:
	TextLog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextLog();

    void AddString(const CString& pi_String);
    

// Dialog Data
	enum { IDD = IDD_TEXTLOG };

private:
    CRect   m_PrevPos;
    UINT32  m_textlimit;
 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CEdit m_TestLog;
    virtual BOOL DestroyWindow();
    virtual BOOL OnInitDialog();
    virtual BOOL Create(UINT nID, CWnd* pParentWnd = NULL);
    afx_msg void OnBnClickedClose();
    afx_msg void OnBnClickedClear();
    afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};
