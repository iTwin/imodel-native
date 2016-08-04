/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/IMSSearch/IMSSearchBuffer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CWSCCInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_DataBufferFree
(
LPCWSCC api,
HCWSCCBUFFER buf
)
    {
    for (uint64_t index = 0; index < buf->lItems.size(); index++)
        {
        if (buf->lItems[index] != nullptr)
            {
            switch(buf->lClassType)
                {
                case BUFF_TYPE_IMSUSER:
                    {
                    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
                    delete imsUserBuf;
                    }
                    break;
                default:
                    continue;
                }
            }
        }
    free(buf);
    api->SetStatusMessage("Successful operation");
    api->SetStatusDescription("The ConnectWebServicesClientC_DataBufferFree function successfully completed.");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetStringProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetStringProperty(api, buf, bufferProperty, index, strLength, str);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetStringLength
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetStringLength(api, buf, bufferProperty, index, outStringSize);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetDatetimeProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetDatetimeProperty(api, buf, bufferProperty, index, strLength, dateTime);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetGuidProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return Organization_GetGuidProperty(api, buf, bufferProperty, index, strLength, guid);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetBooleanProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
bool* boolean
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetBooleanProperty(api, buf, bufferProperty, index, boolean);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetIntProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetIntProperty(api, buf, bufferProperty, index, integer);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetDoubleProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
double* pDouble
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetDoubleProperty(api, buf, bufferProperty, index, pDouble);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsSearch_GetLongProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
)
    {
    switch (buf->lClassType)
        {
        case BUFF_TYPE_IMSUSER:
            {
            return ImsUser_GetLongProperty(api, buf, bufferProperty, index, pLong);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetStringProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
)
    {
    if (buf == nullptr || bufferProperty == 0 || str == nullptr || strLength == 0)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
        return INVALID_PARAMETER;
        }

    if (index >= buf->lCount)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
        return INVALID_PARAMETER;
        }

    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER)buf->lItems[index];
    if (IMSUSER_BUFF_ORGANIZATIONNAME == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("OrganizationName", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("OrganizationName", true)] == false)
            {
            str = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("OrganizationName property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        BeStringUtilities::Wcsncpy(str, strLength, imsUserBuf->OrganizationName.c_str());
        }
    else if (IMSUSER_BUFF_ACCOUNTID == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("AccountId", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("AccountId", true)] == false)
            {
            str = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("AccountId property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        BeStringUtilities::Wcsncpy(str, strLength, imsUserBuf->AccountId.c_str());
        }
    else if (IMSUSER_BUFF_EMAIL == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("Email", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("Email", true)] == false)
            {
            str = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("Email property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        BeStringUtilities::Wcsncpy(str, strLength, imsUserBuf->Email.c_str());
        }
    else
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
        return INVALID_PARAMETER;
        }
    api->SetStatusMessage("Successful operation");
    api->SetStatusDescription("The property retrieval function completed successfully.");
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetStringLength
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
)
    {
    if (buf == nullptr || bufferProperty == 0 || outStringSize == nullptr)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
        return INVALID_PARAMETER;
        }

    if(index >= buf->lCount)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
        return INVALID_PARAMETER;
        }

    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
    if (IMSUSER_BUFF_ORGANIZATIONNAME == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("OrganizationName", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("OrganizationName", true)] == false)
            {
            outStringSize = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("OrganizationName property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        *outStringSize = imsUserBuf->OrganizationName.length();
        }
    else if (IMSUSER_BUFF_ACCOUNTID == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("AccountId", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("AccountId", true)] == false)
            {
            outStringSize = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("AccountId property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        *outStringSize = imsUserBuf->AccountId.length();
        }
    else if (IMSUSER_BUFF_EMAIL == bufferProperty)
        {
        if (imsUserBuf->IsSet.find(WString("Email", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("Email", true)] == false)
            {
            outStringSize = nullptr;
            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
            api->SetStatusDescription("Email property is not set, so it can not be retrieved.");
            return PROPERTY_HAS_NOT_BEEN_SET;
            }
        *outStringSize = imsUserBuf->Email.length();
        }
    else
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
        return INVALID_PARAMETER;
        }
    api->SetStatusMessage("Successful operation");
    api->SetStatusDescription("The property retrieval function completed successfully.");
    return SUCCESS;
    }
//
//TODO: Note -- GetImsUserData needs to have all the properties from IMS Search API. They're stubbed in below but don't have all of the user properties
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetDatetimeProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || dateTime == nullptr || strLength == 0)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (PROJECTFAVORITE_V2_BUFF_LASTMODIFIED == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("LastModified", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("LastModified", true)] == false)
//            {
//            dateTime = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("LastModified property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        BeStringUtilities::Wcsncpy(dateTime, strLength, imsUserBuf->LastModified.c_str());
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }
//
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetGuidProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || guid == nullptr || strLength == 0)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (IMSUSER_BUFF_ORGANIZATIONGUID == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("OrganizationGuid", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("OrganizationGuid", true)] == false)
//            {
//            guid = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("OrganizationGuid property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        BeStringUtilities::Wcsncpy(guid, strLength, imsUserBuf->OrganizationGuid.c_str());
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetBooleanProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
bool* boolean
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || boolean == nullptr)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (PROJECT_BUFF_ACTIVE == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("Active", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("Active", true)] == false)
//            {
//            boolean = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("Active property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        *boolean = imsUserBuf->Active;
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }
//
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetIntProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || integer == nullptr)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (PROJECT_BUFF_STATUS == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("Status", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("Status", true)] == false)
//            {
//            integer = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("Status property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        *integer = imsUserBuf->Status;
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetDoubleProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
double* pDouble
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || pDouble == nullptr)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (PROJECTFAVORITE_BUFF_LATITUDE == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("Latitude", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("Latitude", true)] == false)
//            {
//            pDouble = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("Latitude property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        *pDouble = imsUserBuf->Latitude;
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                    07/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ImsUser_GetLongProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
)
    {
    return SUCCESS;
    }
//    if (buf == nullptr || bufferProperty == 0 || pLong == nullptr)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("An invalid buffer pointer or invalid property pointer was passed into the get property function.");
//        return INVALID_PARAMETER;
//        }
//
//    if(index >= buf->lCount)
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The index parameter passed into the get property function is out of bounds.");
//        return INVALID_PARAMETER;
//        }
//
//    LPCWSCCIMSUSERBUFFER imsUserBuf = (LPCWSCCIMSUSERBUFFER) buf->lItems[index];
//    if (PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER == bufferProperty)
//        {
//        if (imsUserBuf->IsSet.find(WString("LastAccessedByUser", true)) == imsUserBuf->IsSet.end() || imsUserBuf->IsSet[WString("LastAccessedByUser", true)] == false)
//            {
//            pLong = nullptr;
//            api->SetStatusMessage("The buffer property passed to function has not been set in the buffer");
//            api->SetStatusDescription("LastAccessedByUser property is not set, so it can not be retrieved.");
//            return PROPERTY_HAS_NOT_BEEN_SET;
//            }
//        *pLong = imsUserBuf->LastAccessedByUser;
//        }
//    else
//        {
//        api->SetStatusMessage("Invalid parameter passed to function");
//        api->SetStatusDescription("The bufferProperty is invalid. It did not match up with any of the buffer's properties.");
//        return INVALID_PARAMETER;
//        }
//    api->SetStatusMessage("Successful operation");
//    api->SetStatusDescription("The property retrieval function completed successfully.");
//    return SUCCESS;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ImsUser_BufferStuffer
(
LPCWSCCIMSUSERBUFFER imsUserBuf,
Json::Value properties
)
    {
    if(properties.isMember("OrganizationName") && properties["OrganizationName"].isString())
        imsUserBuf->OrganizationName = WString(properties["OrganizationName"].asCString(), true);
    imsUserBuf->IsSet[WString("OrganizationName", true)] = (properties.isMember("OrganizationName") && properties["OrganizationName"].isString());
    if (properties.isMember("Email") && properties["Email"].isString())
        imsUserBuf->Email = WString(properties["Email"].asCString(), true);
    imsUserBuf->IsSet[WString("Email", true)] = (properties.isMember("Email") && properties["Email"].isString());
    if (properties.isMember("AccountId") && properties["AccountId"].isString())
        imsUserBuf->AccountId = WString(properties["AccountId"].asCString(), true);
    imsUserBuf->IsSet[WString("AccountId", true)] = (properties.isMember("AccountId") && properties["AccountId"].isString());
    }