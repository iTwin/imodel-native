//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGdalUtilities.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGdalUtilities
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPMAttributeSet.h>
#include <Imagepp/all/h/HRFGdalUtilities.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <ImagePP-GdalLib/cpl_conv.h>
#include <ImagePP-GdalLib/ogr_spatialref.h>


// All this ugly stuff below exists so we do not need to add gdal internals to our search path.
// Hopefully we are planning or redesigning how we handle geocoding. 
// #ifndef BEIJING_WIP_IMAGEPP
     #define GEO_CONFIG_H // avoid inclusion of geo_config.h stuff
     #include "../../../../ext/gdal/frmts/gtiff/libgeotiff/geo_normalize.h"
// #endif

extern "C" char CPL_DLL* GTIFGetOGISDefn(GTIF*, GTIFDefn*);
extern "C" int  CPL_DLL  GTIFSetFromOGISDefn(GTIF*, const char*);

//-----------------------------------------------------------------------------
// Public
// Static method used to convert a WKT string to Geotiff keys.
//-----------------------------------------------------------------------------
HCPGeoTiffKeys* HRFGdalUtilities::ConvertOGCWKTtoGeotiffKeys(char const* pi_pOGCWKT)
    {
    HCPGeoTiffKeys* pGeoTiffKeys = new HCPGeoTiffKeys();
    
    void* pTif = NULL;
    GTIF* pGTIF = GTIFNew(pTif);
    GTIFSetFromOGISDefn(pGTIF, pi_pOGCWKT);

    //TR 234118 - It is really necessary to initialize this variable to zero, since a memcpy is used
    //by GTIFKeyGet to assign the variable.
    uint32_t ULongVal = 0;
    double DoubleVal = 0;
    char   CharVal[1000];
       
    if (GTIFKeyGet(pGTIF, GTModelTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GTModelTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GTRasterTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GTRasterTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GTCitationGeoKey, CharVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GTCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, GeographicTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeographicTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogCitationGeoKey, CharVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, GeogGeodeticDatumGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogGeodeticDatumGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogPrimeMeridianGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogPrimeMeridianGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogLinearUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogLinearUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogLinearUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogLinearUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAngularUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAngularUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAngularUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAngularUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogEllipsoidGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogEllipsoidGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogSemiMajorAxisGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogSemiMajorAxisGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogSemiMinorAxisGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogSemiMinorAxisGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogInvFlatteningGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogInvFlatteningGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, GeogAzimuthUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogAzimuthUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, GeogPrimeMeridianLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(GeogPrimeMeridianLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjectedCSTypeGeoKey, &ULongVal, 0, 1))
        {
        //The GeoTIFF specification doesn't allow the use of integer value greater
        //than 65535. So it can be assumed that GDAL won't return values greater
        //than 65535 for ProjectedCSTypeGeoKey.
        HASSERT(ULongVal <= USHRT_MAX);
        pGeoTiffKeys->AddKey(ProjectedCSTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, PCSCitationGeoKey, CharVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(PCSCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, ProjectionGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjectionGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCoordTransGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCoordTransGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjLinearUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjLinearUnitsGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, ProjLinearUnitSizeGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjLinearUnitSizeGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStdParallel1GeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStdParallel1GeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStdParallel2GeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStdParallel2GeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjNatOriginLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjNatOriginLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjNatOriginLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjNatOriginLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjFalseOriginNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjFalseOriginNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterLatGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterLatGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterEastingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterEastingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjCenterNorthingGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjCenterNorthingGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjScaleAtNatOriginGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjScaleAtNatOriginGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjScaleAtCenterGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjScaleAtCenterGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjAzimuthAngleGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjAzimuthAngleGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjStraightVertPoleLongGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjStraightVertPoleLongGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, ProjRectifiedGridAngleGeoKey, &DoubleVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(ProjRectifiedGridAngleGeoKey, DoubleVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalCSTypeGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalCSTypeGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalCitationGeoKey, CharVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalCitationGeoKey, CharVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalDatumGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalDatumGeoKey, ULongVal);
        }

    if (GTIFKeyGet(pGTIF, VerticalUnitsGeoKey, &ULongVal, 0, 1))
        {
        pGeoTiffKeys->AddKey(VerticalUnitsGeoKey, ULongVal);
        }

    GTIFFree(pGTIF);

    return pGeoTiffKeys;
    }

//-----------------------------------------------------------------------------
// Public
// Static method used to convert some Geotiff keys to a WKT string.
//-----------------------------------------------------------------------------
bool HRFGdalUtilities::ConvertGeotiffKeysToOGCWKT(const HFCPtr<HCPGeoTiffKeys>& pi_rpGeoTiffKeys,
                                                   WString&                     po_rOGCWKT)
    {
    void*                 pTif = NULL;
    GTIF*                 pGTIF = GTIFNew(pTif);
    GTIFDefn              GTIFDefn;

    uint32_t ULongVal = 0;
    double DoubleVal = 0.0;
    WString CharVal = L"";

    if (pi_rpGeoTiffKeys->GetValue(GTModelTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GTModelTypeGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GTRasterTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GTRasterTypeGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GTCitationGeoKey, &CharVal))
        {
        size_t  destinationBuffSize = CharVal.GetMaxLocaleCharBytes();
        char*   multiByteDestination= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(multiByteDestination,CharVal.c_str(),destinationBuffSize);

        GTIFKeySet(pGTIF, GTCitationGeoKey, TYPE_ASCII, 1, multiByteDestination);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeographicTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeographicTypeGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogCitationGeoKey, &CharVal))
        {
        size_t  destinationBuffSize = CharVal.GetMaxLocaleCharBytes();
        char*   multiByteDestination= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(multiByteDestination,CharVal.c_str(),destinationBuffSize);

        GTIFKeySet(pGTIF, GeogCitationGeoKey, TYPE_ASCII, 1, multiByteDestination);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogGeodeticDatumGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogGeodeticDatumGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogPrimeMeridianGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogPrimeMeridianGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogLinearUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogLinearUnitsGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogLinearUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogLinearUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogAngularUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogAngularUnitsGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogAngularUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogAngularUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogEllipsoidGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogEllipsoidGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogSemiMajorAxisGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogSemiMinorAxisGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogSemiMinorAxisGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogInvFlatteningGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogAzimuthUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, GeogAzimuthUnitsGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(GeogPrimeMeridianLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, GeogPrimeMeridianLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjectedCSTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(PCSCitationGeoKey, &CharVal))
        {
        size_t  destinationBuffSize = CharVal.GetMaxLocaleCharBytes();
        char*   multiByteDestination= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(multiByteDestination,CharVal.c_str(),destinationBuffSize);

        GTIFKeySet(pGTIF, PCSCitationGeoKey, TYPE_ASCII, 1, multiByteDestination);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjectionGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjectionGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjCoordTransGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjCoordTransGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjLinearUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, ProjLinearUnitsGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjLinearUnitSizeGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjLinearUnitSizeGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjStdParallel1GeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStdParallel1GeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjStdParallel2GeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStdParallel2GeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjNatOriginLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjNatOriginLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjNatOriginLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseOriginLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseOriginLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseOriginEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjFalseOriginNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjFalseOriginNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjCenterLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjCenterLatGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjCenterEastingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterEastingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjCenterNorthingGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjCenterNorthingGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjScaleAtNatOriginGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjScaleAtCenterGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjScaleAtCenterGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjAzimuthAngleGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjAzimuthAngleGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjStraightVertPoleLongGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjStraightVertPoleLongGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(ProjRectifiedGridAngleGeoKey, &DoubleVal))
        {
        GTIFKeySet(pGTIF, ProjRectifiedGridAngleGeoKey, TYPE_DOUBLE, 1, DoubleVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(VerticalCSTypeGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalCSTypeGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(VerticalCitationGeoKey, &CharVal))
        {
        size_t  destinationBuffSize = CharVal.GetMaxLocaleCharBytes();
        char*   multiByteDestination= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(multiByteDestination,CharVal.c_str(),destinationBuffSize);

        GTIFKeySet(pGTIF, VerticalCitationGeoKey, TYPE_ASCII, 1, multiByteDestination);
        }

    if (pi_rpGeoTiffKeys->GetValue(VerticalDatumGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalDatumGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    if (pi_rpGeoTiffKeys->GetValue(VerticalUnitsGeoKey, &ULongVal))
        {
        GTIFKeySet(pGTIF, VerticalUnitsGeoKey, TYPE_SHORT, 1, (short)ULongVal);
        }

    GTIFGetDefn(pGTIF, &GTIFDefn);

    char* pWKT = 0;
    pWKT = GTIFGetOGISDefn(pGTIF, &GTIFDefn);

    BeStringUtilities::CurrentLocaleCharToWChar(po_rOGCWKT,pWKT);

    CPLFree(pWKT);
    GTIFFree(pGTIF);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFGdalUtilities::ConvertERMToOGCWKT(  WStringR  po_rOGCWKT,
                                            WStringCR pi_rErmProjection, 
                                            WStringCR pi_rErmDatum, 
                                            WStringCR pi_rErmUnits )
    {
    char* wkt = 0;
    OGRSpatialReference oSRS;

    AString szProjectionMBS(pi_rErmProjection.c_str());
    AString szDatumMBS(pi_rErmDatum.c_str());
    AString szUnitsMBS(pi_rErmUnits.c_str());

    // If the projection is user-defined or unknown ... we will try to use GDAL to obtain the information required to use this "user-defined"
    if( oSRS.importFromERM( szProjectionMBS.c_str(), szDatumMBS.c_str(), szUnitsMBS.c_str() ) == OGRERR_NONE )
        {
        if (OGRERR_NONE == oSRS.exportToWkt(&wkt))
            {
            HASSERT(wkt != 0);

            char localCsPrefix[] = "LOCAL_CS";

            if (strncmp(localCsPrefix, wkt, strlen(localCsPrefix)) == 0)
                {
                delete wkt;

                wkt = 0;
                }
            }
        }
    if (wkt != 0)
        {
        po_rOGCWKT = WString(wkt, false);
        delete wkt;
        return true;
        }
    return false;
    }
