//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGeoTiffKeys.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCPGeoTiffKeys
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <ImagePP/all/h/HTIFFGeoKey.h>
#include <ImagePP/all/h/HTIFFTag.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HGF2DWorldCluster.h>
#include <ImagePP/all/h/HVE2DRectangle.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCPGeoTiffKeys::HCPGeoTiffKeys()
    {
    m_ErrorCode               = GEOCODING_NO_ERROR;
    //For now, the only time this variable should be false is when a WKT cannot be
    //represented by a list of GeoTIFF keys.
    m_HasValidGeoTIFFKeysList = true;
    }





//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCPGeoTiffKeys::HCPGeoTiffKeys(const HCPGeoTiffKeys& pi_rObj)
    {
    m_GeoKeyList              = pi_rObj.m_GeoKeyList;
    m_GeoKeyListItr           = m_GeoKeyList.cend();

    m_ErrorCode               = pi_rObj.m_ErrorCode;
    m_HasValidGeoTIFFKeysList = pi_rObj.m_HasValidGeoTIFFKeysList;


    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCPGeoTiffKeys::~HCPGeoTiffKeys()
    {
    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HFCPtr<HCPGeoTiffKeys> HCPGeoTiffKeys::Clone() const
    {
    return new HCPGeoTiffKeys(*this);
    }


//-----------------------------------------------------------------------------
// public
// GetErrorCode
//-----------------------------------------------------------------------------
HCPGeoTiffKeys::ErrorCode HCPGeoTiffKeys::GetErrorCode()
    {
    return m_ErrorCode;
    }

//-----------------------------------------------------------------------------
// public
// SetErrorCode
//-----------------------------------------------------------------------------
void HCPGeoTiffKeys::SetErrorCode(HCPGeoTiffKeys::ErrorCode pi_ErrorCode)
    {
    HPRECONDITION(pi_ErrorCode != GEOCODING_NO_ERROR);

    m_ErrorCode         = pi_ErrorCode;
    }




//-----------------------------------------------------------------------------
// public
// GetValue methods
//-----------------------------------------------------------------------------
bool HCPGeoTiffKeys::GetValue (unsigned short pi_Key, uint32_t* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::LONG == ((*Itr).second).KeyDataType);

        *po_pVal = ((*Itr).second).KeyValue.LongVal;

        return true;
        }
    return false;
    }

bool HCPGeoTiffKeys::GetValue (unsigned short pi_Key, double* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::DOUBLE == ((*Itr).second).KeyDataType);

        *po_pVal = ((*Itr).second).KeyValue.DoubleVal;

        return true;
        }
    return false;
    }

bool HCPGeoTiffKeys::GetValue (unsigned short pi_Key, WString* po_pVal) const
    {
    HPRECONDITION(po_pVal != 0);

    GeoKeyList::const_iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::ASCII == ((*Itr).second).KeyDataType);

        BeStringUtilities::CurrentLocaleCharToWChar(*po_pVal,((*Itr).second).KeyValue.StringVal);

        return true;
        }
    return false;
    }

//-----------------------------------------------------------------------------
// public
// SetValue methods
//-----------------------------------------------------------------------------
bool HCPGeoTiffKeys::SetValue (unsigned short pi_Key, uint32_t pi_Val)
    {
    //InvalidateCachedInfo should be called before modifying the list of keys

    GeoKeyList::iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::LONG == ((*Itr).second).KeyDataType);

        ((*Itr).second).KeyValue.LongVal = pi_Val;


        return true;
        }
    return false;
    }

bool HCPGeoTiffKeys::SetValue (unsigned short pi_Key, double pi_Val)
    {
    //InvalidateCachedInfo should be called before modifying the list of keys

    GeoKeyList::iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::DOUBLE == ((*Itr).second).KeyDataType);

        ((*Itr).second).KeyValue.DoubleVal = pi_Val;


        return true;
        }
    return false;
    }

bool HCPGeoTiffKeys::SetValue (unsigned short pi_Key, WString& pi_Val)
    {
    //InvalidateCachedInfo should be called before modifying the list of keys

    GeoKeyList::iterator Itr;
    if ((Itr = m_GeoKeyList.find((TIFFGeoKey)pi_Key)) != m_GeoKeyList.end())
        {
        HASSERT(IGeoTiffKeysList::ASCII == ((*Itr).second).KeyDataType);

        size_t  destinationBuffSize = pi_Val.GetMaxLocaleCharBytes();
        char*  pStr= new char[destinationBuffSize];
        BeStringUtilities::WCharToCurrentLocaleChar(pStr,pi_Val.c_str(),destinationBuffSize);
        
        ((*Itr).second).KeyValue.StringVal = pStr;


        return true;
        }
    return false;
    }


//-----------------------------------------------------------------------------
// public
// methods from IGeoTiffKeysList
//-----------------------------------------------------------------------------
bool HCPGeoTiffKeys::GetFirstKey(GeoKeyItem* po_Key) const
    {
    HPRECONDITION(po_Key != 0);

    m_GeoKeyListItr = m_GeoKeyList.cbegin();

    if (m_GeoKeyListItr != m_GeoKeyList.cend())
        {
        *po_Key =  (*m_GeoKeyListItr).second;
        return true;
        }
    return false;
    }
bool HCPGeoTiffKeys::GetNextKey(GeoKeyItem* po_Key) const
    {
    m_GeoKeyListItr++;

    if (m_GeoKeyListItr != m_GeoKeyList.cend())
        {
        *po_Key =  (*m_GeoKeyListItr).second;
        return true;
        }

    return false;
    }

void HCPGeoTiffKeys::AddKey (unsigned short pi_KeyID, uint32_t pi_value)
    {
    //Currently BaseGCS has no knowledge of what a vertical datum is, so adding a
    //vertical unit key won't changed its definition or behavior.

    //In the GeoTIFF standard the integer data are stored as a unsigned short data.
    //ProjectedCSTypeLong is a private key that is used to represent EPSG code for
    //projected coordinate systems whose value is greater than 65535.
    HPRECONDITION((pi_value <= USHRT_MAX) || (pi_KeyID == ProjectedCSTypeLong));

    GeoKeyItem CurKey;

    CurKey.KeyID = pi_KeyID;
    CurKey.KeyDataType = IGeoTiffKeysList::LONG;
    CurKey.KeyValue.LongVal = pi_value;

    m_GeoKeyList.insert(GeoKeyList::value_type((TIFFGeoKey)CurKey.KeyID, CurKey));


    if (pi_KeyID == ProjectedCSTypeLong)
        m_HasValidGeoTIFFKeysList = false;
    }

