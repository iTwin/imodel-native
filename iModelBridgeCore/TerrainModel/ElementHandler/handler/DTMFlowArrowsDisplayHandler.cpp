/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMFlowArrowsDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

// Create Flow Arrow PointDrawer, this is different from the others because the element draw is an arrow and not a point.
struct FlowArrowPointDrawer : PointDrawer
    {
    struct FlowArrowSymbol : IDisplaySymbol
        {
        Transform m_scaleTransformation;
        public: FlowArrowSymbol (double arrowSize)
            {
            m_scaleTransformation.InitIdentity();
            m_scaleTransformation.ScaleMatrixColumns (arrowSize, arrowSize, arrowSize);
            }

        virtual StatusInt _GetRange (DRange3dR range) const override
            {
            range.low.x = -.5; range.low.y = -.2; range.low.z = 0;
            range.high.x = .5; range.high.y = .2; range.high.z = 0;
            m_scaleTransformation.Multiply (range, range);
            return SUCCESS;
            }

        virtual void _Draw (ViewContextR context) override
            {
            static const DPoint3d arrowPoints[] = {{-.5, 0., 0.}, {.5,0.,0.}, {.2, .2, 0.}, {.5,0.,0.}, {.2, -.2, 0.}};
        
            context.PushTransform (m_scaleTransformation);
            context.DrawStyledLineString3d (_countof (arrowPoints), arrowPoints, nullptr);
            context.PopTransformClip ();
            }
        };

    double m_arrowSize;

    public: FlowArrowPointDrawer (DTMDrawingInfo& drawingInfo, ViewContextR context, double scale, double arrowSize) : PointDrawer (drawingInfo, context, scale), m_arrowSize (arrowSize)
                {
                }

    virtual void CreateDefaultSymbol ()
        {
        ClearSymbol ();
        m_symbol = new FlowArrowSymbol (m_arrowSize);
        }
    };

//=======================================================================================
// @bsiclass                                            Steve.Jones 08/10
//=======================================================================================
struct DTMStrokeForCacheFlowArrows : IStrokeForCache
{
private: 
    typedef DTMElementFlowArrowsHandler::DisplayParams DP;
    FlowArrowPointDrawer* m_pointDrawer;
    BcDTMR m_dtmElement;
    DP const &m_dp;
    DTMDrawingInfo& m_drawingInfo;
    const UInt32 m_textStyleId;
    ViewContextP m_context;

    static int StrokeCBAsPoints (DTMFeatureType featureType, DTMUserTag eltId, DTMFeatureId id, DPoint3dP tPoint, size_t nPoint, void* userArg)
        {
        if (tPoint[1].z == 0)
            return SUCCESS;

        DTMStrokeForCacheFlowArrows* stroker = (DTMStrokeForCacheFlowArrows*)userArg;
        if (stroker->m_context->CheckStop())
            return ERROR;

        DPoint3d centroid = tPoint[0];
        stroker->m_pointDrawer->DrawPoint (centroid);
        return SUCCESS;
        }

    static int StrokeCB (DTMFeatureType featureType, DTMUserTag eltId, DTMFeatureId id, DPoint3dP tPoint, size_t nPoint, void* userArg)
        {
        if (tPoint[1].z == 0)
            return SUCCESS;

        DTMStrokeForCacheFlowArrows* stroker = (DTMStrokeForCacheFlowArrows*)userArg;
        if (stroker->m_context->CheckStop())
            return ERROR;

        DPoint3d centroid = tPoint[0];

        RotMatrix rotMatrixX;
        RotMatrix rotMatrixY;
        RotMatrix rotMatrix;
        rotMatrixX.initFromAxisAndRotationAngle (2, tPoint[1].y);
        rotMatrixY.initFromAxisAndRotationAngle (1, atan2(tPoint[1].z, 1));
        rotMatrix.productOf (&rotMatrixX, &rotMatrixY);

        //stroker->m_drawingInfo.FullStorageToUors (centroid);
        stroker->m_pointDrawer->DrawPoint (centroid, &rotMatrix);
        return SUCCESS;
        }

    //=======================================================================================
    // @bsimethod                                                   Steve.Jones 08/10
    //=======================================================================================
    public: DTMStrokeForCacheFlowArrows
    (
    BcDTMR              DTMDataRefXAttribute,
    DP const            &displayParams,
    DTMDrawingInfo      &drawingInfo,
    UInt32 textStyleId
    ) : m_drawingInfo (drawingInfo), m_dp (displayParams), m_dtmElement (DTMDataRefXAttribute), m_textStyleId (textStyleId)
        {}

