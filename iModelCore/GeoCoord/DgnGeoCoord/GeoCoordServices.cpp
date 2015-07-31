/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/GeoCoordServices.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <Geom\msgeomstructs.hpp>
#include    <DgnGeoCoord\DgnGeoCoord.h>
#include    <DgnGeoCoord\DgnGeoCoordApi.h>
#include    <DgnPlatform\DgnCoreAPI.h>
#include    <DgnPlatform\IGeoCoordServices.h>
// needed for LoadLibraryW
#include <windows.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {



/*=================================================================================**//**
* This is the class that calculates reference the transform parameters.
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
struct  ReferenceParameterCalculator
{
DgnModelP           m_refCache;             // in the case where we are checking to see if we can re-use a dgnCache (CanShareDgn), m_refModelRef doesn't yet have a cache.
DgnModelRefP        m_parentModelRef;
DgnGCSPtr           m_refGCSPtr;
DgnGCSPtr           m_parentGCSPtr;
double              m_refMetersPerUor;
double              m_masterMetersPerUor;
double              m_refMasterUnitsPerMeter;
bool                m_haveSolution;

DPoint3d            m_masterOrigin;
DPoint3d            m_refOrigin;
double              m_refUnitScale;
double              m_coordSysScale;
RotMatrix           m_rotMatrix;
Transform           m_transform;
double              m_maximumError;
DPoint3dCP          m_fixedMasterOrigin;        // we have a non-NULL fixedMasterOrigin iff we are trying to calculate a Transform that can be used instead doing a Reprojection.

ReferenceParameterCalculator
(
DgnModelP       refCache,
DgnGCSP         refGCS,
DgnModelRefP    parentModelRef,
DgnGCSP         parentGCS,
bool            analyticalOnly,
DPoint3dCP      fixedMasterOrigin
)
    {
    m_haveSolution      = false;

    m_refCache          = refCache;
    m_refGCSPtr         = refGCS;
    m_parentModelRef    = parentModelRef;
    m_parentGCSPtr      = parentGCS;
    m_fixedMasterOrigin = fixedMasterOrigin;

    // get the uors per meter.
    m_refMetersPerUor           = 1.0 / dgnModel_getUorPerMeter (refCache);
    m_masterMetersPerUor        = 1.0 / dgnModel_getUorPerMeter (parentModelRef->GetDgnModelP());
    m_refMasterUnitsPerMeter    = 1.0 / (m_refMetersPerUor * dgnModel_getUorPerMaster (refCache));

    // calculate refScale and set maximumError.
    m_refUnitScale   = m_refMetersPerUor / m_masterMetersPerUor;
    m_maximumError   = 0.0;

    // calculate the transform stuff.

    // The special case where the reference and master have the same GCS, except for local transforms on either or both, can be handled analytically.
    bool    datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent;
    m_parentGCSPtr->Compare (*m_refGCSPtr.get(), datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent, false);
    if (!datumDifferent && !csDifferent && !verticalDatumDifferent)
        {
        // we know that the only difference (if any) is with the design file units or master origins, or a LocalTransformer.
        if (SolveAnalytically (localTransformDifferent))
            {
            m_haveSolution = true;
            return;
            }
        }

    // if we only want to check for an analytical solution, abort here (m_haveSolution still false).
    if (analyticalOnly)
        return;

    // For the reference origin, we want to use some reasonable point within the range of the projection as the center.
    // For a while I thought that falseEasting/falseNorthing equated to the originlongitude/originLatitude, but that is not
    // completely correct, and probably only right for azimuthal coordinate systems. So now we do this:
    // 1. Determine a reasonable point in lat long. Might be origin longitude, origin longitude, or central meridian, origin longitude.
    // 2. Find the XY corresponding to that lat/long in the reference coordinate system - that's the reference origin.
    // 3. Find the XY corresponding to the same lat/long in the master file coordinate system - that's the master origin.

    GeoPoint    refGCSCenterPoint;
    if ( (BaseGCS::pcvLambertEqualAreaAzimuthal == refGCS->GetProjectionCode()) || (!GetGeoCenterPointFromModel (refGCSCenterPoint, m_refOrigin, refCache)) )
        {
        refGCS->GetCenterPoint (refGCSCenterPoint);
        refGCS->UorsFromLatLong (m_refOrigin, refGCSCenterPoint);
        }

    // we have to do any datum conversion because the masterOrigin is in the master file coordinate system.
    GeoPoint    parentCenterPoint;
    refGCS->LatLongFromLatLong (parentCenterPoint, refGCSCenterPoint, *parentGCS);
    parentGCS->UorsFromLatLong (m_masterOrigin, parentCenterPoint);

    if (!refCache->Is3d() && !parentModelRef->Is3d())
        {
        m_masterOrigin.z = 0.0;
        m_refOrigin.z    = 0.0;
        }

    // Get the rotation by sending two points, separated by 100 meters, through the geocoordinate transformation.
    DPoint3d points[2];
    double   oneHundredMetersInRefUors = 100.0 / m_refMetersPerUor;
    GetTrueParentXY (points[0], m_refOrigin.x, m_refOrigin.y);
    GetTrueParentXY (points[1], m_refOrigin.x + oneHundredMetersInRefUors, m_refOrigin.y);

    DVec3d  delta;
    delta.differenceOf (&points[1], &points[0]);
    m_rotMatrix.initFromAxisAndRotationAngle (2, atan2 (delta.y, delta.x));

    m_coordSysScale = delta.magnitude() / (oneHundredMetersInRefUors * m_refUnitScale);
    m_haveSolution  = true;

    // calculate the 4x3 transform
    GetTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetCenterPointFromModel (DPoint3dR centerPoint, DgnModelP refCache)
    {
    DRange3d        cacheRange;
    if (SUCCESS != refCache->GetRange(cacheRange))
        return false;

    DPoint3d        extent;
    extent.x = cacheRange.high.x - cacheRange.low.x;
    extent.y = cacheRange.high.y - cacheRange.low.y;

    // check the extent of the range. If it's too small, we got nothing. If it's too large, it would be inadvisable to use placemarks based on it.
    if ( (extent.x <= 0) || (extent.y <= 0) )
        return false;

    // use center point as the origin.
    centerPoint.x = (cacheRange.low.x + cacheRange.high.x) / 2.0;
    centerPoint.y = (cacheRange.low.y + cacheRange.high.y) / 2.0;
    centerPoint.z = 0;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetGeoCenterPointFromModel (GeoPoint& refGCSCenterPoint, DPoint3dR centerPoint, DgnModelP dgnCache)
    {
    // calculate the lat long center point.
    if (!GetCenterPointFromModel (centerPoint, dgnCache))
        return false;

    m_refGCSPtr->LatLongFromUors (refGCSCenterPoint, centerPoint);
    return true;;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
~ReferenceParameterCalculator ()
    {
    // smartpointers are automatically destroyed, don't really need anything here.
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
double GetMaximumError ()
    {
    m_maximumError = 0.0;

    // calculate the error at each corner of the range.
    // Start by getting design file range in uors.
    DRange3d           range;
    m_refCache->GetRange(range);

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
    GetTrueParentXY (trueXY, x, y);

    DPoint3d    approximateXY;
    GetApproximateParentXY (approximateXY, x, y);

    // calculate distance from approximate to true.
    double distanceError = trueXY.distanceXY (&approximateXY);

    // the distance error we have calculated is in master file UORS. We want to report it in reference master units.
    distanceError = (distanceError * m_masterMetersPerUor) * m_refMasterUnitsPerMeter;

    if (distanceError > m_maximumError)
        m_maximumError = distanceError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetTrueParentXY
(
DPoint3d&   trueXY,
double      x,
double      y
)
    {
    DPoint3d    inPoint;
    inPoint.init (x, y);
    m_refGCSPtr->ReprojectUors (&trueXY, NULL, NULL, &inPoint, 1, *m_parentGCSPtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetApproximateParentXY
(
DPoint3d&   approximateXY,
double      x,
double      y
)
    {
    approximateXY.init (x, y, 0.0);

    m_transform.Multiply(approximateXY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetTransform
(
)
    {
    DPoint3d        refOrg      = m_refOrigin;
    DPoint3d        mastOrg     = m_masterOrigin;
    DVec3d          translation;
    double          netScale = m_refUnitScale * m_coordSysScale;

    m_rotMatrix.Multiply(refOrg);
    ((DVec3d *)&refOrg)->Scale (*( (DVec3d *)&refOrg),  netScale);
    translation.DifferenceOf (mastOrg, *( (DVec3d *)&refOrg));

    m_transform.InitIdentity ();
    m_transform.TranslateInLocalCoordinates (m_transform,  translation.x,  translation.y,  translation.z);
    m_transform.InitProduct (m_transform,m_rotMatrix);

    // if we're dealing with a 2d reference, set zScale to 1.0
    m_transform.ScaleMatrixColumns (m_transform,  netScale,  netScale,  m_refCache->Is3d() ? netScale : 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/07
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool    doublesEqual
(
double          dist1,
double          dist2
)
    {
    double  doubleTol = 1e-8;

    return ! (fabs (dist1 - dist2) > doubleTol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SolveAnalytically (bool localTransformDifferent)
    {
    // here we have two files for which the GCS is identical except for possibly the local transform. 
    // In this case, we can calculate the matrix exactly rather than use the two point method.
    // 
    // Observe that C = Ur Xr, where C is Cartesian coordinates, Ur is the transform that goes from UORs to Cartesian, and Xr is the reference UORs.
    //                        (Ur is Sr Gr, where Sr is scale to cartesian and Gr subtracts off the global origin).
    // Also IC = Hr C whre IC is Internal Cartesian coordinates and Hr is the Helmert local transform, so IC = Hr Ur Xr
    // Similarly, of course, IC = Hp Up Xp, so Hp Up Xp = Hr Ur Xr.
    // Also Xp = T Xr. We want to calculate that T.
    // So Hp Up T Xr = Hr Ur Xr
    // Therefore T = UpInv HpInv Hr Ur

    // start by testing for transforms.
    HelmertLocalTransformer*    refHelmertTransformer       = NULL;
    HelmertLocalTransformer*    parentHelmertTransformer    = NULL;
    if (localTransformDifferent)
        {
        LocalTransformerP           parentTransformer       = m_parentGCSPtr->GetLocalTransformer();
        LocalTransformerP           refTransformer          = m_refGCSPtr->GetLocalTransformer();

        // if there is a transformer, it must be a Helmert Transformer.
        if ( (NULL != refTransformer) && (NULL == (refHelmertTransformer = dynamic_cast <HelmertLocalTransformer*> (refTransformer))) )
            return false;

        if ( (NULL != parentTransformer) && (NULL == (parentHelmertTransformer = dynamic_cast <HelmertLocalTransformer*> (parentTransformer))) )
            return false;
        
        assert ( (NULL != refHelmertTransformer) || (NULL != parentHelmertTransformer) );
        }

    // Calculate UpInv:
    double      gcsCartesianPerMeter    = m_refGCSPtr->UnitsFromMeters();
    double      inverseScale            = 1.0 / (m_masterMetersPerUor * gcsCartesianPerMeter);
    DPoint3d    parentGlobalOrigin;
    dgnModel_getGlobalOrigin (m_parentModelRef->GetDgnModelP(), &parentGlobalOrigin);

    Transform   UpInv;
    UpInv.initIdentity();
    UpInv.scaleMatrixColumns (&UpInv, inverseScale, inverseScale, inverseScale);
    UpInv.setTranslation (&parentGlobalOrigin);

    // Get HpInv:
    Transform   HpInv;
    if (NULL != parentHelmertTransformer)
        parentHelmertTransformer->GetCartesianFromInternalCartesianTransform (HpInv);
    else
        HpInv.initIdentity();

    // Get Hr:
    Transform   Hr;
    if (NULL != refHelmertTransformer)
        refHelmertTransformer->GetInternalCartesianFromCartesianTransform (Hr);
    else
        Hr.initIdentity();

    // Get Ur:
    double      refScale                = gcsCartesianPerMeter * m_refMetersPerUor;
    DPoint3d    refGlobalOrigin;
    dgnModel_getGlobalOrigin (m_refCache, &refGlobalOrigin);
    refGlobalOrigin.negate();

    Transform   Ur;
    Ur.initIdentity();
    Ur.setTranslation (&refGlobalOrigin);
    Ur.scaleCompleteRows (&Ur, refScale, refScale, refScale);

    // Calculate the overall transform:
    Transform   T;
    T.productOf (&UpInv, &HpInv);
    T.productOf (&T, &Hr);
    T.productOf (&T, &Ur);

    RotMatrix   rotMatrix;
    DVec3d      scales;
    if (!FactorMatrixWithColumnScales (T, rotMatrix, scales))
        {
        assert (false);
        return false;
        }

    // Note: scales.z allowed to be different from scales.x, scales.y.
    if (!doublesEqual (scales.x, scales.y))
        {
        assert (false);
        return false;
        }

    // the rotMatrix is allowed to vary from a pure rotation matrix to include a scaling in the Z direction.
    if (!doublesEqual (scales.z / scales.x, 1.0))
        rotMatrix.form3d[2][2] = rotMatrix.form3d[2][2] * scales.z / scales.x;

    m_coordSysScale = scales.x / m_refUnitScale;
    m_transform     = T;
    m_rotMatrix     = rotMatrix;

    // If we have a fixed Master Origin, that's because we are trying to find a transform to use in place of a Reprojection.
    // In that case, we must keep the Master Origin invariant, because clip points are relative to it, and we don't want to change them.
    if (NULL != m_fixedMasterOrigin)
        {
        Transform   inverseT;
        bool        invertible = inverseT.inverseOf (&T);
        if (!invertible)
            {
            // I don't expect this to ever happen.
            assert (false);
            return false;
            }
        m_masterOrigin = *m_fixedMasterOrigin;
        m_refOrigin = m_masterOrigin;
        inverseT.multiply (&m_refOrigin);
        }
    else
        {
        // use the center of the data in the reference as the ref center point
        if (!GetCenterPointFromModel (m_refOrigin, m_refCache))
            m_refOrigin.zero();

        // calculate the master origin.
        m_masterOrigin = m_refOrigin;
        T.multiply (&m_masterOrigin);
        }

    return true;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SetReferenceParameters (DgnAttachmentP refP, bool setMasterZ)
    {
    // note: changing mast_org.z is used on initial attach. Subsequent calculations on file open don't do that because it can be set by the user to the desired elevation.
    DPoint3dR   masterOrigin    = refP->GetStoredMasterOriginR();
    DPoint3dR   refOrigin       = refP->GetRefOriginR();
    masterOrigin.x              = this->m_masterOrigin.x;
    masterOrigin.y              = this->m_masterOrigin.y;
    if (setMasterZ)
        masterOrigin.z          = this->m_masterOrigin.z;

    refOrigin                   = this->m_refOrigin;
    refP->SetDisplayScale (this->m_refUnitScale * this->m_coordSysScale);
    refP->GetRotMatrixR()       = this->m_rotMatrix;
    refP->SetScaleByUnits (true);
    refP->SetStoredScale (this->m_coordSysScale);
    }

private:
#if defined (DONT_NEED_YET)
// This code came from Earlin Lutz after I asked for a solution to taking a Helmert 4x3 transform and decomposing it into the parameters stored 
// in a reference attachment: Master Origin, Reference Origin, rotation matrix, and scale:

// reference transform construction sequence
// A reference file transform is constructed with the sequence
//    trans.initFrom (&display.trns_mtrx, &masterOrg);
//    trans.scaleMatrixColums (&trans, scale.x, scale.y, scale.z);
//    trans.translateInLocalCoordinates (&trans, -display.ref_org.x, -display.ref_org.y, -display.ref_org.z);
// Symbolically, the matrix part M and translation T are
// [M T] = [trns_mtrx  masterOrg] * [diag(sx,sy,sz] 0] * [I -ref_org]
// Where
//    masterOrg and refOrg are origins of the two files.
//    trns_mtrx is an (arbitrary) 3x3 matrix
//    scale.x,scale.y,scale.z are scale factors for respective axes of the ref file data.
// Hence
//     M = trns_mtrx * diag(sx,sy,sz)
//     T = masterOrg - trns_mtrx * diag(scale.x,scale.y,scale.z) * ref_org
//
// Inversions:
// WARNING 1: Once the master and reference file origins are swirled into the transform, they cannot be separated
//    BUT... If one of the two origins is known, the transform contents uniquely determine the other.
//                (See referenceFile_recoverRefOrigin, referenceFile_recoverMasterOrigin)
// WARNING 2: Once the matrix and scales are swirled into the transform, they cannot be separated.
//    BUT... (a) If the matrix part was a had all column lengths 1, they can be separated (see referenceFile_factorMatrixWithColumnScales)
//    OR ... (b) If the scales are known the original matrix can be recovered (see referenceFile_recoverUnscaledMatrix)
//    OR ... (c) If the matrix is known, the original scales can be recovered (see referenceFile_recoverScales)

//! @param [out] computed transform
//! @param [in] masterOrigin putdown point in master file
//! @param [in] referenceOrigin pickup point in master file.
//! @param [in] matrix matrix whose columns are putdown directions of x,y,z axes from the reference file
//! @param [in] axisScales scale factors to be applied to axis direction in the reference file (with referenceOrigin as fixed point)
void BuildReferenceTransform (TransformR transform, DPoint3dCR masterOrigin, DPoint3d referenceOrigin, RotMatrixCR matrix, DVec3dCR axisScales)
    {
    transform.initFrom (&matrix, &masterOrigin);
    transform.scaleMatrixColumns (&transform, axisScales.x, axisScales.y, axisScales.z);
    transform.translateInLocalCoordinates (&transform, -referenceOrigin.x, -referenceOrigin.y, -referenceOrigin.z);
    }

//! @param [in] knownTransform reference file transform as constructed by referenceFile::GetTransformToParent
//! @param [in] knownRefOrigin origin used as the ref file origin when knownTransform was constructed.
//! @param [out] masterOrigin computed master file origin
//! @return false (FAILURE) if the matrix part of knownTransform cannot be inverted.
bool RecoverReferenceRefOrigin (TransformCR knownTransform, DPoint3dCR knownMasterOrigin, DPoint3dR displayRefOrigin)
    {
    RotMatrix matrix, inverse;
    matrix.initFrom (&knownTransform);
    DPoint3d translation;
    knownTransform.getTranslation (&translation);
    DVec3d shift;
    shift.differenceOf (&knownMasterOrigin, &translation);
    displayRefOrigin.zero ();
    if (!inverse.inverseOf (&matrix))
        return false;
    inverse.multiply (&displayRefOrigin, &shift);
    return true;
    }

//! @param [in] knownTransform reference file transform as constructed by referenceFile::GetTransformToParent
//! @param [in] knownRefOrigin origin used as the ref file origin when knownTransform was constructed.
//! @param [out] masterOrigin computed master file origin
void RecoverReferenceMasterOrigin (TransformCR knownTransform, DPoint3dCR knownRefOrigin, DPoint3dR masterOrg)
    {
    RotMatrix matrix;
    matrix.initFrom (&knownTransform);
    DPoint3d knownOrigin;
    knownTransform.getTranslation (&knownOrigin);
    DVec3d delta;
    matrix.multiply (&delta, &knownRefOrigin);
    masterOrg.sumOf (&knownOrigin, &delta);
    }                                                                                                 
#endif

//! @param [in] knownTransform reference file transform as constructed by referenceFile::GetTransformToParent
//! @param [out] matrix matrix with unit length columns
//! @param [out] scales original length of columns
//! @return false (FAILURE) if the length (scale) from any column of the matrix is zero.
bool FactorMatrixWithColumnScales (TransformCR knownTransform, RotMatrixR matrix, DVec3dR scales)
    {
    DVec3d columnX, columnY, columnZ;
    knownTransform.getMatrixColumn (&columnX, 0);
    knownTransform.getMatrixColumn (&columnY, 1);
    knownTransform.getMatrixColumn (&columnZ, 2);
    scales.x = columnX.magnitude ();
    scales.y = columnY.magnitude ();
    scales.z = columnZ.magnitude ();
    matrix.initIdentity ();
    if (scales.x == 0.0 || scales.y == 0.0 || scales.z == 0.0)
        return false;
    columnX.scale (1.0 / scales.x);
    columnY.scale (1.0 / scales.y);
    columnZ.scale (1.0 / scales.z);
    matrix.initFromColumnVectors (&columnX, &columnY, &columnZ);
    return true;
    }

#if defined (NOT_NEEDED_YET)
//! @param [in] knownTransform reference file transform as constructed by referenceFile::GetTransformToParent
//! @param [in] knownScales scales as used when knownTransform was constructed.
//! @param [out] matrix descaled matrix
//! @return false (FAILURE) if any of the known scales are zero.
bool RecoverReferenceUnscaledMatrix (TransformCR  knownTransform, DVec3dCR knownScales, RotMatrixR matrix)
    {
    matrix.initIdentity ();
    if (knownScales.x == 0.0 || knownScales.y == 0.0 || knownScales.z == 0.0)
        return false;
    DVec3d columnX, columnY, columnZ;
    knownTransform.getMatrixColumn (&columnX, 0);
    knownTransform.getMatrixColumn (&columnY, 1);
    knownTransform.getMatrixColumn (&columnZ, 2);
    columnX.scale (1.0 / knownScales.x);
    columnY.scale (1.0 / knownScales.y);
    columnZ.scale (1.0 / knownScales.z);
    matrix.initFromColumnVectors (&columnX, &columnY, &columnZ);
    return true;
    }

//! @param [in] knownTransform reference file transform as constructed by referenceFile::GetTransformToParent
//! @param [in] originalMatrix scales as used when knownTransform was constructed.
//! @param [out] matrix descaled matrix
//! @return false (FAILURE) if (1) knownMatrix is singular or (2) knownMatrix differs from matrix part of knownTransform by "more than" column scaling.
bool RecoverReferenceScales (TransformCR  knownTransform, RotMatrixCR originalMatrix, DVec3dR scales)
    {
    RotMatrix matrix;
    RotMatrix originalInverse;
    RotMatrix scaleMatrix;
    matrix.initFrom (&knownTransform);
    scales.zero ();
    if (!originalInverse.inverseOf (&originalMatrix))
        return false;
    scaleMatrix.productOf (&originalInverse, &matrix);
    if (scaleMatrix.isDiagonal ())
        return false;
    scales.x = scaleMatrix.getComponentByRowAndColumn (0,0);
    scales.y = scaleMatrix.getComponentByRowAndColumn (1,1);
    scales.z = scaleMatrix.getComponentByRowAndColumn (2,2);
    return true;
    }
#endif

};

typedef StatusInt       (*EditRefReprojectionSettingsFunc) (DgnModelRefListP modelRefList);
typedef StatusInt       (*SaveDefaultRefReprojectionSettingsFunc)(DgnModelRefP modelRef);
static void dummy() {;}

/*=================================================================================**//**
* This is the class that implements geocoordinate reference interface.
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
class   GeoCoordinateServices : public IGeoCoordinateServices
{
private:
bool                                        m_lookedForSaveDefaultSettingsFunction;
bool                                        m_lookedForEditReprojectionSettingsFunction;
SaveDefaultRefReprojectionSettingsFunc      m_saveDefaultRefReprojectionSettingsFunc;
EditRefReprojectionSettingsFunc             m_editRefReprojectionSettingsFunc;

public:
GeoCoordinateServices ()
    {
    // we look these functions up at runtime when needed.
    m_lookedForSaveDefaultSettingsFunction      = false;
    m_lookedForEditReprojectionSettingsFunction = false;
    m_saveDefaultRefReprojectionSettingsFunc    = nullptr;
    m_editRefReprojectionSettingsFunc           = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
virtual GeoReferenceStatus  CanAttachGeographically
(
bool&           canProject,
bool&           canTransform,
DgnModelRefP    refModelRef,
double         *maximumError
) override
    {
    // We have selected a reference and we need to know whether it can be attached geographically or not.
    if (NULL != maximumError)
        *maximumError = 0.0;

    canProject      = false;
    canTransform    = false;
    // if not configured for GeoCoordinate referencing, don't do it.
    if (ConfigurationManager::IsVariableDefined (L"MS_GEOCOORDINATE_NO_REFERENCE"))
        return GEOREF_STATUS_ReprojectDisabled;

    // should always have a modelRef, but test anyway.
    DgnModelRefP    parentModelRef;
    if ( (NULL == refModelRef) || (NULL == (parentModelRef = refModelRef->GetParentModelRefP())) )
        return GEOREF_STATUS_BadModelRef;

    DgnGCSP     parentGCS;
    DgnGCSP     referenceGCS;

    // if we've just filled a rootModelRef cache, see if it has a coordinate system element.
    if (NULL == (parentGCS = DgnGCS::FromModel (parentModelRef, true)))
        return GEOREF_STATUS_MasterHasNoGCS;

    // if we've just filled a reference modelRef cache, see if both it and the master file have a coordinate system.
    if (NULL == (referenceGCS = DgnGCS::FromModel (refModelRef, true)))
        return GEOREF_STATUS_AttachmentHasNoGCS;

    canProject = true;

    // to do a transform, both unit base values have to be the same. Otherwise, except at the equator, the X and Y scaling would have to be nonuniform
    // and we don't want to introduce that possibility.
    UnitInfo    parentUnits;
    UnitInfo    refUnits;
    dgnModel_getStorageUnit (parentModelRef->GetDgnModelP(), &parentUnits);
    dgnModel_getStorageUnit (refModelRef->GetDgnModelP(), &refUnits);
    if (parentUnits.flags.base != refUnits.flags.base)
        return GEOREF_STATUS_Success;

    canTransform = true;

    if (NULL != maximumError)
        {
        // if we get here, we've got geo coordinate systems for both the parent and reference, so we know we can attach
        //  the reference geographically. However, we need to figure out whether we will get a reasonable approximation by
        //  calculating a scale/rotation linear transform, or whether a full geographic transformation of each graphic
        //  element is necessary. We don't actually make that decision here, we simply calculate the maximum error and
        //  return that to the caller, who allows the user to decide.
        ReferenceParameterCalculator calculator (refModelRef->GetDgnModelP(), referenceGCS, parentModelRef, parentGCS, false, NULL);
        *maximumError = calculator.GetMaximumError();
        }

    return GEOREF_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  CalculateAttachParameters
(
DgnModelRefP    refModelRef,
bool            setMasterZ
) override
    {
    DgnAttachmentP   refP;
    if (NULL == (refP = refModelRef->AsDgnAttachmentP ()))
        return GEOREF_STATUS_BadModelRef;

    DgnGCSP         targetGCS;
    DgnModelRefP    targetModelRef;
    DgnGCS::GetReprojectionTarget (targetModelRef, targetGCS, refModelRef, NULL);

    // should always have a targetModelRef and targetGCS, but test anyway.
    if ( (NULL == targetModelRef) || (NULL == targetGCS) )
        return GEOREF_STATUS_MasterHasNoGCS;

    // if we've just filled a reference modelRef cache, see if both it and the master file have a coordinate system.
    DgnGCSP     referenceGCS;
    if (NULL == (referenceGCS = DgnGCS::FromModel (refModelRef, true)))
        return GEOREF_STATUS_AttachmentHasNoGCS;

    ReferenceParameterCalculator calculator (refModelRef->GetDgnModelP(), referenceGCS, targetModelRef, targetGCS, false, NULL);
    if (!calculator.m_haveSolution)
        return GEOREF_STATUS_NoLinearTransform;

    calculator.SetReferenceParameters (refP, setMasterZ);
    return GEOREF_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP GetReferenceGCSName (WStringR gcsName, DgnModelRefP refModelRef) override
    {
    gcsName.clear();
    DgnGCSP     refGCS;
    if (NULL == (refGCS = DgnGCS::FromModel (refModelRef, true)))
        return gcsName.c_str();

    return refGCS->GetDisplayName (gcsName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP GetGCSName (WStringR gcsName, DgnGCSP sourceGCS) override
    {
    gcsName.clear();

    if (sourceGCS !=NULL && sourceGCS->IsValid())
        return sourceGCS->GetDisplayName (gcsName);

    return gcsName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP GetDescription (WStringR gcsDescription, DgnGCSP sourceGCS) override
    {
    gcsDescription.clear();
    if (sourceGCS != NULL && sourceGCS->IsValid())
        gcsDescription.assign (sourceGCS->GetDescription ());

    return gcsDescription.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  ReprojectUorPoints
(
DgnModelRefP    refModelRef,
DgnModelRefP    target,
DPoint3dP       outUors,
DPoint3dCP      inUors,
int             numPoints
) override
    {
    if ( (NULL == outUors) || (NULL == inUors) || (numPoints <= 0) )
        return GEOREF_STATUS_BadArg;

    DgnGCSP     refGCS;
    if (NULL == (refGCS = DgnGCS::FromModel (refModelRef, true)))
        return GEOREF_STATUS_AttachmentHasNoGCS;

    DgnGCSP     targetGCS;
    if (NULL == (targetGCS = DgnGCS::FromModel (target, true)))
        return GEOREF_STATUS_MasterHasNoGCS;

    return (GeoReferenceStatus) refGCS->ReprojectUors (outUors, NULL, NULL, inUors, numPoints, *targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  ReprojectUorPoints
(
DgnGCSP         sourceGCS,
DgnGCSP         targetGCS,
DPoint3dP       outUors,
DPoint3dCP      inUors,
int             numPoints
) override
    {
    if ( (NULL == outUors) || (NULL == inUors) || (numPoints <= 0) )
        return GEOREF_STATUS_BadArg;

    if ( (NULL == sourceGCS) || (NULL == targetGCS) )
        return GEOREF_STATUS_BadArg;

    return (GeoReferenceStatus) sourceGCS->ReprojectUors (outUors, NULL, NULL, inUors, numPoints, *targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  CalculateReferenceParameters
(
DPoint3dP       masterOrigin,
DPoint3dP       referenceOrigin,
double*         trueScale,
RotMatrixP      rotMatrix,
double*         storedScale,
DgnModelRefP    refModelRef
) override
    {
    // should always have a modelRef, but test anyway.
    DgnGCSP         targetGCS;
    DgnModelRefP    targetModelRef;
    DgnGCS::GetReprojectionTarget (targetModelRef, targetGCS, refModelRef, NULL);

    // should always have a targetModelRef and targetGCS, but test anyway.
    if ( (NULL == targetModelRef) || (NULL == targetGCS) )
        return GEOREF_STATUS_MasterHasNoGCS;

    DgnGCSP     referenceGCS;
    if (NULL == (referenceGCS = DgnGCS::FromModel (refModelRef, true)))
        return GEOREF_STATUS_AttachmentHasNoGCS;

    ReferenceParameterCalculator calculator (refModelRef->GetDgnModelP(), referenceGCS, targetModelRef, targetGCS, false, NULL);

    if (NULL != masterOrigin)
        *masterOrigin       = calculator.m_masterOrigin;
    if (NULL != referenceOrigin)
        *referenceOrigin    = calculator.m_refOrigin;
    if (NULL != trueScale)
        *trueScale          = calculator.m_refUnitScale * calculator.m_coordSysScale;
    if (NULL != rotMatrix)
        *rotMatrix          = calculator.m_rotMatrix;
    if (NULL != storedScale)
        *storedScale        = calculator.m_coordSysScale;

    return GEOREF_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        WillReprojectToParent (DgnModelRefP refModelRef) override
    {
    // if not a reference, don't transform.
    DgnAttachmentP      refP;
    if (NULL == (refP = refModelRef->AsDgnAttachmentP ()))
        return false;

    RefAttachMethod     attachMethod = refP->GetAttachMethod();

    // if the attachment method isn't appropriate for transform, don't do it,
    if ( (ATTACHMETHOD_Unknown != attachMethod) && (ATTACHMETHOD_GeographicProjected != attachMethod) )
        return false;

    // need a GCS for the reference.
    if (NULL == DgnGCS::FromModel (refModelRef, true))
        return false;

    // need a GCS for the parent.
    DgnModelRefP parentModelRef = refModelRef->GetParentModelRefP();
    if ( (NULL == parentModelRef) || (NULL == DgnGCS::FromModel (parentModelRef, true)) )
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        HasGCS (DgnModelRefP modelRef) override
    {
    return (NULL != DgnGCS::FromModel (modelRef, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCSP     CreateGCSFromModel (DgnModelRefP modelRef) override
    {
    DgnGCSP gcs = DgnGCS::FromModel (modelRef, true);

    // increment reference count.
    if (NULL != gcs)
        gcs->AddRef();

    return gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCSP     CreateGCSFromElement (ElementHandleCR eh) override
    {
    DgnGCSP  gcs = DgnGCS::FromGeoCoordType66 (&eh.GetElementCP()->applicationElm, eh.GetDgnModelP());
    
    if (NULL != gcs)
        gcs->AddRef();

    return gcs;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCSP  CreateGCSFromKeyName (WCharCP keyName, DgnModelRefP modelRef) override
    {
    DgnGCSPtr  gcsPtr = DgnGCS::CreateGCS (keyName, modelRef);
    DgnGCSP    gcs = gcsPtr.get();

    //The caller must call IGeoCoordinateServices::FreeGCS on the returned DgnGCSP.
    if (NULL != gcs)
        gcs->AddRef();
    
    return gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool GetGCSElementHandle (ElementHandleR eh, DgnGCSP sourceGCS, DgnModelRefP modelRef) override
    {
    MSElement El;
    DgnModelP dgnCache (modelRef ? modelRef->GetDgnModelP () : NULL);
    assert(dgnCache);
    if (SUCCESS == sourceGCS->CreateGeoCoordType66(reinterpret_cast<ApplicationElm*>(&El),dgnCache,true))
        {
        eh = ElementHandle(&El,modelRef);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        FreeGCS (DgnGCSP *gcs) override
    {
    if ( (NULL == gcs) || (NULL == *gcs) )
        return;

    (*gcs)->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        GetReprojectionTarget (DgnGCSP &targetGCS, DgnModelRefP &targetModelRef, DgnModelRefP refModelRef) override
    {
    if (NULL == refModelRef)
        return false;

    DgnGCS::GetReprojectionTarget (targetModelRef, targetGCS, refModelRef, NULL);

    if (NULL != targetGCS)
        targetGCS->AddRef();

    return (NULL != targetModelRef) && (NULL != targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        RequiresReprojection (DgnGCSP sourceGCS, DgnGCSP targetGCS)
    {
    if ( (NULL == sourceGCS) || (NULL == targetGCS) )
        return false;

    if (!sourceGCS->IsValid() || !targetGCS->IsValid())
        return false;

    if (!sourceGCS->UnitsIdentical (*targetGCS))
        return true;

    return !sourceGCS->IsEquivalent (*targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  EditReprojectionSettings
(
DgnModelRefListP    modelRefList
) override
    {
    if ((NULL == modelRefList) || (0 == modelRefList->size ()))
        return GEOREF_STATUS_BadArg;

    FindEditReprojectionSettingsFunction();
    if (nullptr == m_editRefReprojectionSettingsFunc)
        return  GEOREF_STATUS_CantFindFunction;

    return (GeoReferenceStatus)(*m_editRefReprojectionSettingsFunc) (modelRefList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HMODULE FindManagedDll ()
    {
    BeFileName  dllFileName;
    if (SUCCESS != Bentley::BeGetModuleFileName (dllFileName, (void*)&dummy))
        return nullptr;

    WString device;
    WString dir;
    dllFileName.ParseName (&device, &dir, NULL, NULL);

    BeFileName  managedDllName;
    managedDllName.BuildName (device.c_str(), dir.c_str(), L"Bentley.DgnGeoCoord2", L".dll");

    // try to load the library
    return LoadLibraryW ( managedDllName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    FindEditReprojectionSettingsFunction ()
    {
    // try once to find the dll and function that edits the Reprojection Settings for a reference.
    // The function is in the Bentley.DgnGeoCoord2 mixed DLL. Since that dll depends on this dll, we can't depend on it, so we have to look it up at runtime.
    if (m_lookedForEditReprojectionSettingsFunction)
        return;
    m_lookedForEditReprojectionSettingsFunction  = true;

    HMODULE hLib;
    if (nullptr == (hLib = FindManagedDll()))
        return;

    // Get the function pointer
    m_editRefReprojectionSettingsFunc = (EditRefReprojectionSettingsFunc) GetProcAddress ( hLib, "dgnGeoCoord_editRefReprojectionSettings");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  SaveDefaultReprojectionSettings
(
DgnModelRefP        modelRef
) override
    {
    if (NULL == modelRef || ! modelRef->IsDgnAttachment())
        return  GEOREF_STATUS_BadArg;

    FindSaveDefaultSettingsFunction();
    if (nullptr == m_saveDefaultRefReprojectionSettingsFunc)
        return  GEOREF_STATUS_CantFindFunction;

    return (GeoReferenceStatus) (*m_saveDefaultRefReprojectionSettingsFunc) (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    FindSaveDefaultSettingsFunction ()
    {
    // try once to find the dll and function that saves default Reprojection Settings to a reference.
    // The function is in the Bentley.DgnGeoCoord2 mixed DLL. Since that dll depends on this dll, we can't depend on it, so we have to look it up at runtime.
    if (m_lookedForSaveDefaultSettingsFunction)
        return;
    m_lookedForSaveDefaultSettingsFunction = true;

    HMODULE hLib;
    if (nullptr == (hLib = FindManagedDll()))
        return;

    // Get the function pointer
    m_saveDefaultRefReprojectionSettingsFunc = (SaveDefaultRefReprojectionSettingsFunc) GetProcAddress ( hLib, "dgnGeoCoord_saveDefaultRefReprojectionSettings");
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinationState    GetGCState
(
int     attachMethod
)
    {
    if (ATTACHMETHOD_GeographicProjected == attachMethod)
        return Reprojected;
    if (ATTACHMETHOD_GeographicTransformed == attachMethod)
        return AECTransform;
    return NoGeoCoordination;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        SendBeforeGeoCoordinationChanged
(
DgnModelRefP        modelRef,
int                 oldAttachMethod,
int                 newAttachMethod
) override
    {
    DgnGCS::SendBeforeGeoCoordinationChanged (modelRef, GetGCState (oldAttachMethod), GetGCState (newAttachMethod));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        SendAfterGeoCoordinationChanged
(
DgnModelRefP        modelRef,
int                 oldAttachMethod,
int                 newAttachMethod
) override
    {
    DgnGCS::SendAfterGeoCoordinationChanged (modelRef, GetGCState (oldAttachMethod), GetGCState (newAttachMethod));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        ModelUnitsChanged (DgnModelP model) override
    {
    return DgnGCS::UpdateUnitInfo (model);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        GetUnitDefinition (DgnGCSP sourceGCS, UnitDefinitionR unitDef, DgnPlatform::StandardUnit&  standardUnitNumber) override
    {
    return (SUCCESS == sourceGCS->GetUnitDefinition(unitDef,standardUnitNumber));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        SetMstnGeoEventHandler  (IGeoCoordinateEventHandler* eventHandler) override
    {
    DgnGCS::SetEventHandler (eventHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        RemoveMstnGeoEventHandler (IGeoCoordinateEventHandler* eventHandler) override
    {
    DgnGCS::RemoveEventHandler (eventHandler);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateServicesP   DgnGeoCoordinationAdmin::_GetServices () const
    {
    if (NULL == m_gcrp)
        {
        CompleteInitialization();
        // if we don't have a data directory, can't provide services.
        if (!m_dataDirectory.empty())
            m_gcrp = new Bentley::GeoCoordinates::GeoCoordinateServices();
        }

    return m_gcrp;
    }

}   // Ends GeoCoordinates namespace

END_BENTLEY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            dgnGeoCoord_canSubstituteLinearTransformForReprojection
(
DgnAttachmentP  refP,
DgnModelP       refCache,
DgnGCSP         referenceGCS,
DgnModelRefP    parentModelRef,
DgnGCSP         parentGCS
)
    {
    if ( (NULL == refP) || (NULL == refCache) )
        {
        assert (false);
        return false;
        }

    Bentley::GeoCoordinates::ReferenceParameterCalculator calculator (refCache, referenceGCS, parentModelRef, parentGCS, true, &refP->GetStoredMasterOrigin());
    if (calculator.m_haveSolution)
        {
        bool    setZ = (0 == refP->GetFBOptsR ().userSpecifiedZForGeoReprojected);
        calculator.SetReferenceParameters (refP, setZ);

        // record that we have used a calculated transform.
        refP->SetCalculatedTransformUsed (true);
        }

    return calculator.m_haveSolution;
    }

