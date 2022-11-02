//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#pragma once
#include <string.h>


/** @namespace BentleyApi::GeoCoordinates Geographic Coordinate System classes @see GeoCoordinate */
BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

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
          uint32_t  LongVal;
          double         DoubleVal;
          const char*    StringVal;
          }KeyValue;

    GeoKeyItem() { KeyID = 0; KeyDataType = LONG;  KeyValue.StringVal = 0; }
    ~GeoKeyItem()
        {
        if ((KeyDataType == ASCII) && (KeyValue.StringVal != 0))
            delete[] KeyValue.StringVal;
        }

    GeoKeyItem(const GeoKeyItem& pi_rObj) { CopyMemberData(pi_rObj); }

    GeoKeyItem& operator=(const GeoKeyItem& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            if ((KeyDataType == ASCII) && (KeyValue.StringVal != 0))
                delete[] KeyValue.StringVal;

            CopyMemberData(pi_rObj);
            }
        return *this;
        }

    private :
    void CopyMemberData(const GeoKeyItem& pi_rObj)
        {
        KeyID = pi_rObj.KeyID;
        KeyDataType = pi_rObj.KeyDataType;

        if (KeyDataType == ASCII)
            {
            size_t strLength = strlen(pi_rObj.KeyValue.StringVal) + 1;
            char* pStrVal = new char[strLength];

PUSH_DISABLE_DEPRECATION_WARNINGS
            strcpy(pStrVal, pi_rObj.KeyValue.StringVal); // strcpy_s() not available on linux
POP_DISABLE_DEPRECATION_WARNINGS

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

/*---------------------------------------------------------------------------------**//**
* Adds a GeoKeyItem to the list. The GeoKeyItem is copied before addition to the list.
* @param    key    IN  The key to add.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            AddKey (const GeoKeyItem& key)=0;



virtual                 ~IGeoTiffKeysList() {}
};

} // ends GeoCoordinates namespace
END_BENTLEY_NAMESPACE
