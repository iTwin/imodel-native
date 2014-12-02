/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XAttributeChange.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "DgnCore.h"

DGNPLATFORM_TYPEDEFS (XAttributeChange)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

class XAttributeChangeDataHolder;

//__PUBLISH_SECTION_END__

//#define XATTRIBUTE_CHANGE_TRACK_LEAKS

//__PUBLISH_SECTION_START__
//=======================================================================================
//! A scheduled change to an XAttribute stored on an EditElementHandle.
//! @bsiclass
//=======================================================================================
struct XAttributeChange
{
    enum ChangeType
    {
                CHANGETYPE_Unknown=0,
                CHANGETYPE_Write,                           //!< The referenced XAttribute is scheduled to be added or replaced on the element.
                CHANGETYPE_Delete,                          //!< The referenced XAttribute is scheduled to be deleted from the element.
                CHANGETYPE_HistoricalVersion,               //!< A historical version of an XAttribute is being reported. If the element is rewritten, this behaves the same as CHANGETYPE_Write .
                CHANGETYPE_HistoricalVersionDidNotExist     //!< A historical version of an XAttribute is being reported: The XAttribute did not exist as of the specified revision. If the element is rewritten, this behaves the same as CHANGETYPE_Delete .
    };

private:
    XAttributeHandlerId         m_handlerId;
    UInt32                      m_attrId;
    UInt32                      m_reserved_;
    XAttributeChangeDataHolder* m_data;
    ChangeType                  m_changeType;

public:
    XAttributeChange() {m_data = NULL;}

    //! Initialize a XAttributeChange object.
    //! Adds a reference to the specified XAttributeChangeDataHolder object.
    DGNPLATFORM_EXPORT XAttributeChange (XAttributeHandlerIdCR, UInt32 xAttrId, XAttributeChangeDataHolder&, ChangeType);

    //! Initialize a copy of an XAttributeChange.
    //! @remarks This copy constructor does not make a copy of the data referenced by the source object.
    //!          Instead, this is set up to reference the same XAttributeDataHolder object that the input object references.
    //!          The reference count on the shared XAttributeDataHolder object is incremented.
    DGNPLATFORM_EXPORT XAttributeChange (XAttributeChangeCR);

    //! Releases reference to XAttributeChangeDataHolder.
    DGNPLATFORM_EXPORT ~XAttributeChange ();

    DGNPLATFORM_EXPORT UInt64 AttributeUInt64Key () const;

    //! Get the ID of the XAttribute's IXAttributeHandler.
    XAttributeHandlerId GetHandlerId () const {return m_handlerId;}
    //! Get the ID of the XAttribute within the element.
    UInt32 GetId () const {return m_attrId;}
    //! Get the number of bytes of the XAttribute's raw data.
    DGNPLATFORM_EXPORT UInt32 GetSize () const;
    //! Get a pointer to the XAttribute's raw data.
    DGNPLATFORM_EXPORT void const* PeekData () const;
    //! Get the type of change scheduled to the XAttribute.
    ChangeType GetChangeType () const {return m_changeType;}

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static XAttributeChange GetNilObject ();

    //! Normalize change type
    //! @return
    //!   <p>CHANGETYPE_Write,  CHANGETYPE_HistoricalVersion            => CHANGETYPE_Write
    //!   <p>CHANGETYPE_Delete, CHANGETYPE_HistoricalVersionDidNotExist => CHANGETYPE_Delete
    DGNPLATFORM_EXPORT static ChangeType NormalizeChangeType (ChangeType);

    #if defined (TEST_XAttributeChange)
        void  Dump () const;
    #endif

    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        static UInt32           s_count;
    #endif
//__PUBLISH_SECTION_START__

};

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