    virtual ~DTMStrokeForCacheFlowArrows()
        {}
    //=======================================================================================
    // @bsimethod                                                   Steve.Jones 08/10
    //
    // Strokes the DTM for the cache
    // At the moment, everything is handled by this stroke method. In the future, each
    // part (triangle, features, ...), will have its own stroking method.
    //
    //=======================================================================================
    void _StrokeForCache (ElementHandleCR el, ViewContextR context, double pixelSize)
        {   
        if (context.CheckStop())
            return;

        // Get the unmanaged handle.... everything must be very fast here.

        double length = m_dp.GetPointCellSize().x; // * tPoint[1].z;
        double lengthUOR = length * dgnModel_getUorPerMaster (el.GetDgnModelP());

        m_pointDrawer = new FlowArrowPointDrawer(m_drawingInfo, context, 1, lengthUOR);
        m_pointDrawer->CreateSymbolFromPointDisplayParams (m_dp, m_textStyleId);
        m_context = &context;

        if (m_pointDrawer->IsSymbolAPoint (pixelSize))
            {
            m_pointDrawer->ClearSymbol ();
            m_dtmElement.BrowseFeatures (DTMFeatureType::FlowArrow, m_drawingInfo.GetFence(), 10000, this, &DTMStrokeForCacheFlowArrows::StrokeCBAsPoints);
            }
        else
            m_dtmElement.BrowseFeatures (DTMFeatureType::FlowArrow, m_drawingInfo.GetFence(), 10000, this, &DTMStrokeForCacheFlowArrows::StrokeCB);
        m_pointDrawer->FinishedDrawing ();
        delete m_pointDrawer;
        }

    void DrawTriangle (ElementHandleCR el, ViewContextP context, double elevation, double slope, double aspect, DPoint3d* trianglePts)
        {
        // Get the unmanaged handle.... everything must be very fast here.
        double length = m_dp.GetPointCellSize().x; // * tPoint[1].z;
        double lengthUOR = length * dgnModel_getUorPerMaster (el.GetDgnModelP());

        m_pointDrawer = new FlowArrowPointDrawer(m_drawingInfo, *context, length, lengthUOR);
        m_pointDrawer->CreateSymbolFromPointDisplayParams (m_dp, m_textStyleId);
        DPoint3d pts[2];

        pts[0] = ( trianglePts[0] + trianglePts[1] + trianglePts[2] ) / 3.;
        double dc_pi = Angle::Pi();
        pts[1].x = (1.5 * dc_pi) - ((aspect * dc_pi) / 180.0);
        pts[1].y = pts[1].x - dc_pi;
        pts[1].z = 0.01 * slope;

        m_context = context;
        StrokeCB (DTMFeatureType::None, 0, 0, pts, 2, this);
        delete m_pointDrawer;
        }
};

