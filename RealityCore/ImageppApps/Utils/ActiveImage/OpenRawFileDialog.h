/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/OpenRawFileDialog.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(AFX_OPENRAWFILEDIALOG_H__AAB5CCB5_D17A_4C29_BBD7_22457A7F01E0__INCLUDED_)
#define AFX_OPENRAWFILEDIALOG_H__AAB5CCB5_D17A_4C29_BBD7_22457A7F01E0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OpenRawFileDialog.h : header file
//

#include <Imagepp/all/h/HFCPtr.h>

/////////////////////////////////////////////////////////////////////////////
// COpenRawFileDialog dialog

class COpenRawFileDialog : public CDialog
{
// Construction
public:
	COpenRawFileDialog(CWnd* pParent = NULL, 
                       const WChar* pi_pFileName = 0, 
                       const uint32_t pi_Offset = 0,
                       const uint32_t pi_Footer = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpenRawFileDialog)
	enum { IDD = IDD_OPEN_RAWFILE };
	CComboBox	m_PixelTypeCmb;
	uint32_t	m_Height;
	uint32_t	m_Width;
	UINT	m_Footer;
	BOOL	m_UseSisterFile;
	UINT	m_Offset;
	CString	m_FileName;
	CString	m_FileSize;
	BOOL	m_AutoDetect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenRawFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    HFCPtr<HFCURL> m_pFileName;
//    UInt32         m_Offset;

	// Generated message map functions
	//{{AFX_MSG(COpenRawFileDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeWidth();
	afx_msg void OnSwap();
	afx_msg void OnSelchangePixeltype();
	afx_msg void OnChangeOffset();
	afx_msg void OnChangeFooter();
	afx_msg void OnAutodetect();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    bool HasFor      (HFCPtr<HFCURL>&       pi_rpForRasterFile) const;
    void  ReadHGRFile (const HFCPtr<HFCURL>& pi_rpURL, 
                       uint32_t*               po_ImageWidth, 
                       uint32_t*               po_ImageHeight);
    bool ConvertStringToUnsignedLong(const AString& pi_rString, uint32_t* po_pLong) const;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENRAWFILEDIALOG_H__AAB5CCB5_D17A_4C29_BBD7_22457A7F01E0__INCLUDED_)
