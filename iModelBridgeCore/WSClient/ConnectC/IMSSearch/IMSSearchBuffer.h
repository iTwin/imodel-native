/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/IMSSearch/IMSSearchBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../CWSCCBuffer.h"
#include "../CWSCCPrivate.h"

#define BUFF_TYPE_IMSUSER 1

/*--------------------------------------------------------------------------------------+
| Schema-level free functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsSearch_DataBufferFree
(
    LPCWSCC api,
    HCWSCCBUFFER buf
);

/*--------------------------------------------------------------------------------------+
| Schema-level accessor functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsSearch_GetStringProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP str
);

CallStatus ImsSearch_GetStringLength
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    size_t* outStringSize
);

CallStatus ImsSearch_GetDatetimeProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP dateTime
);

CallStatus ImsSearch_GetGuidProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP guid
);

CallStatus ImsSearch_GetIntProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    int32_t* integer
);

CallStatus ImsSearch_GetDoubleProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    double* pDouble
);

CallStatus ImsSearch_GetLongProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    int64_t* pLong
);

CallStatus ImsSearch_GetBooleanProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    bool* boolean
);

/*--------------------------------------------------------------------------------------+
| String functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetStringProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP str
);

/*--------------------------------------------------------------------------------------+
| String length functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetStringLength
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    size_t* outStringSize
);


/*--------------------------------------------------------------------------------------+
| DateTime functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetDatetimeProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP dateTime
);

/*--------------------------------------------------------------------------------------+
| Guid functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetGuidProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    uint32_t strLength,
    WCharP guid
);


/*--------------------------------------------------------------------------------------+
| Bool functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetBooleanProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    bool* boolean
);


/*--------------------------------------------------------------------------------------+
| Integer functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetIntProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    int32_t* integer
);


/*--------------------------------------------------------------------------------------+
| Double functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetDoubleProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    double* pDouble
);

/*--------------------------------------------------------------------------------------+
| Long functions
+--------------------------------------------------------------------------------------*/
CallStatus ImsUser_GetLongProperty
(
    LPCWSCC api,
    HCWSCCBUFFER buf,
    int16_t bufferProperty,
    uint32_t index,
    int64_t* pLong
);

/*--------------------------------------------------------------------------------------+
| Internal buffers
TODO: Fill in all the IMS User Data
+--------------------------------------------------------------------------------------*/
typedef struct _CWSCC_IMSUSER_BUFFER
    {
    bmap<WString, bool> IsSet;
    WString OrganizationName;
    WString Email;
    WString AccountId;
    } CWSCCIMSUSERBUFFER, *LPCWSCCIMSUSERBUFFER;

void ImsUser_BufferStuffer
(
    LPCWSCCIMSUSERBUFFER imsUserBuf,
    Json::Value properties
);