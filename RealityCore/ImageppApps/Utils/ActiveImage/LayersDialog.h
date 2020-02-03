/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/LayersDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "afxwin.h"

#include <Imagepp/all/h/HRARaster.h>
// CLayersDialog dialog
class CActiveImageView;

class CLayersDialog : public CDialog
{
	//DECLARE_DYNAMIC(CLayersDialog)

public:
	CLayersDialog(HFCPtr<HRARaster>& pi_prRaster, CActiveImageView* pi_pActiveImageView);   // standard constructor
	virtual ~CLayersDialog();

// Dialog Data
	enum { IDD = IDD_LAYERS_DIALOG };
    CCheckListBox m_LayerList;

protected:

    virtual BOOL OnInitDialog();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
        
    HFCPtr<HRARaster> m_pRaster;
    CActiveImageView* m_pActiveImageView;

public:
    afx_msg void OnBnClickedApply();
};

/*
/////////////////////////////////////////////////////////////////////////////
// CCheckListBoxCBNDlg dialog

class CLayersDialog : public CDialog
{
    // Construction
public:
    CLayersDialog(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    //{{AFX_DATA(CLayersDialog)
    enum { IDD = IDD_LAYERS_DIALOG };
    //}}AFX_DATA
    CCheckListBox	m_ctlCheckList;

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CLayersDialog)
public:
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CCheckListBoxCBNDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSelchangeList1();
    afx_msg void OnCheckchangeList1();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};*/