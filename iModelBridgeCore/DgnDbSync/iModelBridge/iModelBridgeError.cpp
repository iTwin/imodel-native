/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeError.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <rapidjson/filewritestream.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeError::iModelBridgeError()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeError::iModelBridgeError(Error const& hubError)
    {
    m_id = static_cast<iModelBridgeErrorId>(hubError.GetId());
    m_message = hubError.GetMessage();
    m_description = hubError.GetDescription();
    m_extendedData = hubError.GetExtendedData();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeError::WriteErrorMessage(BeFileNameCR errorFileName)
    {
    Utf8String fileNameutf8(errorFileName);
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();
    document.AddMember("Id", rapidjson::Value((int)m_id), allocator);
    if (!m_message.empty())
        document.AddMember("Message", rapidjson::Value(m_message.c_str(), allocator), allocator);
    if (!m_description.empty())
        document.AddMember("Description", rapidjson::Value(m_description.c_str(), allocator), allocator);
    //if (!m_extendedData.isNull())
    //    document.AddMember("Details", m_extendedData, allocator);
    
    FILE* fp = fopen(fileNameutf8.c_str(), "w");
    char buffer[1024];
    rapidjson::FileWriteStream fs(fp, buffer, sizeof(buffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(fs);
    //rapidjson::StringBuffer buffer;
    //rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    fclose(fp);
    }