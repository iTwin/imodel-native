/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Linkage.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ElementHandle.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct MSSymbologyLinkage
    {
    LinkageHeader               header;
    MSSymbologyLinkageData      data;
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct MSXMLLinkage
    {
    LinkageHeader           header;
    MSXMLLinkageData        data;
    };


#if !defined (type_resource_generator)
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct MSDoubleArrayLinkageData
    {
    UInt16                  linkageKey;                     // id identifying type of double array linkage
    UInt16                  padding;
    UInt32                  linkageArrayLength;             // size of double array linkage data
    double                  linkageArray[1];
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct MSByteArrayLinkageData
    {
    UInt16                  linkageKey;                     // id identifying type of byte array linkage 
    UInt16                  padding;
    UInt32                  linkageArrayLength;             // size of byte array linkage data 
    Byte                    linkageArray[8];                // this is declared to have struct align to 8 bytes 
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct MSByteArrayLinkage
    {
    LinkageHeader               header;
    MSByteArrayLinkageData      data;
    };

#endif

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ElementLinkageUtil
{
public:

DGNPLATFORM_EXPORT static void InitLinkageHeader (LinkageHeader& linkHdr, UInt16 primaryID, size_t rawDataBytes);

DGNPLATFORM_EXPORT static BentleyStatus AddLinkage (DgnElementR elm, UInt16 primaryID, DataExternalizer& writer);
DGNPLATFORM_EXPORT static BentleyStatus AddLinkage (EditElementHandleR eeh, UInt16 primaryID, DataExternalizer& writer);

DGNPLATFORM_EXPORT static double const* GetDoubleArrayDataCP (ConstElementLinkageIterator li, UInt16& linkageKey, UInt32& numEntries);
DGNPLATFORM_EXPORT static BentleyStatus AppendDoubleArrayData (EditElementHandleR eeh, UInt16 linkageKey, UInt32 numEntries, double const* doubleData);
DGNPLATFORM_EXPORT static BentleyStatus DeleteDoubleArrayData (EditElementHandleR eeh, UInt16 linkageKey, size_t index);

DGNPLATFORM_EXPORT static byte const* GetByteArrayDataCP (ConstElementLinkageIterator li, UInt16& linkageKey, UInt32& numEntries);
DGNPLATFORM_EXPORT static BentleyStatus AppendByteArrayData (EditElementHandleR eeh, UInt16 linkageKey, UInt32 numEntries, byte const* byteData);

DGNPLATFORM_EXPORT static BentleyStatus GetClippingDepths (ElementHandleCR eh, double& front, double& back, ClipMask& clipMask);
DGNPLATFORM_EXPORT static void SetClippingDepths (EditElementHandleR eh, double front, double back, ClipMask clipMask);
}; 

//=======================================================================================
//! Helper class for working with the XData element linkage.
//! @bsiclass
//=======================================================================================
struct XDataHelper
{
public:

enum DWGXDataType
    {
    DWGXDATA_String           = 1000, //! char*
    DWGXDATA_Application_Name = 1001, //! UInt64 (NOTE: Application id stored with byte order reversed)
    DWGXDATA_ControlString    = 1002, //! char*
    DWGXDATA_LayerName        = 1003, //! char*
    DWGXDATA_BinaryData       = 1004, //! UInt8*
    DWGXDATA_DatabaseHandle   = 1005, //! UInt64 (NOTE: Handle id stored with byte order reversed)
    DWGXDATA_Point            = 1010, //! DPoint3d
    DWGXDATA_Space_Point      = 1011, //! DPoint3d
    DWGXDATA_Disp_Point       = 1012, //! DVec3d
    DWGXDATA_Dir_Point        = 1013, //! DVec3d
    DWGXDATA_Real             = 1040, //! double
    DWGXDATA_Dist             = 1041, //! double
    DWGXDATA_Scale            = 1042, //! double
    DWGXDATA_Integer          = 1070, //! Int16
    DWGXDATA_Long_Integer     = 1071, //! Int32
    };

typedef UInt8 const* UInt8CP;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct Iterator : std::iterator<std::forward_iterator_tag, UInt8CP const>
    {
    private:

    ConstElementLinkageIterator m_li;
    size_t                      m_dataOffset;
    UInt8CP                     m_data;
    ElementId                   m_appId;
    bool                        m_inGroup;

    DGNPLATFORM_EXPORT void ToNext ();
    Iterator (ConstElementLinkageIteratorCR li, ElementId appId) {m_dataOffset = 0; m_data = NULL; m_li = li; m_appId = appId; m_inGroup = false; ToNext ();}
    Iterator () {m_dataOffset = 0; m_data = NULL;}

    friend struct XDataHelper;

    public:

    Iterator&       operator++() {ToNext (); return *this;}
    bool            operator==(Iterator const& rhs) const {return m_dataOffset == rhs.m_dataOffset;}
    bool            operator!=(Iterator const& rhs) const {return !(*this == rhs);}
    UInt8CP const&  operator*() const {return m_data;}

    //! Query group code at current iterator position
    DGNPLATFORM_EXPORT static DWGXDataType GetGroupCode (UInt8 const* xDataHdr);

    //! Query group data size at current iterator position
    DGNPLATFORM_EXPORT static size_t GetDataSize (UInt8 const* xDataHdr);

    //! Query group data pointer at current iterator position
    DGNPLATFORM_EXPORT static UInt8 const* GetDataCP (UInt8 const* xDataHdr);
    };

//=======================================================================================
//! The collection of xdata on an element.
//! \code
//! for each (UInt8* xDataHdr in XDataHelper::Collection (eh))
//!     {
//!     switch (XDataHelper::Iterator::GetGroupCode (xDataHdr))
//!         {
//!         case XDataHelper::DWGXDATA_String:
//!             {
//!             (CharCP) XDataHelper::Iterator::GetDataCP (xDataHdr);
//!             break;
//!             }
//!         ...
//!         }
//!     }
//! \endcode
//! @bsiclass
//=======================================================================================
struct Collection
    {
    ConstElementLinkageIterator m_li;
    ElementId                   m_appId;

    public:

    typedef Iterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible
    
    //! Supply appId to iterate only the data of the specified application instead of all XData.
    Collection (ElementHandleCR eh, ElementId appId = ElementId()) {m_li = eh.BeginElementLinkages (LINKAGEID_XData); m_appId = appId;}
    Collection (ConstElementLinkageIteratorR li, ElementId appId = ElementId()) {m_li = li; m_appId = appId;}

    const_iterator begin () const {return const_iterator (m_li, m_appId);}
    const_iterator end   () const {return const_iterator ();}
    };

private:

static bool CollectOtherData (DataExternalizer&, ConstElementLinkageIteratorR, UInt64);

public:

//! Add handle data types DWGXDATA_Application_Name, DWGXDATA_DatabaseHandle
//! @note Stored with the byte order reversed, keep this in mind if comparing ids using a iterator.
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, ElementId);

//! Add string data types DWGXDATA_String, DWGXDATA_ControlString, DWGXDATA_LayerName
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, CharCP); // WIP_CHAR_OK - Persisted as Char; don't mislead with a WChar API.

//! Add point/vector data types DWGXDATA_Point, DWGXDATA_Space_Point, DWGXDATA_Disp_Point, DWGXDATA_Dir_Point
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, DPoint3dCR);

//! Add double data types DWGXDATA_Real, DWGXDATA_Dist, DWGXDATA_Scale
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, double);

