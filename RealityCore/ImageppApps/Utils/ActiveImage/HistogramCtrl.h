/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HistogramCtrl.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// HistogramCtrl.h : header file
//-----------------------------------------------------------------------------

#ifndef __HistogramCtrl_H__
#define __HistogramCtrl_H__

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include <Imagepp/all/h/HRPHistogram.h>

class HistogramCtrl : public CStatic
{
public:
    enum DISPLAY_CHANNEL
    {
        CH_RGB = 0,
        CH_RED,
        CH_GREEN,
        CH_BLUE,
    };

    HistogramCtrl();
    virtual ~HistogramCtrl();

    void SetHistogramData(const HFCPtr<HRPHistogram >& pi_rpHistogram, HistogramCtrl::DISPLAY_CHANNEL pi_DisplayChannel);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HistogramCtrl)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(HistogramCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private :
    long  GetMaxFrequency(const HFCPtr<HRPHistogram >& pi_pHistogram, int pi_ChannelIndex) const;
    int   GetChannelIndex() const;

    CRect DrawHistogramPannel(CPaintDC& pio_PaintDC) const;

    void  RenderHistogramCurve(CPaintDC& pio_PaintDC, CRect pi_CurveRect) const;
    void  DisplayStats(CPaintDC& pio_PaintDC) const;

    HFCPtr<HRPHistogram > m_pHistogram;

    bool   m_MouseIsCaptured;
    CPoint m_MousePosition;

    int    m_StatsDisplayRect;
    bool   m_DisplayStat;

    DISPLAY_CHANNEL m_DisplayChannel;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __HistogramCtrl_H__
