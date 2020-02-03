//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/LineTracker.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#ifndef __LINE_TRACKER_H__
#define __LINE_TRACKER_H__

#pragma once

class KLineTracker
{
    public:

        KLineTracker();
        KLineTracker( POINT const& Begin, POINT const& End );

        
        void Draw( CDC* pDC ) const;

        //****************************************************************
        // attributes

        // tracker position
        void GetPos( POINT* Begin, POINT* End ) const;
        void SetPos( POINT const& Begin, POINT const& End );

        // checks whether given position is valid for rectangle set by
        // SetValidRect(); if valid rectangle is empty, always returns TRUE
        BOOL IsValidPos() { return IsValidPos(CurPos); };

        // width of the line on display
        // default width is 1
        void SetLineWidth( int Width )          { LineWidth = Width; };

        // color of the line on display; does not influence on color of tracked line
        // default color is white, i.e. RGB(255,255,255)
        void SetLineColor( COLORREF Color )     { LineColor = Color; };

        // size of handle; handle is always centered around endpoint of the line;
        // this version creates square handle nSize x nSize pixels
        // default handle size is 8
        void SetHandleSize( int nSize )         { HandleSize = CSize(nSize,nSize); };

        // size of handle: this version allows not square handles
        // default handle size is 8x8
        void SetHandleSize( SIZE const& Size )  { HandleSize = Size; };

        // handle color
        // default handle color is white
        void SetHandleColor( COLORREF Color )   { HandleColor = Color; };

        // vicinity size for hit tests; if a point is not within handle rectangle and
        // it is closer to the line than vicinity size, its hittest will be hitMiddle
        // default vicinity size is 10
        void SetVicinitySize( int Size )        { VicinitySize = Size; };

        // SetValidRect() sets rectangle where endpoints of line may be situated.
        // Valid rectangle is specified in client coordinates.
        // If this rectangle is empty, any tracker position is valid
        //
        // It is responsibility of caller to make sure that initial 
        // position of the tracker is valid
        void SetValidRect( CRect const& Rect );

        int      GetLineWidth() const           { return LineWidth; };
        COLORREF GetLineColor() const           { return LineColor; };
        CSize    GetHandleSize() const          { return HandleSize; };
        COLORREF GetHandleColor() const         { return HandleColor; };
        int      GetVicinitySize() const        { return VicinitySize; };
        CRect    GetValidRect() const           { return ValidRect; };

        //****************************************************************
        // operations

        enum
        {
            // values are compatible with corresponding CRectTracker values
            hitNothing = -1,
            hitBegin   = 0,
            hitEnd     = 2,
            hitMiddle  = 8
        };

        // Test position of given point relative to the line
        int HitTest( CPoint point ) const;

        // Set the cursor shape according to current point position;
        // This function should be called from OnSetCursor() message handler
        BOOL SetCursor( CWnd* pWnd, UINT nHitTest ) const;
        
        // move or resize the tracker
        BOOL Track( CWnd* pWnd, CPoint point ); 

        // create new tracker from scratch
        BOOL TrackRubberBand( CWnd* pWnd, CPoint point );

    private:
        
        struct KLinePos
        {
            CPoint Begin;
            CPoint End;
        };

        KLinePos CurPos;

        int      LineWidth;
        COLORREF LineColor;
        CSize    HandleSize;
        COLORREF HandleColor;
        int      VicinitySize;
        CRect    ValidRect;

        void SetDefaults();

        enum KHandleType
        {
            handleBegin,
            handleEnd
        };

        CRect GetHandleRect( KHandleType HandleType ) const;
        BOOL TrackHandle( CWnd* pWnd, CPoint point, int HitTest );

        BOOL IsValidPos( KLinePos const& Pos );
};

#endif