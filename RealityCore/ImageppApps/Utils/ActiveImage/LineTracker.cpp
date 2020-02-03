//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/LineTracker.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "LineTracker.h"
#include <math.h>

static COLORREF const CLR_WHITE = RGB(255,255,255);

//------------------------------------------------------
KLineTracker::KLineTracker()
{
    SetDefaults();
    CurPos.Begin = CPoint(0,0);
    CurPos.End   = CPoint(0,0);
};

//------------------------------------------------------
KLineTracker::KLineTracker( POINT const& Begin, POINT const& End )
{
    SetDefaults();
    CurPos.Begin = Begin;
    CurPos.End = End;
};

//------------------------------------------------------
void KLineTracker::SetDefaults()
{
    LineWidth = 1;
    LineColor = CLR_WHITE;
    HandleSize = CSize(8,8);
    HandleColor = CLR_WHITE;
    VicinitySize = 10;
    ValidRect = CRect(0,0,0,0);
};

//------------------------------------------------------
void KLineTracker::GetPos( POINT* Begin, POINT* End ) const
{
    *Begin = CurPos.Begin;
    *End = CurPos.End;
};

//------------------------------------------------------
void KLineTracker::SetPos( POINT const& Begin, POINT const& End )
{
    CurPos.Begin = Begin;
    CurPos.End = End;
};

//------------------------------------------------------    
void KLineTracker::SetValidRect( CRect const& Rect )
{
    ValidRect = Rect; 
    ValidRect.NormalizeRect(); 
}; 

//------------------------------------------------------    
BOOL KLineTracker::IsValidPos( KLinePos const& Pos )
{
    if (ValidRect.IsRectEmpty())
        return TRUE;

    return ValidRect.PtInRect(Pos.Begin) && ValidRect.PtInRect(Pos.End);
};

//------------------------------------------------------    
CRect KLineTracker::GetHandleRect( KHandleType H ) const
{
    CRect R;
    
    if (H == handleBegin)
    {
        R.left = CurPos.Begin.x - HandleSize.cx/2;
        R.top  = CurPos.Begin.y - HandleSize.cy/2;
    }
    else
    {
        R.left = CurPos.End.x - HandleSize.cx/2;
        R.top  = CurPos.End.y - HandleSize.cy/2;
    };

    R.right = R.left + HandleSize.cx;
    R.bottom = R.top + HandleSize.cy;
    return R;
};

//------------------------------------------------------    
void KLineTracker::Draw( CDC* pDC ) const
{
    pDC->SaveDC();
    CPen LinePen( PS_SOLID, LineWidth, LineColor );
    CBrush HandleBrush( HandleColor );

    pDC->SelectObject(&LinePen);

    pDC->MoveTo(CurPos.Begin);
    pDC->LineTo(CurPos.End);

    pDC->FillRect( GetHandleRect(handleBegin), &HandleBrush );
    pDC->FillRect( GetHandleRect(handleEnd), &HandleBrush );
};

//------------------------------------------------------    
int KLineTracker::HitTest( CPoint point ) const
{
    // check handles
    CRect R = GetHandleRect(handleBegin);

    if (R.PtInRect(point))
        return hitBegin;

    R = GetHandleRect(handleEnd);
    if (R.PtInRect(point))
        return hitEnd;

    R = CRect(CurPos.Begin, CurPos.End );
    R.NormalizeRect();
    CRect VicX = R;
    CRect VicY = R;

    VicX.InflateRect( VicinitySize, 0 );
    VicY.InflateRect( 0, VicinitySize );

    if (!VicX.PtInRect(point) && !VicY.PtInRect(point))
        return hitNothing;

    // check distance from the point to the line by x and by y
    int dx = CurPos.End.x - CurPos.Begin.x;
    int dy = CurPos.End.y - CurPos.Begin.y;
    double r = sqrt((double)(dx*dx + dy*dy));

    // compute coordinates relative to the origin of the line
    int x1 = point.x - CurPos.Begin.x;
    int y1 = point.y - CurPos.Begin.y;

    // compute coordinates relative to the line
    double x2 = (x1*dx + y1*dy)/r;
    double y2 = (-x1*dy + y1*dx)/r;

    if ((x2>=0) && (x2<=r) && (y2<=VicinitySize) && (y2 >= -VicinitySize))
        return hitMiddle;

    return hitNothing;
};

