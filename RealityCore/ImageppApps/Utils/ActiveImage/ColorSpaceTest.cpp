/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorSpaceTest.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------------
// ColorSpaceTest.cpp : implementation file
//------------------------------------------------------------------------------------

#include "stdafx.h"
#include "ActiveImage.h"
#include <Imagepp/all/h/HGFLuvColorSpace.h>
#include "ColorSpaceTest.h"
#include ".\colorspacetest.h"

//------------------------------------------------------------------------------------
// ColorSpaceTest dialog
//------------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(ColorSpaceTest, CDialog)
ColorSpaceTest::ColorSpaceTest(CWnd* pParent /*=NULL*/)
	: CDialog(ColorSpaceTest::IDD, pParent)
{
    m_pColorSpaceConverter = new HGFLuvColorSpace(2.2, 8);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

ColorSpaceTest::~ColorSpaceTest()
{
    delete m_pColorSpaceConverter;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SRC_RED   , m_EditSrcRed);
    DDX_Control(pDX, IDC_EDIT_SRC_GREEN , m_EditSrcGreen);
    DDX_Control(pDX, IDC_EDIT_SRC_BLUE  , m_EditSrcBlue);
    DDX_Control(pDX, IDC_EDIT_SRC_L , m_EditSrcL);
    DDX_Control(pDX, IDC_EDIT_SRC_U , m_EditSrcu);
    DDX_Control(pDX, IDC_EDIT_SRC_V , m_EditSrcv);

    DDX_Control(pDX, IDC_EDIT_DEST_L , m_EditDstL);
    DDX_Control(pDX, IDC_EDIT_DEST_U , m_EditDstu);
    DDX_Control(pDX, IDC_EDIT_DEST_V , m_EditDstv);
    DDX_Control(pDX, IDC_EDIT_NEW_L  , m_EditNewL);
    DDX_Control(pDX, IDC_EDIT_DEST_RED  , m_EditDstRed);
    DDX_Control(pDX, IDC_EDIT_DEST_GREEN, m_EditDstGreen);
    DDX_Control(pDX, IDC_EDIT_DEST_BLUE , m_EditDstBlue);

    DDX_Control(pDX, IDC_EDIT_DEST_RED2  , m_EditDstRed2);
    DDX_Control(pDX, IDC_EDIT_DEST_GREEN2, m_EditDstGreen2);
    DDX_Control(pDX, IDC_EDIT_DEST_BLUE2 , m_EditDstBlue2);
    DDX_Control(pDX, IDC_EDIT_DEST_L2 , m_EditDstL2);
    DDX_Control(pDX, IDC_EDIT_DEST_U2 , m_EditDstu2);
    DDX_Control(pDX, IDC_EDIT_DEST_V2 , m_EditDstv2);

    DDX_Control(pDX, IDC_EDIT_DEST_RED3  , m_EditDstRed3);
    DDX_Control(pDX, IDC_EDIT_DEST_GREEN3, m_EditDstGreen3);
    DDX_Control(pDX, IDC_EDIT_DEST_BLUE3 , m_EditDstBlue3);
    DDX_Control(pDX, IDC_EDIT_DEST_L3 , m_EditDstL3);
    DDX_Control(pDX, IDC_EDIT_DEST_U3 , m_EditDstu3);
    DDX_Control(pDX, IDC_EDIT_DEST_V3 , m_EditDstv3);

    DDX_Control(pDX, IDC_BT_SRC_COLOR  , m_BtSrcColor);
    DDX_Control(pDX, IDC_BT_DEST_COLOR , m_BtDestColor);
    DDX_Control(pDX, IDC_BT_DEST_COLOR2, m_BtDestColor2);
    DDX_Control(pDX, IDC_BT_DEST_COLOR3, m_BtDestColor3);
    DDX_Control(pDX, IDC_SLIDER_L_ADJUST, m_SliderLAdust);
}

//------------------------------------------------------------------------------------
// ColorSpaceTest message handlers
//------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ColorSpaceTest, CDialog)
    ON_BN_CLICKED(IDC_BT_SRC_COLOR, OnBnClickedBtSrcColor)
    ON_BN_CLICKED(IDC_BT_CLOSE, OnBnClickedBtClose)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_L_ADJUST, OnNMReleasedcaptureSliderLAdjust)
    ON_MESSAGE(WM_BUTTON_MOUSE_RCLICKED, OnColorButtonRClicked)
    ON_BN_CLICKED(IDC_BT_COMPUTE2, OnBnClickedBtCompute2)
    ON_EN_CHANGE(IDC_EDIT_NEW_L, OnEnChangeEditNewL)
    ON_BN_CLICKED(IDC_BT_COMPUTE3, OnBnClickedBtCompute3)
END_MESSAGE_MAP()

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------

