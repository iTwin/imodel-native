/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/AuxCoordSysProcessor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <Geom\msgeomstructs.hpp>

#include    <DgnGeoCoord\DgnGeoCoord.h>
#include    <DgnPlatform\Tools\stringop.h>
#include    <DgnPlatform\DgnCore\DgnCoreAPI.h>

USING_NAMESPACE_BENTLEY_DGN

#define GEOCOORD_ACS_EXTENDERID     0x04151956
#define GEOCOORD_MGRS_EXTENDERID    0x03271957

BEGIN_UNNAMED_NAMESPACE

enum    ReturnCodes
    {
    MDLERR_WRITEINHIBIT                 = (-110),
    MDLERR_BADFORMAT                    = (-195),
    };

/*=================================================================================**//**
* This class implements the IAuxCoordSysP interface.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct GeoAuxCoordSys : IAuxCoordSys
{
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

DgnGCSPtr   m_dgnGCSPtr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
GeoAuxCoordSys
(
DgnGCSP     dgnGCS
)
    {
    m_dgnGCSPtr = dgnGCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
~GeoAuxCoordSys()
    {
    // smart pointer automatically destructed, don't need anything here yet.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _CompleteSetupFromViewInfo (ViewInfoCP info)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysPtr _Clone () const override
    {
    return new GeoAuxCoordSys (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            _Equals (IAuxCoordSysCP other) const override
    {
    if (NULL == other)
        return false;
    if (this == other)
        return true;
    GeoAuxCoordSys const*  otherGACS;
    if (NULL == (otherGACS = dynamic_cast <GeoAuxCoordSys const *> (other)))
        return false;
    return m_dgnGCSPtr.get() == otherGACS->m_dgnGCSPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetName () const override
    {
    WString displayName;
    m_dgnGCSPtr->GetDisplayName (displayName);
    return displayName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetDescription () const override
    {
    return WString (m_dgnGCSPtr->GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSType         _GetType () const override
    {
    return ACS_TYPE_Extended;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetTypeName () const override
    {
    WChar buffer[1024];
    return WString (BaseGeoCoordResource::GetLocalizedStringW (buffer, BentleyApi::GeoCoordinates::DGNGEOCOORD_Msg_GeoCoordACSType, _countof (buffer)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double          _GetScale () const override
    {
    return 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint3dR       _GetOrigin (DPoint3dR pOrigin) const override
    {
    pOrigin.init (0.0, 0.0, 0.0);
    return pOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR      _GetRotation (RotMatrixR pRot) const override
    {
    pRot.initIdentity();
    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR      _GetRotation (RotMatrixR pRot, DPoint3dR pPosition) const override
    {
    pRot.initIdentity();
    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ElementId       _GetElementId () const override
    {
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            _GetIsReadOnly () const override
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSFlags        _GetFlags () const override
    {
    return ACS_FLAG_Default;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP       _GetAxisLabel
(
uint32_t    axis,
WCharP    axisLabel,
uint32_t    length
) const override
    {
    int     quadrant = m_dgnGCSPtr->GetQuadrant();

    axisLabel[1] = 0;
    switch (axis)
        {
        case 0:
            if ( (quadrant == 2) || (quadrant == 3) )
                axisLabel[0] = 'W';
            else
                axisLabel[0] = 'E';
            break;
        case 1:
            if ( (quadrant == 3) || (quadrant == 4) )
                axisLabel[0] = 'S';
            else
                axisLabel[0] = 'N';
            break;
        case 2:
            axisLabel[0] = 'H';
            break;
        default:
            axisLabel[0] = 0;
            break;
        }
    return axisLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ParseAngle (double& angle, const char* string)
    {
#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
    // here's what it was:
    mdlString_toAngle (&angle, string);
#else
    angle = 0;
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ParseDistance (double& distance, const char* string, DgnModelRefP modelRef)
    {
#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
    // here's what it was:
    mdlString_toUors2 (&distance, string, modelRef, true);
#else
    distance = 0;
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Copied here until the rest of this can be updated.
*
*     returns pointer to character just after  
*     the first occurrance of specified char.  
*     Zeros character found character.         
*     Returns pointer to NULL if char not found
+---------------+---------------+---------------+---------------+---------------+------*/
static char  *strnxtchr (char *str, int chr)  // NEEDSWORK_UNICODE
    {
    char    *p;

    if ((p = strchr (str, chr)) == NULL)
        return (str + strlen(str));

    *p = 0;
    return (p+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _PointFromString (DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelRefP modelRef) override
    {
    // from inPoint, get Latitude/Longitude.
    WChar     errBuffer[1024];
    DgnGCSP     dgnGCS;

    bool relativeFirstAngle     = relative;
    bool relativeSecondAngle    = relative;
    bool relativeElevation      = relative;

    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_NoGeoCoordinateSystem, _countof(errBuffer)));
        return ERROR;
        }

    // make a local copy of the input string that we can modify.
    char        string[1024];
    WString(inString).ConvertToLocaleChars (string, _countof(string));

    char*       secondAngleStringP;
    char*       elevationStringP;
    char*       firstAngleStringP;

    firstAngleStringP   = string;
    secondAngleStringP  = strnxtchr (string, ','); // NEEDSWORK_UNICODE - should use BeStringUtilites::Split or the new parsers in DgnPlatform
    elevationStringP    = strnxtchr (secondAngleStringP, ',');

    if (!relative && ( (0 == string[0]) || (NULL == secondAngleStringP) || (0 == *secondAngleStringP) ) )
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_PointFromStringRequiresBoth, _countof(errBuffer)));
        return MDLERR_BADFORMAT;
        }

    double  firstAngle = 0.0;
    if ('#' == *firstAngleStringP)
        {
        relativeFirstAngle = true;
        firstAngleStringP++;
        }
    if (SUCCESS != ParseAngle (firstAngle, firstAngleStringP))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_UnparseableInputAngle, _countof(errBuffer)));
        errorMsg.AppendA (firstAngleStringP);
        return MDLERR_BADFORMAT;
        }
    double  secondAngle = 0.0;
    if ('#' == *secondAngleStringP)
        {
        relativeSecondAngle = true;
        secondAngleStringP++;
        }
    if (SUCCESS != ParseAngle (secondAngle, secondAngleStringP))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_UnparseableInputAngle, _countof(errBuffer)));
        errorMsg.AppendA (secondAngleStringP);
        return MDLERR_BADFORMAT;
        }
    double  elevation = 0.0;
    if ('#' == *elevationStringP)
        {
        relativeElevation = true;
        elevationStringP++;
        }
    if (SUCCESS != ParseDistance (elevation, elevationStringP, modelRef))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_UnparseableInputElevation, _countof(errBuffer)));
        errorMsg.AppendA (elevationStringP);
        return MDLERR_BADFORMAT;
        }

    StatusInt   status;
    if ( (!relativeFirstAngle && !relativeSecondAngle && !relativeElevation) || (NULL == lastPoint))
        {
        GeoPoint latLong;
        // here is where we could change from longitude/latitude to latitude/longitude.
        latLong.Init (firstAngle, secondAngle, 0.0);
        if (SUCCESS != (status = dgnGCS->UorsFromLatLong (outPoint, latLong)))
            {
            // fill in error message.
            return status;
            }

        outPoint.z = elevation;
        }
    else
        {
        // get the Latitude/Longitude of the lastPoint
        GeoPoint    originLatLong;
        if (SUCCESS != (status = dgnGCS->LatLongFromUors (originLatLong, *lastPoint)))
            {
            // fill in error message.
            return status;
            }
        // add the relative angle.
        if (relativeFirstAngle)
            originLatLong.longitude += firstAngle;
        else
            originLatLong.longitude = firstAngle;

        if (relativeSecondAngle)
            originLatLong.latitude += secondAngle;
        else
            originLatLong.latitude = secondAngle;


        if (SUCCESS != (status = dgnGCS->UorsFromLatLong (outPoint, originLatLong)))
            {
            // fill in error message.
            return status;
            }

        if (relativeElevation)
            outPoint.z = lastPoint->z + elevation;
        else
            outPoint.z = elevation;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            FormatAngle (WCharP angleString, double value, AngleMode angleFormat, AnglePrecision anglePrecision)
    {
#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
    mdlString_wideFromAngle (angleString, value, ANGLE_FORMAT_Active, false, ANGLE_PRECISION_Active, false, true, true);
#else
    *angleString = 0;
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            FormatDistance (WCharP elevationString, double value, DgnModelRefP modelRef, DgnUnitFormat linearFormat, int linearPrecision, bool fractions)
    {
#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
    mdlString_fromUors2Ext (elevationString, value, modelRef, false, true);
#else
    *elevationString = 0;
#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _StringFromPoint (WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelRefP modelRef,
                                        DgnUnitFormat linearFormat, int linearPrecision, bool fractions, bool scientific, AngleMode angleFormat, AnglePrecision anglePrecision) override
    {
    // format input into Latitude/Longitude/Elevation
    WChar     errBuffer[1024];
    DgnGCSP     dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_NoGeoCoordinateSystem, _countof(errBuffer)));
        return ERROR;
        }
    StatusInt   status;
    GeoPoint    latLong;
    if (SUCCESS != (status = dgnGCS->LatLongFromUors (latLong, inPoint)))
        {
        // fill in error message
        BaseGCS::GetErrorMessage (errorMsg, status);
        return status;
        }
    if (delta && (NULL != deltaOrigin))
        {
        GeoPoint  originLatLong;
        if (SUCCESS != (status = dgnGCS->LatLongFromUors (originLatLong, *deltaOrigin)))
            {
            BaseGCS::GetErrorMessage (errorMsg, status);
            return status;
            }
        latLong.longitude -= originLatLong.longitude;
        latLong.latitude  -= originLatLong.latitude;
        latLong.elevation -= originLatLong.elevation;
        }

    // format the lat/long
    WChar     angleString[1024];
    FormatAngle (angleString, latLong.longitude, angleFormat, anglePrecision);
    outString.assign (angleString);
    outString.append (L", ");
    FormatAngle (angleString, latLong.latitude, angleFormat, anglePrecision);
    outString.append (angleString);
    if (modelRef->Is3d())
        {
        WChar    elevationString[512];
        outString.append (L", ");
        FormatDistance (elevationString, inPoint.z - ( (delta && (NULL != deltaOrigin)) ? deltaOrigin->z : 0.0), modelRef, linearFormat, linearPrecision, fractions);
        outString.append (elevationString);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _DrawGrid (ViewportP vP) const override {}
virtual void            _PointToGrid (ViewportP vP, DPoint3dR point) const override {}

virtual uint32_t        _GetExtenderId() const override {return GEOCOORD_ACS_EXTENDERID;}
virtual uint32_t        _GetSerializedSize () const override {return 0;};
virtual StatusInt       _Serialize (void *buffer, uint32_t maxSize) const override {return SUCCESS;}

virtual StatusInt       _SetName (WCharCP name) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetDescription (WCharCP descr) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetType (ACSType type) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetScale (double scale) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetOrigin (DPoint3dCR pOrigin) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetRotation (RotMatrixCR pRot) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetFlags (ACSFlags flags) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetElementId (ElementId elementId, DgnModelRefP modelRef) override {return MDLERR_WRITEINHIBIT;}

virtual StatusInt       _SaveToFile (DgnModelRefP modelRef, ACSSaveOptions option) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _DeleteFromFile (DgnModelRefP modelRef) override {return MDLERR_WRITEINHIBIT;}

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
GeoAuxCoordSys
(
GeoAuxCoordSys const * source
)
    {
    m_dgnGCSPtr = source->m_dgnGCSPtr.get();
    }

};

