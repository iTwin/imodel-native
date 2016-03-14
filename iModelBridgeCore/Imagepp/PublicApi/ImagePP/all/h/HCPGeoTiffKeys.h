//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGeoTiffKeys.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCPGeoTiffKeys
//-----------------------------------------------------------------------------
#pragma once

#include <GeoCoord/IGeoTiffKeysList.h>

#include "HGF2DCoordSys.h"
#include "HGF2DExtent.h"
#include "HPMAttribute.h"
#include "HTIFFTag.h"
#include "HGF2DWorldCluster.h"

BEGIN_IMAGEPP_NAMESPACE

class HRFRasterFile;

class HCPGeoTiffKeys : public HFCShareableObject<HCPGeoTiffKeys>, public IGeoTiffKeysList
    {


public:

    enum ErrorCode
        {
        GEOCODING_NO_ERROR = 0,
        GEOCODING_ERROR,
        GEOCODING_PDF_MULTI_MODELS //PDF with more than one model on a page
        //(i.e. : two georeferenced areas).
        };

    enum KeysValidStatus
        {
        GEOKEYS_VALID = 0,
        GEOKEYS_NO_CS,          //Impossible to deduce a PCS or GCS
        GEOKEYS_PCS_WITH_NO_GCS //Possible to deduce a PCS but not a GCS
        };

    HDECLARE_BASECLASS_ID(HCPGeoTiffKeysId)

    IMAGEPP_EXPORT HCPGeoTiffKeys();


    /** -----------------------------------------------------------------------------

        Constructor from ERM strings. This constructor takes the definition of the
        projection, datum and units as used by ER Mapper file format and
        attempts to build a set of geokeys if the projection CS type is a known
        EPSG value. If it is not then the strings are converted into a WKT (internally
        using OGR) the remainder of the procedure and behavior is exactly the same
        as the WKT based constructor defined above.

        @param pi_rErmProjection IN The ER Mapper projection string. This string
                             should be one of the normal ER Mapper projection including
                             RAW or LOCAL (Geographic).

        @param pi_rErmProjection IN The ER Mapper datum string. This string
                             should be one of the normal ER Mapper datum

        @param pi_rErmUnit IN The ER Mapper unit. This UNIT string must be one of
                             "METERS"
                             "FEET" or "U.S. SURVEY FOOT" for US Survey foot
                             "IFEET" for international foot
                             "DEGREES"
                             "IMPERIAL YARD" for the British Imperial Yard 1895 definition

        -----------------------------------------------------------------------------
    */


    // Copy constructor
    IMAGEPP_EXPORT HCPGeoTiffKeys(const HCPGeoTiffKeys& pi_rObj);

    IMAGEPP_EXPORT virtual         ~HCPGeoTiffKeys();

    virtual HFCPtr<HCPGeoTiffKeys> Clone() const;


    // All previous GeoKeys are removed

    IMAGEPP_EXPORT ErrorCode            GetErrorCode();
    void                        SetErrorCode(ErrorCode pi_ErrorCode);
    KeysValidStatus             ValidateGeoTIFFKey();
    bool                        HasValidGeoTIFFKeysList () const
        {
        return m_HasValidGeoTIFFKeysList;
        }



    IMAGEPP_EXPORT bool             GetValue (uint16_t pi_Key, uint32_t*  po_pVal) const;
    IMAGEPP_EXPORT bool             GetValue (uint16_t pi_Key, double* po_pVal) const;
    IMAGEPP_EXPORT bool             GetValue (uint16_t pi_Key, WString* po_pVal) const;

    IMAGEPP_EXPORT bool             SetValue (uint16_t pi_Key, uint32_t pi_Val);
    IMAGEPP_EXPORT bool             SetValue (uint16_t pi_Key, double pi_Val);
    IMAGEPP_EXPORT bool             SetValue (uint16_t pi_Key, WString& pi_Val);

    // From IGeoTiffKeysList
    IMAGEPP_EXPORT virtual bool     GetFirstKey(GeoKeyItem* po_Key) const override;
    IMAGEPP_EXPORT virtual bool     GetNextKey(GeoKeyItem* po_Key) const override;

    IMAGEPP_EXPORT virtual void     AddKey (uint16_t pi_KeyID, uint32_t pi_value) override;
    IMAGEPP_EXPORT virtual void     AddKey (uint16_t pi_KeyID, double pi_value) override;
    IMAGEPP_EXPORT virtual void     AddKey (uint16_t pi_KeyID, const std::string& pi_value) override;
    IMAGEPP_EXPORT virtual void     AddKey (const GeoKeyItem& key) override;

    IMAGEPP_EXPORT size_t           EraseKey(uint16_t pi_KeyID);

    // Utility methods
    IMAGEPP_EXPORT bool             HasKey (uint16_t pi_KeyID) const;
    IMAGEPP_EXPORT uint16_t   GetNbKeys() const;




    //Static methods
    IMAGEPP_EXPORT static uint16_t DecodeGeoKeyIDFromString(const WString& pi_rGeoTagLabel);

    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel> GetTransfoModelForReprojection(const HFCPtr<HGF2DCoordSys>&        pi_rpRasterCoordSys,
                                                                                  const HGF2DExtent&                   pi_rRasterExtent,
                                                                                  const HGF2DExtent&                   pi_rMinimumRasterPixelRange,
                                                                                  GeoCoordinates::BaseGCSCR            pi_rpSourceCoordSys,
                                                                                  GeoCoordinates::BaseGCSCR            pi_rpDestCoordSys,
                                                                                  const HFCPtr<HGF2DWorldCluster>&     pi_rpWorldCluster);

    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel>GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&  pi_rpSrcRasterFile,
                                                                          uint32_t                              pi_PageNumber,
                                                                          GeoCoordinates::BaseGCSCP             pi_rpDestCoordSys,
                                                                          const HFCPtr<HGF2DWorldCluster>&      pi_rpWorldCluster,
                                                                          GeoCoordinates::BaseGCSCP             pi_rpOverwriteSourceCoordSys);

    static HFCPtr<HGF2DTransfoModel> GetTransfoModelToMeters(GeoCoordinates::BaseGCSCR pi_rpProjection);

    static HFCPtr<HGF2DTransfoModel> GetTransfoModelFromMeters(GeoCoordinates::BaseGCSCR pi_rpProjection);


     static HFCPtr<HGF2DTransfoModel> GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&         pi_rpSrcRasterFile,
                                                                     uint32_t                             pi_PageNumber,
                                                                     const HFCPtr<HCPGeoTiffKeys>& pi_rpDestCoordSys,
                                                                     const HFCPtr<HGF2DWorldCluster>&     pi_rpWorldCluster,
                                                                     const HFCPtr<HCPGeoTiffKeys>& pi_rpOverwriteSourceCoordSys = HFCPtr<HCPGeoTiffKeys>());

     static HFCPtr<HGF2DTransfoModel> CreateTransfoModelFromGeoTiff(GeoCoordinates::BaseGCSCP pi_pGeocoding,
                                                                    bool                      pi_PixelIsArea,
                                                                    double                    pi_FactorModelToMeter,
                                                                    double*                   pi_pMatrix,    // 4 x 4
                                                                    uint32_t                  pi_MatSize,
                                                                    double*                   pi_pPixelScale=nullptr,
                                                                    uint32_t                  pi_NbPixelScale=0, 
                                                                    double*                   pi_pTiePoints=nullptr,
                                                                    uint32_t                  pi_NbTiePoints=0,
                                                                    bool*                     po_DefaultUnitWasFound=nullptr);

     static void WriteTransfoModelFromGeoTiff(GeoCoordinates::BaseGCSCP           pi_pGeocoding,
                                              bool                                pi_PixelIsArea,
                                              const HFCPtr<HGF2DTransfoModel>&    pi_pModel,
                                              uint64_t                            pi_ImageWidth,
                                              uint64_t                            pi_ImageHeight,
                                              bool                                pi_StoreUsingMatrix,
                                              double*                             po_pMatrix,           // 4 x 4  array[16]
                                              uint32_t&                           pio_MatSize,
                                              double*                             po_pPixelScale,       // array[3]
                                              uint32_t&                           pio_NbPixelScale,
                                              double*                             po_pTiePoints,        // array[24]
                                              uint32_t&                           pio_NbTiePoints,
                                              bool                                pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation);


protected:



private:

    // Not implemented
    HCPGeoTiffKeys&   operator=(const HCPGeoTiffKeys& pi_rObj);

    // Members

    // The key is the GeoKey.
    typedef map<uint16_t, GeoKeyItem> GeoKeyList;
    GeoKeyList                              m_GeoKeyList;
    mutable GeoKeyList::const_iterator      m_GeoKeyListItr;

    bool                                    m_HasValidGeoTIFFKeysList;
    ErrorCode                               m_ErrorCode;

    };

END_IMAGEPP_NAMESPACE