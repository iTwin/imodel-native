/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMPointDrawer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include "DTMPointDrawer.h"
#include "DTMTextDrawer.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

const size_t MAX_POINT_STRING_SIZE = 1000;
struct CellSymbolStroker : IDisplaySymbol
    {
    ElementHandle m_cellElm;
    Transform m_scaleTransformation;

    CellSymbolStroker (ElementHandleCR cellElm, DPoint3dCR scale) : m_cellElm (cellElm)
        {
        m_scaleTransformation.InitIdentity();
        m_scaleTransformation.ScaleMatrixColumns (scale.x, scale.y, scale.z);
        }

    virtual ~CellSymbolStroker ()
        {
        }

    virtual StatusInt _GetRange (DRange3dR range) const override
        {
        DataConvert::ScanRangeToDRange3d (range, *m_cellElm.GetIndexRange());
        m_scaleTransformation.Multiply (range, range);
        return SUCCESS;
        }

    virtual void _Draw (ViewContextR context) override
        {
        DisplayHandlerP displayHandler = m_cellElm.GetDisplayHandler();
        if (displayHandler)
            {
            context.PushTransform (m_scaleTransformation);

            if (m_cellElm.GetElementRef ())
                {
                EditElementHandle editElem (m_cellElm.GetElementRef ());
                editElem.GetElementDescrP ();

                ChildEditElemIter it (editElem, ExposeChildrenReason::Count);

                for (ChildEditElemIter child (editElem); child.IsValid (); child = child.ToNext ())
                    child.GetElementDescrP ()->h.elementRef = nullptr;

                MSElementDescrP descr = editElem.ExtractElementDescr ();

                m_cellElm = ElementHandle (descr, true);
                }
            displayHandler->Draw (m_cellElm, context);

            context.PopTransformClip ();
            }
        }
    };


void PointDrawer::DrawPoint (DPoint3dCR origin, RotMatrixCP rotMatrix)
    {
    m_numberOfPointsDrawn++;
    DPoint3d originUORS = origin;
    m_drawingInfo.FullStorageToUors (originUORS);
    if (m_symbol)
        {
        Transform   tr;

        if (rotMatrix)
            tr.InitFrom (*rotMatrix, originUORS);
        else
            tr.InitFrom (originUORS);

        m_context.DrawSymbol (m_symbol, &tr, nullptr, false, false);
        }
    else
        {
        m_pointStringPoints.push_back (originUORS);

        if (m_pointStringPoints.size () > MAX_POINT_STRING_SIZE)
            {
            m_context.GetIDrawGeom ().DrawPointString3d ((int)m_pointStringPoints.size (), m_pointStringPoints.data(), nullptr);
            m_pointStringPoints.clear ();
            }
        }
    }

void PointDrawer::CreateTextSymbol (long textStyleId, WStringCR text, double scale)
    {
    ClearSymbol();
    TextDrawer drawer (m_drawingInfo, m_context, textStyleId);
    // drawer.FixBG(); ??
    drawer.SetJustification (TextElementJustification::CenterMiddle);
    drawer.Scale (m_scale * scale);
    m_symbol = drawer.CreateTextSymbol (text);
    }

void PointDrawer::CreateCellSymbol (WStringCR cellName, DPoint3dCR cellScale)
    {
    ClearSymbol();

    ElementRefP defElemRef = SharedCellDefHandler::FindDefinitionByName (cellName.GetWCharCP(), *m_drawingInfo.GetSymbologyElement().GetDgnFileP());

    if (defElemRef != nullptr)
        {
        ElementHandle cellInstance (defElemRef);

        m_symbol = new CellSymbolStroker (cellInstance, cellScale);
        }
    else
        CreateDefaultSymbol ();
    }

void PointDrawer::CreateDefaultSymbol ()
    {
    ClearSymbol();
    }

