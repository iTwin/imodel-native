//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/HistogramCtrl.cpp $
//:>    $RCSfile: HistogramCtrl.cpp,v $
//:>   $Revision: 1.4 $
//:>       $Date: 2011/07/18 21:10:47 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "activeimage.h"
#include "HistogramCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// HistogramCtrl
//-----------------------------------------------------------------------------

HistogramCtrl::HistogramCtrl()
{
    m_MouseIsCaptured = false;

    m_DisplayStat      = true;
    m_StatsDisplayRect = 150;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HistogramCtrl::~HistogramCtrl()
{
}

//-----------------------------------------------------------------------------
// HistogramCtrl message handlers
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HistogramCtrl, CStatic)
	//{{AFX_MSG_MAP(HistogramCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::SetHistogramData(const HFCPtr<HRPHistogram >&   pi_rpHistogram,
                                     HistogramCtrl::DISPLAY_CHANNEL pi_DisplayChannel)
{
    m_DisplayChannel = pi_DisplayChannel;
    m_pHistogram     = pi_rpHistogram;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

    CRect MyCtrlRect = DrawHistogramPannel(dc);

    if (m_pHistogram != 0)
    {
        RenderHistogramCurve(dc, MyCtrlRect);
        if (m_MouseIsCaptured)
        {
            dc.FillSolidRect(__min(m_MousePosition.x, MyCtrlRect.right), MyCtrlRect.Height(), 1, -MyCtrlRect.Height(), 0xFF8754);
        }
        if (m_DisplayStat)
        {
            DisplayStats(dc);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

long HistogramCtrl::GetMaxFrequency(const HFCPtr<HRPHistogram >& pi_pHistogram, int pi_ChannelIndex) const
{
    uint32_t MaxFreq = 0;
    if (m_pHistogram != 0)
    {
        uint32_t FrequenciesSize = pi_pHistogram->GetEntryFrequenciesSize(pi_ChannelIndex);

        for (uint32_t Freq = 0; Freq < FrequenciesSize; Freq++)
        {
            if (pi_pHistogram->GetEntryCount(Freq, pi_ChannelIndex) > MaxFreq)
                MaxFreq = pi_pHistogram->GetEntryCount(Freq, pi_ChannelIndex);
        }
    }
    return MaxFreq;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CRect HistogramCtrl::DrawHistogramPannel(CPaintDC& pio_PaintDC) const
{
    CRect MyCtrlRect;
    int   XAxisHeight = 10;

    // Draw Histogram Rect.
    GetClientRect(MyCtrlRect);

    // Remove the Stats Rect.
    if (m_DisplayStat)
        MyCtrlRect.DeflateRect( 0, 0, m_StatsDisplayRect + 5, 0);

    pio_PaintDC.FillSolidRect(MyCtrlRect, 0xFFFFFF);

    // Draw the axes and the scale..
    CRect BottomRect(MyCtrlRect);
    BottomRect.DeflateRect( 0, MyCtrlRect.Height() - XAxisHeight, 0, 0);
    pio_PaintDC.FillSolidRect(BottomRect, 0xC0C0C0);
    pio_PaintDC.Draw3dRect( BottomRect, 0xFFFFFF, 0x000000);

    // Set Histogram client region.
    MyCtrlRect.DeflateRect( 3, 0, 3, XAxisHeight);

    return MyCtrlRect;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::RenderHistogramCurve(CPaintDC& pio_PaintDC, CRect pi_CurveRect) const
{
    CRect BarRect;

    int ChannelIndex = GetChannelIndex();
    HFCPtr<HRPHistogram > pHistogram;

    if (ChannelIndex < 0)
    {
        // Simply combine all Hitograms together.
        pHistogram = new HRPHistogram(m_pHistogram->GetEntryFrequenciesSize(0), 1);

        for (uint32_t ChIndex=0; ChIndex < m_pHistogram->GetChannelCount();  ChIndex++)
        {
            for (uint32_t FreqIndex=0; FreqIndex < m_pHistogram->GetEntryFrequenciesSize(ChIndex); FreqIndex++)
            {
                pHistogram->IncrementEntryCount(FreqIndex, m_pHistogram->GetEntryCount(FreqIndex, ChIndex), 0);
            }
        }
        ChannelIndex = 0;
    }
    else
    {
        pHistogram = m_pHistogram;
    }
        
    // Render Histogram values.
    size_t FrequenciesSize = pHistogram->GetEntryFrequenciesSize(ChannelIndex);
    double BarWidth = pi_CurveRect.Width() / (double)FrequenciesSize;
    double XLeftBar  = 0;
    double BarHeight;

    double MaxFreq = GetMaxFrequency(pHistogram, ChannelIndex);
    double BarYRatio = pi_CurveRect.Height() / MaxFreq;

    for (unsigned long Freq = 0; Freq < FrequenciesSize; Freq++)
    {
        XLeftBar  = __min(((Freq * BarWidth) + BarWidth), pi_CurveRect.Width());
        BarHeight = pHistogram->GetEntryCount(Freq, ChannelIndex) * BarYRatio;

        pio_PaintDC.FillSolidRect((int)XLeftBar, pi_CurveRect.Height(), (int)ceil(BarWidth), (int)-BarHeight, 0x72A90);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::DisplayStats(CPaintDC& pio_PaintDC) const
{
    CBrush CurrentBrush;
    CRect  MyCtrlRect;

    GetClientRect(MyCtrlRect);

    // Remove the Histogram Rect.
    if (m_DisplayStat)
        MyCtrlRect.DeflateRect( MyCtrlRect.Width() - m_StatsDisplayRect + 3, 0, 0, 0);
    
    CurrentBrush.CreateSolidBrush( 0x000000 );
    
    // Erase the Background
    pio_PaintDC.FillSolidRect(MyCtrlRect, 0xFFFFFF);

    // pio_PaintDC.SelectObject(CurrentBrush);
    pio_PaintDC.FrameRect( MyCtrlRect, &CurrentBrush);

    MyCtrlRect.DeflateRect( 5, 10, 0, 0);

    CString DisplayString = "Stat";
    
    // pio_PaintDC.TextOut(MyCtrlRect.left, MyCtrlRect.top, DisplayString, DisplayString.GetLength());
    pio_PaintDC.DrawText( DisplayString, MyCtrlRect, DT_VCENTER);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    m_MouseIsCaptured = true;
    m_MousePosition = point;
    
    // Force to redraw the Histogram Rect.
    Invalidate(true);
	
	// CStatic::OnLButtonDown(nFlags, point);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
    m_MouseIsCaptured = false;
    Invalidate(true);
	// CStatic::OnLButtonUp(nFlags, point);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HistogramCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_MouseIsCaptured)
    {
        m_MousePosition = point;
        Invalidate(true);
    }
    else
    {
	    CStatic::OnMouseMove(nFlags, point);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int HistogramCtrl::GetChannelIndex() const
{
    int ChannelIndex = -1; // CH_RGB
    
    if (m_DisplayChannel == CH_RED)
        ChannelIndex = 0;
    else if (m_DisplayChannel == CH_GREEN)
        ChannelIndex = 1;
    else if (m_DisplayChannel == CH_BLUE)
        ChannelIndex = 2;

    //     CH_RGB
    return ChannelIndex;
}