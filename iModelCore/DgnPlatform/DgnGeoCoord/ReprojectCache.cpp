/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/ReprojectCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma  warning(disable:4189) // local variable is initialized but not referenced

#include    <Geom\msgeomstructs.hpp>
#include    <Bentley\BeTimeUtilities.h>
#include    <DgnPlatform\DgnCore\DgnCoreAPI.h>
#include    <DgnGeoCoord\DgnGeoCoord.h>
#include    <DgnGeoCoord\DgnGeoCoordApi.h>
#include    <DgnPlatform\DgnHandlers\IGeoCoordReproject.h>
#include    <DgnPlatform\DgnCore\Undo.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_USTN

using namespace BentleyApi::GeoCoordinates;


#define MAX_POSTSTROKE_DEPTH    16

class       StandardReprojectionSettings : public IGeoCoordinateReprojectionSettings
    {
    public:

    virtual double                  StrokeTolerance()                       { return 0.1; }
    virtual ReprojectionOption      DoCellElementsIndividually()            { return ReprojectionOptionIfLarge; }
    virtual ReprojectionOption      DoMultilineTextElementsIndividually()   { return ReprojectionOptionIfLarge; }
    virtual bool                    ScaleText()                             { return true; }
    virtual bool                    RotateText()                            { return true; }
    virtual bool                    ScaleCells()                            { return true; }
    virtual bool                    RotateCells()                           { return true; }
    virtual ReprojectionOption      StrokeArcs()                            { return ReprojectionOptionIfLarge; }
    virtual ReprojectionOption      StrokeEllipses()                        { return ReprojectionOptionIfLarge; }
    virtual ReprojectionOption      StrokeCurves()                          { return ReprojectionOptionIfLarge; }
    virtual bool                    PostStrokeLinear()                      { return false; }
    virtual bool                    ReprojectElevation()                    { return false; }
    };


