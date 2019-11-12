//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once
/*__PUBLISH_SECTION_START__*/

/** @namespace BentleyApi::GeoCoordinates Geographic Coordinate System classes @see GeoCoordinate */
BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IGeoTiffKeysList
{
/*__PUBLISH_SECTION_END__*/
// These values are compatible with the Tiff specification.
enum DataType
    {
    ASCII   = 2,        // 8-bit bytes w/ last byte null
    LONG    = 3,        // 16-bit unsigned integer
    DOUBLE  = 12,       // 64-bit IEEE floating point
    };

/*__PUBLISH_SECTION_START__*/
struct GeoKeyItem
    {
/*__PUBLISH_SECTION_END__*/
    public :

    unsigned short  KeyID;
    DataType        KeyDataType;
    union {
          uint32_t  LongVal;
          double         DoubleVal;
          const char*    StringVal;
          }KeyValue;

    GeoKeyItem() { KeyValue.StringVal = 0; }
    ~GeoKeyItem()
        {
        if ((KeyDataType == ASCII) && (KeyValue.StringVal != 0))
            delete[] KeyValue.StringVal;
        }

    GeoKeyItem(const GeoKeyItem& pi_rObj) { CopyMemberData(pi_rObj); }

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

            strcpy (pStrVal, pi_rObj.KeyValue.StringVal);

            KeyValue.StringVal = pStrVal;
            }
        else
            {
            KeyValue = pi_rObj.KeyValue;
            }
        }
/*__PUBLISH_SECTION_START__*/
    };
/*__PUBLISH_SECTION_END__*/

    virtual bool            GetFirstKey(GeoKeyItem* po_Key) const=0;
    virtual bool            GetNextKey(GeoKeyItem* po_Key) const=0;

virtual void            AddKey (unsigned short pi_KeyID, uint32_t pi_value)=0;
virtual void            AddKey (unsigned short pi_KeyID, double pi_value)=0;
virtual void            AddKey (unsigned short pi_KeyID, const std::string& pi_value)=0;

/*---------------------------------------------------------------------------------**//**
* Adds a GeoKeyItem to the list. The GeoKeyItem is copied before addition to the list.
* @param    key    IN  The key to add.
* @bsimethod                                                    Alain.Robert   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            AddKey (const GeoKeyItem& key)=0;

/*__PUBLISH_SECTION_START__*/
};

} // ends GeoCoordinates namespace
END_BENTLEY_NAMESPACE
