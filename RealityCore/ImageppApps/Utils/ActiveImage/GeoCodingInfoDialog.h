/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/GeoCodingInfoDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-------------------------------------------------------------------
// GeoCodingInfoDialog.h : header file
//-------------------------------------------------------------------

#ifndef __GeoCodingInfoDialog_H__
#define __GeoCodingInfoDialog_H__

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include <Imagepp/all/h/HRFRasterFile.h>

class GeoCodingInfoDialog : public CDialog
{
public:
	GeoCodingInfoDialog(HFCPtr<HRFRasterFile >& pi_prRasterFile);

	//{{AFX_DATA(GeoCodingInfoDialog)
	enum { IDD = IDD_DLG_GEOCODING_INFO };
	CButton	m_BtSave;
	CEdit	m_EditGeoKeys;
	CEdit	m_TagValue;
	CComboBox	m_CmbSupportedTag;
	CListCtrl	m_GeoCodingInfoList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GeoCodingInfoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:

	// Generated message map functions
	//{{AFX_MSG(GeoCodingInfoDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCmbTagList();
	afx_msg void OnBtSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private :
    // Disabled method...
    GeoCodingInfoDialog();
    GeoCodingInfoDialog(const GeoCodingInfoDialog& pi_rSrc);
    GeoCodingInfoDialog& operator=(const GeoCodingInfoDialog& pi_rSrc);

    void FeedTagListCtrl();

    HFCPtr<HRFRasterFile >& m_prRasterFile;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __GeoCodingInfoDialog_H__
