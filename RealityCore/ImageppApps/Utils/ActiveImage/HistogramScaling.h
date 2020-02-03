/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HistogramScaling.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// HistogramScaling.h : header file
//-----------------------------------------------------------------------------

#ifndef __CHistogramScaling_H__
#define __CHistogramScaling_H__

#if _MSC_VER >= 1000
    #pragma once
#endif // _MSC_VER >= 1000

#include <Imagepp/all/h/HRPMapFilters8.h>
#include "HistogramCtrl.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ColorButtonCtrl.h"

#define MAX_CHANNELS 4

class CHistogramScaling : public CDialog
{
    public:
        // Filter for which histogram is used
        typedef enum
        {
            FILTER_CONTRAST_STRETCH,
            FILTER_LIGHTNESS_CONTRAST_STRETCH,
            FILTER_DENSITY_SLICING,
            FILTER_LIGHTNESS_DENSITY_SLICING,
        } FilterType;

	    CHistogramScaling(const HRAHistogramOptions* pi_pHistogramOptions, FilterType filterType, UINT nbChannels, CWnd* pParent = NULL);   // standard constructor

        UINT GetMinValue() const;
        UINT GetMaxValue() const;
        UINT GetOpacityValue() const;
        UINT GetDesaturationValue() const;
        COLORREF GetStartColor() const;
        COLORREF GetEndColor() const;

        float  GetIntervalMin(UINT ChannelNo) const;
        float  GetIntervalMax(UINT ChannelNo) const;
        float  GetContrastMin(UINT ChannelNo) const;
        float  GetContrastMax(UINT ChannelNo) const;
        double GetGammaFactor(UINT ChannelNo) const;

        void SetColor(ColorButtonCtrl* pButton, COLORREF srcCol, CEdit* pEditRed, CEdit* pEditGreen, CEdit* pEditBlue);

        HRPHistogramScalingFilter::HistogramScalingMode  GetScalingMode() const;

	    //{{AFX_DATA(CHistogramScaling)
	    enum { IDD = IDD_HISTOGRAM_SCALING };
	    HistogramCtrl	m_HistogramDisplay;
	    UINT	        m_MaxValue;
	    UINT	        m_MinValue;
	    int		        m_ScalingMode;
        CComboBox       m_ComboChannel;

        float           m_IntervalMin[MAX_CHANNELS];
        float           m_IntervalMax[MAX_CHANNELS];
        float           m_ContrastMin[MAX_CHANNELS];
        float           m_ContrastMax[MAX_CHANNELS];
        double          m_GammaFactor[MAX_CHANNELS];

        COLORREF        m_StartColor;
        COLORREF        m_EndColor;
        CEdit           m_EditStartColorRed;
        CEdit           m_EditStartColorGreen;
        CEdit           m_EditStartColorBlue;
        CEdit           m_EditEndColorRed;
        CEdit           m_EditEndColorGreen;
        CEdit           m_EditEndColorBlue;
        ColorButtonCtrl m_BtStartColor;
        ColorButtonCtrl m_BtEndColor;

        CEdit m_GroupDensitySlicing;
        CEdit m_GroupContrastStretch;
	    //}}AFX_DATA

	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CHistogramScaling)
	    protected:
	    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	    //}}AFX_VIRTUAL

    protected:

	    // Generated message map functions
	    //{{AFX_MSG(CHistogramScaling)
	    virtual BOOL OnInitDialog();
	    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnBnClickedOk();
	    //}}AFX_MSG
	    DECLARE_MESSAGE_MAP()

    private :
        void DisplayHistogram();
        void FillChannelCombo();

        void EnableDialogItems(FilterType filterType);

        const HRAHistogramOptions* m_pHistogramOptions;

        FilterType m_FilterType;
        UINT       m_nbChannels;
public:
    afx_msg void OnCbnSelchangeCmbChannel();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedBtStartColor();
    afx_msg void OnBnClickedBtEndColor();
    
    CSliderCtrl m_OpacitySlider;
    CSliderCtrl m_DesaturationSlider;

    int   m_OpacityValue;
    int   m_DesaturationValue;
    CEdit m_EditOpacity;
    afx_msg void OnNMCustomdrawSliderOpacity(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSliderDesaturation(NMHDR *pNMHDR, LRESULT *pResult);
    CEdit m_EditDesaturation;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __CHistogramScaling_H__
