//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGeoTiffKeys.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCPGeoTiffKeys
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>

#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HCPException.h>
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


#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>



USING_NAMESPACE_IMAGEPP



//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCPGeoTiffKeys::HCPGeoTiffKeys()
    : HMDMetaDataContainer(HMDMetaDataContainer::HMD_GEOCODING_INFO)

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
    : HMDMetaDataContainer(HMDMetaDataContainer::HMD_GEOCODING_INFO)
    {
    m_GeoKeyList              = pi_rObj.m_GeoKeyList;
    m_GeoKeyListItr           = m_GeoKeyList.end();

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
HFCPtr<HMDMetaDataContainer> HCPGeoTiffKeys::Clone() const
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
bool HCPGeoTiffKeys::GetFirstKey(GeoKeyItem* po_Key)
    {
    HPRECONDITION(po_Key != 0);

    m_GeoKeyListItr = m_GeoKeyList.begin();

    if (m_GeoKeyListItr != m_GeoKeyList.end())
        {
        *po_Key =  (*m_GeoKeyListItr).second;
        return true;
        }
    return false;
    }
bool HCPGeoTiffKeys::GetNextKey(GeoKeyItem* po_Key)
    {
    m_GeoKeyListItr++;

    if (m_GeoKeyListItr != m_GeoKeyList.end())
        {
        *po_Key =  (*m_GeoKeyListItr).second;
        return true;
        }

    return false;
    }

void HCPGeoTiffKeys::AddKey (unsigned short pi_KeyID, unsigned int pi_value)
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
bool HCPGeoTiffKeys::HasKey(unsigned short pi_KeyID)
    {
    return m_GeoKeyList.find(pi_KeyID) != m_GeoKeyList.end();
    }

//-----------------------------------------------------------------------------
// public
// Return the number of GeoTiff keys
//-----------------------------------------------------------------------------
unsigned short HCPGeoTiffKeys::GetNbKeys()
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
HFCPtr<HGF2DTransfoModel> HCPGeoTiffKeys::GetTransfoModelToMeters(IRasterBaseGcsPtr pi_rpProjection)
    {
    double ToMeters = 1.0 / pi_rpProjection->GetUnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pUnitConvertion(new HGF2DStretch(HGF2DDisplacement(0.0, 0.0),
                                                               ToMeters,
                                                               ToMeters));
    return pUnitConvertion;
    }

//-----------------------------------------------------------------------------
// public
// GetTransfoModelFromMeters
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGeoTiffKeys::GetTransfoModelFromMeters(IRasterBaseGcsPtr pi_rpProjection)
    {
    double FromMeters = pi_rpProjection->GetUnitsFromMeters();

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
HCPGeoTiffKeys::GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&         pi_rpSrcRasterFile,
                                               uint32_t                            pi_PageNumber,
                                               const IRasterBaseGcsPtr              pi_rpDestCoordSys,
                                               const HFCPtr<HGF2DWorldCluster>&     pi_rpWorldCluster,
                                               const IRasterBaseGcsPtr              pi_rpOverwriteSourceCoordSys)
    {
    HPRECONDITION(pi_rpWorldCluster != 0);

    HFCPtr<HGF2DTransfoModel>     pTransfoModelForReprojection;
    IRasterBaseGcsPtr pSrcFileGeocoding;

    if (pi_rpOverwriteSourceCoordSys != 0)
        {
        pSrcFileGeocoding = pi_rpOverwriteSourceCoordSys;
        }
    else
        {
        pSrcFileGeocoding = pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber)->GetGeocoding();
        }

    // Georeference
    if ((pSrcFileGeocoding != 0) &&
        (pSrcFileGeocoding->IsValid()) &&
        (pi_rpDestCoordSys->IsValid()))
        {

        // Compute the image extent expressed in the src projection units
        HFCPtr<HGF2DCoordSys> pSrcWorldMeters(pi_rpWorldCluster->GetCoordSysReference(HGF2DWorld_HMRWORLD));

        HFCPtr<HGF2DCoordSys> pSrcUnitCoordSys = new HGF2DCoordSys(*GetTransfoModelToMeters(pSrcFileGeocoding), pSrcWorldMeters);
        HFCPtr<HGF2DCoordSys> pPhysicalCoordSys;

        HFCPtr<HRFPageDescriptor>       pPage(pi_rpSrcRasterFile->GetPageDescriptor(pi_PageNumber));
        HFCPtr<HRFResolutionDescriptor> pRes(pPage->GetResolutionDescriptor(0));

        if (pPage->HasTransfoModel())
            {
            pPhysicalCoordSys = new HGF2DCoordSys(*pPage->GetTransfoModel(),
                                                  pi_rpWorldCluster->
                                                  GetCoordSysReference(pi_rpSrcRasterFile->
                                                                       GetWorldIdentificator()));
            }
        else
            {
            pPhysicalCoordSys = new HGF2DCoordSys(HGF2DIdentity(),
                                                  pi_rpWorldCluster->
                                                  GetCoordSysReference(pi_rpSrcRasterFile->
                                                                       GetWorldIdentificator()));
            }

        // Create the reprojection transfo model (non adapted)
        HFCPtr<HGF2DTransfoModel> pDstToSrcTransfoModel(new HCPGCoordModel(pi_rpDestCoordSys, pSrcFileGeocoding));

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
        double Step(min(ImageExtent.GetHeight(), ImageExtent.GetWidth()) / NbPoints);

        // Compute the expected mean error
        double ImageCenter_x(ImageWidth / 2.0);
        double ImageCenter_y(ImageHeight / 2.0);

        HFCPtr<HVEShape> pPixelShape(new HVEShape(HVE2DRectangle(ImageCenter_x - 0.5, ImageCenter_y - 0.5, ImageCenter_x + 0.5, ImageCenter_y + 0.5, pPhysicalCoordSys)));
        pPixelShape->ChangeCoordSys(pDstCoordSys);
        HGF2DExtent PixelExtent(pPixelShape->GetExtent());
        double ExpectedMeanError(min(PixelExtent.GetWidth() * 0.5, PixelExtent.GetHeight() * 0.5));
        double ExpectedMaxError (min(PixelExtent.GetWidth(), PixelExtent.GetHeight()));

        pTransfoModelForReprojection = HCPGCoordUtility::GetInstance()->CreateGCoordAdaptedModel(pSrcFileGeocoding,
                                       pi_rpDestCoordSys,
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
        HFCPtr<HGF2DTransfoModel> pFromMeters(GetTransfoModelFromMeters(pSrcFileGeocoding));
        HFCPtr<HGF2DTransfoModel> pToMeters(GetTransfoModelToMeters(pi_rpDestCoordSys));

        pTransfoModelForReprojection = pLogiqueToHmr->ComposeInverseWithDirectOf(*pFromMeters)->
                                       ComposeInverseWithDirectOf(*pTransfoModelForReprojection)->
                                       ComposeInverseWithDirectOf(*pToMeters)->
                                       ComposeInverseWithInverseOf(*pLogiqueToHmr);
        }

    return pTransfoModelForReprojection;
    }


//-----------------------------------------------------------------------------
// public
// TranslateToMeter
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>
HCPGeoTiffKeys::TranslateToMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                  double                           pi_FactorModelToMeter,
                                  bool                             pi_ModelTypeGeographicConsiderDefaultUnit,
                                  bool                             pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                  bool*                            po_pDefaultUnitWasFound,
                                  IRasterBaseGcsPtr                pi_pBaseGcs)
    {
    HPRECONDITION(pi_pModel != 0);
    HFCPtr<HGF2DTransfoModel> pTransfo              = pi_pModel;


    double effectiveFactorModelToMeter = pi_FactorModelToMeter;

    if ((pi_pBaseGcs != NULL) && (pi_pBaseGcs->IsValid()))
        {
        effectiveFactorModelToMeter = 1.0 / pi_pBaseGcs->GetUnitsFromMeters();
        }


    // Apply to Matrix
    if (effectiveFactorModelToMeter != 1.0)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();

        pScaleModel->SetXScaling(effectiveFactorModelToMeter);
        pScaleModel->SetYScaling(effectiveFactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }

//-----------------------------------------------------------------------------
// public
// TranslateFromMeter
//-----------------------------------------------------------------------------

HFCPtr<HGF2DTransfoModel>
HCPGeoTiffKeys::TranslateFromMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                           bool                            pi_ModelTypeGeographicConsiderDefaultUnit,
                                           bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                           bool*                           po_pDefaultUnitWasFound,
                                           IRasterBaseGcsPtr               pi_pBaseGcs)
    {
    HPRECONDITION(pi_pModel != 0);

    double                   FactorModelToMeter = 1.0;
    if ((pi_pBaseGcs != NULL) && (pi_pBaseGcs->IsValid()))
        FactorModelToMeter = 1.0 / pi_pBaseGcs->GetUnitsFromMeters();

    HFCPtr<HGF2DTransfoModel> pTransfo = pi_pModel;


    // Apply inverse factor to Matrix
    if (FactorModelToMeter != 1.0)
        {
        HASSERT(FactorModelToMeter != 0.0);
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(1.0 / FactorModelToMeter);
        pScaleModel->SetYScaling(1.0 / FactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }
