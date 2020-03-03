/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HistogramScaling.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// HistogramScaling.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "activeimage.h"
#include "HistogramScaling.h"
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include ".\histogramscaling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// CHistogramScaling dialog
//-----------------------------------------------------------------------------

CHistogramScaling::CHistogramScaling(const HRAHistogramOptions* pi_pHistogramOptions, FilterType filterType, UINT nbChannels, CWnd* pParent /*=NULL*/)
	: CDialog(CHistogramScaling::IDD, pParent), 
      m_pHistogramOptions(pi_pHistogramOptions)
{
    HASSERT(nbChannels > 0 && nbChannels <= MAX_CHANNELS);

	//{{AFX_DATA_INIT(CHistogramScaling)
	m_MaxValue = 255;
	m_MinValue = 0;
	m_ScalingMode = 0;
	//}}AFX_DATA_INIT

    m_OpacityValue      = 50;
    m_DesaturationValue = 50;
    m_StartColor = 0x000000;
    m_EndColor = 0xFFFFFF;

    for (UINT i = 0; i < MAX_CHANNELS; i++)
    {
        m_IntervalMin[i] = 0.0;
        m_IntervalMax[i] = 255.0;
        m_ContrastMin[i] = 0.0;
        m_ContrastMax[i] = 255.0;
        m_GammaFactor[i] = 1.0;
    }

    m_FilterType = filterType;
    m_nbChannels = nbChannels;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CHistogramScaling)
    DDX_Control(pDX, IDC_HISTOGRAM_DISPLAY, m_HistogramDisplay);
    DDX_Text(pDX, IDC_ED_MAX_VALUE, m_MaxValue);
    DDV_MinMaxUInt(pDX, m_MaxValue, 0, 255);
    DDX_Text(pDX, IDC_ED_MIN_VALUE, m_MinValue);
    DDV_MinMaxUInt(pDX, m_MinValue, 0, 255);
    DDX_Radio(pDX, IDC_RD_INPUT_RANGE_CLIPPING, m_ScalingMode);
    DDX_Control(pDX, IDC_CMB_CHANNEL, m_ComboChannel);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_SLIDER_OPACITY, m_OpacitySlider);
    DDX_Control(pDX, IDC_SLIDER_DESATURATION, m_DesaturationSlider);
    DDX_Control(pDX, IDC_EDIT_OPACITY, m_EditOpacity);
    DDX_Control(pDX, IDC_EDIT_OPACITY2, m_EditDesaturation);

    DDX_Control(pDX, IDC_BUTTON_START_COLOR, m_BtStartColor);
    DDX_Control(pDX, IDC_BUTTON_END_COLOR,   m_BtEndColor);

    DDX_Text(pDX, IDC_ED_INTERVAL_MIN0, m_IntervalMin[0]);
    DDV_MinMaxFloat(pDX, m_IntervalMin[0], 0, 255);
    DDX_Text(pDX, IDC_ED_INTERVAL_MIN1, m_IntervalMin[1]);
    DDV_MinMaxFloat(pDX, m_IntervalMin[1], 0, 255);
    DDX_Text(pDX, IDC_ED_INTERVAL_MIN2, m_IntervalMin[2]);
    DDV_MinMaxFloat(pDX, m_IntervalMin[2], 0, 255);

    DDX_Text(pDX, IDC_ED_INTERVAL_MAX0, m_IntervalMax[0]);
    DDV_MinMaxFloat(pDX, m_IntervalMax[0], 0, 255);
    DDX_Text(pDX, IDC_ED_INTERVAL_MAX1, m_IntervalMax[1]);
    DDV_MinMaxFloat(pDX, m_IntervalMax[1], 0, 255);
    DDX_Text(pDX, IDC_ED_INTERVAL_MAX2, m_IntervalMax[2]);
    DDV_MinMaxFloat(pDX, m_IntervalMax[2], 0, 255);

    DDX_Text(pDX, IDC_ED_CONTRAST_MIN0, m_ContrastMin[0]);
    DDV_MinMaxFloat(pDX, m_ContrastMin[0], 0, 255);
    DDX_Text(pDX, IDC_ED_CONTRAST_MIN1, m_ContrastMin[1]);
    DDV_MinMaxFloat(pDX, m_ContrastMin[1], 0, 255);
    DDX_Text(pDX, IDC_ED_CONTRAST_MIN2, m_ContrastMin[2]);
    DDV_MinMaxFloat(pDX, m_ContrastMin[2], 0, 255);

    DDX_Text(pDX, IDC_ED_CONTRAST_MAX0, m_ContrastMax[0]);
    DDV_MinMaxFloat(pDX, m_ContrastMax[0], 0, 255);
    DDX_Text(pDX, IDC_ED_CONTRAST_MAX1, m_ContrastMax[1]);
    DDV_MinMaxFloat(pDX, m_ContrastMax[1], 0, 255);
    DDX_Text(pDX, IDC_ED_CONTRAST_MAX2, m_ContrastMax[2]);
    DDV_MinMaxFloat(pDX, m_ContrastMax[2], 0, 255);

    DDX_Text(pDX, IDC_ED_GAMMA_FACTOR0, m_GammaFactor[0]);
    DDV_MinMaxDouble(pDX, m_GammaFactor[0], 0.001, 10);
    DDX_Text(pDX, IDC_ED_GAMMA_FACTOR1, m_GammaFactor[1]);
    DDV_MinMaxDouble(pDX, m_GammaFactor[1], 0.001, 10);
    DDX_Text(pDX, IDC_ED_GAMMA_FACTOR2, m_GammaFactor[2]);
    DDV_MinMaxDouble(pDX, m_GammaFactor[2], 0.001, 10);

    DDX_Control(pDX, IDC_ED_START_COLOR_R   , m_EditStartColorRed);
    DDX_Control(pDX, IDC_ED_START_COLOR_G   , m_EditStartColorGreen);
    DDX_Control(pDX, IDC_ED_START_COLOR_B   , m_EditStartColorBlue);
    DDX_Control(pDX, IDC_ED_END_COLOR_R     , m_EditEndColorRed);
    DDX_Control(pDX, IDC_ED_END_COLOR_G     , m_EditEndColorGreen);
    DDX_Control(pDX, IDC_ED_END_COLOR_B     , m_EditEndColorBlue);

    DDX_Control(pDX, IDC_STATIC_GROUP_DENSITY_SLICING, m_GroupDensitySlicing);
    DDX_Control(pDX, IDC_STATIC_GROUP_CONTRAST_STRETCH, m_GroupContrastStretch);
}