void PointDrawer::CreateSymbolFromPointDisplayParams (DTMElementPointsHandler::DisplayParams const & displayParams, int textStyleId)
    {
    ClearSymbol();
    switch (displayParams.GetPointCellType())
        {
        case DTMElementPointsHandler::DisplayParams::Character:
            CreateTextSymbol (textStyleId, displayParams.GetPointCellName());
            break;
        case DTMElementPointsHandler::DisplayParams::Cell:
            CreateCellSymbol (displayParams.GetPointCellName(), displayParams.GetPointCellSize());
            break;
        default:
            CreateDefaultSymbol();
            break;
        }
    }

bool PointDrawer::IsSymbolAPoint (double pixelSize)
    {
    if (m_symbol)
        {
        DRange3d range;
        m_symbol->_GetRange (range);
        double width = range.XLength();
        double height = range.YLength();

        if (width < height)
            width = height;

        return width < pixelSize;
        }
    return true;
    }

void PointDrawer::ClearSymbol()
    {
    if (m_symbol)
        {
        m_context.DeleteSymbol (m_symbol);
        delete m_symbol;
        }
    m_symbol = nullptr;
    }

void PointDrawer::FinishedDrawing ()
    {
    if (m_pointStringPoints.size ())
        {
        m_context.GetIDrawGeom ().DrawPointString3d ((int)m_pointStringPoints.size (), m_pointStringPoints.data (), nullptr);
        m_pointStringPoints.clear ();
        }
    }

struct PointAndTextDrawer : public PointDrawer
{
    TextDrawer m_textDrawer;
    DTMElementPointsHandler::DisplayParams const * m_dp;
    DTMDrawingInfo& m_drawingInfo;
    ViewContextR m_context;
    DistanceFormatterPtr m_distanceFormatter;

    bool m_drawText;
    double m_elevation;
    WString m_text;

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
    PointAndTextDrawer
    (
    ViewContextR                   context,
    DTMElementPointsHandler::DisplayParams const * dp,
    double                          rescaleFactor,
    double                          cellScaleFactor,
    DTMDrawingInfo&                 drawingInfo,
    DgnModelRefP                    modelRef,
    uint32_t                          textStyleId,
    DgnModelRefP                    contextModelRef,
    double pixelSize
    ) : PointDrawer (drawingInfo, context, rescaleFactor), m_dp (dp), m_drawingInfo (drawingInfo), m_textDrawer (drawingInfo, context, textStyleId), m_context (context)
        {
        m_textDrawer.SetJustification (TextElementJustification::LeftBaseline);
        m_elevation = DBL_MAX;
// ToDo Not sure what this is. -- m_textScale *= rescaleFactor;
        CreateSymbolFromPointDisplayParams (*m_dp, textStyleId);
        m_drawText = m_dp->GetWantPointText() && m_textDrawer.IsValid() && m_textDrawer.IsTextVisible (pixelSize);

        if (IsSymbolAPoint (pixelSize))
            ClearSymbol ();

        }

    virtual void DrawPoint (DPoint3dCR origin, RotMatrixCP rotMatrix = nullptr) override
        {
        PointDrawer::DrawPoint (origin, rotMatrix);

        if (m_drawText)
            {
            DPoint3d originUORS = origin;

            m_drawingInfo.FullStorageToUors (originUORS);
            if (m_elevation != origin.z)
                {
                m_elevation = origin.z;
                DPoint3d globalOrigin;

                dgnModel_getGlobalOrigin (m_drawingInfo.GetOriginalElement ().GetDgnModelP(), &globalOrigin);
                if (m_distanceFormatter.IsNull ())
                    m_distanceFormatter = DistanceFormatter::Create (*m_drawingInfo.GetOriginalElement ().GetDgnModelP ());

                m_text = m_dp->GetPointTextPrefix () + m_distanceFormatter->ToString (originUORS.z - globalOrigin.z) + m_dp->GetPointTextSuffix ();        // includeUnits ??
                }
            m_textDrawer.DrawText (m_text, originUORS, rotMatrix);
            }
        }

