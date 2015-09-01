//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFGeoKey.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTIFFGeoKey
//-----------------------------------------------------------------------------

#pragma once

#include "HTagDefinition.h"


BEGIN_IMAGEPP_NAMESPACE
class HTIFFGeoKey
    {
public:
    typedef unsigned short GeoKeyID;

    // Primary methods

    HTIFFGeoKey();
    HTIFFGeoKey(const unsigned short* pi_pGeoKeyDirectory, uint32_t pi_GeoKeyCount,
                const double* pi_pGeoDoubleParams, uint32_t pi_GeoDoubleCount,
                const char* pi_pGeoASCIIParams);
    virtual         ~HTIFFGeoKey();

    void            Reset();

    bool           GetValue  (GeoKeyID pi_Key, unsigned short* po_pVal) const;
    bool           GetValues (GeoKeyID pi_Key, double* po_pVal, uint32_t pi_Index=0, uint32_t pi_Count=1) const;
    bool           GetValues (GeoKeyID pi_Key, char* po_pVal, uint32_t pi_Index=0) const;
    bool           GetValues (GeoKeyID pi_Key, char** po_pVal, uint32_t pi_Index=0) const;

    bool           SetValue  (GeoKeyID pi_Key, unsigned short pi_Val);
    bool           SetValue  (GeoKeyID pi_Key, double pi_Val, uint32_t pi_Index=0);
    bool           SetValues (GeoKeyID pi_Key, const double* pi_pVal, uint32_t pi_Index=0, uint32_t pi_Count=1);
    bool           SetValues (GeoKeyID pi_Key, const char* pi_pVal, uint32_t pi_Index=0);


    // Remark! This method reset the Dirty flag...
    // and you must delete param...
    void            GetGeoParams(unsigned short** po_ppGeoKeyDirectory, uint32_t* po_pGeoKeyCount,
                                 double** po_ppGeoDoubleParams, uint32_t* po_pGeoDoubleCount,
                                 char** po_ppGeoASCIIParams);

    HTagInfo::DataType
    GetDataType (GeoKeyID pi_Key) const;
    uint32_t        GetCount    (GeoKeyID pi_Key) const;

    bool           IsValid     (HTIFFError**  po_ppError) const;
    bool           IsDirty     () {
        return m_KeyIsDirty;
        };


    // STATIC CLASS METHOD
    static HTagInfo::DataType sGetExpectedDataType(GeoKeyID pi_Key);
protected:

private:

    struct GeoKeyInfo
        {
        unsigned short                 Count;
        HTagInfo::DataType              Type;

        // If Count==1 then, the data is store directly in the
        // struct.
        union
            {
            unsigned short DataShort;
            double DataDouble;
            Byte*  pData;
            };
        };

    struct GeoKeyDefinition
        {
        GeoKeyID                        Key;
        HTagInfo::DataType              Type;
        };

    static const GeoKeyDefinition sGeoKeyInfo[];
    static const uint32_t         sNumberOfDefs;


    // The key is the GeoKey.
    typedef map<GeoKeyID, GeoKeyInfo> GeoKeyList;


    // Members

    GeoKeyList      m_KeyList;
    bool           m_KeyIsDirty;

    HTIFFError*     m_pError;           // Error message

    // Methods

    void            InsertNewKey    (GeoKeyID pi_Key, HTagInfo::DataType pi_Type,
                                     const Byte* pi_pVal, uint32_t pi_Index, uint32_t pi_Count);


    // Not implemented
    HTIFFGeoKey   (const HTIFFGeoKey& pi_rObj);
    HTIFFGeoKey&          operator=(const HTIFFGeoKey& pi_rObj);

    };
END_IMAGEPP_NAMESPACE
