//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/interface/IGeoTiffKeysList.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

//BEGIN_IMAGEPP_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IGeoTiffKeysList
    {
// These values are compatible with the Tiff specification.
    enum DataType
        {
        ASCII   = 2,        // 8-bit bytes w/ last byte null
        LONG    = 3,        // 16-bit unsigned integer
        DOUBLE  = 12,       // 64-bit IEEE floating point
        };

    struct GeoKeyItem
        {
    public :

        unsigned short  KeyID;
        DataType        KeyDataType;
        union {
            unsigned long  LongVal;
            double         DoubleVal;
            const char*    StringVal;
            } KeyValue;

        GeoKeyItem() {
            KeyValue.StringVal = 0;
            }
        ~GeoKeyItem()
            {
            if ((KeyDataType == ASCII) && (KeyValue.StringVal != 0))
                delete[] KeyValue.StringVal;
            }

        GeoKeyItem(const GeoKeyItem& pi_rObj) {
            CopyMemberData(pi_rObj);
            }

        GeoKeyItem& operator=(const GeoKeyItem& pi_rObj)
            {
            if (this != &pi_rObj)
                CopyMemberData(pi_rObj);
            return *this;
            }

    private :
        void CopyMemberData(const GeoKeyItem& pi_rObj)
            {
            KeyID = pi_rObj.KeyID;
            KeyDataType = pi_rObj.KeyDataType;

            if (KeyDataType == ASCII)
                {
                size_t StrLength = strlen(pi_rObj.KeyValue.StringVal) + 1;
                char* pStrVal = new char[StrLength];

                BeStringUtilities::Strncpy(pStrVal, StrLength, pi_rObj.KeyValue.StringVal, StrLength-1);

                KeyValue.StringVal = pStrVal;
                }
            else
                {
                KeyValue = pi_rObj.KeyValue;
                }
            }
        };

    virtual bool            GetFirstKey(GeoKeyItem* po_Key) const=0;
    virtual bool            GetNextKey(GeoKeyItem* po_Key) const=0;

    virtual void            AddKey (unsigned short pi_KeyID, uint32_t pi_value)=0;
    virtual void            AddKey (unsigned short pi_KeyID, double pi_value)=0;
    virtual void            AddKey (unsigned short pi_KeyID, const std::string& pi_value)=0;

    virtual size_t          EraseKey(unsigned short pi_KeyID)=0;
    };


//END_IMAGEPP_NAMESPACE