//-----------------------------------------------------------------------------
// CHistogramScaling message handlers
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CHistogramScaling, CDialog)
	//{{AFX_MSG_MAP(CHistogramScaling)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_START_COLOR, OnBnClickedBtStartColor)
    ON_BN_CLICKED(IDC_BUTTON_END_COLOR,   OnBnClickedBtEndColor)
	//}}AFX_MSG_MAP
    ON_CBN_SELCHANGE(IDC_CMB_CHANNEL, OnCbnSelchangeCmbChannel)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OPACITY, OnNMCustomdrawSliderOpacity)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_DESATURATION, OnNMCustomdrawSliderDesaturation)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BOOL CHistogramScaling::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillChannelCombo();
    m_ComboChannel.SetCurSel(__min(m_ComboChannel.GetCount() - 1, 1));

    m_OpacitySlider.SetRange( 0, 100, true);
    m_OpacitySlider.SetPos(m_OpacityValue);

    m_DesaturationSlider.SetRange( 0, 100, true);
    m_DesaturationSlider.SetPos(m_DesaturationValue);

    DisplayHistogram();

    SetColor(&m_BtStartColor, m_StartColor, &m_EditStartColorRed, &m_EditStartColorGreen, &m_EditStartColorBlue);
    SetColor(&m_BtEndColor, m_EndColor, &m_EditEndColorRed, &m_EditEndColorGreen, &m_EditEndColorBlue);

    EnableDialogItems(m_FilterType);

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CHistogramScaling::EnableDialogItems(FilterType filterType)
{
    bool enableContrastGroup = false;

    // Assign group names according to the selected filter
    CString GroupName;
    CString GroupNameUnused = "Unused";
    switch (filterType)
    {
        case FILTER_CONTRAST_STRETCH:
            m_GroupDensitySlicing.SetWindowText(GroupNameUnused);
            GroupName = "Contrast Stretch";
            m_GroupContrastStretch.SetWindowText(GroupName);
            enableContrastGroup = true;
        break;
        case FILTER_LIGHTNESS_CONTRAST_STRETCH:
            m_GroupDensitySlicing.SetWindowText(GroupNameUnused);
            GroupName = "Lightness Contrast Stretch";
            m_GroupContrastStretch.SetWindowText(GroupName);
            enableContrastGroup = true;
        break;
        case FILTER_DENSITY_SLICING:
            GroupName = "Density Slicing";
            m_GroupDensitySlicing.SetWindowText(GroupName);
            m_GroupContrastStretch.SetWindowText(GroupNameUnused);
            enableContrastGroup = false;
        break;
        case FILTER_LIGHTNESS_DENSITY_SLICING:
            GroupName = "Lightness Density Slicing";
            m_GroupDensitySlicing.SetWindowText(GroupName);
            m_GroupContrastStretch.SetWindowText(GroupNameUnused);
            enableContrastGroup = false;
        break;
    };

    //
    // Enable / disable Density Slicing group
    //
    GetDlgItem(IDC_STATIC_GROUP_DENSITY_SLICING)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_START_INDEX)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_END_INDEX)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_MAX_VALUE)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_MIN_VALUE)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_START_COLOR)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_END_COLOR)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_START_COLOR_R)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_START_COLOR_G)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_START_COLOR_B)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_END_COLOR_R)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_END_COLOR_G)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_ED_END_COLOR_B)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_BUTTON_START_COLOR)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_BUTTON_END_COLOR)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_OPACITY)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_STATIC_DESATURATION)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_EDIT_OPACITY)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_EDIT_OPACITY2)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_SLIDER_OPACITY)->EnableWindow(!enableContrastGroup);
    GetDlgItem(IDC_SLIDER_DESATURATION)->EnableWindow(!enableContrastGroup);

    //
    // Enable / disable contrast group
    //
    GetDlgItem(IDC_STATIC_GROUP_CONTRAST_STRETCH)->EnableWindow(enableContrastGroup);

    // Only enable channel 1, 2 fields if filter type is contrast stretch
    bool enableChannel1And2 = (filterType == FILTER_CONTRAST_STRETCH);

    GetDlgItem(IDC_STATIC_CHANNEL0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_STATIC_CHANNEL1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_STATIC_CHANNEL2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);

    GetDlgItem(IDC_STATIC_MIN_CHANNEL0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_STATIC_MIN_CHANNEL1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_STATIC_MIN_CHANNEL2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);
    GetDlgItem(IDC_STATIC_MAX_CHANNEL0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_STATIC_MAX_CHANNEL1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_STATIC_MAX_CHANNEL2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);

    GetDlgItem(IDC_STATIC_INTERVAL)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_INTERVAL_MIN0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_INTERVAL_MIN1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_ED_INTERVAL_MIN2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);
    GetDlgItem(IDC_ED_INTERVAL_MAX0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_INTERVAL_MAX1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_ED_INTERVAL_MAX2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);

    GetDlgItem(IDC_STATIC_CONTRAST)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_CONTRAST_MIN0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_CONTRAST_MIN1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_ED_CONTRAST_MIN2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);
    GetDlgItem(IDC_ED_CONTRAST_MAX0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_CONTRAST_MAX1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_ED_CONTRAST_MAX2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);

    GetDlgItem(IDC_STATIC_GAMMA_FACTOR)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_GAMMA_FACTOR0)->EnableWindow(enableContrastGroup);
    GetDlgItem(IDC_ED_GAMMA_FACTOR1)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 1);
    GetDlgItem(IDC_ED_GAMMA_FACTOR2)->EnableWindow(enableContrastGroup && enableChannel1And2 && m_nbChannels > 2);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

