/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/CachePreferences.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(AFX_CACHEPREFERENCES_H__803EEBF4_4093_11D4_B5B2_0060082BD950__INCLUDED_)
#define AFX_CACHEPREFERENCES_H__803EEBF4_4093_11D4_B5B2_0060082BD950__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CachePreferences.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCachePreferencesDlg dialog
#include <Imagepp/all/h/HFCPtr.h>

class CCachePreferencesDlg : public CDialog
{
    // Construction
    public:
    	CCachePreferencesDlg(CWnd* pParent = NULL);   // standard constructor

        // Dialog Data
    	//{{AFX_DATA(CCachePreferencesDlg)
    	enum { IDD = IDD_DIALOG_CACHE };
    	CComboBox	m_CacheFormatCombo;
    	CComboBox	m_CodecCombo;
    	CComboBox	m_PixelTypeCombo;
    	//}}AFX_DATA

        // Overrides
    	// ClassWizard generated virtual function overrides
    	//{{AFX_VIRTUAL(CCachePreferencesDlg)
    protected:
    	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    	//}}AFX_VIRTUAL

    // Implementation
    protected:
        HRFCacheFileCreator* m_pCacheFileCreator;
        
    	// Generated message map functions
    	//{{AFX_MSG(CCachePreferencesDlg)
    	afx_msg void OnSelchangeComboPixeltype();
    	afx_msg void OnSelchangeComboCodec();
    	virtual BOOL OnInitDialog();
    	//}}AFX_MSG
    	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CACHEPREFERENCES_H__803EEBF4_4093_11D4_B5B2_0060082BD950__INCLUDED_)
