/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/FileMoniker.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS(FileMoniker);
DGNPLATFORM_REF_COUNTED_PTR(FileMoniker);

BEGIN_BENTLEY_DGN_NAMESPACE

struct FileMoniker : RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

protected:
    Utf8String      m_fullPath;
    Utf8String      m_basePath;

    FileMoniker (Utf8StringCR fullPath, Utf8StringCR basePath);
    ~FileMoniker () {};

public:
    DGNPLATFORM_EXPORT static   FileMonikerPtr      Create (Utf8StringCR fullPath, Utf8StringCR basePath);
    DGNPLATFORM_EXPORT          BentleyStatus       ResolveFileName (Utf8StringR resolvedName, Utf8StringCR basePath) const;
    DGNPLATFORM_EXPORT          void                ToJson(JsonValueR outValue) const;
    DGNPLATFORM_EXPORT          void                FromJson(JsonValueCR inValue);
};

END_BENTLEY_DGN_NAMESPACE
