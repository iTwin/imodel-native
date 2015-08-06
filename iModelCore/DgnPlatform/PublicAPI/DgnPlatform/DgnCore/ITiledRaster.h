/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ITiledRaster.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ITiledRaster : IRefCounted
{
protected:
    virtual void _DrawRaster (IViewOutputR viewOutput) = 0;
    virtual void _PrintRaster (IViewOutputR viewOutput) = 0;

public:
    DGNPLATFORM_EXPORT void DrawRaster (IViewOutputR viewOutput);
    DGNPLATFORM_EXPORT void PrintRaster (IViewOutputR viewOutput);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IMRImageTileEventHandler
    {
    protected:
        //! Request to return a corresponding Multi-Resolution Image Tile created in hCache for the mri image.
        virtual QvElemP _OnMRImageTileRequest(QvView* hView, QvCache* hCache, QvMRImage* mri, int layer, int row, int column) = 0;
    public:
        DGNVIEW_EXPORT QvElemP OnMRImageTileRequest(QvView* hView, QvCache* hCache, QvMRImage* mri, int layer, int row, int column);
    };


END_BENTLEY_DGN_NAMESPACE
