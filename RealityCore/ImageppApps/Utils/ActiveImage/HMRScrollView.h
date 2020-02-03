/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRScrollView.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/HMRScrollView.h,v 1.4 2011/07/18 21:10:47 Donald.Morissette Exp $
//
// Class: CHMRScrollView - Supports simple scrolling and scaling with 
//                         positions and ranges in doubles thus allowing a
//                         a range of 1.7E-308 to 1.7E+308 in both positives
//                         and negatives.
//-----------------------------------------------------------------------------

#ifndef __CHMRSCROLLVIEW_H__
#define __CHMRSCROLLVIEW_H__

#include <Imagepp/h/hmrtypes.h>

// Same as Windows SDK's SIZE, but in double
typedef struct _HMRSIZE
{
    double cx;
    double cy;
    _HMRSIZE() 
        { cx = 0.0; cy = 0.0; };
    _HMRSIZE(double pi_CX, double pi_CY)
        { cx = pi_CX; cy = pi_CY; };
    void operator=(const _HMRSIZE& pi_rObj)
    {
        cx = pi_rObj.cx;
        cy = pi_rObj.cy;
    };
} HMRSIZE;

// Same as Windows SDK's POINT, but in double
typedef struct _HMRPOINT
{
    double x;
    double y;
    _HMRPOINT() 
        { x = 0.0; y = 0.0; };
    _HMRPOINT(double pi_X, double pi_Y)
        { x = pi_X; y = pi_Y; };
    void operator=(const _HMRPOINT& pi_rObj)
    {
        x = pi_rObj.x;
        y = pi_rObj.y;
    };
} HMRPOINT;


class CHMRScrollView : public CView
{
	DECLARE_DYNAMIC(CHMRScrollView)

// Constructors
protected:
	CHMRScrollView();

public:

	// in logical units - call one of the following Set routines
	void SetScrollSizes(double pi_MinX, double pi_MaxX,
                        double pi_MinY, double pi_MaxY,
                        double pi_SizePageX = 0.0, double pi_SizePageY = 0.0,
                        double pi_SizeLineX = 0.0, double pi_SizePineY = 0.0);

// Attributes
public:
    // for logical units
	HMRPOINT GetScrollPosition() const;       // upper corner of scrolling
    void     GetScrollSizes(double& pi_rMinX, double& pi_rMaxX,
                            double& pi_rMinY, double& pi_rMaxY,
                            double& pi_rSizePageX, double& pi_rSizePageY,
                            double& pi_rSizeLineX, double& pi_rSizeLineY) const;

	// for device units
	CPoint   GetDeviceScrollPosition() const;
	void     GetDeviceScrollSizes(SIZE& sizeTotal, 
                                  SIZE& sizePage, 
                                  SIZE& sizeLine) const;


// Operations
public:
	void ScrollToPosition(HMRPOINT pt);    // set upper left position
	void FillOutsideRect(CDC* pDC, CBrush* pBrush);
	void ResizeParentToFit(BOOL bShrinkOnly = true);

// Scrolling methods that uses doubles instead of ints.  The must
// have a new name, because the CWnd functions are not virtual.
	double HMRGetScrollPos(int nBar) const;
	void    HMRGetScrollRange(int nBar, double* lpMinPos, double* lpMaxPos) const;
	double HMRSetScrollPos(int nBar, double nPos, BOOL bRedraw = true);
	void    HMRSetScrollRange(int nBar, double nMinPos, double nMaxPos,
			                  BOOL bRedraw = true);
	double HMRGetScrollLimit(int nBar);
    void    HMRScrollWindow(double pi_xAmount, 
                            double pi_yAmount, 
                            LPCRECT lpRect = NULL, 
                            LPCRECT lpClipRect = NULL );

// Implementation
protected:
	HMRSIZE m_pageHMR;            // per page scroll size in logical units
	HMRSIZE m_lineHMR;            // per line scroll size in logical units

    // Maximum value for the scroll bar total range
    double m_MaxRange;

    // INT to DOUBLE ratio
    double m_RatioX;
    double m_RatioY;

    // Current position of the scroll bar
    double m_PositionX;
    double m_PositionY;

    // Total range in HMR units
    double m_TotalHMRMinX;
    double m_TotalHMRMaxX;
    double m_TotalHMRMinY;
    double m_TotalHMRMaxY;
    double m_TotalHMRWidth;
    double m_TotalHMRHeight;

    // Total range in device units
    int32_t m_TotalDevMinX;
    int32_t m_TotalDevMaxX;
    int32_t m_TotalDevMinY;
    int32_t m_TotalDevMaxY;
    int32_t m_TotalDevWidth;
    int32_t m_TotalDevHeight;

	BOOL m_bInsideUpdate;					  // internal state for OnSize callback
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking

protected:
	virtual void OnDraw(CDC* pDC) = 0;      // pass on pure virtual

	void UpdateBars();          // adjust scrollbars etc
	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);
		// size with no bars
	void GetScrollBarSizes(CSize& sizeSb);
	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		                   CSize& sizeRange, HMRPOINT& ptMove, 
                           BOOL bInsideClient);

public:
	virtual ~CHMRScrollView();
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
	virtual void AssertValid() const;
#endif //_DEBUG
	virtual void CalcWindowRect(LPRECT lpClientRect,
		UINT nAdjustType = adjustBorder);

	// scrolling implementation support for OLE
	virtual BOOL OnScroll(UINT nScrollCode, double nPos, BOOL bDoScroll = true);
	virtual BOOL OnScrollBy(HMRSIZE sizeScroll, BOOL bDoScroll = true);

	//{{AFX_MSG(CHMRScrollView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	afx_msg LRESULT OnHMRScrollH(WPARAM, LPARAM);
	afx_msg LRESULT OnHMRScrollV(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#endif