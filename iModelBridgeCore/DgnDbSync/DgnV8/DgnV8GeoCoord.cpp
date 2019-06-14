/*----------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+----------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Bentley::GeoCoordinates::DgnGCSPtr fromDgnDbGcsToV8Gcs(Bentley::DgnModelR v8Model, BentleyApi::Dgn::DgnGCSCR dgnGCS, DgnDbR db, bool primary = true)
    {
    DgnV8Api::MSElement test66;
    memset ((void*)&test66, 0, 768*sizeof(short));
    test66.ehdr.type        = 66;//MICROSTATION_ELM;
    test66.ehdr.level       = MSAPPINFO_LEVEL;
    test66.ehdr.isGraphics  = FALSE;
    test66.ehdr.elementSize = 0x2fe;    // this is 2 less than (GEOCOORD66_SIZE + sizeof(ApplicationElm)) / sizeof(short), but it's what the old version size came out
    test66.ehdr.attrOffset  = 0x2fe;
    test66.applicationElm.signatureWord = MSGEOCOORD_SIGNATURE;
    uint32_t test66bytes;
    dgnGCS.PublishedCreateGeoCoordType66((short*)&test66.applicationElm.appData, test66bytes, db, primary);
    
    return Bentley::GeoCoordinates::DgnGCS::FromGeoCoordType66(&test66.applicationElm, &v8Model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnGCSPtr fromV8GcsToDgnDbGcs(Bentley::GeoCoordinates::DgnGCS const& v8Gcs, Bentley::DgnModelR v8Model, DgnDbR db, bool primary = true)
    {
    //  Since DgnDb uses the same binary format as DgnV8, we can "convert" it by just removing the type66 element header.
    DgnV8Api::MSElement test66;
    if (v8Gcs.CreateGeoCoordType66 (&test66.applicationElm, &v8Model, primary) != BSISUCCESS)
        return nullptr;

    // There are only a few cases where we use this output GCS for calculations, but in that case we need the globalOrigin and uorsPerBaseUnit set correctly in the DgnDb GCS.
    DgnGCSPtr dgnGcs = DgnGCS::FromGeoCoordType66AppData((short*)test66.applicationElm.appData, db);

    // This comes from the V8 code, but drops the little-used and poorly understood paperspace.
    double      uorPerStorage = dgnModel_getUorPerStorage (&v8Model);
    double      uorsPerBaseUnit = 100000.0;

    DgnV8Api::UnitInfo    storageUnitInfo;
    if (SUCCESS == dgnModel_getStorageUnit (&v8Model, &storageUnitInfo))
        uorsPerBaseUnit = (uorPerStorage * storageUnitInfo.numerator) / storageUnitInfo.denominator;

    DPoint3d globalOrigin;
    dgnModel_getGlobalOrigin (&v8Model, (Bentley::DPoint3d*)&globalOrigin);

    dgnGcs->SetGlobalOriginAndUnitScaling (globalOrigin, uorsPerBaseUnit);

    return dgnGcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialConverterBase::_SetOutputGCS()
    {
    //  See if the caller of the converter specifies a GCS
    iModelBridge::GCSDefinition gcsDefinition = _GetSpatialParams().GetOutputGcsDefinition();

    // if no GCS specified, that's ok we'll continue without it.
    if (!gcsDefinition.m_isValid)
        return DgnDbStatus::Success;
         
    // we have a specification, get the outputDgnGcs from it.
    m_outputDgnGcs = gcsDefinition.CreateGcs(GetDgnDb());
    if (!m_outputDgnGcs.IsValid() || !m_outputDgnGcs->IsValid())
        {
        // bad definition, can't continue.
        ReportError(Converter::IssueCategory::MissingData(), Converter::Issue::MissingGCS(), L"");
        return Dgn::DgnDbStatus::BadArg;
        }

    // store the outputDgnGcs into the BIM file.
    m_outputDgnGcs->Store(GetDgnDb());
    return Dgn::DgnDbStatus::Success;
    }


/*=================================================================================**//**
// This class determines whether a linear transform can be used in lieu of a full GCS Reprojection.
// It borrows heavily from the the DgnV8 ReferenceParameterCalculator class
// @bsiclass                                                     Barry.Bentley   02/17
+===============+===============+===============+===============+===============+======*/
struct  GCSTransformCalculator
{
// inputs
DgnV8ModelP             m_sourceModel;
DgnGCSPtr               m_sourceGCSPtr;             // The GCS from which we might be transforming (comes from the DgnV8 root model, or as specified on the command line as --input-GCS).

DgnGCSPtr               m_outputGCSPtr;             // The GCS to which we might be transforming (comes from the BIM file, or as specified on the command line as --output-GCS)
DPoint3dCP              m_fixedOutputGlobalOrigin;  // we have a non-NULL fixedOutputGlobalOrigin iff m_sourceModel isn't the source for the Global Origin (which happens when we aren't creating the BIM file from scratch).
DPoint3d                m_outputGlobalOrigin;       // the calculated output global origin, when m_fixedOutputGlobalOrigin is null.

DPoint3d                m_outputCenterPoint;        // the center point of the sourceGCS, in output coordinates.


// calculated directly from inputs
DPoint3d                m_sourceCenterPoint;
double                  m_sourceMetersPerUor;

double                  m_sourceUnitScale;

// results, if we can do it.
bool                    m_haveSolution;
Transform               m_calculatedTransform;

double                  m_maximumError;

GCSTransformCalculator
(
DgnV8ModelP             sourceModel,
DgnGCSPtr               sourceGCS,
DgnGCSPtr               outputGCS,
DPoint3dCP              fixedOutputGlobalOrigin,
bool                    makeAecTransform,
bool                    scaleToGcsGrid
)
    {
    m_haveSolution              = false;
    m_calculatedTransform.InitIdentity();

    m_sourceModel               = sourceModel;
    m_sourceGCSPtr              = sourceGCS;

    m_outputGCSPtr              = outputGCS;
    m_outputGlobalOrigin.Init(0.0, 0.0, 0.0);

    m_fixedOutputGlobalOrigin   = fixedOutputGlobalOrigin;

    // get the uors per meter.
    m_sourceMetersPerUor        = 1.0 / dgnModel_getUorPerMeter (sourceModel);

    // calculate sourceScale. Since destination units are meters, it's the same as sourceMetersPerUor.
    m_sourceUnitScale   = m_sourceMetersPerUor;

    // calculate the transform stuff.

    // The special case where the source and output have the same GCS, except for local transforms on either or both, can be handled analytically.
    bool    datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent;
    m_outputGCSPtr->Compare (*m_sourceGCSPtr.get(), datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent, false);
    if (!datumDifferent && !csDifferent && !verticalDatumDifferent)
        {
        // we know that the only difference (if any) is with the design file units or master origins, or a LocalTransformer.
        if (SolveAnalytically (localTransformDifferent))
            {
            m_haveSolution = true;
            return;
            }
        }

    // if we get here, there is no analytic solution. If the caller has called for an "AECTransform" approximation, calculate that.
    if (!makeAecTransform)
        return;

    // For the source origin, we want to use some reasonable point within the range of the projection as the center.
    // 1. Determine a reasonable point in lat long. Might be origin longitude, origin longitude, or central meridian, origin longitude.
    // 2. Find the XY corresponding to that lat/long in the source GCS system - that's the source origin.
    // 3. Find the XY corresponding to the same lat/long in the output file coordinate system - that's the output origin.
    GeoPoint    sourceGCSCenterPoint;
    if ( (BentleyApi::GeoCoordinates::BaseGCS::pcvLambertEqualAreaAzimuthal == sourceGCS->GetProjectionCode()) || (!GetGeoCenterPointFromModel (sourceGCSCenterPoint, m_sourceCenterPoint, m_sourceModel)) )
        {
        sourceGCS->GetCenterPoint (sourceGCSCenterPoint);
        sourceGCS->UorsFromLatLong (m_sourceCenterPoint, sourceGCSCenterPoint);
        }

    // we have to do any datum conversion because the outputOrigin is in the output BIM file GCS.
    GeoPoint    outputGCSCenterPoint;
    sourceGCS->LatLongFromLatLong (outputGCSCenterPoint, sourceGCSCenterPoint, *outputGCS.get());
    outputGCS->UorsFromLatLong (m_outputCenterPoint, outputGCSCenterPoint);

    // Get the rotation angle by sending two points, separated by 100 meters, through the geocoordinate transformation.
    DPoint3d points[2];
    double   oneHundredMetersInSourceUors = 100.0 / m_sourceMetersPerUor;
    GetTrueOutputXY (points[0], m_sourceCenterPoint.x, m_sourceCenterPoint.y);
    GetTrueOutputXY (points[1], m_sourceCenterPoint.x + oneHundredMetersInSourceUors, m_sourceCenterPoint.y);

    DVec3d  delta;
    delta.DifferenceOf (points[1], points[0]);
    // this is essentially the same as the "K" factor at the origin.
    double  gridToGroundScale = delta.Magnitude() / (oneHundredMetersInSourceUors * m_sourceUnitScale);

    // initialize the rotation matrix.
    RotMatrix               rotMatrix;
    rotMatrix.InitFromAxisAndRotationAngle (2, atan2 (delta.y, delta.x));

    // There are two possibilities - either we use a scale of 1.0, which assumes that the incoming data is in ground coordinates and we want to keep those,
    // or we scale to the GCS or "grid" coordinates.
    DPoint3d        sourceOrg   = m_sourceCenterPoint;
    DPoint3d        outputOrg   = m_outputCenterPoint;
    double          netScale    = m_sourceUnitScale * (scaleToGcsGrid ? gridToGroundScale : 1.0);

    rotMatrix.Multiply (sourceOrg);
    ((DVec3d *)&sourceOrg)->Scale (*( (DVec3d *)&sourceOrg),  netScale);

    DVec3d          translation;
    translation.DifferenceOf (outputOrg, *( (DVec3d *)&sourceOrg));

    m_calculatedTransform.InitIdentity ();
    m_calculatedTransform.SetTranslation (translation);
    m_calculatedTransform.InitProduct (m_calculatedTransform, rotMatrix);

    // if we're dealing with a 2d reference, set zScale to 1.0
    m_calculatedTransform.ScaleMatrixColumns (m_calculatedTransform,  netScale,  netScale,  m_sourceModel->Is3d() ? netScale : 1.0);

    m_haveSolution  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
~GCSTransformCalculator ()
    {
    // smartpointers are automatically destroyed, don't really need anything here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HaveLinearTransform (TransformR outputTransform, DPoint3dR outputGlobalOrigin)
    {
    outputTransform.Copy (m_calculatedTransform);
    outputGlobalOrigin = m_outputGlobalOrigin;
    return m_haveSolution;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetCenterPointFromModel (DPoint3dR centerPoint, DgnV8ModelP sourceModel)
    {
    Bentley::DRange3d        range;
    if (SUCCESS != sourceModel->GetRange(range))
        return false;

    DPoint3d        extent;
    extent.x = range.high.x - range.low.x;
    extent.y = range.high.y - range.low.y;

    // check the extent of the range. If it's too small, we got nothing.    
    if ( (extent.x <= 0) || (extent.y <= 0) )
        return false;

    // use center point as the origin.
    centerPoint.x = (range.low.x + range.high.x) / 2.0;
    centerPoint.y = (range.low.y + range.high.y) / 2.0;
    centerPoint.z = 0;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetGeoCenterPointFromModel (GeoPoint& sourceGCSCenterPoint, DPoint3dR centerPoint, DgnV8ModelP sourceModel)
    {
    // calculate the lat long center point.
    if (!GetCenterPointFromModel (centerPoint, sourceModel))
        return false;

    m_sourceGCSPtr->LatLongFromUors (sourceGCSCenterPoint, centerPoint);
    return true;;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
double GetMaximumError ()
    {
    m_maximumError = 0.0;

    // calculate the error at each corner of the range.
    // Start by getting design file range in uors.
    Bentley::DRange3d           range;
    m_sourceModel->GetRange(range);

    // convert to meters.
    GetMaxError (range.low.x, range.low.y);
    GetMaxError (range.low.x, range.high.y);
    GetMaxError (range.high.x, range.high.y);
    GetMaxError (range.high.x, range.low.y);

    return m_maximumError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            GetMaxError
(
double      x,
double      y
)
    {
    // calculate true XY
    DPoint3d    trueXY;
    GetTrueOutputXY (trueXY, x, y);

    DPoint3d    approximateXY;
    GetApproximateOutputXY (approximateXY, x, y);

    // calculate distance from approximate to true.
    double distanceError = trueXY.DistanceXY (approximateXY);

    if (distanceError > m_maximumError)
        m_maximumError = distanceError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetTrueOutputXY
(
DPoint3d&   trueXY,
double      x,
double      y
)
    {
    DPoint3d    inPoint;
    inPoint.Init (x, y, 0.0);
    m_sourceGCSPtr->ReprojectUors (&trueXY, NULL, NULL, &inPoint, 1, *m_outputGCSPtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetApproximateOutputXY
(
DPoint3d&   approximateXY,
double      x,
double      y
)
    {
    approximateXY.Init (x, y, 0.0);

    m_calculatedTransform.Multiply(approximateXY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SolveAnalytically (bool localTransformDifferent)
    {
    // here we have a source and output for which the GCS is identical except for possibly the local transform. 
    // In this case, we can calculate the matrix exactly.
    // 
    // Observe that C = Us Xs, where C is Cartesian coordinates, Us is the transform that goes from UORs to Cartesian, and Xs is the source UORs.
    //                        (Us is Ss Gs, where Ss is scale to cartesian and Gs subtracts off the global origin).
    // Also IC = Hs C whre IC is Internal Cartesian coordinates and Hs is the Helmert local transform, so IC = Hs Us Xs
    // Similarly, of course, IC = Ho Uo Xo, so Ho Uo Xo = Hs Us Xs.
    // Suppose Xo = T Xs. We want to calculate that T.
    // Write it as Ho Uo T Xs = Hs Us Xs
    // Therefore T = UoInv HoInv Hs Us

    // start by testing for local transforms.
    GeoCoordinates::HelmertLocalTransformer*    sourceHelmertTransformer    = NULL;
    GeoCoordinates::HelmertLocalTransformer*    outputHelmertTransformer    = NULL;
    if (localTransformDifferent)
        {
        GeoCoordinates::LocalTransformerP       outputTransformer       = m_outputGCSPtr->GetLocalTransformer();
        GeoCoordinates::LocalTransformerP       sourceTransformer       = m_sourceGCSPtr->GetLocalTransformer();

        // if there is a transformer, it must be a Helmert Transformer.
        if ( (NULL != sourceTransformer) && (NULL == (sourceHelmertTransformer = dynamic_cast <GeoCoordinates::HelmertLocalTransformer*> (sourceTransformer))) )
            return false;

        if ( (NULL != outputTransformer) && (NULL == (outputHelmertTransformer = dynamic_cast <GeoCoordinates::HelmertLocalTransformer*> (outputTransformer))) )
            return false;
        
        assert ( (NULL != sourceHelmertTransformer) || (NULL != outputHelmertTransformer) );
        }

    // Get HoInv:
    Transform   HoInv;
    if (NULL != outputHelmertTransformer)
        outputHelmertTransformer->GetCartesianFromInternalCartesianTransform (HoInv);
    else
        HoInv.InitIdentity();

    // Get Hs:
    Transform   Hs;
    if (nullptr != sourceHelmertTransformer)
        sourceHelmertTransformer->GetInternalCartesianFromCartesianTransform (Hs);
    else
        Hs.InitIdentity();

    double      gcsCartesianPerMeter    = m_sourceGCSPtr->UnitsFromMeters();    // we know that m_sourceGCSPtr same as m_outputGCSPtr, so it doesn't matter which we get it from.
    double      inverseScale            = 1.0 / gcsCartesianPerMeter;           // in the BIM file the UORs are always in meters.

    // We never need Ss and Gs separately, so just get Us:
    double              sourceScale = gcsCartesianPerMeter * m_sourceMetersPerUor;
    DPoint3d            sourceGlobalOrigin;
    dgnModel_getGlobalOrigin (m_sourceModel, (Bentley::DPoint3d*)&sourceGlobalOrigin);
    sourceGlobalOrigin.Negate();

    Transform   Us;
    Us.InitIdentity();
    Us.SetTranslation (sourceGlobalOrigin);
    Us.ScaleCompleteRows (Us, sourceScale, sourceScale, sourceScale);


    DPoint3d    outputGlobalOrigin;
    if (nullptr != m_fixedOutputGlobalOrigin)
        {
        outputGlobalOrigin = *m_fixedOutputGlobalOrigin;
        }
    else
        {
        // this is the case where we are given an output GCS, but we have to figure out the best global origin. The best global origin is the one that transforms the point (0,0,0) in the source 
        // cartesian coordinates to 0,0,0 in the destination cartesian coordinates.
        // In that case Hs Us [0,0,0] = Ho So Go [0,0,0], so SoInv HoInv Us  = Go [0,0,0]. But Go [0,0,0] is just Go inverting the signs of the three components.

        // We have HoInv from above. SoInv is just inverseScale in all diagonal positions

        // Calculate SoInv:
        Transform   SoInv;
        SoInv.InitIdentity();
        SoInv.ScaleMatrixColumns (SoInv, inverseScale, inverseScale, inverseScale);

        Transform   computedTransform;
        computedTransform.InitProduct (SoInv, HoInv);
        computedTransform.InitProduct (computedTransform, Us);

        DPoint3d    zeroVector = DPoint3d::From (0.0, 0.0, 0.0);
        computedTransform.Multiply (outputGlobalOrigin, zeroVector);
        
        m_outputGlobalOrigin = outputGlobalOrigin;
        outputGlobalOrigin.Negate();
        }

    Transform   UoInv;
    UoInv.InitIdentity();
    UoInv.ScaleMatrixColumns (UoInv, inverseScale, inverseScale, inverseScale);
    UoInv.SetTranslation (outputGlobalOrigin);

    // Calculate the overall transform:
    Transform   m_calulatedTransform;
    m_calculatedTransform.InitProduct (UoInv, HoInv);
    m_calculatedTransform.InitProduct (m_calculatedTransform, Hs);
    m_calculatedTransform.InitProduct (m_calculatedTransform, Us);

    return true;
    }


};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool haveAnySpatialData(DgnDbR db)
    {
    BeSQLite::EC::ECSqlStatement stmt;
    stmt.Prepare(db, "SELECT * FROM BisCore.GeometricElement3d LIMIT 1");
    return BeSQLite::BE_SQLITE_ROW == stmt.Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            SpatialConverterBase::ComputeTransformAndGlobalOriginFromRootModel (DgnV8ModelCR rootModel, bool adoptSourceGoIfBimIsEmpty)
    {
    double scaleFactor = ComputeUnitsScaleFactor (rootModel);

    // we always set the scale factor.
    m_rootTrans.ScaleMatrixColumns (scaleFactor, scaleFactor, scaleFactor);

    // Get the DgnV8 global origin from the root model.
    auto const& modelInfo = rootModel.GetModelInfo();
    DPoint3d* temp = (DPoint3d*)&modelInfo.GetGlobalOrigin();
    DPoint3d sourceGlobalOrigin = *temp;

    // Get Source global origin in meters
    m_rootTrans.Multiply (sourceGlobalOrigin);

    if (IsCreatingNewDgnDb() || (adoptSourceGoIfBimIsEmpty && !haveAnySpatialData(GetDgnDb())))
        {
        // If we are creating a new BIM file, we set the scaling and set the BIM's file to the DgnV8 model's (scaled) global origin.
        GetDgnDb().GeoLocation().SetGlobalOrigin(sourceGlobalOrigin);
        GetDgnDb().GeoLocation().Save();
        }
    else
        {
        // If we are importing a model into an existing BIM file, we have to adjust for the BIM file's global origin.
        DPoint3d    existingGlobalOrigin = GetDgnDb().GeoLocation().GetGlobalOrigin();
        existingGlobalOrigin.Subtract (sourceGlobalOrigin);
        m_rootTrans.SetTranslation (existingGlobalOrigin);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static iModelBridge::GCSCalculationMethod GetGcsCalculationMethodFromSourceGcs (Dgn::DgnGCSPtr sourceGcs)
    {
    WCharCP projectionName = sourceGcs->GetProjection();
    if ( (NULL == projectionName) || (0 != wcscmp (L"AZMEA", projectionName)) )
        return iModelBridge::GCSCalculationMethod::UseReprojection;

    // we've got an AZMEA, if it's not a library GCS, use transform, else use reprojection.
    WCharCP gcsName = sourceGcs->GetName();
    return ( (NULL == gcsName) || (0 == *gcsName) ) ? iModelBridge::GCSCalculationMethod::UseGcsTransform : iModelBridge::GCSCalculationMethod::UseReprojection;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileStatus SpatialConverterBase::_ComputeCoordinateSystemTransform()
    {
    DgnV8ModelP     rootModel;
    Dgn::DgnGCSPtr  sourceGcs;

    // First, get the GCS from the source rootModel.
    if (nullptr != (rootModel = GetRootModelP()))
        {
        // Make sure the source DgnV8 has linear units (either "unknown" or "degree" are invalid)
        if (ComputeUnitsScaleFactor (*rootModel) <= 0.0)
            {
            ReportError(Converter::IssueCategory::Compatibility(), Converter::Issue::IllegalUnits(), "");
            return DgnV8Api::DGNFILE_STATUS_UnknownError;
            }

        // make sure the root model control section is loaded so we can get the sourceGCS from it.
        bool needsFilling = (DgnV8Api::DgnModelSections::ControlElements != rootModel->IsFilled(DgnV8Api::DgnModelSections::ControlElements));
        if (needsFilling)
            rootModel->FillCache(DgnV8Api::DgnModelSections::ControlElements);

        // Get the DgnV8 version of the GCS and convert it to the BIM equivalent.
        auto dgnGcsP = Bentley::GeoCoordinates::DgnGCS::FromModel(rootModel, true);
        if (nullptr != dgnGcsP)
            sourceGcs = fromV8GcsToDgnDbGcs(*dgnGcsP, *rootModel, GetDgnDb());

        // get rid of the loaded control elements.
        if (needsFilling)
            rootModel->Empty();

        // if the sourceGcs isn't valid, see if the user specified --input-GCS on the command line.
        if (!sourceGcs.IsValid())
            {
            auto inputGcsDef = _GetSpatialParams().GetInputGcsDefinition();
            if (inputGcsDef.m_isValid)
                sourceGcs = inputGcsDef.CreateGcs(GetDgnDb());
            }
        }
    else
        {
        BeAssert (false);
        return DgnV8Api::DGNFILE_STATUS_UnknownError;
        }

    // Now, get the GCS from the BIM file.
    // If the BIM file already has a GCS, we are going to use that, and ignore any --output-GCS on the command line.
    m_outputDgnGcs = GetDgnDb().GeoLocation().GetDgnGCS();
    if (!m_outputDgnGcs.IsValid())
        {
        // The BIM file doesn't yet have a GCS. Get it from the command line argument, if specified.
        // _SetOutputGCS either sets m_outputDgnGcs or leaves the BIM without a GCS. Either is fine.
        if (Dgn::DgnDbStatus::Success != _SetOutputGCS())   
            return DgnV8Api::DGNFILE_STATUS_UnknownError;
        }
    
    // if the BIM file doesn't have a GCS, the source GCS (if there is one) becomes the BIM's GCS.
    if (!m_outputDgnGcs.IsValid())
        {
        // We don't have an existing GCS transform to worry about. Compute the transform and set the Global Origin if it's a new GCS.
        ComputeTransformAndGlobalOriginFromRootModel (*rootModel, true);

        // If the root DGN model DOES have a GCS, it becomes the BIM files's GCS. This is a common case.
        if (sourceGcs.IsValid())             
            {
            m_outputDgnGcs = sourceGcs;
            m_outputDgnGcs->Store(GetDgnDb());
            }
        else
            {// Check whether we have an ECFlocation
            auto    ecefLocation = m_dgndb->GeoLocation().GetEcefLocation();
            if (!ecefLocation.m_isValid )//Do not overwrite if it is already defined.
                {
                auto newLocation = _GetSpatialParams().GetEcefLocation();
                if (newLocation.m_isValid)
                    m_dgndb->GeoLocation().SetEcefLocation(_GetSpatialParams().GetEcefLocation());
			    return DgnV8Api::DGNFILE_STATUS_Success;
                }
            }
/*<==*/ return DgnV8Api::DGNFILE_STATUS_Success;
        }

    // If we get here, the BIM file has a GCS, either pre-existing or specified.
    if (!sourceGcs.IsValid())             
        {
        // No sourceGcs from either the model or command line. 
        //We must assume that root DGN model's origin lines up with 0,0,0 of the BIM file. Just apply units scaling.
        
        ComputeTransformAndGlobalOriginFromRootModel (*rootModel, true);
/*<==*/ return DgnV8Api::DGNFILE_STATUS_Success;
        }
    
    // Here, the DgnDb AND the root DGN model both have a GCS. Either we can do a linear transform or we need a full reprojection.
    // Use GCSTransformCalculator to see whether we can get a linear transform.

    // See if we already have a global origin.
    DPoint3dCP  globalOriginP = nullptr;
    DPoint3d    existingGlobalOrigin;
    if (!IsCreatingNewDgnDb())
        {
        existingGlobalOrigin = GetDgnDb().GeoLocation().GetGlobalOrigin();
        globalOriginP = &existingGlobalOrigin;
        }

    bool useGcsTransform = false;
    bool scaleToGcsGrid  = false;

    // Get the GCS calculation method:
    //  UseDefault              - Base conversion method on source GCS. If it is unnamed AZMEA, assume that comes from placemarks and use Transform, else use Reproject
    //  UseReprojection         - always reproject (unless we can use exact transform, which happens when source and dest GCS are the same).
    //  UseGcsTransform         - use a calculated linear transform without scaling.
    //  UseGcsTransformScaled   - use a calculated linear transform, including the ground to grid scaling (K-factor).
    iModelBridge::GCSCalculationMethod method = _GetSpatialParams().GetGcsCalculationMethod ();
    if (iModelBridge::GCSCalculationMethod::UseDefault == method)
        {
        method = GetGcsCalculationMethodFromSourceGcs (sourceGcs);
        }

    if (iModelBridge::GCSCalculationMethod::UseReprojection == method)
        {
        useGcsTransform = false;
        scaleToGcsGrid  = false;
        }
    else if (iModelBridge::GCSCalculationMethod::UseGcsTransform == method)
        {
        useGcsTransform = true;
        scaleToGcsGrid  = false;
        }
    else if (iModelBridge::GCSCalculationMethod::UseGcsTransformWithScaling == method)
        {
        useGcsTransform = true;
        scaleToGcsGrid  = true;
        }
    else
        {
        BeAssert(false && "incorrect GCSCalculationMethod");
        }

    GCSTransformCalculator calculator (rootModel, sourceGcs, m_outputDgnGcs, globalOriginP, useGcsTransform, scaleToGcsGrid);
    Transform   calculatedTransform;
    DPoint3d    calculatedGlobalOrigin;
    if (calculator.HaveLinearTransform (calculatedTransform, calculatedGlobalOrigin))
        {
        // For equivalent GCSs, a simple offset and rotate is enough to reconcile the two.
        m_rootTrans = calculatedTransform;
        // If we are creating a new DgnDb, we have calculated the best global origin.
        if (IsCreatingNewDgnDb())
            {
            GetDgnDb().GeoLocation().SetGlobalOrigin (calculatedGlobalOrigin);
            GetDgnDb().GeoLocation().Save();
            }
/*<==*/ return DgnV8Api::DGNFILE_STATUS_Success;
        }

    // If we get here, the sourceGCS and outputGCS differ such that a linear transform will not work.
    // We must do reproject the V8 RootDgnModel into the outputGCS.
    Bentley::GeoCoordinates::DgnGCSPtr  outputV8Gcs = fromDgnDbGcsToV8Gcs(*rootModel, *m_outputDgnGcs, GetDgnDb());
    BeAssert (outputV8Gcs.IsValid());

    // Fill the root model cache so we can reproject it.
    if (true)
        {
        auto fillMsg = ProgressMessage::GetString(ProgressMessage::TASK_FILLING_V8_MODELS());
        DgnV8Api::IDgnProgressMeter::TaskMark loading(Bentley::WString(fillMsg.c_str(),true).c_str());
        rootModel->FillCache(DgnV8Api::DgnModelSections::All);
        }

    // reproject the root model
    StatusInt   result = dgnGeoCoord_reprojectToGCS (rootModel, outputV8Gcs.get(), true);

    // put the outputGcs GCS
    outputV8Gcs->ToModel (rootModel, true, false, false, false);

    // reload the references that are already loaded, since they need to be reprojected.
    rootModel->ReloadNestedDgnAttachments (true);

    // We have reprojected the root model to the correct GCS, but we still have to account for the global origin and scaling.
    ComputeTransformAndGlobalOriginFromRootModel (*rootModel, false);

    return DgnV8Api::DGNFILE_STATUS_Success;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
