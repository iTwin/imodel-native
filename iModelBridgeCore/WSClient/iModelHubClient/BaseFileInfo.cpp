/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/BaseFileInfo.h>
#include "Utils.h"

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
BaseFileInfo::BaseFileInfo(Dgn::DgnDbCR db, Utf8StringCR description) : m_description(description), m_areFileDetailsAvailable(false)
    {
    BeFileName fileName = db.GetFileName();
    BeStringUtilities::WCharToUtf8(m_fileName, BeFileName::GetFileNameAndExtension(fileName).c_str());
    m_fileId = db.GetDbGuid();
    fileName.GetFileSize(m_fileSize);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis             08/2016
//---------------------------------------------------------------------------------------
BaseFileInfo::BaseFileInfo(RapidJsonValueCR properties) : m_fileSize(0), m_areFileDetailsAvailable(false)
    {
    UpdateBaseFileInfo (properties);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             10/2016
//---------------------------------------------------------------------------------------
Utf8String BaseFileInfo::GetProperty(RapidJsonValueCR properties, Utf8StringCR member)
    {
    if (properties.HasMember(member.c_str()))
        {
        if (properties[member.c_str()].IsString())
            {
            return properties[member.c_str()].GetString();
            }
        }
    return "";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                12/2017
//---------------------------------------------------------------------------------------
void BaseFileInfo::UpdateBaseFileInfo(RapidJsonValueCR properties)
    {
    Utf8String fileName = GetProperty(properties, ServerSchema::Property::FileName);
    if (!fileName.empty())
        m_fileName = fileName;

    Utf8String fileId = GetProperty(properties, ServerSchema::Property::FileId);
    if (!fileId.empty())
        m_fileId.FromString(fileId.c_str());

    Utf8String description = GetProperty(properties, ServerSchema::Property::FileDescription);
    if (!description.empty())
        m_description = description;

    Utf8String sizeString = GetProperty(properties, ServerSchema::Property::FileSize);
    if (!sizeString.empty())
        {
        uint64_t size;
        BeStringUtilities::ParseUInt64(size, sizeString.c_str());
        if (size > 0)
            {
            m_fileSize = size;
            m_areFileDetailsAvailable = true;
            }
        }
    }