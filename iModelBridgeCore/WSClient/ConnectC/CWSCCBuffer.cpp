/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCCBuffer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CWSCCInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferFree
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer
)
    {
    VERIFY_API
    if (nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC_DataBufferFree is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER)dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferFree(api, buf);
        }
    else
        {
        return NonWSG_DataBufferFree(api, buf);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t ConnectWebServicesClientC_DataBufferGetCount
(
CWSCCDATABUFHANDLE dataBuffer
)
    {
    if (nullptr == dataBuffer)
        return 0;

    HCWSCCBUFFER buf = (HCWSCCBUFFER)dataBuffer;
    return buf->lCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetStringProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER)dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetStringProperty(api, buf, bufferProperty, index, strLength, str);
        }
    else
        {
        return NonWSG_DataBufferGetStringProperty(api, buf, bufferProperty, index, strLength, str);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetStringLength
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetStringLength(api, buf, bufferProperty, index, outStringSize);
        }
    else
        {
        return NonWSG_DataBufferGetStringLength(api, buf, bufferProperty, index, outStringSize);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetDatetimeProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetDatetimeProperty(api, buf, bufferProperty, index, strLength, dateTime);
        }
    else
        {
        return NonWSG_DataBufferGetDatetimeProperty(api, buf, bufferProperty, index, strLength, dateTime);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetGuidProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetGuidProperty(api, buf, bufferProperty, index, strLength, guid);
        }
    else
        {
        return NonWSG_DataBufferGetGuidProperty(api, buf, bufferProperty, index, strLength, guid);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetBooleanProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
bool* boolean
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetBooleanProperty(api, buf, bufferProperty, index, boolean);
        }
    else
        {
        return NonWSG_DataBufferGetBooleanProperty(api, buf, bufferProperty, index, boolean);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetIntProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetIntProperty(api, buf, bufferProperty, index, integer);
        }
    else
        {
        return NonWSG_DataBufferGetIntProperty(api, buf, bufferProperty, index, integer);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetDoubleProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
double* pDouble
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetDoubleProperty(api, buf, bufferProperty, index, pDouble);
        }
    else
        {
        return NonWSG_DataBufferGetDoubleProperty(api, buf, bufferProperty, index, pDouble);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_DataBufferGetLongProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
)
    {
    VERIFY_API
    if(nullptr == dataBuffer)
        {
        api->SetStatusMessage("Invalid parameter passed to function");
        api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC data access function is invalid.");
        return INVALID_PARAMETER;
        }

    HCWSCCBUFFER buf = (HCWSCCBUFFER) dataBuffer;
    if (buf->isWSGBuffer)
        {
        return WSG_DataBufferGetLongProperty(api, buf, bufferProperty, index, pLong);
        }
    else
        {
        return NonWSG_DataBufferGetLongProperty(api, buf, bufferProperty, index, pLong);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus NonWSG_DataBufferFree
(
LPCWSCC api,
HCWSCCBUFFER buf
)
    {
    switch(buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_DataBufferFree(api, buf);
            }
        default:
            api->SetStatusMessage("Successful operation");
            api->SetStatusDescription("The dataBuffer passed into ConnectWebServicesClientC_DataBufferFree is invalid.");
            return INVALID_PARAMETER;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus NonWSG_DataBufferGetStringProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetStringProperty(api, buf, bufferProperty, index, strLength, str);
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
CallStatus NonWSG_DataBufferGetStringLength
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetStringLength(api, buf, bufferProperty, index, outStringSize);
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
CallStatus NonWSG_DataBufferGetDatetimeProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetDatetimeProperty(api, buf, bufferProperty, index, strLength, dateTime);
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
CallStatus NonWSG_DataBufferGetGuidProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetGuidProperty(api, buf, bufferProperty, index, strLength, guid);
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
CallStatus NonWSG_DataBufferGetBooleanProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
bool* boolean
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetBooleanProperty(api, buf, bufferProperty, index, boolean);
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
CallStatus NonWSG_DataBufferGetIntProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetIntProperty(api, buf, bufferProperty, index, integer);
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
CallStatus NonWSG_DataBufferGetDoubleProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
double* pDouble
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetDoubleProperty(api, buf, bufferProperty, index, pDouble);
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
CallStatus NonWSG_DataBufferGetLongProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
)
    {
    switch (buf->lSchemaType)
        {
        case SCHEMA_TYPE_IMSSEARCH:
            {
            return ImsSearch_GetLongProperty(api, buf, bufferProperty, index, pLong);
            }
        default:
            api->SetStatusMessage("Invalid parameter passed to function");
            api->SetStatusDescription("The buffer type passed in is invalid.");
            return INVALID_PARAMETER;
        }
    }