    //
    ///// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
    //void DrawCellAndText (DPoint3dCR pt)
    //{
    //    m_pt = pt;
    //    m_stroker.m_drawingInfo.FullStorageToUors (m_pt);
    //    DrawCellAndText (pt.z);
    //}
    //
    ///// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
    //void DrawCellAndText (double elevation)
    //{
    //    DrawCellOnly ();
    //
    //    if (m_elevation != elevation)
    //    {
    //        mdlString_fromUors2 (m_buf, m_pt.z - m_ptGO.z, m_context.GetCurrentModel (), false);
    //        mdlCnv_convertMultibyteToUnicode (m_text, m_buf);
    //        m_elevation = elevation;
    //        swprintf_s (m_buffer, L"%s%s%s", m_textPrefix, m_text, m_textSuffix);
    //    }
    //    m_textParamWide.just = TXTJUST_LB;
    //    DrawTextString (m_buffer, m_pt + m_textOffset, m_textScale, m_zeroRotMatrix);
    //}

};

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
int DTMStrokeForPoints::StrokeCB (DTMFeatureType featureType, DTMUserTag eltId, DTMFeatureId id, DPoint3dP tPoint, size_t nPoint, void *userArgP)
    {
    DTMStrokeForPoints* stroker = reinterpret_cast<DTMStrokeForPoints*> (userArgP);

    if (stroker->m_context->CheckStop())
        return ERROR;
    for (size_t i = 0; i < nPoint; i++, tPoint++)
        stroker->m_drawer->DrawPoint (*tPoint);
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
DTMStrokeForPoints::DTMStrokeForPoints
    (
    BcDTMP DTMDataRefXAttribute,
    DTMElementPointsHandler::DisplayParams& displayParams,
    DTMDrawingInfo  &drawingInfo,
    uint32_t id,
    double rescaleFactor,
    std::function<void (DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)> browsePointsFunction
    ) : m_drawingInfo (drawingInfo), m_rescaleFactor (rescaleFactor), m_displayParams (displayParams), m_id (id), m_context (nullptr), m_dtmElement (DTMDataRefXAttribute), m_browsePointsFunction (browsePointsFunction), m_numberOfPointsDrawn (0)
    {
    }
//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//
// Strokes the DTM for the cache
// At the moment, everything is handled by this stroke method. In the future, each
// part (triangle, features, ...), will have its own stroking method.
//
//=======================================================================================
void DTMStrokeForPoints::_StrokeForCache (ElementHandleCR el, ViewContextR context, double pixelSize)
    {
    if (context.CheckStop())
        return;

    if (!m_dtmElement)
        return;
    // Get the unmanaged handle.... everything must be very fast here.
    BcDTMP bcDTM = m_dtmElement;

    DgnModelRefP    foundModelRef = GetModelRef(el);
    DgnModelRefP    contextModelRef = context.GetViewport() ? context.GetViewport()->GetRootModel() : foundModelRef;
    PointAndTextDrawer drawer (context, &m_displayParams, m_rescaleFactor, 1, m_drawingInfo, foundModelRef, m_id, contextModelRef, pixelSize);

    m_drawer = &drawer;
    m_context = &context;
    if (m_context->GetDrawPurpose () == DrawPurpose::Pick)
        {
        DPoint3d startPt, endPt;
        DPoint3d pickSize;
        m_drawer->GetPickSize (pickSize);
        GetViewVectorPoints (m_drawingInfo, context, bcDTM, startPt, endPt);
        DRange3d range;
        range.initFrom (&startPt, &endPt);
        range.low.subtract (&pickSize);
        range.high.add (&pickSize);

        DPoint3d pts[8];

        range.Get8Corners (pts);
        pts[4] = pts[0];
        BrowsePoints (bcDTM, DTMFenceParams (DTMFenceType::Block, DTMFenceOption::Inside, pts, 5));
        }
    else
        BrowsePoints (bcDTM, m_drawingInfo.GetFence ());
    m_drawer->FinishedDrawing ();
    m_numberOfPointsDrawn = m_drawer->NumberOfPointsDrawn();
    }

void DTMStrokeForPoints::BrowsePoints (BcDTMP dtm, const DTMFenceParams& fenceParams)
    {
    if (m_browsePointsFunction)
        m_browsePointsFunction (*this, dtm, fenceParams, &StrokeCB);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
