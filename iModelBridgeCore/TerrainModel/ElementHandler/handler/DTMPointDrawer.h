/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMPointDrawer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include "DTMTextDrawer.h"
#include <functional>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct PointDrawer
    {
    IDisplaySymbol* m_symbol;
    const ViewContextR m_context;
    const DTMDrawingInfo& m_drawingInfo;
    double m_scale;
    bvector<DPoint3d> m_pointStringPoints;
    bool m_numberOfPointsDrawn;

    public: PointDrawer (const DTMDrawingInfo& drawingInfo, ViewContextR context, double scale) : m_context (context), m_symbol (nullptr), m_drawingInfo (drawingInfo), m_scale (scale), m_numberOfPointsDrawn (0)
                {
                }

    public: virtual ~PointDrawer()
                {
                ClearSymbol();
                BeAssert (m_pointStringPoints.size () == 0);
                }

            int NumberOfPointsDrawn () const
                {
                return m_numberOfPointsDrawn;
                }
    virtual void FinishedDrawing ();
    virtual void DrawPoint (DPoint3dCR origin, RotMatrixCP rotMatrix = nullptr);

    void CreateTextSymbol (long textStyleId, WStringCR text, double scale = 1);

    void CreateCellSymbol (WStringCR cellName, DPoint3dCR cellScale);

    virtual void CreateDefaultSymbol ();

    void CreateSymbolFromPointDisplayParams (DTMElementPointsHandler::DisplayParams const & displayParams, int textStyleId);
    bool IsSymbolAPoint (double pixelSize);

    void ClearSymbol();
    virtual void GetPickSize (DPoint3dR size)
        {
        if (!m_symbol)
            size.Init (20, 20, 20);
        else
            {
            DRange3d range;
            m_symbol->_GetRange (range);
            size.Init(range.XLength(), range.YLength(), range.ZLength());
            }
        }
    protected: StatusInt CreateDisplayableCell (EditElementHandleR eeh, ElementHandleCR eh, DPoint3dCR scale, WStringCR cellName, DgnModelRefP foundModelRef) const;
    };

struct PointAndTextDrawer;
//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForPoints : IStrokeForCache
{
private:
    BcDTMP m_dtmElement;
    DTMElementPointsHandler::DisplayParams &m_displayParams;
    DTMDrawingInfo& m_drawingInfo;
    const UInt32 m_id;
    double const m_rescaleFactor;
    PointAndTextDrawer* m_drawer;
    ViewContextP m_context;
    int m_numberOfPointsDrawn;

    std::function<void (DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)> m_browsePointsFunction;

protected:
    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
    static int StrokeCB (DTMFeatureType featureType, DTMUserTag eltId, DTMFeatureId id, DPoint3dP tPoint, size_t nPoint, void *userArgP);
public:

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/08
    //=======================================================================================
    DTMStrokeForPoints
        (
        BcDTMP DTMDataRefXAttribute,
        DTMElementPointsHandler::DisplayParams& displayParams,
        DTMDrawingInfo  &drawingInfo,
        UInt32 id,
        double rescaleFactor,
        std::function<void (DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)> browsePointsFunction = nullptr
        );

    virtual ~DTMStrokeForPoints()
        {
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 02/15
    //=======================================================================================
    int NumberOfPointsDrawn () const
        {
        return m_numberOfPointsDrawn;
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/08
    //
    // Strokes the DTM for the cache
    // At the moment, everything is handled by this stroke method. In the future, each
    // part (triangle, features, ...), will have its own stroking method.
    //
    //=======================================================================================
    void _StrokeForCache (ElementHandleCR element, ViewContextR context, double pixelSize) override;
    virtual void BrowsePoints (BcDTMP dtm, const DTMFenceParams& fenceParams);
}; // End DTMStrokeForPoints struct

    END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