//------------------------------------------------------    
BOOL KLineTracker::SetCursor( CWnd* pWnd, UINT nHitTest ) const
{
    // trackers should only be in client area
    if (nHitTest != HTCLIENT)
	    return FALSE;

    // convert cursor position to client co-ordinates
    CPoint point;
    GetCursorPos(&point);
    pWnd->ScreenToClient(&point);

    // do hittest and normalize hit
    switch (HitTest(point))
    {
        case hitBegin:
            ::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
            break;

        case hitEnd:
            ::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEWE ) );
            break;

        case hitMiddle:
            ::SetCursor( AfxGetApp()->LoadStandardCursor( IDC_SIZEALL ) );
            break;

        default:
            return FALSE;
    };

    return TRUE;
};        

//------------------------------------------------------    
BOOL KLineTracker::Track( CWnd* pWnd, CPoint point )
{
    return TrackHandle( pWnd, point, HitTest(point) );
};

//------------------------------------------------------    
BOOL KLineTracker::TrackRubberBand( CWnd* pWnd, CPoint point )
{
    CurPos.Begin = point;
    CurPos.End = point;
    return TrackHandle( pWnd, point, hitEnd );
};

//------------------------------------------------------    
BOOL KLineTracker::TrackHandle( CWnd* pWnd, CPoint point, int HitTest )
{
    if ((HitTest != hitBegin) && (HitTest != hitEnd) && (HitTest != hitMiddle))
        return FALSE;

    if (!IsValidPos())
        return FALSE;

    // set mouse cursor parameters
    CRect CursorRect;

    if (ValidRect.IsRectEmpty())
    {
        pWnd->GetClientRect(&CursorRect);
    }
    else
    {
        // valid rectangle is not empty
        if (HitTest == hitMiddle)
        {
            CRect BeginRect = ValidRect + point - CurPos.Begin;
            BeginRect.NormalizeRect();

            CRect EndRect = ValidRect + point - CurPos.End;
            EndRect.NormalizeRect();

            CursorRect = ValidRect;
            CursorRect &= BeginRect;
            CursorRect &= EndRect;
        }
        else
            CursorRect = ValidRect;
    };

    if (CursorRect.IsRectEmpty())
        return FALSE;

    pWnd->ClientToScreen(&CursorRect);
    ClipCursor(&CursorRect);
    pWnd->SetCapture();

	// get DC for drawing
	CClientDC dc(pWnd);

    // set dc parameters
    CPen LinePen( PS_SOLID, LineWidth, CLR_WHITE );
    dc.SelectObject(&LinePen);
    dc.SetROP2( R2_XORPEN );

    KLinePos OriginalPos = CurPos;
    BOOL bCanceled = FALSE;

    // draw the line for the first time
    dc.MoveTo( CurPos.Begin );
    dc.LineTo( CurPos.End );

	// get messages until capture lost or cancelled/accepted
    BOOL bExit = FALSE;
    KLinePos NewPos;

	while (!bExit)
	{
		MSG msg;
		VERIFY(::GetMessage(&msg, NULL, 0, 0));

		if (CWnd::GetCapture() != pWnd)
			break; // exit loop

		switch (msg.message)
		{
		    // handle movement/accept messages
		    case WM_LBUTTONUP:
		    case WM_MOUSEMOVE:
                {
			        NewPos = CurPos;
                    CPoint MousePoint( (int)(short)LOWORD(msg.lParam), (int)(short)HIWORD(msg.lParam) );

                    switch (HitTest)
                    {
                        case hitBegin:
                            NewPos.Begin = MousePoint;
                            break;

                        case hitEnd:
                            NewPos.End = MousePoint;
                            break;

                        case hitMiddle:
                             NewPos.Begin = OriginalPos.Begin + (MousePoint-point);
                             NewPos.End = OriginalPos.End + (MousePoint-point);
                             break;
                    };

                    // redraw the line
                    if ((NewPos.Begin != CurPos.Begin) || (NewPos.End != CurPos.End))
                    {
                        // draw new line
                        dc.MoveTo( NewPos.Begin );
                        dc.LineTo( NewPos.End );

                        // erase old line
                        dc.MoveTo( CurPos.Begin );
                        dc.LineTo( CurPos.End );
                    };

                    if (IsValidPos(NewPos))
                        CurPos = NewPos;

                    if (msg.message == WM_LBUTTONUP)
                        bExit = TRUE;
                };
    	        break;

		    // handle cancel messages
		    case WM_KEYDOWN:
			    if (msg.wParam == VK_ESCAPE)
                {
                    bCanceled = TRUE;
                    bExit = TRUE;
                };
			    break;

		    case WM_RBUTTONDOWN:
                bCanceled = TRUE;
                bExit = TRUE;
                break;

		    // just dispatch rest of the messages
		    default:
			    DispatchMessage(&msg);
			    break;
	    }
    };

    ClipCursor(NULL);
    ReleaseCapture();

    if (bCanceled)
        CurPos = OriginalPos;

    return !bCanceled;
};