void HCPGeoTiffKeys::AddKey (unsigned short pi_KeyID, double pi_value)
    {
    //InvalidateCachedInfo should be called before modifying the list of keys

    GeoKeyItem CurKey;

    CurKey.KeyID = pi_KeyID;
    CurKey.KeyDataType = IGeoTiffKeysList::DOUBLE;
    CurKey.KeyValue.DoubleVal = pi_value;

    m_GeoKeyList.insert(GeoKeyList::value_type((TIFFGeoKey)CurKey.KeyID, CurKey));

    }

void HCPGeoTiffKeys::AddKey (unsigned short pi_KeyID, const std::string& pi_value)
    {
    //InvalidateCachedInfo should be called before modifying the list of keys

    GeoKeyItem CurKey;

    CurKey.KeyID = pi_KeyID;
    CurKey.KeyDataType = IGeoTiffKeysList::ASCII;

    char* pStr = new char[pi_value.size()+1];
    BeStringUtilities::Strncpy(pStr, pi_value.size()+1, pi_value.c_str(), pi_value.size());

    CurKey.KeyValue.StringVal = pStr;

    m_GeoKeyList.insert(GeoKeyList::value_type((TIFFGeoKey)CurKey.KeyID, CurKey));

    }

//-----------------------------------------------------------------------------
// public
// Return true if the requested key is found, otherwise false
//-----------------------------------------------------------------------------
bool HCPGeoTiffKeys::HasKey(unsigned short pi_KeyID) const
    {
    return m_GeoKeyList.find(pi_KeyID) != m_GeoKeyList.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HCPGeoTiffKeys::EraseKey(unsigned short pi_KeyID)
    {
    return m_GeoKeyList.erase(pi_KeyID);
    }

//-----------------------------------------------------------------------------
// public
// Return the number of GeoTiff keys
//-----------------------------------------------------------------------------
unsigned short HCPGeoTiffKeys::GetNbKeys() const
    {
    return (unsigned short)m_GeoKeyList.size();
    }




//-----------------------------------------------------------------------------
//  With any given TAG label, search if a corresponding GeoKey exist and return it
//
//  Note : These Geokeys value are define by an enum into HTIFFTag.h
//-----------------------------------------------------------------------------
unsigned short HCPGeoTiffKeys::DecodeGeoKeyIDFromString(const WString& pi_rGeoTagLabel)
    {
    unsigned short GeoKey;

    if (pi_rGeoTagLabel == L"GTModelType")
        GeoKey = GTModelType;
    else if (pi_rGeoTagLabel == L"GTRasterType")
        GeoKey = GTRasterType;
    else if (pi_rGeoTagLabel == L"PCSCitation")
        GeoKey = PCSCitation;
    else if (pi_rGeoTagLabel == L"ProjectedCSType")
        GeoKey = ProjectedCSType;
    else if (pi_rGeoTagLabel == L"ProjectedCSTypeLong")
        GeoKey = ProjectedCSTypeLong;
    else if (pi_rGeoTagLabel == L"GTCitation")
        GeoKey = GTCitation;
    else if (pi_rGeoTagLabel == L"Projection")
        GeoKey = Projection;
    else if (pi_rGeoTagLabel == L"ProjCoordTrans")
        GeoKey = ProjCoordTrans;
    else if (pi_rGeoTagLabel == L"ProjLinearUnits")
        GeoKey = ProjLinearUnits;
    else if (pi_rGeoTagLabel == L"ProjLinearUnitSize")
        GeoKey = ProjLinearUnitSize;
    else if (pi_rGeoTagLabel == L"GeographicType")
        GeoKey = GeographicType;
    else if (pi_rGeoTagLabel == L"GeogCitation")
        GeoKey = GeogCitation;
    else if (pi_rGeoTagLabel == L"GeogGeodeticDatum")
        GeoKey = GeogGeodeticDatum;
    else if (pi_rGeoTagLabel == L"GeogPrimeMeridian")
        GeoKey = GeogPrimeMeridian;
    else if (pi_rGeoTagLabel == L"GeogLinearUnits")
        GeoKey = GeogLinearUnits;
    else if (pi_rGeoTagLabel == L"GeogLinearUnitSize")
        GeoKey = GeogLinearUnitSize;
    else if (pi_rGeoTagLabel == L"GeogAngularUnits")
        GeoKey = GeogAngularUnits;
    else if (pi_rGeoTagLabel == L"GeogAngularUnitSize")
        GeoKey = GeogAngularUnitSize;
    else if (pi_rGeoTagLabel == L"GeogEllipsoid")
        GeoKey = GeogEllipsoid;
    else if (pi_rGeoTagLabel == L"GeogSemiMajorAxis")
        GeoKey = GeogSemiMajorAxis;
    else if (pi_rGeoTagLabel == L"GeogSemiMinorAxis")
        GeoKey = GeogSemiMinorAxis;
    else if (pi_rGeoTagLabel == L"GeogInvFlattening")
        GeoKey = GeogInvFlattening;
    else if (pi_rGeoTagLabel == L"GeogAzimuthUnits")
        GeoKey = GeogAzimuthUnits;
    else if (pi_rGeoTagLabel == L"GeogPrimeMeridianLong")
        GeoKey = GeogPrimeMeridianLong;
    else if (pi_rGeoTagLabel == L"ProjStdParallel1")
        GeoKey = ProjStdParallel1;
    else if (pi_rGeoTagLabel == L"ProjStdParallel2")
        GeoKey = ProjStdParallel2;
    else if (pi_rGeoTagLabel == L"ProjNatOriginLong")
        GeoKey = ProjNatOriginLong;
    else if (pi_rGeoTagLabel == L"ProjNatOriginLat")
        GeoKey = ProjNatOriginLat;
    else if (pi_rGeoTagLabel == L"ProjFalseEasting")
        GeoKey = ProjFalseEasting;
    else if (pi_rGeoTagLabel == L"ProjFalseNorthing")
        GeoKey = ProjFalseNorthing;
    else if (pi_rGeoTagLabel == L"ProjFalseOriginLong")
        GeoKey = ProjFalseOriginLong;
    else if (pi_rGeoTagLabel == L"ProjFalseOriginLat")
        GeoKey = ProjFalseOriginLat;
    else if (pi_rGeoTagLabel == L"ProjFalseOriginEasting")
        GeoKey = ProjFalseOriginEasting;
    else if (pi_rGeoTagLabel == L"ProjFalseOriginNorthing")
        GeoKey = ProjFalseOriginNorthing;
    else if (pi_rGeoTagLabel == L"ProjCenterLong")
        GeoKey = ProjCenterLong;
    else if (pi_rGeoTagLabel == L"ProjCenterLat")
        GeoKey = ProjCenterLat;
    else if (pi_rGeoTagLabel == L"ProjCenterEasting")
        GeoKey = ProjCenterEasting;
    else if (pi_rGeoTagLabel == L"ProjCenterNorthing")
        GeoKey = ProjCenterNorthing;
    else if (pi_rGeoTagLabel == L"ProjScaleAtNatOrigin")
        GeoKey = ProjScaleAtNatOrigin;
    else if (pi_rGeoTagLabel == L"ProjScaleAtCenter")
        GeoKey = ProjScaleAtCenter;
    else if (pi_rGeoTagLabel == L"ProjAzimuthAngle")
        GeoKey = ProjAzimuthAngle;
    else if (pi_rGeoTagLabel == L"ProjStraightVertPoleLong")
        GeoKey = ProjStraightVertPoleLong;
    else if (pi_rGeoTagLabel == L"ProjRectifiedGridAngle")
        GeoKey = ProjRectifiedGridAngle;
    else if (pi_rGeoTagLabel == L"VerticalCSType")
        GeoKey = VerticalCSType;
    else if (pi_rGeoTagLabel == L"VerticalCitation")
        GeoKey = VerticalCitation;
    else if (pi_rGeoTagLabel == L"VerticalDatum")
        GeoKey = VerticalDatum;
    else if (pi_rGeoTagLabel == L"VerticalUnits")
        GeoKey = VerticalUnits;
    else
        GeoKey = EndGeoKey;

    return GeoKey;
    
    }



HCPGeoTiffKeys::KeysValidStatus HCPGeoTiffKeys::ValidateGeoTIFFKey()
    {
    bool  IsEnoughPCSInfo = false;
    bool  IsEnoughGCSInfo = false;

    uint32_t        KeyValue;
    KeysValidStatus IsValid = GEOKEYS_VALID;

    if (HasKey(ProjectedCSTypeLong) == true)
        {
        GetValue(ProjectedCSTypeLong, &KeyValue);

        if (KeyValue > USHRT_MAX)
            {
            IsEnoughPCSInfo = true;
            IsEnoughGCSInfo = true;
            }
        }

    if ((IsEnoughPCSInfo == false) && (HasKey(ProjectedCSType) == true))
        {
        GetValue(ProjectedCSType, &KeyValue);

        if ((KeyValue != TIFFGeo_Undefined) && (KeyValue != TIFFGeo_UserDefined))
            {
            IsEnoughPCSInfo = true;
            IsEnoughGCSInfo = true;
            }
        }

    if ((IsEnoughPCSInfo == false) && (HasKey(Projection) == true))
        {
        GetValue(Projection, &KeyValue);

        if ((KeyValue != TIFFGeo_Undefined) && (KeyValue != TIFFGeo_UserDefined))
            {
            IsEnoughPCSInfo    = true;
            }
        }

    if ((IsEnoughPCSInfo == false) && (HasKey(ProjCoordTrans) == true))
        {
        GetValue(ProjCoordTrans, &KeyValue);

        if ((KeyValue != TIFFGeo_Undefined) && (KeyValue != TIFFGeo_UserDefined))
            {
            IsEnoughPCSInfo    = true;
            }
        }

    if (HasKey(GeographicType) == true)
        {
        GetValue(GeographicType, &KeyValue);

        if ((KeyValue != TIFFGeo_Undefined) && (KeyValue != TIFFGeo_UserDefined))
            {
            IsEnoughGCSInfo = true;
            }
        }

    if ((IsEnoughGCSInfo == false) && (HasKey(GeogGeodeticDatum) == true))
        {
        GetValue(GeogGeodeticDatum, &KeyValue);

        if ((KeyValue != TIFFGeo_Undefined) && (KeyValue != TIFFGeo_UserDefined))
            {
            IsEnoughGCSInfo    = true;
            }
        }

    if ((IsEnoughGCSInfo == false) && (IsEnoughPCSInfo == false))
        {
        IsValid = GEOKEYS_NO_CS;
        }
    else if ((IsEnoughPCSInfo == true) && (IsEnoughGCSInfo == false))
        {
        IsValid = GEOKEYS_PCS_WITH_NO_GCS;
        }

    return IsValid;
    }


//-----------------------------------------------------------------------------
// public
// GetTransfoModelToMeters
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGeoTiffKeys::GetTransfoModelToMeters(GeoCoordinates::BaseGCSCR pi_rpProjection)
    {
    double ToMeters = 1.0 / pi_rpProjection.UnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pUnitConvertion(new HGF2DStretch(HGF2DDisplacement(0.0, 0.0),
                                                               ToMeters,
                                                               ToMeters));
    return pUnitConvertion;
    }

//-----------------------------------------------------------------------------
// public
// GetTransfoModelFromMeters
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGeoTiffKeys::GetTransfoModelFromMeters(GeoCoordinates::BaseGCSCR pi_rpProjection)
    {
    double FromMeters = pi_rpProjection.UnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pUnitConvertion(new HGF2DStretch(HGF2DDisplacement(0.0, 0.0),
                                                               FromMeters,
                                                               FromMeters));
    return pUnitConvertion;
    }




/** -----------------------------------------------------------------------------
Public / Static
This method must be called before any Geoding requested to know if the
given file (using the file format capabilities) support the minimal Geocoding
information.

@param pi_rpRasterFile    The rasterfile will be use to retreive it's file
formats capabilities.  Then the function will verify
if three tags are supported :

GTModelType
Projection
ProjectedCSType 

Return true if at least ONE of these two TAG are supported.
-----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> 
HCPGeoTiffKeys::GetTransfoModelForReprojection(const HFCPtr<HGF2DCoordSys>&     pi_rpRasterCoordSys,
                                               const HGF2DExtent&               pi_rRasterExtent,
                                               const HGF2DExtent&               pi_rMinimumRasterPixelRange,
                                               GeoCoordinates::BaseGCSCR        pi_rSourceCoordSys,
                                               GeoCoordinates::BaseGCSCR        pi_rDestCoordSys,
                                               const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster)
{
    HPRECONDITION(pi_rpWorldCluster != 0);
                   
    // Create the reprojection transfo model (non adapted)
    HFCPtr<HGF2DTransfoModel> pDstToSrcTransfoModel(new HCPGCoordModel(pi_rDestCoordSys, pi_rSourceCoordSys));
        
    // Create the reprojected coordSys
    HFCPtr<HGF2DCoordSys> pDstCoordSys(new HGF2DCoordSys(*pDstToSrcTransfoModel, pi_rpRasterCoordSys));
    
    HGF2DLiteExtent ImageLiteExtent(pi_rRasterExtent.GetXMin(), pi_rRasterExtent.GetYMin(), pi_rRasterExtent.GetXMax(), pi_rRasterExtent.GetYMax());
    
    // Compute step
    long   NbPoints (5);
    double Step(MIN(pi_rRasterExtent.GetHeight(),pi_rRasterExtent.GetWidth()) / NbPoints);

    // Compute the expected mean error
    double ImageCenter_x(pi_rRasterExtent.GetXMin() + pi_rRasterExtent.GetWidth() / 2);
    double ImageCenter_y(pi_rRasterExtent.GetYMin() + pi_rRasterExtent.GetHeight() / 2);
    
    HFCPtr<HVEShape> pPixelShape(new HVEShape(HVE2DRectangle(ImageCenter_x - pi_rMinimumRasterPixelRange.GetWidth() / 2, 
                                                             ImageCenter_y - pi_rMinimumRasterPixelRange.GetHeight() / 2, 
                                                             ImageCenter_x + pi_rMinimumRasterPixelRange.GetWidth() / 2, 
                                                             ImageCenter_y + pi_rMinimumRasterPixelRange.GetHeight() / 2, 
                                                             pi_rRasterExtent.GetCoordSys())));
    pPixelShape->ChangeCoordSys(pDstCoordSys);
    HGF2DExtent PixelExtent(pPixelShape->GetExtent());
    double ExpectedMeanError(MIN(PixelExtent.GetWidth() * 0.5, PixelExtent.GetHeight() * 0.5));
    double ExpectedMaxError(MIN(PixelExtent.GetWidth(), PixelExtent.GetHeight()));

    HFCPtr<HGF2DTransfoModel> pTransfoModelForReprojection;

    pTransfoModelForReprojection = HCPGCoordUtility::CreateGCoordAdaptedModel(pi_rSourceCoordSys, 
                                                                              pi_rDestCoordSys, 
                                                                              ImageLiteExtent, 
                                                                              Step, 
                                                                              ExpectedMeanError, 
                                                                              ExpectedMaxError, 
                                                                              NULL, 
                                                                              NULL, 
                                                                              NULL, 
                                                                              NULL);

    return pTransfoModelForReprojection;
}

/** -----------------------------------------------------------------------------
Public / Static
This method must be called before any Geoding requested to know if the
given file (using the file format capabilities) support the minimal Geocoding
information.

@param pi_rpRasterFile    The rasterfile will be use to retreive it's file
formats capabilities.  Then the function will verify
if three tags are supported :

GTModelType
Projection
ProjectedCSType

Return true if at least ONE of these two TAG are supported.
-----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel>
HCPGeoTiffKeys::GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&     pi_rpSrcRasterFile,
                                               uint32_t                         pi_PageNumber,
                                               GeoCoordinates::BaseGCSCP        pi_pDestCoordSys,
                                               const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                                               GeoCoordinates::BaseGCSCP        pi_pOverwriteSourceCoordSys)
    {
    HPRECONDITION(pi_rpWorldCluster != 0);

    HFCPtr<HGF2DTransfoModel> pTransfoModelForReprojection;
    GeoCoordinates::BaseGCSCP pSrcFileGeocoding = nullptr;

    if (pi_pOverwriteSourceCoordSys != nullptr)
        {
        pSrcFileGeocoding = pi_pOverwriteSourceCoordSys;
        }
    else
        {
        pSrcFileGeocoding = pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetGeocodingCP();
        }

    // Georeference
    if (pSrcFileGeocoding != nullptr && pSrcFileGeocoding->IsValid() &&
        pi_pDestCoordSys != nullptr && pi_pDestCoordSys->IsValid())
        {
        // Compute the image extent expressed in the src projection units
        HFCPtr<HGF2DCoordSys> pSrcWorldMeters(pi_rpWorldCluster->GetCoordSysReference(HGF2DWorld_HMRWORLD));

        HFCPtr<HGF2DCoordSys> pSrcUnitCoordSys = new HGF2DCoordSys(*GetTransfoModelToMeters(*pSrcFileGeocoding), pSrcWorldMeters);
        HFCPtr<HGF2DCoordSys> pPhysicalCoordSys;

        HFCPtr<HRFPageDescriptor>       pPage(pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber));
        HFCPtr<HRFResolutionDescriptor> pRes(pPage->GetResolutionDescriptor(0));

        if (pPage->HasTransfoModel())
            {
            pPhysicalCoordSys = new HGF2DCoordSys(*pPage->GetTransfoModel(), pi_rpWorldCluster->GetCoordSysReference(pi_rpSrcRasterFile->GetWorldIdentificator()));
            }
        else
            {
            pPhysicalCoordSys = new HGF2DCoordSys(HGF2DIdentity(), pi_rpWorldCluster->GetCoordSysReference(pi_rpSrcRasterFile->GetWorldIdentificator()));
            }

        // Create the reprojection transfo model (non adapted)
        HFCPtr<HGF2DTransfoModel> pDstToSrcTransfoModel(new HCPGCoordModel(*pi_pDestCoordSys, *pSrcFileGeocoding));

        // Create the reprojected coordSys
        HFCPtr<HGF2DCoordSys> pDstCoordSys(new HGF2DCoordSys(*pDstToSrcTransfoModel, pSrcUnitCoordSys));

        CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0)->GetWidth())
        CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0)->GetHeight())

        double ImageWidth = (double)pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0)->GetWidth();
        double ImageHeight = (double)pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetResolutionDescriptor(0)->GetHeight();

        HFCPtr<HVEShape> pImageRectangle(new HVEShape(HVE2DRectangle(0.0, 0.0, ImageWidth, ImageHeight, pPhysicalCoordSys)));
        pImageRectangle->ChangeCoordSys(pSrcUnitCoordSys);
        HGF2DExtent ImageExtent(pImageRectangle->GetExtent());
        HGF2DLiteExtent ImageLiteExtent(ImageExtent.GetXMin(), ImageExtent.GetYMin(), ImageExtent.GetXMax(), ImageExtent.GetYMax());

        // Compute step
        int32_t NbPoints (5);
        double Step(MIN(ImageExtent.GetHeight(), ImageExtent.GetWidth()) / NbPoints);

        // Compute the expected mean error
        double ImageCenter_x(ImageWidth / 2.0);
        double ImageCenter_y(ImageHeight / 2.0);

        HFCPtr<HVEShape> pPixelShape(new HVEShape(HVE2DRectangle(ImageCenter_x - 0.5, ImageCenter_y - 0.5, ImageCenter_x + 0.5, ImageCenter_y + 0.5, pPhysicalCoordSys)));
        pPixelShape->ChangeCoordSys(pDstCoordSys);
        HGF2DExtent PixelExtent(pPixelShape->GetExtent());
        double ExpectedMeanError(MIN(PixelExtent.GetWidth() * 0.5, PixelExtent.GetHeight() * 0.5));
        double ExpectedMaxError (MIN(PixelExtent.GetWidth(), PixelExtent.GetHeight()));

        pTransfoModelForReprojection = HCPGCoordUtility::CreateGCoordAdaptedModel(*pSrcFileGeocoding,
                                       *pi_pDestCoordSys,
                                       ImageLiteExtent,
                                       Step,
                                       ExpectedMeanError,
                                       ExpectedMaxError,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);

        HFCPtr<HGF2DCoordSys> pSrcFileLogicalCoordSys(pi_rpWorldCluster->
                                                      GetCoordSysReference(pi_rpSrcRasterFile->
                                                                           GetWorldIdentificator()));

        HFCPtr<HGF2DTransfoModel> pLogiqueToHmr(pSrcFileLogicalCoordSys->
                                                GetTransfoModelTo(pi_rpWorldCluster->
                                                                  GetCoordSysReference(HGF2DWorld_HMRWORLD)));
        HFCPtr<HGF2DTransfoModel> pFromMeters(GetTransfoModelFromMeters(*pSrcFileGeocoding));
        HFCPtr<HGF2DTransfoModel> pToMeters(GetTransfoModelToMeters(*pi_pDestCoordSys));

        pTransfoModelForReprojection = pLogiqueToHmr->ComposeInverseWithDirectOf(*pFromMeters)->
                                       ComposeInverseWithDirectOf(*pTransfoModelForReprojection)->
                                       ComposeInverseWithDirectOf(*pToMeters)->
                                       ComposeInverseWithInverseOf(*pLogiqueToHmr);
        }

    return pTransfoModelForReprojection;
    }


#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

/** -----------------------------------------------------------------------------
This method create a HGF2DTransfoModel using the information found in the
geo tiff file tag and geo reference information
-----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HCPGeoTiffKeys::CreateTransfoModelFromGeoTiff(
                                                        HCPGeoTiffKeys const*   pi_rpGeoTiffKeys,
                                                        double                  pi_FactorModelToMeter,
                                                        double*                 pi_pMatrix,    // 4 x 4
                                                        uint32_t                pi_MatSize,
                                                        double*                 pi_pPixelScale,
                                                        uint32_t                pi_NbPixelScale,
                                                        double*                 pi_pTiePoints, 
                                                        uint32_t                pi_NbTiePoints,
                                                        bool                    pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                                        bool*                   po_DefaultUnitWasFound)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    bool IsValidModel = false;
    bool NeedFlip = false;

    // Case 0 : No positionning tag in the file.
    if (pi_NbTiePoints == 0 && pi_NbPixelScale == 0 && pi_MatSize == 0)
        {
        IsValidModel = true;
        NeedFlip = true;
        }

    // Case 1: One pair of tie point present. (And Case 6)
    else if (pi_NbTiePoints == 6 && pi_NbPixelScale == 0)
        {
        // We do not support both tie point and matrix to be present
        // This is Case 6, so we ignore the matrix.
        HWARNING((pi_MatSize == 0), L"HRFGeoTiff: TiePoints are used, Matrix ignored...");

        double OffsetX = pi_pTiePoints[3] - pi_pTiePoints[0];
        double OffsetY = pi_pTiePoints[4] + pi_pTiePoints[1];

        pTransfoModel = new HGF2DTranslation(HGF2DDisplacement(OffsetX, OffsetY));

        IsValidModel = true;
        NeedFlip = true;        // if modified see the same case in WriteTransfoModelFromGeoTiff
        }

    // Case 3: One pair of tie point present and pixelscale. (And Case 6)
    else if (pi_NbTiePoints == 6 && pi_NbPixelScale == 3)
        {
        // We do not support both tie point and matrix to be present
        // This is Case 6, so we ignore the matrix.
        HWARNING((pi_MatSize == 0), L"HRFGeoTiff: TiePoints are used, Matrix ignored...");

        double OffsetX = pi_pTiePoints[3] - (pi_pTiePoints[0] * pi_pPixelScale[0]);
        double OffsetY = pi_pTiePoints[4] + (pi_pTiePoints[1] * pi_pPixelScale[1]);

        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(OffsetX, OffsetY),
            pi_pPixelScale[0],
            pi_pPixelScale[1]);
        IsValidModel = true;
        NeedFlip = true;        // if modified see the same case in WriteTransfoModelFromGeoTiff
        }

    // Case 4: Model Transformation tag present. (And Case 7)
    else if (pi_NbTiePoints == 0 && pi_MatSize == 16)
        {
        // We do not support both pixelscale and matrix to be present
        // This is Case 7, so we ignore the pixelscale.
        HASSERT(pi_NbPixelScale == 0);

        HFCMatrix<3, 3> TheMatrix;

        TheMatrix[0][0] = pi_pMatrix[0];
        TheMatrix[0][1] = pi_pMatrix[1];
        TheMatrix[0][2] = pi_pMatrix[3];
        TheMatrix[1][0] = pi_pMatrix[4];
        TheMatrix[1][1] = pi_pMatrix[5];
        TheMatrix[1][2] = pi_pMatrix[7];
        TheMatrix[2][0] = pi_pMatrix[12];
        TheMatrix[2][1] = pi_pMatrix[13];

        // Try to not invalidate the model just because the
        // global scale is not set. TR# 138746
        if (!HDOUBLE_EQUAL_EPSILON(pi_pMatrix[15], 0.0))
            TheMatrix[2][2] = pi_pMatrix[15];
        else
            TheMatrix[2][2] = 1.0;

        pTransfoModel = new HGF2DProjective(TheMatrix);

        // Get the simplest model possible.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

        if (pTempTransfoModel != 0)
            pTransfoModel = pTempTransfoModel;

        IsValidModel = true;
        }

    // Case 5: Many pairs of tie points defined. (And Case 9, Case 6)
    else if (pi_NbTiePoints > 6 && pi_NbTiePoints <= 24)
        {
        // We do not support both, more than one ties points and matrix to be present
        // This is Case 6, so we ignore the matrix.
        HWARNING((pi_MatSize == 0), L"HRFGeoTiff: TiePoints are used, Matrix ignored...");
        // We do not support both, more than one ties points and pixelscale to be present
        // This is Case 6, so we ignore the pixelscale.
        HWARNING((pi_NbPixelScale == 0), L"HRFGeoTiff: TiePoints are used, Pixelscale ignored...");

        double pMatrix[4][4];

        GetTransfoMatrixFromScaleAndTiePts(pMatrix, (unsigned short)pi_NbTiePoints, pi_pTiePoints, 0, 0);

        HFCMatrix<3, 3> TheMatrix;

        TheMatrix[0][0] = pMatrix[0][0];
        TheMatrix[0][1] = pMatrix[0][1];
        TheMatrix[0][2] = pMatrix[0][3];
        TheMatrix[1][0] = pMatrix[1][0];
        TheMatrix[1][1] = pMatrix[1][1];
        TheMatrix[1][2] = pMatrix[1][3];
        TheMatrix[2][0] = pMatrix[3][0];
        TheMatrix[2][1] = pMatrix[3][1];
        TheMatrix[2][2] = pMatrix[3][3];

        pTransfoModel = new HGF2DProjective(TheMatrix);

        // Get the simplest model possible.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

        if (pTempTransfoModel != 0)
            pTransfoModel = pTempTransfoModel;

        IsValidModel = true;
        if (ImageppLib::GetHost().GetImageppLibAdmin()._IsSetFlipInY_IfModelDefinedBy7to24TiePoints())
            NeedFlip = true;
    }
    else if (pi_NbTiePoints > 24)
        {
        // We do not support both, more than one ties points and matrix to be present
        // This is Case 6, so we ignore the matrix.
        HWARNING((pi_MatSize == 0), L"HRFGeoTiff: TiePoints are used, Matrix ignored...");
        // We do not support both, more than one ties points and pixelscale to be present
        // This is Case 6, so we ignore the pixelscale.
        HWARNING((pi_NbPixelScale == 0), L"HRFGeoTiff: TiePoints are used, Pixelscale ignored...");

        double pMatrix[4][4];

        GetTransfoMatrixFromScaleAndTiePts(pMatrix, (unsigned short)pi_NbTiePoints, pi_pTiePoints, (unsigned short)pi_NbPixelScale, pi_pPixelScale);
        HFCMatrix<3, 3> TheMatrix;

        TheMatrix[0][0] = pMatrix[0][0];
        TheMatrix[0][1] = pMatrix[0][1];
        TheMatrix[0][2] = pMatrix[0][3];
        TheMatrix[1][0] = pMatrix[1][0];
        TheMatrix[1][1] = pMatrix[1][1];
        TheMatrix[1][2] = pMatrix[1][3];
        TheMatrix[2][0] = pMatrix[3][0];
        TheMatrix[2][1] = pMatrix[3][1];
        TheMatrix[2][2] = pMatrix[3][3];

        pTransfoModel = new HGF2DProjective(TheMatrix);

        // Get the simplest model possible.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

        if (pTempTransfoModel != 0)
            pTransfoModel = pTempTransfoModel;

        IsValidModel = true;
        NeedFlip = false;       // if modified see the same case in WriteTransfoModelFromGeoTiff
        }

    // If pixelIsPoint(origin center of the pixel), we need to translate the origin to
    // the upper-left corner.
    uint32_t GTRasterTypeValue(TIFFGeo_RasterPixelIsArea);
    if (pi_rpGeoTiffKeys->HasKey(GTRasterType))
        pi_rpGeoTiffKeys->GetValue(GTRasterType, &GTRasterTypeValue);
    if (GTRasterTypeValue == TIFFGeo_RasterPixelIsPoint)
        {
        HGF2DDisplacement Depl(-0.5, -0.5);
        HGF2DTranslation TranslateModel(Depl);
        pTransfoModel = TranslateModel.ComposeInverseWithDirectOf(*pTransfoModel);
        }

    // Flip the model if needed
    if (IsValidModel && NeedFlip)
        {
        // Flip the Y Axe because the origin of ModelSpace is lower-left
        HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

        pFlipModel->SetYScaling(-1.0);
        pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);
        }

    if (pi_rpGeoTiffKeys != 0)
    {
        RasterFileGeocodingPtr pFileGeocoding(RasterFileGeocoding::Create(pi_rpGeoTiffKeys));
        pTransfoModel = pFileGeocoding->TranslateToMeter(pTransfoModel,
            pi_FactorModelToMeter,
            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
            po_DefaultUnitWasFound);
    }
    else if (po_DefaultUnitWasFound != 0)
        *po_DefaultUnitWasFound = false;

    return pTransfoModel;
}

void HCPGeoTiffKeys::WriteTransfoModelFromGeoTiff(HCPGeoTiffKeys const*                 pi_rpGeoTiffKeys,
                                                  const HFCPtr<HGF2DTransfoModel>&      pi_pModel,
                                                  uint64_t                              pi_ImageWidth,
                                                  uint64_t                              pi_ImageHeight,
                                                  bool                                  pi_StoreUsingMatrix,
                                                  double*                               po_pMatrix,           // 4 x 4  array[16]
                                                  uint32_t&                               pio_MatSize,
                                                  double*                               po_pPixelScale,       // array[3]
                                                  uint32_t&                               pio_NbPixelScale,
                                                  double*                               po_pTiePoints,        // array[24]
                                                  uint32_t&                               pio_NbTiePoints,
                                                  bool                                  pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                                  bool*                                 po_DefaultUnitWasFound)

{

    // Translate the model units in geotiff units.
    HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_pModel);
    if (pi_rpGeoTiffKeys != 0)
    {
        RasterFileGeocodingPtr pFileGeocoding(RasterFileGeocoding::Create(pi_rpGeoTiffKeys));
        pTransfoModel = pFileGeocoding->TranslateFromMeter(pi_pModel,
            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
            po_DefaultUnitWasFound);
    }
    else if (po_DefaultUnitWasFound != 0)
        *po_DefaultUnitWasFound = false;

    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();
    if (pTempTransfoModel != 0)
        pTransfoModel = pTempTransfoModel;

    // Write a matrix, if ask for(pi_StoreUsingMatrix),
    //                 if the file already have a matrix((pio_NbTiePoints == 0 && pio_MatSize == 16))
    //                 if ask for limited Tiepoints model(m_sStoreLimitedTiePoints) and
    //                      model is not (Translation or stretch)
    //
    bool UseMatrix = pi_StoreUsingMatrix || (pio_NbTiePoints == 0 && pio_MatSize == 16) ||
        (ImageppLib::GetHost().GetImageppLibAdmin()._IsGeoTiffFileStoreLimitedTiePointsModel() &&
        !pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID) &&
        !pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID));

    // Set the model using matix if the user had ask so or
    // if the file already have a matrix.
    if (UseMatrix && pTransfoModel->CanBeRepresentedByAMatrix())
    {
        HFCMatrix<3, 3> TheMatrix;

        // Atention: Same code in the else below
        //
        // If pixelIsPoint(origin center of the pixel), we need to translate the origin to
        // the upper-left corner.
        uint32_t GTRasterTypeValue(TIFFGeo_RasterPixelIsArea);
        if (pi_rpGeoTiffKeys->HasKey(GTRasterType))
            pi_rpGeoTiffKeys->GetValue(GTRasterType, &GTRasterTypeValue);
        if (GTRasterTypeValue == TIFFGeo_RasterPixelIsPoint)
        {
            HGF2DDisplacement Depl(-0.5, -0.5);
            HGF2DTranslation TranslateModel(Depl);
            pTransfoModel = TranslateModel.ComposeInverseWithDirectOf(*pTransfoModel);
        }


        TheMatrix = pTransfoModel->GetMatrix();

        // a-b-c-d
        po_pMatrix[0] = TheMatrix[0][0];
        po_pMatrix[1] = TheMatrix[0][1];
        po_pMatrix[2] = 0.0;
        po_pMatrix[3] = TheMatrix[0][2];

        // e-f-g-h
        po_pMatrix[4] = TheMatrix[1][0];
        po_pMatrix[5] = TheMatrix[1][1];
        po_pMatrix[6] = 0.0;
        po_pMatrix[7] = TheMatrix[1][2];

        // i-j-k-l
        po_pMatrix[8] = 0.0;
        po_pMatrix[9] = 0.0;
        po_pMatrix[10] = 1.0;
        po_pMatrix[11] = 0.0;

        // m-n-o-p
        po_pMatrix[12] = TheMatrix[2][0];
        po_pMatrix[13] = TheMatrix[2][1];
        po_pMatrix[14] = 0.0;
        po_pMatrix[15] = TheMatrix[2][2];

        pio_MatSize = 16;   // Mat is used
        pio_NbPixelScale = 0;
        pio_NbTiePoints = 0;
    }
    else
    {
        // TiePoints cases

        bool NeedToFlip = false;
        // Case 1: One pair of tie point present. (And Case 6)
        if (pio_NbTiePoints == 6 && pio_NbPixelScale == 0)
            NeedToFlip = true;
        // Case 3: One pair of tie point present and pixelscale. (And Case 6)
        else if (pio_NbTiePoints == 6 && pio_NbPixelScale == 3)
            NeedToFlip = true;
        // Case 5: Many pairs of tie points defined. (And Case 9, Case 6)
        else if (pio_NbTiePoints > 6 && pio_NbTiePoints <= 24)
        {
            if (ImageppLib::GetHost().GetImageppLibAdmin()._IsSetFlipInY_IfModelDefinedBy7to24TiePoints())
                NeedToFlip = true;
        }
        // A new file
        else if (pio_NbPixelScale == 0 && pio_NbTiePoints == 0 && (pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID) || 
                                                                   pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID)))
            NeedToFlip = true;


        // Flip the model if needed
        if (NeedToFlip)
        {
            // Flip the Y Axe because the origin of ModelSpace is lower-left
            HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

            pFlipModel->SetYScaling(-1.0);
            pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);
        }

        // Atention: Same code in the if above
        //
        // If pixelIsPoint(origin center of the pixel), we need to translate the origin from
        // the upper-left corner to center.
        uint32_t GTRasterTypeValue(TIFFGeo_RasterPixelIsArea);
        if (pi_rpGeoTiffKeys->HasKey(GTRasterType))
            pi_rpGeoTiffKeys->GetValue(GTRasterType, &GTRasterTypeValue);
        if (GTRasterTypeValue == TIFFGeo_RasterPixelIsPoint)
        {
            HGF2DDisplacement Depl(-0.5, -0.5);
            HGF2DTranslation TranslateModel(Depl);
            pTransfoModel = TranslateModel.ComposeInverseWithDirectOf(*pTransfoModel);
        }

        // Try to set the georef tag as it was originaly in the file

        // Case 1: One pair of tie point present. (And Case 6)
        if ((pio_NbTiePoints == 6 && pio_NbPixelScale == 0) && pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID))
        {
            po_pTiePoints[0] = 0.0;
            po_pTiePoints[1] = 0.0;
            po_pTiePoints[2] = 0.0;

            // Transfome the pixel (0,0)
            pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
            po_pTiePoints[5] = 0.0;

            pio_MatSize = 0;      
            pio_NbPixelScale = 0;
            pio_NbTiePoints = 6;    // Tie point used
        }

        // Case 3: One pair of tie point present and pixelscale. (And Case 6)
        else if ((pio_NbTiePoints == 6 && pio_NbPixelScale == 3) && pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID))
        {
            HGF2DDisplacement Displacement;

            pTransfoModel->GetStretchParams(&po_pPixelScale[0], &po_pPixelScale[1], &Displacement);

            // Scale x,y,z
            po_pPixelScale[2] = 0.0;

            po_pTiePoints[0] = 0.0;
            po_pTiePoints[1] = 0.0;
            po_pTiePoints[2] = 0.0;

            // Transfome the point (0,0)
            pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
            po_pTiePoints[5] = 0.0;

            // TiePoint Pixel(0,0,0) System(T(0),T(0),T(0)) ...
            pio_MatSize = 0;
            pio_NbPixelScale = 3;   // PixelScale is used
            pio_NbTiePoints = 6;    // Tie point is used
        }

        // In all other case set set the georef using the rule discribe in HRFGeoTiffFile.doc
        else
        {
            if (pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID))
            {
                // 1 pair of tie points is needed.

                po_pTiePoints[0] = 0.0;
                po_pTiePoints[1] = 0.0;
                po_pTiePoints[2] = 0.0;

                // Transfome the pixel (0,0)
                pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
                po_pTiePoints[5] = 0.0;

                // TiePoint Pixel(0,0,0) System(T(0),T(0),T(0)) ...
                pio_MatSize = 0;
                pio_NbPixelScale = 0;   
                pio_NbTiePoints = 6;    // Tie point is used
            }
            else if (pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID))
            {
                // 1 pair of tie points + a scaling is needed.
                HGF2DDisplacement Displacement;

                pTransfoModel->GetStretchParams(&po_pPixelScale[0], &po_pPixelScale[1], &Displacement);

                // Scale x,y,z
                po_pPixelScale[2] = 0.0;

                po_pTiePoints[0] = 0.0;
                po_pTiePoints[1] = 0.0;
                po_pTiePoints[2] = 0.0;

                // Transfome the point (0,0)
                pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
                po_pTiePoints[5] = 0.0;

                // TiePoint Pixel(0,0,0) System(T(0),T(0),T(0)) ...
                pio_MatSize = 0;
                pio_NbPixelScale = 3;   // PixelScale is used
                pio_NbTiePoints = 6;    // Tie point is used
            }
            else if (pTransfoModel->IsCompatibleWith(HGF2DSimilitude::CLASS_ID))
            {
                // 2 pairs of tie points is needed.

                // Frist pair.
                po_pTiePoints[0] = 0.0;
                po_pTiePoints[1] = 0.0;
                po_pTiePoints[2] = 0.0;

                // Transfome the pixel (0,0)
                pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
                po_pTiePoints[5] = 0.0;

                // Second pair.
                po_pTiePoints[6] = (double)pi_ImageWidth;
                po_pTiePoints[7] = (double)pi_ImageHeight;
                po_pTiePoints[8] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[6], po_pTiePoints[7], &po_pTiePoints[9], &po_pTiePoints[10]);
                po_pTiePoints[11] = 0.0;

                // TiePoint Pixel(0,0,0)                     System(T(0),T(0),T(0)) ...
                // TiePoint Pixel(pi_ImageWidth,pi_ImageHeight,0)  System(T(pi_ImageWidth),T(pi_ImageHeight),T(0)) ...
                pio_MatSize = 0;
                pio_NbPixelScale = 0; 
                pio_NbTiePoints = 12;    // Tie point is used
            }
            else if (pTransfoModel->IsCompatibleWith(HGF2DAffine::CLASS_ID))
            {
                // 3 pairs of tie points is needed.

                // Frist pair.
                po_pTiePoints[0] = 0.0;
                po_pTiePoints[1] = 0.0;
                po_pTiePoints[2] = 0.0;

                // Transfome the pixel (0,0)
                pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
                po_pTiePoints[5] = 0.0;

                // Second pair.
                po_pTiePoints[6] = (double)pi_ImageWidth;
                po_pTiePoints[7] = (double)pi_ImageHeight;
                po_pTiePoints[8] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[6], po_pTiePoints[7], &po_pTiePoints[9], &po_pTiePoints[10]);
                po_pTiePoints[11] = 0.0;

                // Third pair.
                po_pTiePoints[12] = (double)pi_ImageWidth;
                po_pTiePoints[13] = 0.0;
                po_pTiePoints[14] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[12], po_pTiePoints[13], &po_pTiePoints[15], &po_pTiePoints[16]);
                po_pTiePoints[17] = 0.0;

                // TiePoint Pixel(0,0,0)                     System(T(0),T(0),T(0)) ...
                // TiePoint Pixel(pi_ImageWidth,pi_ImageHeight,0)  System(T(pi_ImageWidth),T(pi_ImageHeight),T(0)) ...
                // TiePoint Pixel(pi_ImageWidth,0,0)            System(T(pi_ImageWidth),T(0),T(0)) ...
                pio_MatSize = 0;
                pio_NbPixelScale = 0;
                pio_NbTiePoints = 18;    // Tie point is used
            }
            else if (pTransfoModel->IsCompatibleWith(HGF2DProjective::CLASS_ID))
            {
                // 4 pairs of tie points is needed.

                // Frist pair.
                po_pTiePoints[0] = 0.0;
                po_pTiePoints[1] = 0.0;
                po_pTiePoints[2] = 0.0;

                // Transfome the pixel (0,0)
                pTransfoModel->ConvertDirect(po_pTiePoints[0], po_pTiePoints[1], &po_pTiePoints[3], &po_pTiePoints[4]);
                po_pTiePoints[5] = 0.0;

                // Second pair.
                po_pTiePoints[6] = (double)pi_ImageWidth;
                po_pTiePoints[7] = (double)pi_ImageHeight;
                po_pTiePoints[8] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[6], po_pTiePoints[7], &po_pTiePoints[9], &po_pTiePoints[10]);
                po_pTiePoints[11] = 0.0;

                // Third pair.
                po_pTiePoints[12] = (double)pi_ImageWidth;
                po_pTiePoints[13] = 0.0;
                po_pTiePoints[14] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[12], po_pTiePoints[13], &po_pTiePoints[15], &po_pTiePoints[16]);
                po_pTiePoints[17] = 0.0;

                // Forth pair.
                po_pTiePoints[18] = 0.0;
                po_pTiePoints[19] = (double)pi_ImageHeight;
                po_pTiePoints[20] = 0.0;

                // Transfome the pixel (pi_ImageWidth,pi_ImageHeight)
                pTransfoModel->ConvertDirect(po_pTiePoints[18], po_pTiePoints[19], &po_pTiePoints[21], &po_pTiePoints[22]);
                po_pTiePoints[23] = 0.0;

                // TiePoint Pixel(0,0,0)                     System(T(0),T(0),T(0)) ...
                // TiePoint Pixel(pi_ImageWidth,pi_ImageHeight,0)  System(T(pi_ImageWidth),T(pi_ImageHeight),T(0)) ...
                // TiePoint Pixel(pi_ImageWidth,0,0)            System(T(pi_ImageWidth),T(0),T(0)) ...
                // TiePoint Pixel(0,pi_ImageHeight,0)           System(T(0),T(pi_ImageHeight),T(0)) ...
                pio_MatSize = 0;
                pio_NbPixelScale = 0;
                pio_NbTiePoints = 24;    // Tie point is used
            }
            else
            {
                HASSERT(0);
            }
        }
    }
}