/*=================================================================================**//**
* This is the class that helps with the reprojection of a cache from reference GCS to master GCS.
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
class  CacheReproject : public IGeoCoordinateReprojectionHelper
{
private:
    DgnModelRefP        m_refModelRef;
    DgnGCSPtr           m_refGCSPtr;
    DgnModelRefP        m_rootModelRef;
    DgnGCSPtr           m_rootGCSPtr;
    bool                m_startingReadOnly;
    uint32_t            m_pointsReprojected;
    uint32_t            m_transformsComputed;
    uint32_t            m_detailPointsAdded;
    IDgnProgressMeterP  m_progressMeter;

    double              m_startTime;
    IGeoCoordinateReprojectionSettingsP     m_reprojectionSettings;
    double              m_strokeToleranceRefUors;
    double              m_strokeToleranceRootUors;
    double              m_strokeRangeThresholdRefUorsSquared;
    double              m_toleranceSquared;

    // "post stroke" member variables
    DPoint3dP           m_allocatedPoints;
    int                 m_numPoints;
    int                 m_currentPoint;
    int                 m_domainErrors;
    int                 m_usefulRangeErrors;
    int                 m_datumConvertNotSetErrors;
    int                 m_otherErrors;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
CacheReproject (DgnModelRefP refModelRef, DgnGCSP refGCS, DgnModelRefP rootModelRef, DgnGCSP rootGCS)
    {
    m_refModelRef               = refModelRef;
    m_refGCSPtr                 = refGCS;
    m_rootModelRef              = rootModelRef;
    m_rootGCSPtr                = rootGCS;
    m_startTime                 = BeTimeUtilities::QuerySecondsCounter();

    DgnModelP cache             = m_refModelRef->GetDgnModelP();
    m_startingReadOnly          = cache->SetReadOnly (false);
    m_pointsReprojected         = 0;
    m_transformsComputed        = 0;
    m_detailPointsAdded         = 0;
    m_domainErrors              = 0;
    m_usefulRangeErrors         = 0;
    m_datumConvertNotSetErrors  = 0; 
    m_otherErrors               = 0;
    m_progressMeter             = DgnPlatformLib::GetHost().GetProgressMeter();
#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
    m_reprojectionSettings      = dgnGeoCoord_getRefReprojectionSettings (refModelRef);
#else
    m_reprojectionSettings      = new StandardReprojectionSettings();
#endif

    // calculate the stroke range. We use 1000 meters, and we need it in reference modelRef UORs
    double  refUorsPerMeter     = dgnModel_getUorPerMeter (refModelRef->GetDgnModelP());
    // we consider something to be "spatially large" if it occupies an area greater than .2 square kilometers in the X-Y plane.
    m_strokeRangeThresholdRefUorsSquared = 0.2 * (1000.0 * refUorsPerMeter * 1000.0 * refUorsPerMeter);

    // the stroke tolerance is stored in master file units of m_rootModelRef. First get masterFileUors.
    double  rootUorsPerMaster   = dgnModel_getUorPerMaster (rootModelRef->GetDgnModelP());
    m_strokeToleranceRootUors   = m_reprojectionSettings->StrokeTolerance() * rootUorsPerMaster;

    double uorScale = 1.0;
    if (m_rootModelRef && m_refModelRef)
        modelInfo_getUorScaleBetweenModels (&uorScale, m_rootModelRef->GetDgnModelP (), m_refModelRef->GetDgnModelP ());
    m_strokeToleranceRefUors    = m_strokeToleranceRootUors * uorScale;

    // set up post stroking member variables
    m_allocatedPoints           = NULL;
    m_numPoints                 = 0;

    // From the settings, find whether elevations are supposed to be reprojected and set that.
    rootGCS->SetReprojectElevation (m_reprojectionSettings->ReprojectElevation());
    refGCS->SetReprojectElevation (m_reprojectionSettings->ReprojectElevation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~CacheReproject ()
    {
    // restore the readonly state of the ref cache.
    DgnModelP cache = (m_refModelRef ? m_refModelRef->GetDgnModelP () : NULL);
    cache->SetReadOnly (m_startingReadOnly);

    if (NULL != m_allocatedPoints)
        free (m_allocatedPoints);

    if (NULL != m_reprojectionSettings)
        delete m_reprojectionSettings;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DoReprojection (bool undoable, bool highFidelity)
    { 
    DgnModelP cache = (m_refModelRef ? m_refModelRef->GetDgnModelP () : NULL);

    // clear (and repopulate as we go) the range tree, so that the overall range is corrected.
    cache->ClearRangeIndex();

    // set up the progress reporter.
    IDgnProgressMeter::TaskMark progressTaskMark;
    if (m_progressMeter)
        {
        WChar msgBuf[512];
        BaseGeoCoordResource::GetLocalizedStringW (msgBuf, DGNGEOCOORD_Msg_ReprojectingCoordinateData, _countof (msgBuf));
        
        progressTaskMark.Push (msgBuf);
        }

    uint32_t                        currentCount;
    PersistentElementRefP           elemRef;
    DgnModel::ElementRefIterator    iter;
    iter.SetModel (cache);
    iter.SetSections (DGNMODEL_SECTION_GRAPHIC_ELMS);
    for (currentCount=0, elemRef = iter.GetFirstElementRef(); (NULL != elemRef) && !iter.HitEOF(); elemRef = iter.GetNextElementRef())
        {
        bool    replaced = false;

        if (elemRef->IsDeleted())
            continue;

        EditElementHandle  elemHandle (elemRef, m_refModelRef);
        DisplayHandlerP  displayHandler;
        if (NULL != (displayHandler = elemHandle.GetDisplayHandler()))
            {
            ReprojectStatus status = displayHandler->GeoCoordinateReprojection (elemHandle, *this, false);

            if ( (REPROJECT_NoChange != status) && (REPROJECT_CSMAPERR_OutOfMathematicalDomain != status) && (REPROJECT_DontValidateRange != status) )
                displayHandler->ValidateElementRange (elemHandle, false);

            if (REPROJECT_NoChange != status)
                {
                // this kludge is in here to communicate to the dimension element that it should not execute its _OnReplace code.
                // see DimensionHandler::_OnReplace for the other end of this communication. Fixes TR#311767.
                MSElementDescrP    edP;
                if ( (DIMENSION_ELM == elemHandle.GetElementType()) && (NULL != (edP = const_cast <MSElementDescrP> (elemHandle.PeekElementDescrCP()))) )
                    edP->h.appData2 = 0x04151956;

                elemHandle.ReplaceInModel (elemRef);
                replaced = true;
                }
            }

        // if we haven't replaced it, we have to reinsert it into the range tree.
        if (!replaced)
            cache->InsertRangeElement (elemRef, true);

        // update progress bar
        if (m_progressMeter && (0 == ++currentCount % 64))
            m_progressMeter->UpdateTaskProgress ();
        }

    // fix up the reference attachments.
    iter.SetModel (cache);
    iter.SetSections (DGNMODEL_SECTION_CONTROL_ELMS);

    for (currentCount=0, elemRef = iter.GetFirstElementRef(); (NULL != elemRef) && !iter.HitEOF(); elemRef = iter.GetNextElementRef())
        {
        bool    replaced = false;

        if (elemRef->IsDeleted())
            continue;

        EditElementHandle  elemHandle (elemRef, m_refModelRef);
        HandlerR           handler = elemHandle.GetHandler();

        ReprojectStatus    status = handler.GeoCoordinateReprojection (elemHandle, *this, false);

        if (REPROJECT_NoChange != status)
            {
            elemHandle.ReplaceInModel (elemRef);
            replaced = true;
            }

        // update progress bar (90% to 100%)
        if (m_progressMeter && (0 == ++currentCount % 10))
            m_progressMeter->UpdateTaskProgress ();
        }

    // set up the units in the cache we're transforming to match those we're transforming to.
    // don't call the routines to do this, because we don't want any of the notification messages sent.
    ModelInfoCP rootModelInfo      = m_rootModelRef->GetModelInfoCP();
    ModelInfoP  refModelInfo       = const_cast <ModelInfoP> (m_refModelRef->GetModelInfoCP());
    refModelInfo->m_uorPerStorage  = rootModelInfo->m_uorPerStorage;
    refModelInfo->m_storageUnit    = rootModelInfo->m_storageUnit;

    // make sure we never save this (Reprojected) cache.
    if (!undoable)
        cache->SetIsFileImage (false);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   03/11
+===============+===============+===============+===============+===============+======*/
struct          ReprojectTxn : DgnCacheTxn
{
ITxn*   m_oldTxn;

ReprojectTxn ()  { m_oldTxn =  &ITxnManager::GetManager().SetCurrentTxn (*this); }
~ReprojectTxn () { ITxnManager::GetManager().SetCurrentTxn (*m_oldTxn); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DoReproject (bool undoable, bool highFidelity)
    {
    if (!undoable)
        {
        // replace txn manager and do the reprojection.
        ReprojectTxn    replacementTxn;
        return DoReprojection (undoable, highFidelity);
        }
    else
        {
        return DoReprojection (undoable, highFidelity);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         ReprojectPoints
(
DPoint3dP   outCartesianDest,   // <= array of cartesian values, returned in UORS of master modelRef UORs.
GeoPointP   outLatLongDest,     // <= (optional) array of lat long in destination Geocoordinate system.
GeoPointP   outLatLongSrc,      // <= (optional) array of lat long in this Geocoordinate system.
DPoint3dCP  inCartesian,        // => array of cartesian coordinate points in source modelRef UORs.
int         numPoints           // => number of points.
) override
    {
    m_pointsReprojected += numPoints;
    return TrackErrors (m_refGCSPtr->ReprojectUors (outCartesianDest, outLatLongDest, outLatLongSrc, inCartesian, numPoints, *m_rootGCSPtr.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         ReprojectPoints2D
(
DPoint2dP   outCartesianDest,   // <= array of cartesian values, returned in UORS of master modelRef UORs.
GeoPoint2dP outLatLongDest,     // <= (optional) array of lat long in destination Geocoordinate system.
GeoPoint2dP outLatLongSrc,      // <= (optional) array of lat long in this Geocoordinate system.
DPoint2dCP  inCartesian,        // => array of cartesian coordinate points in source modelRef UORs.
int         numPoints           // => number of points.
) override
    {
    m_pointsReprojected += numPoints;
    return TrackErrors (m_refGCSPtr->ReprojectUors2D (outCartesianDest, outLatLongDest, outLatLongSrc, inCartesian, numPoints, *m_rootGCSPtr.get()));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ResizeAllocatedPoints
(
int     pointsNeeded
)
    {
    if (m_numPoints < pointsNeeded)
        {
        // round up to nearest 100 so we don't keep reallocating.
        pointsNeeded = 100 * ((pointsNeeded + 99) / 100);
        m_allocatedPoints   = (DPoint3dP)realloc (m_allocatedPoints, pointsNeeded * sizeof(DPoint3d));
        m_numPoints         = pointsNeeded;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void                    InterpolateSegment
(
DPoint3dCR      startPointInUors,
DPoint3dCR      endPointInUors,
DPoint3dCR      startPointOutUors,
DPoint3dCR      endPointOutUors,
int             depth
)
    {
    if (depth > MAX_POSTSTROKE_DEPTH)
        {
        // if we haven't succeeded by the time we've divided by 2^16, we're probably not going to.
        ResizeAllocatedPoints (m_currentPoint+1);
        m_allocatedPoints[m_currentPoint++] = startPointOutUors;
        return;
        }

    DPoint3d    centerPointInUors;
    centerPointInUors.interpolate (&startPointInUors, 0.5, &endPointInUors);

    DPoint3d    centerPointOutUors;
    m_refGCSPtr->ReprojectUors (&centerPointOutUors, NULL, NULL, &centerPointInUors, 1, *m_rootGCSPtr.get());

    DPoint3d    centerPointInterpolated;
    centerPointInterpolated.interpolate (&startPointOutUors, 0.5, &endPointOutUors);

    double      distanceSquared = centerPointInterpolated.distanceSquared (&centerPointOutUors);
    if (distanceSquared > m_toleranceSquared)
        {
        // see if we need to keep going for the left segment.
        InterpolateSegment (startPointInUors, centerPointInUors, startPointOutUors, centerPointOutUors, depth+1);

        // see if we need to keep going for the right point
        InterpolateSegment (centerPointInUors, endPointInUors, centerPointOutUors, endPointOutUors, depth+1);
        }
    else
        {
        // store the startPoint in the output array.
        ResizeAllocatedPoints (m_currentPoint+1);
        m_allocatedPoints[m_currentPoint++] = startPointOutUors;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         ReprojectPointsMoreDetail
(
DPoint3dP   *outUors,
int         *outPointCount,
DPoint3dCP  inUors,
int         numPoints,
double      tolerance
) override
    {
    m_currentPoint      = 0;
    m_toleranceSquared  = tolerance * tolerance;

    // set up the output size.
    ResizeAllocatedPoints (numPoints);

    // calculate startPointOutUors and endPointOutUors to start the process.
    DPoint3d            startPointOutUors;
    DPoint3d            endPointOutUors;
    ReprojectStatus     status = TrackErrors (m_refGCSPtr->ReprojectUors (&startPointOutUors, NULL, NULL, inUors, 1, *m_rootGCSPtr.get()));

    for (int iPoint=0; iPoint < (numPoints-1); iPoint++)
        {
        // startPoint is already computed, don't want to compute it twice.
        ReprojectStatus     status1 = TrackErrors (m_refGCSPtr->ReprojectUors (&endPointOutUors, NULL, NULL, &inUors[iPoint+1], 1, *m_rootGCSPtr.get()));
        if (REPROJECT_Success == status)
            status = status1;

        InterpolateSegment (inUors[iPoint], inUors[iPoint+1], startPointOutUors, endPointOutUors, 0);

        startPointOutUors = endPointOutUors;
        }

    // store the end point.
    ResizeAllocatedPoints (m_currentPoint+1);
    m_allocatedPoints[m_currentPoint++] = endPointOutUors;

    *outUors        = m_allocatedPoints;
    *outPointCount  = m_currentPoint;

    m_pointsReprojected += m_currentPoint;
    m_detailPointsAdded += (m_currentPoint - numPoints);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         ReprojectPointsMoreDetail2D
(
DPoint3dP   *outUors,
int         *outPointCount,
DPoint2dCP  inUors,
int         numPoints,
double      tolerance
) override
    {
    m_currentPoint      = 0;
    m_toleranceSquared  = tolerance * tolerance;

    // set up the output size.
    ResizeAllocatedPoints (numPoints);

    // reproject the first point to get the process started.
    DPoint3d    startPointOutUors;
    DPoint3d    startPointInUors;
    startPointInUors.init (inUors);
    ReprojectStatus    status = TrackErrors (m_refGCSPtr->ReprojectUors (&startPointOutUors, NULL, NULL, &startPointInUors, 1, *m_rootGCSPtr.get()));

    DPoint3d    endPointOutUors;
    DPoint3d    endPointInUors;
    for (int iPoint=0; iPoint < (numPoints-1); iPoint++)
        {
        startPointInUors.init (&inUors[iPoint]);
        endPointInUors.init (&inUors[iPoint+1]);
        ReprojectStatus    status1 = TrackErrors (m_refGCSPtr->ReprojectUors (&endPointOutUors, NULL, NULL, &endPointInUors, 1, *m_rootGCSPtr.get()));
        if (REPROJECT_Success == status)
            status = status1;

        InterpolateSegment (startPointInUors, endPointInUors, startPointOutUors, endPointOutUors, 0);

        startPointOutUors = endPointOutUors;
        }

    // store the end point.
    ResizeAllocatedPoints (m_currentPoint+1);
    m_allocatedPoints[m_currentPoint++] = endPointOutUors;

    *outUors        = m_allocatedPoints;
    *outPointCount  = m_currentPoint;

    m_pointsReprojected += m_currentPoint;
    m_detailPointsAdded += (m_currentPoint - numPoints);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         GetLocalTransform
(
TransformP  outTransform,       // <= the transform effective at the point elementOrigin in source coordinates
DPoint3dCR  elementOrigin,      // => the point to use to find the transform.
DPoint3dCP  extent,             // => the extent to use to find the rotation and scale, NULL to use a default range.
bool        doRotate,           // => whether to apply a rotation.
bool        doScale             // => whehter to apply a scale based on the coordinate system distortion.
) override
    {
    m_pointsReprojected++;
    m_transformsComputed++;

    return TrackErrors (m_refGCSPtr->GetLocalTransform (outTransform, elementOrigin, extent, doRotate, doScale, *m_rootGCSPtr.get()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP                 GetSourceGCS () override
    {
    return m_refGCSPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP                 GetDestinationGCS () override
    {
    return m_rootGCSPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* Returns the tolerance in source model UORS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double          GetStrokeToleranceDestUors() override
    {
    return m_strokeToleranceRootUors;
    }

/*---------------------------------------------------------------------------------**//**
* Returns the tolerance in source model UORS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double          GetStrokeToleranceSourceUors() override
    {
    return m_strokeToleranceRefUors;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateReprojectionSettingsP     GetSettings() override
    {
    return m_reprojectionSettings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelRefP            GetSourceModelRef () override
    {
    return m_refModelRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelRefP    GetDestinationModelRef () override
    {
    return m_rootModelRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
double                  GetUnitRatio () override
    {
    double  uorFactor = 1.0;
    if (SUCCESS == modelInfo_getUorScaleBetweenModels (&uorFactor, m_refModelRef->GetDgnModelP (), m_rootModelRef->GetDgnModelP ()))
        return uorFactor;

    return dgnModel_getUorPerStorage (m_rootModelRef->GetDgnModelP()) / dgnModel_getUorPerStorage (m_refModelRef->GetDgnModelP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         TrackErrors (ReprojectStatus status)
    {
    if (REPROJECT_CSMAPERR_OutOfMathematicalDomain == status)
        m_domainErrors++;
    else if (REPROJECT_CSMAPERR_OutOfUsefulRange == status)
        m_usefulRangeErrors++;
    else if (REPROJECT_CSMAPERR_DatumConverterNotSet == status)
        m_datumConvertNotSetErrors++; // The actual number of occurence could be irrelevant in this case 
                                      // but it may provide an indication if multiple reference files are attached with various
                                      // GCS set which of the reference GCS is offending.
    else if (REPROJECT_Success != status)
        m_otherErrors++;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void                    GetFinishMessages
(
bool*       hasWarnings,
bool*       hasErrors,
WChar*    message,
WChar*    detailMessage
)
    {
    // report the number of points reprojected and the time it took.
    WCharCP modelName = m_refModelRef->GetModelNameCP();
    WString fileName  = m_refModelRef->GetDgnFileP()->GetFileName();
    double seconds = BeTimeUtilities::QuerySecondsCounter() - m_startTime;

    *hasWarnings = (m_usefulRangeErrors > 0) || (m_otherErrors > 0) || (m_datumConvertNotSetErrors > 0);
    *hasErrors   = (m_domainErrors > 0);

    WChar format[512];
    BaseGeoCoordResource::GetLocalizedStringW (format, *hasErrors ? DGNGEOCOORD_Msg_ReprojectedPointsWithErrors : *hasWarnings ? DGNGEOCOORD_Msg_ReprojectedPointsWithWarnings : DGNGEOCOORD_Msg_ReprojectedPoints, _countof (format));
    swprintf (message, format, fileName.c_str(), modelName, seconds);

    BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_PointsReprojectedDetail, _countof (format));
    swprintf (detailMessage, format, m_pointsReprojected, m_detailPointsAdded, m_transformsComputed);

    if (m_domainErrors > 0)
        {
        BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_DomainErrors, _countof (format));
        wcscat (detailMessage, L"\n");
        swprintf (&detailMessage[wcslen(detailMessage)], format, m_domainErrors);
        }
    if (m_usefulRangeErrors > 0)
        {
        BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_UsefulRangeErrors, _countof (format));
        wcscat (detailMessage, L"\n");
        swprintf (&detailMessage[wcslen(detailMessage)], format, m_usefulRangeErrors);
        }
    if (m_datumConvertNotSetErrors > 0)
        {
        // We cannot unfortunately use the complete datum error message (DGNGeoCoord_Msg_DatumError) as we do not know the
        // name of the datums for which a datum converter could not be set.
        // We use a simpler message that stipulates the number of points as an indication of the DGN model that 
        // uses one of the datum.		
        BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_DatumConvertNotSetErrors, _countof (format));
        wcscat (detailMessage, L"\n");
        swprintf (&detailMessage[wcslen(detailMessage)], format, m_datumConvertNotSetErrors);
       }
    if (m_otherErrors > 0)
        {
        BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_OtherErrors, _countof (format));
        wcscat (detailMessage, L"\n");
        swprintf (&detailMessage[wcslen(detailMessage)], format, m_otherErrors);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool ShouldStroke (EditElementHandleR elemHandle, ReprojectionOption option) override
    {
    if (option == ReprojectionOptionNever)
        return false;
    if (option == ReprojectionOptionAlways)
        return true;

    // option value is "IfLarge". We need to figure out whether a distortion larger than the tolerance
    // will result from transforming the element as a unit using the best transform.
    MSElementDescrP edP      = elemHandle.GetElementDescrP();

    assert ( (NULL != edP) && edP->el.ehdr.isGraphics);
    if ( (NULL == edP) || !edP->el.ehdr.isGraphics)
        return false;

    DRange3d   range;
    DataConvert::ScanRangeToDRange3d (range, edP->el.hdr.dhdr.range);
    DPoint3d    diagonal;
    diagonal.DifferenceOf (*( &range.high), *( &range.low));
    double      magnitudeSquaredXY = diagonal.x * diagonal.x + diagonal.y * diagonal.y;
    return (magnitudeSquaredXY > m_strokeRangeThresholdRefUorsSquared);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeUntransformedAttachment (DgnAttachmentP refP)
    {
    DPoint3d origin;
    origin.x = origin.y = origin.z = 0.0;

    refP->SetMasterOrigin (origin);
    refP->SetRefOrigin (origin);

    RotMatrix   identity;
    identity.InitIdentity();
    refP->SetRotMatrix (identity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool untransformedAttachment (bool& refElementReprojected, DgnModelRefCP parentModelRef, DgnAttachmentP refP)
    {
    refElementReprojected = false;

    // if it's in the "CouldBeReprojectedList" of the parentModelRef, then we've changed the origin and transform
    //  in RefAttachHandler::_OnGeoCoordinateReprojection, and we should not conclude that this reference is
    //  not to be reprojected.
    ReprojectionInfo*  reprojectInfo;
    if (NULL != (reprojectInfo = static_cast <ReprojectionInfo*> (refP->FindAppData (ReprojectionInfo::GetKey()))))
        {
        T_AttachIdList::iterator iterator;
        for (iterator = reprojectInfo->m_couldBeReprojectedArray.begin(); iterator != reprojectInfo->m_couldBeReprojectedArray.end(); iterator++)
            {
            ElementId testElementId = (ElementId) *iterator;
            if (testElementId == refP->GetElementId())
                {
                refElementReprojected = false;
                return true;
                }
            }
        }

    DPoint3d    masterOrigin = refP->GetStoredMasterOrigin();
    DPoint3d    refOrigin    = refP->GetRefOrigin();
    if ( (0.0 != masterOrigin.x) || (0.0 != masterOrigin.y) || (0.0 != masterOrigin.z) )
        return false;
    if ( (0.0 != refOrigin.x) || (0.0 != refOrigin.y) || (0.0 != refOrigin.z) )
        return false;
    if (!(refP->GetRotMatrix()).IsIdentity())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void reportDatumShiftError (DgnGCSP sourceGCS, DgnGCSP targetGCS, DgnModelRefP modelRef)
    {
    // if we get here, it's because we can't create a DatumConverter. We report that error,
    // and the reference will be marked as not found.
    // report the number of points reprojected and the time it took.
    WChar     format[512];
    WChar     message[2048];
    WCharCP modelName = modelRef->GetModelNameCP();
    WString   fileName  = modelRef->GetDgnFileP()->GetFileName();

    BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_DatumError, _countof (format));
    swprintf (message, format, sourceGCS->GetDatumName(), targetGCS->GetDatumName(), fileName.c_str(), modelName);
    NotifyMessageDetails details (MESSAGE_ERROR, message);
    NotificationManager::OutputMessage (details);
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool designCoordinateSystemsIdentical (DgnModelP targetCache, DgnModelP sourceCache)
    {
    double  targetUorsPerMeter;
    double  sourceUorsPerMeter;

    // for the coordinate systems to be identical, the unit setups must be the same, and the global origins the same.
    targetUorsPerMeter  = dgnModel_getUorPerMeter (targetCache);
    sourceUorsPerMeter  = dgnModel_getUorPerMeter (sourceCache);
    if ( fabs (targetUorsPerMeter - sourceUorsPerMeter) > 1.0e-9)
        return false;

    DPoint3d    targetGlobalOrigin;
    DPoint3d    sourceGlobalOrigin;

    dgnModel_getGlobalOrigin (targetCache, &targetGlobalOrigin);
    dgnModel_getGlobalOrigin (sourceCache, &sourceGlobalOrigin);
    return targetGlobalOrigin.IsEqual(sourceGlobalOrigin,  1.0e-6);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void getLinearTransformMessages (WCharP message, size_t messageCount, WCharP detailMessage, size_t detailMessageCount, DgnModelRefP modelRef)
    {
    WChar     format[512];
    WCharCP   modelName = modelRef->GetModelNameCP();
    WString     fileName  = modelRef->GetDgnFileP()->GetFileName();

    // mark it as reprojected, even though no reprojection was actually required.
    BaseGeoCoordResource::GetLocalizedStringW (format, DGNGEOCOORD_Msg_SubstituteLinearTransform, _countof (format));
    swprintf (message, format, fileName.c_str(), modelName);
    BaseGeoCoordResource::GetLocalizedStringW (detailMessage, DGNGEOCOORD_Msg_SubstituteLinearTransformDetails, detailMessageCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoCoordinationAdmin::_OnPostModelFill (DgnModelR cache, DgnModelFillContextP context) const 
    {
    CompleteInitialization ();

    // A DgnModel has been filled. Now is the time when we can do the geocoordinate
    //  transform if desired.

    // if not configured for OTF transform, don't do it.
    if (!m_otfEnabled)
        return;

    // should always have a cache, a context
    if (NULL == context)
        return;

    DgnAttachmentP   refP = context->GetFillDgnAttachment();
    if (NULL == refP) // only if filling model for a ref attachment
        return;

    // if the attachment method isn't appropriate for reprojection, don't do it,
    if ((ATTACHMETHOD_Unknown != refP->GetAttachMethod()) && (ATTACHMETHOD_GeographicProjected != refP->GetAttachMethod()) )
        return;

    // if it's an unknown, only do if if master origin and reference origin are 0.0, and transform is identity.
    bool    refElementReprojected = false;
    if ( (ATTACHMETHOD_Unknown == refP->GetAttachMethod()) && !untransformedAttachment (refElementReprojected, refP->GetParentModelRefP(), refP))
        return;

    // if the GeoHandling of the root is set to anything other than DGNMODEL_GeoAttachmentHandling_Default, don't attempt to reproject.
    // By default, DgnModels opened with mdlModelRef_createWorking are set to DGNMODEL_GeoAttachmentHandling_DontReproject.
    DgnModelP   rootModel;
    if ( (NULL != (rootModel = refP->GetRoot())) && (DGNMODEL_GeoAttachmentHandling_Default != rootModel->GetGeoAttachmentHandling()) )
        return;

    // start non-undoble txn
    SaveSettingsTxnMark __nonUndo;

    // work backwards through parents until we find the topmost one with a coordinate system. That's the one we transform to.
    DgnGCSP         targetGCS;
    DgnModelRefP    targetModelRef;
    DgnGCS::GetReprojectionTarget (targetModelRef, targetGCS, refP, NULL);

    // if we couldn't find a parent modelRef with a geo coordinate system, can't transform.
    if ( (NULL == targetModelRef) || (NULL == targetGCS) )
        return;

    // If the reference has "useAlternateFile" set, it's an extraction, which came from an already-transformed cache, so don't transform it.
    if (refP->UseAlternateFile())
        {
        // even though it isn't really, tell the cache that it's been geographically reprojected, since it's an extraction of data that's been georeprojected.
        // NOTE: We really don't know if this is right or not - we don't know if the original (not extraction) was reprojected or not.
        refP->SetAttachMethod(ATTACHMETHOD_GeographicProjected);
        cache.SetIsGeographicReprojected (true);
        return;
        }

    DgnGCSP     sourceGCS;
    WChar       message[2048];
    WChar       detailMessage[2048];
    // if we've just filled a reference modelRef cache, see if both it and the master file have a coordinate system.
    if (NULL != (sourceGCS = DgnGCS::FromModel (refP, true)))
        {
        // if we get this far, we really want to reproject. We want refP->GetAttachMethod() to reflect ATTACHMETHOD_GeographicProjected to the user interface
        //   (even though it may actually be ATTACHMETHOD_Unknown for references attached prior to 08.11.xx). This isn't written immediately to the file, but if the user
        //   changes any other reference parameter that causes the reference attachment to be written, it is written permanently to the file.
        refP->SetAttachMethod(ATTACHMETHOD_GeographicProjected);
        
        if (refElementReprojected)
            {
            // we previously transformed the attachment in antipation that it might not have a GCS, so might not be reprojected. It does have a GCS, so we are reprojecting it, 
            // so we need to set the attach parameters back to untransformed.
            makeUntransformedAttachment (refP);
            }

        // if the source and target GCS are the same, even if there are dgn unit differences and/or linear LocalTransformers, we can calculate a linear transform and don't actually reproject.
        // if the coordinate systems are the same, we don't have to reproject.
        if (dgnGeoCoord_canSubstituteLinearTransformForReprojection (refP, &cache, sourceGCS, targetModelRef, targetGCS))
            {
            getLinearTransformMessages (message, _countof (message), detailMessage, _countof (detailMessage), refP);
            NotifyMessageDetails details (MESSAGE_INFO, message, detailMessage);
            NotificationManager::OutputMessage (details);
            return;
            }

        // if we can't successfully create the datum shift, then we're unable to proceed.
        if (NULL == sourceGCS->SetDatumConverter (*targetGCS))
            {
            reportDatumShiftError (sourceGCS, targetGCS, refP);
            return;
            }

        CacheReproject reprojector (refP, sourceGCS, targetModelRef, targetGCS);
        reprojector.DoReproject (false, false);
        cache.SetIsGeographicReprojected (true);

        bool        hasWarnings;
        bool        hasErrors;
        reprojector.GetFinishMessages (&hasWarnings, &hasErrors, message, detailMessage);
        NotifyMessageDetails details (hasErrors ? (MESSAGE_ERROR) : hasWarnings ? MESSAGE_WARNING : MESSAGE_INFO, message, detailMessage);
        NotificationManager::OutputMessage (details);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeoCoordinationAdmin::_CanShareDgnFile (DgnFileR testDgnFile, DgnAttachmentR refP) const
    {
    CompleteInitialization ();

    // We need to decide whether the existing testDgnFile is compatible with the modelRef from CacheFillContext.
    // Note: The files for which this method is called HAVE NOT BEEN previously reprojected. That is insured because
    //       we call dgnCache->IsFileImage (false) when we reproject, and fillModelCache calls dgnFile->SetShareFlag (false).
    //       We never get called for file s for which the ShareFlag is false.

    // if not configured for OTF transform, we don't care.
    if (!m_otfEnabled)
        return true;

    // If we're not going to transform the cache, we can share.
    if ( (ATTACHMETHOD_Unknown != refP.GetAttachMethod()) && (ATTACHMETHOD_GeographicProjected != refP.GetAttachMethod()) )
        return true;

    // If the reference has "useAlternateFile" set, we won't transform it, so we can share.
    if (refP.UseAlternateFile())
        return true;

    DgnModelRefP  parentModelRef = &refP.GetParentR();
    
    // if it's an unknown, only do if if master origin and reference origin are 0.0, and transform is identity.
    bool    refElementReprojected = false;
    if ( (ATTACHMETHOD_Unknown == refP.GetAttachMethod()) && !untransformedAttachment (refElementReprojected, parentModelRef, &refP))
        return true;

    DgnGCSP         targetGCS;
    DgnModelRefP    targetModelRef;
    DgnGCS::GetReprojectionTarget (targetModelRef, targetGCS, &refP, parentModelRef);

    // if we couldn't find a parent modelRef with a geo coordinate system, can't transform, so we can share.
    if ( (NULL == targetModelRef) || (NULL == targetGCS) )
        return true;

    // If the root parent is from the same file, we can't use this fileObj, because we don't want to share with the master file.
    if (parentModelRef->GetRoot()->GetDgnFileP() == &testDgnFile)
        return false;

    // the case that we are trying to veto is when our attachMethod allows for reprojection, and we have a target and
    //  source geo coordinate system so we could do the reprojection, but the testDgnFile hasn't been reprojected.
    //  In that case, we don't want to reproject that fileObj or we'll screw up the reference that is already using it.
    //  (When we get here, we know that testDgnFile has not been projected yet, because if it was it would have
    //   been rejected as a nonshareable file upstream of this check.)
    // A complication is that when we get here, refP does not yet have a cache assigned. We need to see if
    //  testDgnFile has a model of the desired name, and a cache that has been filled for that model. Then we see
    //  if that cache has a projection. If it

    WCharCP modelName = refP.GetAttachModelName();
    DgnModelId   modelID;
    if ((NULL == modelName) || (0 == *modelName))
        {
        /* the attachment element does not store a model name, use the default model */
        modelID = testDgnFile.GetDefaultModelId();
        }
    else
        {
        modelID = testDgnFile.FindModelIdByName(modelName);
        if (!modelID.IsValid())
            return true;
        }

    DgnModelP   refCache;
    // if we can't find the cache, it hasn't been loaded yet, and we're OK to share.
    if (NULL == (refCache = testDgnFile.FindLoadedModelById (modelID)))
        return true;

    // if the cache isn't filled, we're going through the cache reload associated with changing the reprojection option.
    if (!dgnModel_isFilled (refCache, DGNMODEL_SECTION_CONTROL_ELMS) && (ATTACHMETHOD_GeographicProjected == refP.GetAttachMethod()))
        return false;

    DgnGCSP     sourceGCS;
    if (NULL == (sourceGCS = DgnGCS::FromCache (refCache, true)))
        return true;

    // if we won't have to reproject (because the source and target GCS are the same, possibly with dgn unit differences and/or linear LocalTransformers for which we can calculate a linear transform), we can share.
    if (dgnGeoCoord_canSubstituteLinearTransformForReprojection (&refP, refCache, sourceGCS, parentModelRef, targetGCS))
        {
        // we have to set refP->GetAttachMethod() here - we would reproject, but we don't need to, and the cache has already been filled.
        // Thus, we won't go through the "ModelFillComplete" method above, and this is our only chance to indicate that the attachmethod is GeographicProjected.
        refP.SetAttachMethod(ATTACHMETHOD_GeographicProjected);
        return true;
        }

    // here we have a cache in testDgnFile, it can be transformed, but hasn't been (or we wouldn't have gotten here).
    // It is thus in use by another reference attachment (or something) and can't be used.
    return false;
    }


#define MDLERR_GEOCOORD_NOMASTERGCS (-481)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt dgnGeoCoord_reprojectToGCS (DgnModelRefP modelRef, DgnGCSP newGCS, bool reportProblems)
    {
    DgnGCSP     sourceGCS;
    if (NULL == (sourceGCS = DgnGCS::FromModel (modelRef, true)))
        return MDLERR_GEOCOORD_NOMASTERGCS;

    CacheReproject reprojector (modelRef, sourceGCS, modelRef, newGCS);
    reprojector.DoReproject (true, true);

    if (reportProblems)
        {
        bool        hasWarnings;
        bool        hasErrors;
        WChar       message[2048];
        WChar       detailMessage[2048];
        reprojector.GetFinishMessages (&hasWarnings, &hasErrors, message, detailMessage);
        NotifyMessageDetails details (hasErrors ? (MESSAGE_ERROR) : hasWarnings ? MESSAGE_WARNING : MESSAGE_INFO, message, detailMessage);
        NotificationManager::OutputMessage (details);
        }

    return SUCCESS;
    }