//! Add integer data type DWGXDATA_Integer
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, Int16);

//! Add long integer data type DWGXDATA_Long_Integer
DGNPLATFORM_EXPORT static BentleyStatus Add (DataExternalizer&, DWGXDataType, Int32);

//! Add binary or other unspecified data types DWGXDATA_BinaryData
DGNPLATFORM_EXPORT static void Add (DataExternalizer&, DWGXDataType, UInt8 const*, size_t);

//! Append or replace application XData, new data MUST start with DWGXDATA_Application_Name.
//! \code
//! DataExternalizer writer;
//! XDataHelper::Add (writer, XDataHelper::DWGXDATA_Application_Name, appId); // registered app id
//! XDataHelper::Add (writer, XDataHelper::DWGXDATA_String, "Testing");
//! XDataHelper::Add (writer, XDataHelper::DWGXDATA_Real, 1.0);
//! XDataHelper::Append (eeh, writer);
//! \endcode
DGNPLATFORM_EXPORT static BentleyStatus Append (EditElementHandleR, DataExternalizer&);

//! Remove XData from element. If applicationId is 0 all XData is removed instead.
DGNPLATFORM_EXPORT static BentleyStatus Delete (EditElementHandleR, ElementId appId = ElementId());

}; // XDataHelper

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE

//! @description  The mdlLinkage_getClipElementIds function is used to extract the clipping
//! ,mask element IDs from an reference file or view element.
//! files or shared cells.
//! @param        uniqueIdsP OUT is a buffer to return the unique clipping element IDs.
//! @param        maxClipElms IN is the max number of IDs to return.
//! @param        elmP IN is the element to extract IDs from.
DGNPLATFORM_EXPORT int mdlLinkage_getClipMaskElementIds (DgnPlatform::ElementId *uniqueIdsP, int maxClipElms, DgnElementCP elmP);
//DGNPLATFORM_EXPORT int mdlLinkage_setElementIds (MSElementDescrP, DgnPlatform::ElementId const* uniqueIdsP, UInt16 numUniqueIds, UShort applicationID, UShort applicationValue, int copyOption = DEPENDENCY_ON_COPY_RemapRootsWithinSelection);
DGNPLATFORM_EXPORT int mdlLinkage_getElementIds (DgnPlatform::ElementId *uniqueIdsP, int maxUniqueIds, DgnElementCP elmP, UShort applicationID, UShort applicationValue);
DGNPLATFORM_EXPORT int linkage_appendDoubleArrayLinkage (DgnElementP pElementIn, UShort linkageKey, UInt32 elementCount, const double *pLinkageDoubleArrayIn);
DGNPLATFORM_EXPORT int linkage_appendByteArrayLinkage (DgnElementP pElementIn, UShort linkageKey, UInt32 elementCount, const byte *pLinkageByteArrayIn);
DGNPLATFORM_EXPORT StatusInt linkage_setSymbologyLinkage (EditElementHandleR eh, UInt16 linkageKeyIn, bool overrideStyle, bool overrideWeight, bool overrideColor, bool overrideLevel, Int32 style, UInt32 weight, UInt32 color, UInt32 level);
DGNPLATFORM_EXPORT StatusInt linkage_getSymbologyLinkage (bool *pOverrideStyle, bool *pOverrideWeight, bool *pOverrideColor, bool *pOverrideLevel, Int32 *pStyle, UInt32 *pWeight, UInt32 *pColor, UInt32 *pLevel, int linkageKeyIn, ElementHandleCR eh);
DGNPLATFORM_EXPORT StatusInt linkage_deleteSymbologyLinkage (EditElementHandleR eh, int linkageKeyIn);

END_BENTLEY_API_NAMESPACE