//=======================================================================================
// @bsimethod                                                   Steve.Jones 08/10
//=======================================================================================
bool DTMElementFlowArrowsDisplayHandler::_Draw (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
{
    DrawPurpose purpose = context.GetDrawPurpose ();
    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.
    if (purpose == DrawPurpose::ChangedPre)
        {
        context.DrawElementRange (el.GetElementCP());
        return true;
        }

    // Create a DTM element from the XAttributes (this is is a very lightweight operation that
    // just assigns the dtm internal arrays to their addresses inside the XAttributes).
    RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef (); 

    if (DrawPurpose::FitView == purpose)
        {
        DrawScanRange (context, el, drawingInfo.GetDTMDataRef());
        return false;
        }

    if (DrawPurpose::RangeCalculation == purpose)
        return true;

    DTMElementFlowArrowsHandler::DisplayParams params (xAttr);

    if (!SetSymbology (params, drawingInfo, context))
        return false;

    Bentley::TerrainModel::DTMPtr dtmPtr (DTMDataRef->GetDTMStorage(DrawFeatures, context));
    BcDTMP dtm = NULL;

    if (dtmPtr != 0)
        dtm = dtmPtr->GetBcDTM();

    if (!dtm || dtm->GetDTMState() != DTMState::Tin)
        return false;

    if (DrawPurpose::Pick == purpose || DrawPurpose::Flash == purpose)
        {
        if (!CanDoPickFlash (DTMDataRef, purpose))
            return false;

        DPoint3d startPt;
        if (DrawPurpose::Flash == purpose)
            {
            // Need to do something else here as they may not be anything visible if we just flash the hull.
            HitPathCP hitPath = dynamic_cast<HitPathCP> (context.GetSourceDisplayPath ());

            if (!hitPath)
                return true;

            hitPath->GetHitPoint (startPt);
            drawingInfo.RootToStorage (startPt);
            }
        else
            {
            DPoint3d endPt;
            if (!GetViewVectorPoints (drawingInfo, context, dtmPtr, startPt, endPt))
                return true;

            if (startPt.x != endPt.x || startPt.y != endPt.y)
                {
                // Non Top View
                DPoint3d trianglePts[4], point;
                long drapedType;
                BC_DTM_OBJ* bcDTM = dtm->GetTinHandle();
                long voidFlag;

                if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&startPt), ((DPoint3d*)&endPt), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, &voidFlag) != DTM_SUCCESS || drapedType == 0 || voidFlag != 0)
                    return true;

                startPt = point;
                }
            }                

        // TopView
        DPoint3d trianglePts[4];
        int drapedType;
        double elevation, slope, aspect;

        if (dtm->DrapePoint(&elevation, &slope, &aspect, trianglePts, &drapedType, &startPt) == DTM_SUCCESS)
            {
            if (drapedType == 1 || drapedType == 3)
                {
                // Draw Triangle.
                DTMStrokeForCacheFlowArrows stroker (*dtm, params, drawingInfo, xAttr.GetId());
                stroker.DrawTriangle (el, &context, elevation, slope, aspect, trianglePts);
                }
            }
        return true;
        }

    DTMStrokeForCacheFlowArrows (*dtm, params, drawingInfo, xAttr.GetId())._StrokeForCache (el, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
    return true;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/10
//=======================================================================================
void DTMElementFlowArrowsDisplayHandler::_GetPathDescription
(
ElementHandleCR                        elem,
ElementHandle::XAttributeIter const&   xAttr,
LazyDTMDrawingInfoProvider&         ldip,
WStringR                            string,
HitPathCR                           path,
WCharCP                           levelStr,
WCharCP                           modelStr,
WCharCP                           groupStr,
WCharCP                           delimiterStr
)
    {
    double elev;
    double slope;
    double aspect;
    DPoint3d tri[3];
    DPoint3d pt;
    DPoint3d globalOrigin;

    path.GetHitPoint (pt);

    _GetDescription (elem, xAttr, string, 255);
    ldip.Get().GetRootToCurrLocalTrans().multiplyAndRenormalize (&pt, &pt, 1);
    ldip.Get().FullUorsToStorage (pt);
    RefCountedPtr<IDTM> dtm;
    RefCountedPtr<DTMDataRef> dtmRef = ldip.Get().GetDTMDataRef();

    if (dtmRef.IsValid())
        dtmRef->GetDTMReferenceStorage (dtm);

    if (dtm == nullptr)
        { return; }
    dtm->GetDTMDraping()->DrapePoint(&elev, &slope, &aspect, tri, nullptr, pt);

    pt.z = elev;
    ldip.Get().FullStorageToUors (pt);

    WString wElevString;
    WString wSlopeString;
    WString wAspectString;

    WCharCP delim = delimiterStr ? delimiterStr : L",";

    dgnModel_getGlobalOrigin (path.GetRoot()->GetDgnModelP(), &globalOrigin);
    wElevString = DistanceFormatter::Create(*path.GetRoot ()->GetDgnModelP())->ToString (pt.z - globalOrigin.z);        // includeUnits ?? 
    WString::Sprintf (wSlopeString, L"%.2f", slope);
    WString::Sprintf (wAspectString, L"%.2f", aspect);

    WString elevationString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Elevation);
    WString slopeString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Slope);
    WString aspectString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Aspect);

    elevationString.ReplaceAll (L"{1}", wElevString.c_str());
    slopeString.ReplaceAll (L"{1}", wSlopeString.c_str());
    aspectString.ReplaceAll (L"{1}", wAspectString.c_str());

    string.append (delim + elevationString + delim + slopeString + delim + aspectString);
    }

SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementFlowArrowsDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
