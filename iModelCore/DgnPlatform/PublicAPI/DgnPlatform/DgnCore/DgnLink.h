/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnLink.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>

DGNPLATFORM_TYPEDEFS(DgnLink);
DGNPLATFORM_REF_COUNTED_PTR(DgnLink);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup DgnLinks
//! @{

//=======================================================================================
// Used in persistence; do not change values. If you add or remove values, also ensure you update the #define's in the implemetation file for queries.
// @bsiclass
//=======================================================================================
enum class DgnLinkType
{
    Invalid = 0,

    Url = 1,
    ExternalFile = 2,
    EmbeddedFile = 3,
    View = 4

}; // DgnLinkType

//=======================================================================================
//! This single link class encompasses all link types. Calling Set methods such as SetUrl set the type as well as the data. When reading, use GetType to determine an appropriate Get method to call.
// @bsiclass                                                    Jeff.Marker     02/2015
//=======================================================================================
struct DgnLink : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase);

    DgnDbP m_db;
    DgnLinkId m_id;
    DgnLinkType m_type;
    Utf8String m_displayLabel;
    Json::Value m_data;

    DgnLink(DgnDbR);
    DgnLink(DgnLinkCR);
    DgnLinkR operator=(DgnLinkCR);

    void CopyFrom(DgnLinkCR);
    void GetStringFromJson(Utf8StringR value, Utf8CP key) const;

public:
    DGNPLATFORM_EXPORT void SetId(DgnLinkId);
    
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    DGNPLATFORM_EXPORT static DgnLinkPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT DgnLinkPtr Clone() const;

    DGNPLATFORM_EXPORT DgnLinkId GetId() const;
    DGNPLATFORM_EXPORT DgnLinkType GetType() const;
    DGNPLATFORM_EXPORT Utf8StringCR GetDisplayLabel() const;
    DGNPLATFORM_EXPORT void SetDisplayLabel(Utf8CP);

    DGNPLATFORM_EXPORT BentleyStatus GetUrl(Utf8StringR) const;
    DGNPLATFORM_EXPORT void SetUrl(Utf8CP);
    DGNPLATFORM_EXPORT BentleyStatus GetExternalFilePaths(Utf8StringP dmsPath, Utf8StringP portablePath, Utf8StringP lastKnownLocalPath) const;
    DGNPLATFORM_EXPORT void SetExternalFilePaths(Utf8CP dmsPath, Utf8CP portablePath, Utf8CP lastKnownLocalPath);
    DGNPLATFORM_EXPORT BentleyStatus GetEmbeddedDocumentName(Utf8StringR) const;
    DGNPLATFORM_EXPORT void SetEmbeddedDocumentName(Utf8CP);
    DGNPLATFORM_EXPORT BentleyStatus GetViewId(DgnViewId&) const;
    DGNPLATFORM_EXPORT void SetViewId(DgnViewId);

}; // DgnLink

//! @}

END_BENTLEY_DGN_NAMESPACE
