/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/SCVWritter.h>

#include <Bentley/BeFile.h>

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SCVWritter::ToString()
    {
    return Utf8String(m_sstr.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SCVWritter::WriteToFile(BeFileNameCR filePath)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Create(filePath))
        return ERROR;

    if (BeFileStatus::Success != file.Write(nullptr, m_sstr.str().c_str(), (uint32_t)m_sstr.str().size()))
        return ERROR;

    if (BeFileStatus::Success != file.Close())
        return ERROR;

    return SUCCESS;
    }
