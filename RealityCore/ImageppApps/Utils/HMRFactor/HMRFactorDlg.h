/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFactorDlg.h $
|    $RCSfile: HMRFactorDlg.h,v $
|   $Revision: 1.3 $
|       $Date: 2006/02/17 14:26:06 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// HMRFactorDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "NumEditCtrl.h"
#include "HMRListCtrl.h"
#include "UnitComboBox.h"


// HMRFactorDlg dialog
class HMRFactorDlg : public CDialog
{
// Construction
public:
        HMRFactorDlg(CWnd* pParent = NULL);     // standard constructor
        ~HMRFactorDlg ();

// Dialog Data
        enum { IDD = IDD_HMRFACTOR_DIALOG };

        protected:
        virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Implementation
protected:
        HICON m_hIcon;

        // Generated message map functions
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        afx_msg void OnBnClickedAddfiles();
        afx_msg void OnBnClickedRemovefiles();
        afx_msg void OnBnClickedProcess();
        afx_msg void OnBnClickedFrombtn();
        afx_msg void OnBnClickedCustombtn();
        afx_msg void OnNMClickListFiles(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg void OnNMKillfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg void OnLVNItemChangedListFiles(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg void OnOK ();
        afx_msg void OnCbnSetfocusFromcombo();
        afx_msg void OnCbnKillfocusFromcombo();
        afx_msg void OnCbnSetfocusTocombo();
        afx_msg void OnCbnKillfocusTocombo();

        DECLARE_MESSAGE_MAP()

private:
        void CustomFactorMode (bool);
        void SetStatusText (LPCTSTR lpszString);
        
        HMRListCtrl     m_ListFiles;
        NumEditCtrl     m_CustomFactor;
        UnitComboBox   m_FromComboBox;
        UnitComboBox   m_ToComboBox;
        CImageList      m_SmallImageList;

};
