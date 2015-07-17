/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMLowPointsDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct DTMLowPointsAppData : BcDTMAppData
    {
    const static BcDTMAppData::Key AppDataID;
    Int64 m_dtmCreationTime;
    double m_minimumDepth;
    bvector<DPoint3d> m_points;
    protected:
        DTMLowPointsAppData () : m_dtmCreationTime (0), m_minimumDepth (0)
            {
            }
        ~DTMLowPointsAppData ()
            {
            }

    public:
        static DTMLowPointsAppData* Create ()
            {
            return new DTMLowPointsAppData  ();
            }

    };

const BcDTMAppData::Key DTMLowPointsAppData::AppDataID;

static int storePoints (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
    {
    bvector<DPoint3d>& m_points = *reinterpret_cast<bvector<DPoint3d>*>(userArg);

    if (numPoints == 0)
        return DTM_SUCCESS;

    for (size_t i = 0; i < numPoints; i++)
        m_points.push_back (*points++);
    return DTM_SUCCESS;
    }


//=======================================================================================
// @bsimethod                                                   Steve.Jones      08/10
//=======================================================================================
bool DTMElementLowPointsDisplayHandler::_Draw
(
 ElementHandleCR   el,
 ElementHandle::XAttributeIter const& xAttr,
 DTMDrawingInfo& drawingInfo,
 ViewContextR context
)
    {
    DrawPurpose purpose = context.GetDrawPurpose ();
    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.

    if (purpose == DrawPurpose::ChangedPre)
        context.DrawElementRange(el.GetElementCP());
    else
        {
        // Create a DTM element from the XAttributes (this is is a very lightweight operation that
        // just assigns the dtm internal arrays to their addresses inside the XAttributes).
        RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef();

        if (DrawPurpose::Flash == purpose && CanDoPickFlash(DTMDataRef, purpose) == false)
            // Need to do something else here as they may not be anything visible if we just flash the hull.
            return false;

        if (DrawPurpose::FitView == purpose)
            {
            DrawScanRange (context, el, drawingInfo.GetDTMDataRef());
            return false;
            }

        if (DrawPurpose::RangeCalculation != purpose)
            {
            DTMElementLowPointsHandler::DisplayParams params (xAttr);

            if (!SetSymbology (params, drawingInfo, context))
                return false;
            
            Bentley::TerrainModel::DTMPtr dtmPtr (DTMDataRef->GetDTMStorage (DrawFeatures, context));
            BcDTMP dtm = 0;

            if (dtmPtr.IsValid ())
                dtm = dtmPtr->GetBcDTM();

            if (!dtm || dtm->GetDTMState() != DTMState::Tin)
                return false;

            AnnotationDisplayParameters adp;
            GetSharedAnnotationHandler()->ComputeAnnotationDisplayParameters ( adp, el, context );

            double minimumDepth = drawingInfo.ScaleUorsToStorageZ(params.GetLowPointMinimumDepth());

            if (dtm)
                {
                BC_DTM_OBJ* bcDTM = dtm->GetTinHandle ();
                DTMLowPointsAppData* lowPointsAppData = reinterpret_cast<DTMLowPointsAppData*>(bcDTM->FindAppData (DTMLowPointsAppData::AppDataID));

                if (!lowPointsAppData)
                    {
                    lowPointsAppData = DTMLowPointsAppData::Create ();
                    bcDTM->AddAppData (DTMLowPointsAppData::AppDataID, lowPointsAppData);
                    }

                if (lowPointsAppData->m_dtmCreationTime != bcDTM->lastModifiedTime || lowPointsAppData->m_minimumDepth != minimumDepth)
                    {
                    lowPointsAppData->m_dtmCreationTime = bcDTM->lastModifiedTime;
                    lowPointsAppData->m_minimumDepth = minimumDepth;
                    lowPointsAppData->m_points.clear ();
                    lowPointsAppData->m_points.resize (0);
                    dtm->BrowseDrainageFeatures (DTMFeatureType::LowPoint, &minimumDepth, DTMFenceParams (), &lowPointsAppData->m_points, storePoints);
                    }
                DTMStrokeForPoints stroker (dtm, params, drawingInfo, xAttr.GetId (), adp.GetRescaleFactor (), [&](DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)
                    {
                    defaultCallback (DTMFeatureType::LowPoint, 0, 0, lowPointsAppData->m_points.data (), lowPointsAppData->m_points.size (), &stroker);
                    });

                stroker._StrokeForCache (el, context, context.GetViewport () ? context.GetViewport ()->GetPixelSizeAtPoint (NULL) : 0.);
                }
            else
                {
            DTMStrokeForPoints stroker ( dtm, params, drawingInfo, xAttr.GetId(), adp.GetRescaleFactor (), [&] (DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)
                {
                dtm->BrowseDrainageFeatures (DTMFeatureType::LowPoint, &minimumDepth, DTMFenceParams(), &stroker, defaultCallback);
                });

            stroker._StrokeForCache (el, context, context.GetViewport () ? context.GetViewport ()->GetPixelSizeAtPoint (NULL) : 0.);
                }
            }
        }
    return true;
    }


SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementLowPointsDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
