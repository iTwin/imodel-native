/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMFeaturesDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct BrowseFeaturesUserArg
    {
    ViewContextP viewContext;
    int numberOfFeatures;
    };

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int browseFeatures (DTMFeatureType featureType, Int64  eltId, DTMFeatureId id, DPoint3d *tPoint, size_t nPoint, void *userArgP)
    {
    BrowseFeaturesUserArg* args = (BrowseFeaturesUserArg*)userArgP;
    ViewContextP viewContext = args->viewContext;

    args->numberOfFeatures++;
    if (viewContext->CheckStop ())
        return ERROR;

    viewContext->DrawStyledLineString3d ((int)nPoint, tPoint, nullptr);

    return SUCCESS;
    }

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForCacheFeatures : IStrokeForCache
{
private:
    BcDTMP m_dtmElement;
    DTMElementFeaturesHandler::DisplayParams* m_displayParams;
    DTMDrawingInfo& m_drawingInfo;
    int m_numberOfFeatures;
public:

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/08
    //=======================================================================================
    DTMStrokeForCacheFeatures(BcDTMP DTMDataRefXAttribute, DTMElementFeaturesHandler::DisplayParams* displayParams, DTMDrawingInfo& drawingInfo) : m_drawingInfo(drawingInfo)
        {
        m_dtmElement = DTMDataRefXAttribute;
        m_displayParams = displayParams;
        m_numberOfFeatures = 0;
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/08
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

        if (m_dtmElement == nullptr)
            return;

        // Get the unmanaged handle.... everything must be very fast here.
        BcDTMP bcDTM = m_dtmElement;

        // Push the transformation matrix to transform the coordinates to UORS.
        DrawSentinel   sentinel (context, m_drawingInfo);

        DTMFeatureType featureType = DTMFeatureType::None;

        DTMElementFeaturesHandler::FeatureTypes type = m_displayParams->GetTag();

        if (type >= DTMElementFeaturesHandler::Breakline && type <= DTMElementFeaturesHandler::Contour)
            {
            DTMFeatureType typeMap[] =
                {
                DTMFeatureType::Breakline,
                DTMFeatureType::Hole,
                DTMFeatureType::Island,
                DTMFeatureType::Void,
                DTMFeatureType::Hull,
                DTMFeatureType::ContourLine,
                };
            featureType = typeMap[type];
            }

        BrowseFeaturesUserArg args;
        args.numberOfFeatures = 0;
        args.viewContext = &context;
        bcDTM->BrowseFeatures(featureType, m_drawingInfo.GetFence() /* DTMFenceOption::Overlap */, 10000, &args, browseFeatures);
        m_numberOfFeatures = args.numberOfFeatures;
    }

    int NumberOfFeatures() const
        {
        return m_numberOfFeatures;
        }
};

//=======================================================================================
// @bsimethod                                                   Sylvain.Pucci
//=======================================================================================
bool DTMElementFeaturesDisplayHandler::_Draw (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
{
    DrawPurpose purpose = context.GetDrawPurpose ();
    // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
    // So, we should call draw with the previous state of the element, but we do not have this
    // previous state, so we just redraw the range and it works.
    if (purpose == DrawPurpose::ChangedPre)
        {
        context.DrawElementRange (el.GetElementCP ());
        return true;
        }

    //GeomPickModes pickMode = \
    //        (DrawPurpose::Pick == purpose && 1 == el.GetElementCP()->hdr.dhdr.props.b.s) ? \
    //        PICK_MODE_Cached : \
    //        context->GetGeomPickModes ();

    // Create a DTM element from the XAttributes (this is is a very lightweight operation that
    // just assigns the dtm internal arrays to their addresses inside the XAttributes).
    RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef ();

    if (DrawPurpose::Flash == purpose && !CanDoPickFlash (DTMDataRef, purpose))
        // Need to do something else here as they may not be anything visible if we just flash the hull.
        return false;

    if (DrawPurpose::FitView == purpose)
        {
        DrawScanRange (context, el, drawingInfo.GetDTMDataRef());
        return false;
        }

    if (DrawPurpose::RangeCalculation == purpose)
        return true;

    DTMElementFeaturesHandler::DisplayParams params (xAttr);
    if (!SetSymbology(params, drawingInfo, context))
        return false;

    Bentley::TerrainModel::DTMPtr dtmPtr(DTMDataRef->GetDTMStorage(DrawFeatures, context));
    BcDTMP dtm = dtmPtr != 0 ? dtmPtr->GetBcDTM () : NULL;

    if (!dtm)
        return false;

    DTMStrokeForCacheFeatures featureStroker (dtm, &params, drawingInfo);
    featureStroker._StrokeForCache (el, context, context.GetViewport () ? context.GetViewport ()->GetPixelSizeAtPoint (NULL) : 0.);
    //context->DrawCached (el, Featurestroker, 4, pickMode, DrawExpense::High);
    return featureStroker.NumberOfFeatures () != 0;
    }

SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementFeaturesDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
