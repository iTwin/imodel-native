/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorSpaceTest.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------------
// ColorSpaceTest dialog
//------------------------------------------------------------------------------------

#pragma once

#include "afxwin.h"
#include "ColorButtonCtrl.h"
#include "ColorEditBox.h"
#include "afxcmn.h"

class ColorSpaceTest : public CDialog
{
	DECLARE_DYNAMIC(ColorSpaceTest)

public:
	ColorSpaceTest(CWnd* pParent = NULL);   // standard constructor
	virtual ~ColorSpaceTest();

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtSrcColor();
    afx_msg void OnBnClickedBtCompute();
    afx_msg void OnBnClickedBtClose();
    afx_msg void OnNMCustomdrawSliderLAdjust(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMReleasedcaptureSliderLAdjust(NMHDR *pNMHDR, LRESULT *pResult);

    LRESULT OnColorButtonRClicked(WPARAM wParam, LPARAM lParam);

	enum { IDD = IDD_DLG_COLORSPACE_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    
private:
    
    void SetSrcColor();

    CEdit m_EditSrcRed;
    CEdit m_EditSrcGreen;
    CEdit m_EditSrcBlue;

    CEdit m_EditSrcL;
    CEdit m_EditSrcu;
    CEdit m_EditSrcv;

    CEdit m_EditDstRed;
    CEdit m_EditDstGreen;
    CEdit m_EditDstBlue;

    ColorEditBox m_EditDstL;
    ColorEditBox m_EditDstu;
    ColorEditBox m_EditDstv;

    CEdit m_EditDstRed2;
    CEdit m_EditDstGreen2;
    CEdit m_EditDstBlue2;

    CEdit m_EditDstL2;
    CEdit m_EditDstu2;
    CEdit m_EditDstv2;

    CEdit m_EditDstRed3;
    CEdit m_EditDstGreen3;
    CEdit m_EditDstBlue3;

    CEdit m_EditDstL3;
    CEdit m_EditDstu3;
    CEdit m_EditDstv3;

    CEdit m_EditNewL;

    ColorButtonCtrl m_BtSrcColor;
    ColorButtonCtrl m_BtDestColor;
    ColorButtonCtrl m_BtDestColor2;
    ColorButtonCtrl m_BtDestColor3;

    HGFLuvColorSpace* m_pColorSpaceConverter;

    double GrayLigthnessValues[256];

    COLORREF m_SrcColor;
    CSliderCtrl m_SliderLAdust;
public:
    afx_msg void OnBnClickedBtCompute2();
    afx_msg void OnEnChangeEditNewL();
    afx_msg void OnBnClickedBtCompute3();
};