UINT CHistogramScaling::GetMinValue() const
{
    return m_MinValue;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

UINT CHistogramScaling::GetMaxValue() const
{
    return m_MaxValue;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float CHistogramScaling::GetIntervalMin(UINT ChannelNo) const
{
    return m_IntervalMin[ChannelNo];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float CHistogramScaling::GetIntervalMax(UINT ChannelNo) const
{
    return m_IntervalMax[ChannelNo];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float CHistogramScaling::GetContrastMin(UINT ChannelNo) const
{
    return m_ContrastMin[ChannelNo];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float CHistogramScaling::GetContrastMax(UINT ChannelNo) const
{
    return m_ContrastMax[ChannelNo];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
double CHistogramScaling::GetGammaFactor(UINT ChannelNo) const
{
    return m_GammaFactor[ChannelNo];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::HistogramScalingMode  CHistogramScaling::GetScalingMode() const
{
    if (m_ScalingMode == 0)
        return HRPHistogramScalingFilter::INPUT_RANGE_CLIPPING;
    else
        return HRPHistogramScalingFilter::OUTPUT_RANGE_COMPRESSION;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::DisplayHistogram()
{
    if (m_pHistogramOptions)
    {
        HFCPtr<HRPHistogram> pHistogram = m_pHistogramOptions->GetHistogram();
        m_HistogramDisplay.SetHistogramData(pHistogram, (HistogramCtrl::DISPLAY_CHANNEL)m_ComboChannel.GetItemData(m_ComboChannel.GetCurSel()));
    }
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_HistogramDisplay.SetCapture();
	
	CDialog::OnLButtonDown(nFlags, point);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // m_HistogramDisplay.ReleaseCapture();
	
	CDialog::OnLButtonUp(nFlags, point);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnBnClickedOk()
{
    m_OpacityValue      = m_OpacitySlider.GetPos();
    m_DesaturationValue = m_DesaturationSlider.GetPos();

    OnOK();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CHistogramScaling::OnBnClickedBtStartColor()
{
    CColorDialog ColorPicker(m_StartColor, CC_FULLOPEN, this);
    if (ColorPicker.DoModal() == IDOK)
    {
        m_StartColor = ColorPicker.GetColor();
        SetColor(&m_BtStartColor, m_StartColor, &m_EditStartColorRed, &m_EditStartColorGreen, &m_EditStartColorBlue);
    }
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CHistogramScaling::OnBnClickedBtEndColor()
{
    CColorDialog ColorPicker(m_EndColor, CC_FULLOPEN, this);
    if (ColorPicker.DoModal() == IDOK)
    {
        m_EndColor = ColorPicker.GetColor();
        SetColor(&m_BtEndColor, m_EndColor, &m_EditEndColorRed, &m_EditEndColorGreen, &m_EditEndColorBlue);
    }
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CHistogramScaling::SetColor(ColorButtonCtrl* pButton, COLORREF srcCol, CEdit* pEditRed, CEdit* pEditGreen, CEdit* pEditBlue)
{
    pButton->SetBGColor(srcCol);
    pButton->RedrawWindow();

    CString DisplayBuffer;
    DisplayBuffer.Format(L"%u", GetRValue(srcCol));
    pEditRed->SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetGValue(srcCol));
    pEditGreen->SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetBValue(srcCol));
    pEditBlue->SetWindowText(DisplayBuffer);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::FillChannelCombo()
{
    if(NULL != m_pHistogramOptions)
    {
        if (m_pHistogramOptions->GetHistogram()->GetChannelCount() == 3)
        {
            m_ComboChannel.SetItemData(m_ComboChannel.AddString(_TEXT("RGB"))  , HistogramCtrl::CH_RGB);
            m_ComboChannel.SetItemData(m_ComboChannel.AddString(_TEXT("Red"))  , HistogramCtrl::CH_RED);
            m_ComboChannel.SetItemData(m_ComboChannel.AddString(_TEXT("Green")), HistogramCtrl::CH_GREEN);
            m_ComboChannel.SetItemData(m_ComboChannel.AddString(_TEXT("Blue")) , HistogramCtrl::CH_BLUE);
        }
    }
    else
    {
        m_ComboChannel.SetItemData(m_ComboChannel.AddString(_TEXT("RGB")), HistogramCtrl::CH_RGB);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnCbnSelchangeCmbChannel()
{
    DisplayHistogram();
    Invalidate();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnBnClickedButton1()
{
    // Dump Histogram values..
    CFileDialog HistoDumpFile( false, _TEXT("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _TEXT("Text file(*.txt)|*.txt||"), NULL, 0 );
    HFCPtr<HRPHistogram> pHistogram = m_pHistogramOptions->GetHistogram();

    if (pHistogram != 0 && HistoDumpFile.DoModal()== IDOK )
    {
        CString DumpFileName = HistoDumpFile.GetPathName();

        FILE* pHistoFile;
        _tfopen_s(&pHistoFile, DumpFileName, _TEXT("wt+"));
        ASSERT(pHistoFile);

        for (uint32_t ChIndex=0; ChIndex < pHistogram->GetChannelCount();  ChIndex++)
        {
            for (uint32_t FreqIndex=0; FreqIndex < pHistogram->GetEntryFrequenciesSize(ChIndex); FreqIndex++)
            {
                CString Value;
                Value.Format(_TEXT("%ld\n"), pHistogram->GetEntryCount(FreqIndex, ChIndex));
                
                fwrite((LPCTSTR)Value, 1, Value.GetLength(), pHistoFile);
            }
        }
        fclose(pHistoFile);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

UINT CHistogramScaling::GetOpacityValue() const
{
    return m_OpacityValue;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

UINT CHistogramScaling::GetDesaturationValue() const
{
    return m_DesaturationValue;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COLORREF CHistogramScaling::GetStartColor() const
{
    return m_StartColor;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COLORREF CHistogramScaling::GetEndColor() const
{
    return m_EndColor;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnNMCustomdrawSliderOpacity(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString DisplayText;
    DisplayText.Format(L"%d", m_OpacitySlider.GetPos());
    m_EditOpacity.SetWindowText(DisplayText);
    
    *pResult = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CHistogramScaling::OnNMCustomdrawSliderDesaturation(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString DisplayText;
    DisplayText.Format(L"%d", m_DesaturationSlider.GetPos());
    m_EditDesaturation.SetWindowText(DisplayText);

    *pResult = 0;
}
