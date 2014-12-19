//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGeoTiffKeys.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCPGeoTiffKeys
//-----------------------------------------------------------------------------
#pragma once

#include "interface/IGeoTiffKeysList.h"

#include "HGF2DCoordSys.h"
#include "HGF2DExtent.h"
#include "HPMAttribute.h"
#include "HTIFFTag.h"
#include "HGF2DWorldCluster.h"
#include "HMDMetaDataContainer.h"

class HRFRasterFile;

#include "HRFRasterFile.h"

class HCPGeoTiffKeys : public HMDMetaDataContainer, public IGeoTiffKeysList
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

    HDECLARE_CLASS_ID(1380, HMDMetaDataContainer)

    _HDLLg HCPGeoTiffKeys();

//    _HDLLg HCPGeoTiffKeys(UInt32 pi_ProjectedCSType);

//    _HDLLg HCPGeoTiffKeys(const WString&   pi_rWellKnownText,
//                          int              pi_WktFlavor);
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
//    _HDLLg HCPGeoTiffKeys(const WString&   pi_rErmProjection,
//                          const WString&   pi_rErmDatum,
//                          const WString&   pi_rErmUnits);

//    _HDLLg HCPGeoTiffKeys(const WString&   pi_rCSKeyName);

//    _HDLLg HCPGeoTiffKeys(IRasterBaseGcsPtr& pi_rpBaseGeoCoord);

    // Copy constructor
    _HDLLg HCPGeoTiffKeys(const HCPGeoTiffKeys& pi_rObj);

    _HDLLg virtual         ~HCPGeoTiffKeys();

    virtual HFCPtr<HMDMetaDataContainer> Clone() const;


    // All previous GeoKeys are removed

    _HDLLg ErrorCode            GetErrorCode();
    void                        SetErrorCode(ErrorCode pi_ErrorCode);
    KeysValidStatus             ValidateGeoTIFFKey();
    bool                        HasValidGeoTIFFKeysList () 
        {
        return m_HasValidGeoTIFFKeysList;
        }



    _HDLLg bool             GetValue (unsigned short pi_Key, uint32_t*  po_pVal) const;
    _HDLLg bool             GetValue (unsigned short pi_Key, double* po_pVal) const;
    _HDLLg bool             GetValue (unsigned short pi_Key, WString* po_pVal) const;

    _HDLLg bool             SetValue (unsigned short pi_Key, uint32_t pi_Val);
    _HDLLg bool             SetValue (unsigned short pi_Key, double pi_Val);
    _HDLLg bool             SetValue (unsigned short pi_Key, WString& pi_Val);

    // From IGeoTiffKeysList
    _HDLLg virtual bool     GetFirstKey(GeoKeyItem* po_Key);
    _HDLLg virtual bool     GetNextKey(GeoKeyItem* po_Key);

    _HDLLg virtual void     AddKey (unsigned short pi_KeyID, unsigned int pi_value);
    _HDLLg virtual void     AddKey (unsigned short pi_KeyID, double pi_value);
    _HDLLg virtual void     AddKey (unsigned short pi_KeyID, const std::string& pi_value);

    // Utility methods
    _HDLLg bool             HasKey (unsigned short pi_KeyID);
    unsigned short          GetNbKeys();




    //Static methods
   static unsigned short DecodeGeoKeyIDFromString(const WString& pi_rGeoTagLabel);


    _HDLLg static HFCPtr<HGF2DTransfoModel>GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&         pi_rpSrcRasterFile,
                                                                          uint32_t                            pi_PageNumber,
                                                                          const IRasterBaseGcsPtr              pi_rpDestCoordSys,
                                                                          const HFCPtr<HGF2DWorldCluster>&     pi_rpWorldCluster,
                                                                          const IRasterBaseGcsPtr              pi_rpOverwriteSourceCoordSys);

     static HFCPtr<HGF2DTransfoModel> GetTransfoModelToMeters(IRasterBaseGcsPtr pi_rpProjection);

     static HFCPtr<HGF2DTransfoModel> GetTransfoModelFromMeters(IRasterBaseGcsPtr pi_rpProjection);


     static HFCPtr<HGF2DTransfoModel> GetTransfoModelForReprojection(const HFCPtr<HRFRasterFile>&         pi_rpSrcRasterFile,
                                                                     uint32_t                             pi_PageNumber,
                                                                     const HFCPtr<HCPGeoTiffKeys>& pi_rpDestCoordSys,
                                                                     const HFCPtr<HGF2DWorldCluster>&     pi_rpWorldCluster,
                                                                     const HFCPtr<HCPGeoTiffKeys>& pi_rpOverwriteSourceCoordSys = HFCPtr<HCPGeoTiffKeys>());

    static _HDLLg HFCPtr<HGF2DTransfoModel>
    TranslateToMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                      double                          pi_FactorModelToMeter=1.0,
                      bool                            pi_ModelTypeGeographicConsiderDefaultUnit=true,
                      bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation=false,
                      bool*                           po_DefaultUnitWasFound=0,
                      IRasterBaseGcsPtr               po_pGeocoding = NULL);
    static _HDLLg HFCPtr<HGF2DTransfoModel>
    TranslateFromMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                        bool                            pi_ModelTypeGeographicConsiderDefaultUnit=true,
                        bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation=false,
                        bool*                           po_DefaultUnitWasFound=0,
                        IRasterBaseGcsPtr               po_pGeocoding = NULL);



protected:

//    static  bool FindUnits(HRFGeoTiffTable*                             pi_pGeoTables,
//                            UInt32                                       pi_ProjectionKey,
//                            UShort                                      pi_UnitsSource,
//                            double&                                     po_rFactorToMeter,
//                            HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord* po_pUnitsRecord = 0);

private:

//    bool            FillGeoKeyItem (GeoKeyItem* po_Key,
//                                    HPMAttributeSet::HPMASiterator& pi_Itr);

//    void            CopyGeoTiffKeyOnly (const HPMAttributeSet& pi_TagList);

//    int             CreateBaseGeoCoord(bool pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation = false);

//    void            FinalizeInit(Int32                 pi_BaseGCSInitStatus,
//                                 IRasterBaseGcsPtr&    pi_rpBaseGeoCoord,
//                                 bool                 pi_InitFromWKTorBaseGCS = false);

    // Not implemented
    HCPGeoTiffKeys&   operator=(const HCPGeoTiffKeys& pi_rObj);

    // Members

    // The key is the GeoKey.
    typedef map<unsigned short, GeoKeyItem> GeoKeyList;
    GeoKeyList                              m_GeoKeyList;
    GeoKeyList::iterator                    m_GeoKeyListItr;

    bool                                    m_HasValidGeoTIFFKeysList;
    ErrorCode                               m_ErrorCode;

    };
