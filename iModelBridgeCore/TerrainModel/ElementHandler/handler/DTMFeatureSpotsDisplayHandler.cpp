/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMFeatureSpotsDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//=======================================================================================
// @bsimethod                                                   Steve.Jones 02/11
//=======================================================================================
bool DTMElementFeatureSpotsDisplayHandler::_Draw (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
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
            DTMElementFeatureSpotsHandler::DisplayParams params (xAttr);

            if (!SetSymbology (params, drawingInfo, context))
                return false;

            Bentley::TerrainModel::DTMPtr dtmPtr (DTMDataRef->GetDTMStorage (DrawFeatures, context));
            BcDTMP dtm = 0;

            if (dtmPtr.IsValid ())
                dtm = dtmPtr->GetBcDTM();

            if (!dtm)
                return false;

            AnnotationDisplayParameters adp;
            GetSharedAnnotationHandler()->ComputeAnnotationDisplayParameters ( adp, el, context );

            DTMStrokeForPoints stroker ( dtm, params, drawingInfo, xAttr.GetId(), adp.GetRescaleFactor (), [] (DTMStrokeForPoints& stroker, BcDTMP dtm, const DTMFenceParams& fenceParams, DTMFeatureCallback defaultCallback)
                {
                dtm->BrowseFeatures(DTMFeatureType::RandomSpots, fenceParams, 10000, &stroker, defaultCallback);
                dtm->BrowseFeatures (DTMFeatureType::GroupSpots, fenceParams, 10000, &stroker, defaultCallback);
                });
            stroker._StrokeForCache (el, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
            return stroker.NumberOfPointsDrawn () != 0;
            }
    }
    return true;
}

SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementFeatureSpotsDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