/*=================================================================================**//**
* This is the class that implements the Geocoordinate User Input/Output interface.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
class   GeoCoordinateAuxSystemExtender : public Dgn::IAuxCoordSystemExtender
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
public:
virtual bool            _TraverseExtendedACS
(
IACSTraversalHandler&   traverser,
DgnModelRefP            modelRef
) override
    {
    // If there's a Geo Coordinate system, create an IAuxCoordSysP for it and return it.
    if (ConfigurationManager::IsVariableDefined (L"MS_GEOCOORDINATE_NO_USERIO"))
        return false;

    // should always have a modelRef, but test anyway.
    if (NULL == modelRef)
        return false;

    // if we don't have a GCS, can't do anything.
    DgnGCSP     dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        return false;

    // construct a GeoAuxCoordSys
    GeoAuxCoordSys *gcsAuxSysP = new GeoAuxCoordSys (dgnGCS);
    IAuxCoordSysPtr acsPtr (gcsAuxSysP);

    return (traverser._HandleACSTraversal (acsPtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual uint32_t        _GetExtenderId() const override {return GEOCOORD_ACS_EXTENDERID;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysP   _Deserialize (void *persistentData, uint32_t dataSize, DgnModelRefP modelRef) override
    {
    // if we don't have a GCS, can't do anything.
    DgnGCSP     dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        return NULL;

    return new GeoAuxCoordSys (dgnGCS);
    }

};

/*=================================================================================**//**
* This class implements the IAuxCoordSysP interface.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct MilitaryGridAuxCoordSys : IAuxCoordSys
{
private:

DgnGCSPtr                   m_dgnGCSPtr;
MilitaryGridConverterPtr    m_mgrsConverterPtr;
bool                        m_useBessel;
bool                        m_reportInWGS84Datum;
bool                        m_inUSA;        // used just to get the name right.

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
MilitaryGridAuxCoordSys (DgnGCSP dgnGCS, MilitaryGridConverterP mgrsConverter, bool useBessel, bool reportInWGS84Datum, bool inUSA)
    {
    m_dgnGCSPtr             = dgnGCS;
    m_mgrsConverterPtr      = mgrsConverter;
    m_useBessel             = useBessel;
    m_reportInWGS84Datum    = reportInWGS84Datum;
    m_inUSA                 = inUSA;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
MilitaryGridAuxCoordSys (MilitaryGridAuxCoordSys const *source)
    {
    m_dgnGCSPtr            = source->m_dgnGCSPtr.get();
    m_mgrsConverterPtr      = source->m_mgrsConverterPtr.get();
    m_useBessel             = source->m_useBessel;
    m_reportInWGS84Datum    = source->m_reportInWGS84Datum;
    m_inUSA                 = source->m_inUSA;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static MilitaryGridAuxCoordSys*  Create (DgnGCSP dgnGCS, bool useBessel, bool reportInWGS84Datum, bool inUSA)
    {
    MilitaryGridConverterPtr      mgrsConverter;
    if (NULL == dgnGCS)
        return NULL;

    mgrsConverter = MilitaryGridConverter::CreateConverter (*dgnGCS, useBessel, reportInWGS84Datum);
    if (mgrsConverter.IsNull())
        return NULL;

    return new MilitaryGridAuxCoordSys (dgnGCS, mgrsConverter.get(), useBessel, reportInWGS84Datum, inUSA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
~MilitaryGridAuxCoordSys ()
    {
    // smart pointers automatically destructed, don't need anything for that.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _CompleteSetupFromViewInfo (ViewInfoCP info)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysPtr _Clone () const override 
    {
    return new MilitaryGridAuxCoordSys (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            _Equals (IAuxCoordSysCP other) const override 
    {
    if (NULL == other)
        return false;
    if (this == other)
        return true;
    MilitaryGridAuxCoordSys const*  otherGACS;
    if (NULL == (otherGACS = dynamic_cast <MilitaryGridAuxCoordSys const *> (other)))
        return false;

    if (m_dgnGCSPtr.get() != otherGACS->m_dgnGCSPtr.get())
        return false;

    // NEEDSWORK
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetName () const override 
    {
    WChar             buffer[1024];
    DgnGeoCoordStrings  nameKey;

    if (m_useBessel)
        nameKey = DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesName;
    else if (!m_reportInWGS84Datum)
        nameKey = DGNGEOCOORD_Msg_MiltaryGridCoordinatesName;
    else if (!m_inUSA)
        nameKey = DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Name;
    else
        nameKey = DGNGEOCOORD_Msg_USNationalGridName;

    return WString (BaseGeoCoordResource::GetLocalizedStringW (buffer, nameKey, _countof (buffer)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetDescription () const override
    {
    WChar             buffer[1024];
    DgnGeoCoordStrings  descrKey;

    if (m_useBessel)
        descrKey = DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesDescription;
    else if (!m_reportInWGS84Datum)
        descrKey = DGNGEOCOORD_Msg_MiltaryGridCoordinatesDescription;
    else if (!m_inUSA)
        descrKey = DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Description;
    else
        descrKey = DGNGEOCOORD_Msg_USNationalGridDescription;

    return WString (BaseGeoCoordResource::GetLocalizedStringW (buffer, descrKey, _countof (buffer)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSType         _GetType () const override
    {
    return ACS_TYPE_Extended;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         _GetTypeName () const override
    {
    WChar buffer[1024];
    return WString (BaseGeoCoordResource::GetLocalizedStringW (buffer, DGNGEOCOORD_Msg_MilitaryGridACSType, _countof (buffer)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double          _GetScale () const override
    {
    return 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint3dR       _GetOrigin (DPoint3dR pOrigin) const override
    {
    pOrigin.init (0.0, 0.0, 0.0);
    return pOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR      _GetRotation (RotMatrixR pRot) const override
    {
    pRot.initIdentity();
    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR      _GetRotation (RotMatrixR pRot, DPoint3dR pPosition) const override
    {
    pRot.initIdentity();
    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ElementId       _GetElementId () const override
    {
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            _GetIsReadOnly () const override
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSFlags        _GetFlags () const override
    {
    return ACS_FLAG_Default;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP       _GetAxisLabel
(
uint32_t    axis,
WCharP    axisLabel,
uint32_t    length
) const override
    {
    int     quadrant = m_dgnGCSPtr->GetQuadrant();

    axisLabel[1] = 0;
    switch (axis)
        {
        case 0:
            if ( (quadrant == 2) || (quadrant == 3) )
                axisLabel[0] = 'W';
            else
                axisLabel[0] = 'E';
            break;
        case 1:
            if ( (quadrant == 3) || (quadrant == 4) )
                axisLabel[0] = 'S';
            else
                axisLabel[0] = 'N';
            break;
        case 2:
            axisLabel[0] = 'H';
            break;
        default:
            axisLabel[0] = 0;
            break;
        }
    return axisLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _PointFromString (DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelRefP modelRef) override
    {
    // The input string should be in military grid, which is UTM zone (1 or 2chars); the 8degree latitude zone (1 char, C-X); 100km zone identifier (2chars), easting (up to 5 chars) and northing (up to 5 chars), e.g. 14UNK21345413

    // can't be relative
    WChar     errBuffer[1024];
    if (relative)
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_MilitaryGridNotRelative, _countof (errBuffer)));
        return ERROR;
        }

    DgnGCSP      dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_NoGeoCoordinateSystem, _countof(errBuffer)));
        return ERROR;
        }

    // make sure we have upper case.
    WString localString (inString);
    localString.ToUpper();

    GeoPoint2d    latLong2d;
    if (SUCCESS != m_mgrsConverterPtr->LatLongFromMilitaryGrid (latLong2d, localString.c_str()))
        {
        // fill in error message.
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_CantConvertFromMilitaryGrid, _countof (errBuffer)));
        return ERROR;
        }

    // that lat long is in the datum of dgnGCS.
    StatusInt   status;
    GeoPoint    latLong;
    // here is where we could change from longitude/latitude to latitude/longitude.
    latLong.Init (latLong2d.longitude, latLong2d.latitude, 0.0);
    if (SUCCESS != (status = dgnGCS->UorsFromLatLong (outPoint, latLong)))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _StringFromPoint (WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelRefP modelRef, 
                                            DgnUnitFormat linearFormat, int linearPrecision, bool fractions, bool scientific, AngleMode angleFormat, AnglePrecision anglePrecision) override
    {
    // can't be relative
    WChar     errBuffer[1024];
    if (delta)
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_MilitaryGridNotDelta, _countof (errBuffer)));
        return ERROR;
        }

    // format input UORs into Military Grid.
    DgnGCSP     dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_NoGeoCoordinateSystem, _countof(errBuffer)));
        return ERROR;
        }
    StatusInt   status;
    GeoPoint    latLong;
    if (SUCCESS != (status = dgnGCS->LatLongFromUors (latLong, inPoint)))
        {
        // fill in error message
        BaseGCS::GetErrorMessage (errorMsg, status);
        return status;
        }

    // we derive the precision from the angle format and angle readout precision.
    // 5 decimal places is to the nearest meter, which is the most precise it goes.
    // Each degree on the earth is about 6,378,137 * 2 * PI / 360 = 111318 meters.
    // Therefore, each minute is about 111318/60 = 1855 meters
    // Therefore, each second is about 1855/60 = 30.9 meters.
    double      meters = 1000;
    double      desiredMeterPrecision = meters;
    for (int iPower=0; iPower < (int) anglePrecision; iPower++)
        desiredMeterPrecision /= 10.0;

    int mgPrecision;
    if (desiredMeterPrecision < 5.0)
        mgPrecision = 5;
    else if (desiredMeterPrecision < 50.0)
        mgPrecision = 4;
    else if (desiredMeterPrecision < 500.0)
        mgPrecision = 3;
    else if (desiredMeterPrecision < 5000.0)
        mgPrecision = 2;
    else if (desiredMeterPrecision < 50000.0)
        mgPrecision = 1;
    else
        mgPrecision = 0;

    // format the lat/long as military grid.
    GeoPoint2d  latLong2d;
    latLong2d.Init (latLong.longitude, latLong.latitude);
    WString     mgString;
    if (SUCCESS != m_mgrsConverterPtr->MilitaryGridFromLatLong (mgString, latLong2d, mgPrecision))
        {
        errorMsg.assign (BaseGeoCoordResource::GetLocalizedStringW (errBuffer, DGNGEOCOORD_Msg_CantConvertToMilitaryGrid, _countof (errBuffer)));
        return ERROR;
        }
    // the string as it comes back, it is all crushed together. We change it to the more readable format of something like 18T VK 4230.
    WChar localString[512];
    wcscpy (localString, mgString.data());

    WChar formattedString[512];

    // find grid zone, transfer it, and add a space.
    int outChar = 0;
    int inChar  = 0;
    for (inChar=0; inChar < 3; inChar++, outChar++)
        {
        formattedString[outChar] = localString[inChar];
        if (!isdigit (localString[inChar]))
            break;
        }
    // insert space.
    formattedString[++outChar] = L' ';

    // transfer the two-letter squareID:
    formattedString[++outChar] = localString[++inChar];
    formattedString[++outChar] = localString[++inChar];

    // insert space.
    formattedString[++outChar] = L' ';

    // transfer half of the remainder, then put a space.
    size_t remainingLen = wcslen (&localString[inChar+1]);
    assert (0 == (remainingLen % 2));
    for (size_t iChar = 0; iChar < remainingLen/2; iChar++)
        formattedString[++outChar] = localString[++inChar];

    // insert space.
    formattedString[++outChar] = L' ';

    for (size_t iChar = 0; iChar < remainingLen/2; iChar++)
        formattedString[++outChar] = localString[++inChar];

    // terminate string.
    formattedString[++outChar] = 0;

    outString.assign (formattedString);
    return SUCCESS;
    }

struct SavedParameters
    {
    bool    m_useBessel;
    bool    m_reportInWGS84Datum;
    bool    m_inUSA;
    bool    m_reserved;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual uint32_t        _GetSerializedSize () const override 
    {
    return sizeof (SavedParameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _Serialize (void *buffer, uint32_t maxSize) const override 
    {
    if (maxSize != sizeof (SavedParameters))
        {
        assert (false);
        return BSIERROR;
        }
    SavedParameters*    parameters   = (SavedParameters*) buffer;
    parameters->m_useBessel          = m_useBessel;
    parameters->m_reportInWGS84Datum = m_reportInWGS84Datum;
    parameters->m_inUSA              = m_inUSA;
    parameters->m_reserved           = false;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _DrawGrid (ViewportP vP) const override {}
virtual void            _PointToGrid (ViewportP vP, DPoint3dR point) const override {;}

virtual uint32_t        _GetExtenderId() const override {return GEOCOORD_MGRS_EXTENDERID;}

virtual StatusInt       _SetName (WCharCP name) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetDescription (WCharCP descr) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetType (ACSType type) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetScale (double scale) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetOrigin (DPoint3dCR pOrigin) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetRotation (RotMatrixCR pRot) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetFlags (ACSFlags flags) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _SetElementId (ElementId elementId, DgnModelRefP modelRef) override {return MDLERR_WRITEINHIBIT;}

virtual StatusInt       _SaveToFile (DgnModelRefP modelRef, ACSSaveOptions option) override {return MDLERR_WRITEINHIBIT;}
virtual StatusInt       _DeleteFromFile (DgnModelRefP modelRef) override {return MDLERR_WRITEINHIBIT;}

};




static DPoint2d     continentalUSEnvelope[] = 
                    { {-124.74, 49.07}, {-115.18, 49.00}, {-100.27, 49.00}, { -94.79, 49.12 }, { -84.20, 46.75}, { -82.48, 45.16}, { -82.24, 43.37}, { -83.19, 41.99}, 
                      { -81.50, 41.68}, { -81.25, 42.21}, { -79.00, 42.81}, { -79.19, 43.45 }, { -76.77, 43.67}, { -75.10, 45.00}, { -71.63, 45.00}, { -71.09, 45.30}, 
                      { -69.23, 47.46}, { -68.24, 47.36}, { -66.95, 44.81}, { -74.30, 33.77 }, { -80.00, 31.43}, { -79.67, 26.93}, { -79.53, 24.61}, { -82.85, 23.77},
                      { -83.17, 29.28}, { -95.49, 27.89}, { -97.15, 24.95}, { -99.08, 26.40 }, {-101.36, 29.64}, {-102.72, 29.65}, {-103.12, 28.99}, {-104.38, 29.54},
                      {-106.49, 31.75}, {-108.17, 31.78}, {-108.21, 31.33}, {-111.06, 31.33 }, {-114.81, 32.50}, {-117.12, 32.53}, {-119.89, 32.53}, {-125.78, 40.31}, 
                      {-125.85, 47.47}, {-124.74, 49.07} };

static DPoint2d     continentalUSRange[] = 
                    { {-126.0, 24.0}, {-66.0, 49.5} };

static DPoint2d     alaskaEnvelope[] =
                    { {-180.00, 51.93}, {-170.00, 56.18}, {-175.00, 61.35}, {-161.00, 72.07}, {-141.00, 69.64}, {-141.00, 60.32}, {-135.84, 59.70}, {-130.00, 55.91},
                      {-129.90, 54.83}, {-143.65, 58.81}, {-160.00, 53.00}, {-179.82, 50.86}, {-180.00, 51.93} };

static DPoint2d     alaskaRange[] =
                    { {-180.0, 50.56}, {-130.00, 72.07} };

static DPoint2d     hawaiiRange[] = 
                    { {-164.00, 18.00}, {-154.00, 23.25} };


/*=================================================================================**//**
* This class implements an Auxiliary Coordinate System Extender for Military Grid Reference Systems.
* @bsiclass                                                     Barry.Bentley   06/10
+===============+===============+===============+===============+===============+======*/
class   MilitaryGridAuxCoordSysExtender : public BentleyApi::Dgn::IAuxCoordSystemExtender
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GetCenterPointFromModel (GeoPoint2dR centerPoint, DgnModelRefP modelRef, DgnGCSP dgnGCS)
    {
    DgnModelP       dgnModel;
    if (NULL == (dgnModel = modelRef->GetDgnModelP()))
        return ERROR;

    DRange3d        cacheRange;
    if (SUCCESS != dgnModel->GetRange(cacheRange))
        return ERROR;

    DPoint3d        extent;
    extent.x = cacheRange.high.x - cacheRange.low.x;
    extent.y = cacheRange.high.y - cacheRange.low.y;

    // check the extent of the range. If it's too small, we got nothing. If it's too large, it would be inadvisable to use placemarks based on it.
    if ( (extent.x <= 0) || (extent.y <= 0) )
        return ERROR;

    // use center point as the origin.
    DPoint2d    center;
    center.x = (cacheRange.low.x + cacheRange.high.x) / 2.0;
    center.y = (cacheRange.low.y + cacheRange.high.y) / 2.0;

    // calculate the lat long center point.
    dgnGCS->LatLongFromUors2D (centerPoint, center);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InRange (GeoPoint2dCR latLong, DPoint2d* range)
    {
    // first check range.
    if (latLong.longitude < range[0].x)
        return false;
    if (latLong.longitude > range[1].x)
        return false;
    if (latLong.latitude < range[0].y)
        return false;
    if (latLong.latitude > range[1].y)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GeoPointInside (GeoPoint2dCR latLong, DPoint2dP shape, int shapePointCount)
    {
    return (bsiDPoint2d_PolygonParity ((DPoint2dCP) &latLong, shape, shapePointCount, 0.01) >= 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InContinentalUS (GeoPoint2dCR latLong)
    {
    if (!InRange (latLong, continentalUSRange))
        return false;
    return GeoPointInside (latLong, continentalUSEnvelope, _countof (continentalUSEnvelope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InAlaska (GeoPoint2dCR latLong)
    {
    if (!InRange (latLong, alaskaRange))
        return false;
    return GeoPointInside (latLong, alaskaEnvelope, _countof (alaskaEnvelope));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InHawaii (GeoPoint2dCR latLong)
    {
    return InRange (latLong, hawaiiRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InUnitedStates (GeoPoint2dCR latLong)
    {
    return  InContinentalUS (latLong) || InAlaska (latLong) || InHawaii (latLong);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public:
virtual bool            _TraverseExtendedACS 
(
IACSTraversalHandler&   traverser, 
DgnModelRefP            modelRef
) override
    {
    // allow configuration variable to prevent military grid reference systems.
    if (ConfigurationManager::IsVariableDefined (L"MS_GEOCOORDINATE_NOMGRS"))
        return false;

    // should always have a modelRef, but test anyway.
    if (NULL == modelRef)
        return false;

    // if we don't have a GCS, can't do anything.
    DgnGCSP     dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        return false;

    // If the current GCS has an ellipsoid of the Clarke or Bessel family, we create two MGRS's - one in the old lettering system and one in WGS84 in the new lettering system.
    // If the current GCS has a datum other than WGS84, we create two MGRS's - one in the datum of the GCS and in one in WGS84.
    // If the data appears to be in the United States, we call the WGS84 MGRS "US National Grid".
    WCharCP         ellipsoidName       = dgnGCS->GetEllipsoidName();
    bool            ellipsoidIsBessel   = (0 == wcsncmp (L"CLRK", ellipsoidName, 4)) || (0 == wcsncmp (L"BESL", ellipsoidName, 4)) || (0 == wcsncmp (L"BESSEL", ellipsoidName, 6));

    WCharCP         datumName           = dgnGCS->GetDatumName();
    bool            datumIsWGS84        = (0 == wcscmp (L"WGS84", datumName)) || (0 == wcscmp (L"NAD83", datumName));

    bool            inUSA               = false;
    GeoPoint2d      centerPoint;
    if (SUCCESS == GetCenterPointFromModel (centerPoint, modelRef, dgnGCS))
        inUSA = InUnitedStates (centerPoint);

    MilitaryGridAuxCoordSys *mgrsAuxSysP;
    if (ellipsoidIsBessel)
        {
        assert (!datumIsWGS84);
        // we make available a MGRS converter that uses the "old" lettering system. Do not call that the US National Grid, even if the data is in the USA.
        if (NULL != (mgrsAuxSysP = MilitaryGridAuxCoordSys::Create (dgnGCS, true, false, false)))
            {
            IAuxCoordSysPtr acsPtr (mgrsAuxSysP);
            if (traverser._HandleACSTraversal (acsPtr))
                return true;
            }
        }
    else if (!datumIsWGS84)
        {
        // make available a MGRS converter that uses the new lettering system, but uses the datum of the source DGN file. Do not call that the US National Grid, even if the data is in the USA.
        if (NULL != (mgrsAuxSysP = MilitaryGridAuxCoordSys::Create (dgnGCS, false, false, false)))
            {
            IAuxCoordSysPtr acsPtr (mgrsAuxSysP);
            if (traverser._HandleACSTraversal (acsPtr))
                return true;
            }
        }

    // make available a MGRS converter that uses the new lettering system, in WGS84 datum. Call that the US National Grid, if the data is in the USA.
    if (NULL != (mgrsAuxSysP = MilitaryGridAuxCoordSys::Create (dgnGCS, false, true, inUSA)))
        {
        IAuxCoordSysPtr acsPtr (mgrsAuxSysP);
        return (traverser._HandleACSTraversal (acsPtr));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual uint32_t        _GetExtenderId() const override {return GEOCOORD_MGRS_EXTENDERID;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysP   _Deserialize (void *persistentData, uint32_t dataSize, DgnModelRefP modelRef) override
    {
    // if we don't have a GCS, can't do anything.
    DgnGCSP    dgnGCS;
    if (NULL == (dgnGCS = DgnGCS::FromModel (modelRef, true)))
        return NULL;

    MilitaryGridAuxCoordSys::SavedParameters*  parameters = (MilitaryGridAuxCoordSys::SavedParameters*) persistentData;

    return MilitaryGridAuxCoordSys::Create (dgnGCS, parameters->m_useBessel, parameters->m_reportInWGS84Datum, parameters->m_inUSA);
    }

};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGeoCoordinationAdmin::AddAuxCoordSystemProcessor (IACSManagerR mgr) const
    {
    // this is called only once because of the 'Admin' contract.
    mgr.AddExtender (new BentleyApi::GeoCoordinates::GeoCoordinateAuxSystemExtender());
    mgr.AddExtender (new BentleyApi::GeoCoordinates::MilitaryGridAuxCoordSysExtender());
    }