BOOL ColorSpaceTest::OnInitDialog() 
{
	CDialog::OnInitDialog();

    m_SliderLAdust.SetRange(0, 100);

    m_SrcColor = 0x000000;
    
    SetSrcColor();
    m_BtDestColor2.SetBGColor(m_SrcColor);
    m_BtDestColor3.SetBGColor(m_SrcColor);

    // Just correct for calling purposed, value are to be discarded.
    double U, V;    
    for (int GrayValue = 0; GrayValue <= 255; GrayValue++)
         m_pColorSpaceConverter->ConvertFromRGB((Byte)GrayValue, 
                                                (Byte)GrayValue, 
                                                (Byte)GrayValue, 
                                                &GrayLigthnessValues[GrayValue],
                                                &U, 
                                                &V);
    OnBnClickedBtCompute();
    
	return true;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnBnClickedBtSrcColor()
{
    CColorDialog ColorPicker(m_SrcColor, CC_FULLOPEN, this);
    
    if (ColorPicker.DoModal() == IDOK)
    {
        m_SrcColor = ColorPicker.GetColor();
        SetSrcColor();
        OnBnClickedBtCompute();
    }
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnBnClickedBtCompute()
{
    CString DisplayBuffer;
    
    m_EditNewL.GetWindowText(DisplayBuffer);
    double NewL = _tstof(DisplayBuffer);

    m_EditDstu.GetWindowText(DisplayBuffer);
    double Newu = _tstof(DisplayBuffer);

    m_EditDstv.GetWindowText(DisplayBuffer);
    double Newv = _tstof(DisplayBuffer);

    Byte NewRed;
    Byte NewGreen;
    Byte NewBlue;

    m_pColorSpaceConverter->ConvertToRGB(NewL, Newu, Newv, &NewRed, &NewGreen, &NewBlue);

    DisplayBuffer.Format(L"%f", NewL);
    m_EditDstL.SetWindowText(DisplayBuffer);
    
    DisplayBuffer.Format(L"%u", NewRed);
    m_EditDstRed.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", NewGreen);
    m_EditDstGreen.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", NewBlue);
    m_EditDstBlue.SetWindowText(DisplayBuffer);

    m_BtDestColor.SetBGColor(RGB(NewRed, NewGreen, NewBlue));
    m_BtDestColor.RedrawWindow();

    // Show to the user we fall out of gammut..
    if (!m_pColorSpaceConverter->SafeConvertToRGB(NewL, Newu, Newv, &NewRed, &NewGreen, &NewBlue))
    {
        // Out of gammut.
        m_EditDstL.SetBGColor(RGB(0xFF, 0x00, 0x00));
        m_EditDstu.SetBGColor(RGB(0xFF, 0x00, 0x00));
        m_EditDstv.SetBGColor(RGB(0xFF, 0x00, 0x00));
    }
    else
    {
        m_EditDstL.SetBGColor(RGB(0xFF, 0xFF, 0xFF));
        m_EditDstu.SetBGColor(RGB(0xFF, 0xFF, 0xFF));
        m_EditDstv.SetBGColor(RGB(0xFF, 0xFF, 0xFF));
    }
    m_EditDstL.RedrawWindow();
    m_EditDstu.RedrawWindow();
    m_EditDstv.RedrawWindow();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnBnClickedBtClose()
{
    OnOK();
}


//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::SetSrcColor()
{
    HPRECONDITION(m_pColorSpaceConverter != 0);

    CString DisplayBuffer;

    DisplayBuffer.Format(L"%u", GetRValue(m_SrcColor));
    m_EditSrcRed.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetGValue(m_SrcColor));
    m_EditSrcGreen.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetBValue(m_SrcColor));
    m_EditSrcBlue.SetWindowText(DisplayBuffer);

    m_BtSrcColor.SetBGColor(m_SrcColor);
    m_BtSrcColor.RedrawWindow();

    // Convert them to L*u*v*
    double L;
    double u;
    double v;

    m_pColorSpaceConverter->ConvertFromRGB(GetRValue(m_SrcColor), GetGValue(m_SrcColor), GetBValue(m_SrcColor),
                                           &L,                    &u,                    &v);

    DisplayBuffer.Format(L"%f", L);
    m_EditSrcL.SetWindowText(DisplayBuffer);
    m_EditDstL.SetWindowText(DisplayBuffer);
    m_EditNewL.SetWindowText(DisplayBuffer);

    m_SliderLAdust.SetPos((INT)L);

    DisplayBuffer.Format(L"%f", u);
    m_EditSrcu.SetWindowText(DisplayBuffer);
    m_EditDstu.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%f", v);
    m_EditSrcv.SetWindowText(DisplayBuffer);
    m_EditDstv.SetWindowText(DisplayBuffer);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnNMReleasedcaptureSliderLAdjust(NMHDR *pNMHDR, LRESULT *pResult)
{
    DOUBLE NewL = m_SliderLAdust.GetPos();

    CString DisplayBuffer;
    DisplayBuffer.Format(L"%f", NewL);
    m_EditNewL.SetWindowText(DisplayBuffer);

    OnBnClickedBtCompute();

    *pResult = 0;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnNMCustomdrawSliderLAdjust(NMHDR *pNMHDR, LRESULT *pResult)
{
    DOUBLE NewL = m_SliderLAdust.GetPos();

    CString DisplayBuffer;
    DisplayBuffer.Format(L"%f", NewL);
    m_EditNewL.SetWindowText(DisplayBuffer);
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

LRESULT ColorSpaceTest::OnColorButtonRClicked(WPARAM wParam, LPARAM lParam)
{
    SetSrcColor();
    OnBnClickedBtCompute();    

    return 0L;
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnBnClickedBtCompute2()
{
    CWaitCursor ShowWeAreWaiting;

    // For all possible color, find the nearest one!
    CString DisplayBuffer;

    m_EditDstL.GetWindowText(DisplayBuffer);
    double RefL = _tstof(DisplayBuffer);
    
    m_EditDstu.GetWindowText(DisplayBuffer);
    double Refu = _tstof(DisplayBuffer);

    m_EditDstv.GetWindowText(DisplayBuffer);
    double Refv = _tstof(DisplayBuffer);

    Byte BestRed;
    Byte BestGreen;
    Byte BestBlue;

    double L, u, v;
    double Distance;
    double SmallestDistance = 1000000000;

    for (int BlueIndex = 0; BlueIndex <= 255; BlueIndex++)
    {
        for (int GreenIndex = 0; GreenIndex <= 255; GreenIndex++)
        {
            for (int RedIndex = 0; RedIndex <= 255; RedIndex++)
            {
                m_pColorSpaceConverter->ConvertFromRGB( (Byte)RedIndex, (Byte)GreenIndex, (Byte)BlueIndex, &L, &u, &v);

                // Mesure the distance in 3D.. 
                Distance = m_pColorSpaceConverter->EuclideanColorDifference(RefL, Refu, Refv, L, u, v);

                if (Distance < SmallestDistance)
                {
                    BestRed   = (Byte)RedIndex;
                    BestGreen = (Byte)GreenIndex;
                    BestBlue  = (Byte)BlueIndex;

                    SmallestDistance = Distance;
                }
            }
        }
    }

    // We have now the nearest RGB value, so show the result!!
    DisplayBuffer.Format(L"%u", BestRed);
    m_EditDstRed2.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", BestGreen);
    m_EditDstGreen2.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", BestBlue);
    m_EditDstBlue2.SetWindowText(DisplayBuffer);

    m_pColorSpaceConverter->ConvertFromRGB(BestRed, BestGreen, BestBlue, &L, &u, &v);
    m_BtDestColor2.SetBGColor(RGB(BestRed, BestGreen, BestBlue));

    DisplayBuffer.Format(L"%f", L);
    m_EditDstL2.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%f", u);
    m_EditDstu2.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%f", v);
    m_EditDstv2.SetWindowText(DisplayBuffer);

    m_BtDestColor2.RedrawWindow();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorSpaceTest::OnEnChangeEditNewL()
{
    OnBnClickedBtCompute();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

// ITU-R Recommendation  BT.709, Basic Parameter Values for the HDTV Standard 
// for the Studio and for International Programme Exchange (1990), 
// [formerly CCIR Rec.  709], ITU, 1211  Geneva  20, Switzerland.
#define REDFACTOR   (0.2125)
#define GREENFACTOR (0.7154)
#define BLUEFACTOR  (0.0721)


void ColorSpaceTest::OnBnClickedBtCompute3()
{
    CWaitCursor ShowWeAreWaiting;

    // For all possible color, find the nearest one!
    CString DisplayBuffer;

    m_EditDstL.GetWindowText(DisplayBuffer);
    double RefL = _tstof(DisplayBuffer);
    
    m_EditDstu.GetWindowText(DisplayBuffer);

    m_EditDstv.GetWindowText(DisplayBuffer);

    Byte BestRed;
    Byte BestGreen;
    Byte BestBlue;

    int GrayIndex = 0;

    // Find nearest L Value..
    while (RefL > GrayLigthnessValues[GrayIndex])
        GrayIndex++;

    HASSERT(GrayIndex < 256);

    int Tot = (int)((GetRValue(m_SrcColor) * REDFACTOR) + (GetGValue(m_SrcColor) * GREENFACTOR) + (GetBValue(m_SrcColor) * BLUEFACTOR));

    BestRed   = (Byte)(__min(GrayIndex * GetRValue(m_SrcColor) / Tot, 255.0));
    BestGreen = (Byte)(__min(GrayIndex * GetGValue(m_SrcColor) / Tot, 255.0));
    BestBlue  = (Byte)(__min(GrayIndex * GetBValue(m_SrcColor) / Tot, 255.0));

    // We have now the nearest RGB value, so show the result!!
    DisplayBuffer.Format(L"%u", BestRed);
    m_EditDstRed3.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", BestGreen);
    m_EditDstGreen3.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", BestBlue);
    m_EditDstBlue3.SetWindowText(DisplayBuffer);

    double L, u, v;
    m_pColorSpaceConverter->ConvertFromRGB(BestRed, BestGreen, BestBlue, &L, &u, &v);
    m_BtDestColor3.SetBGColor(RGB(BestRed, BestGreen, BestBlue));
    m_BtDestColor3.RedrawWindow();

    DisplayBuffer.Format(L"%f", L);
    m_EditDstL3.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%f", u);
    m_EditDstu3.SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%f", v);
    m_EditDstv3.SetWindowText(DisplayBuffer);
}
