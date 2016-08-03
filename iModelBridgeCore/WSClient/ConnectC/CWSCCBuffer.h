/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCCBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "CWSCCPrivate.h"

typedef struct _CWSCCBUFFER
    {
    bool           isWSGBuffer;
    uint32_t       lClassType;
    uint32_t       lSchemaType;
    uint64_t       lCount;
    bvector<void*> lItems;
    } CWSCCBUFFER, *LPCWSCCBUFFER;

typedef LPCWSCCBUFFER HCWSCCBUFFER;

#define SCHEMA_TYPE_IMSSEARCH                  1


/************************************************************************************//**
* Free an allocated non-wsg data buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferFree
(
LPCWSCC api,
HCWSCCBUFFER buf
);

/************************************************************************************//**
* Get a String property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetStringProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
);

/************************************************************************************//**
* Get a Stringlength property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetStringLength
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
);

/************************************************************************************//**
* Get a Datetime property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetDatetimeProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
);

/************************************************************************************//**
* Get a Guid property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetGuidProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
);

/************************************************************************************//**
* Get a Boolean property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetBooleanProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
bool* boolean
);

/************************************************************************************//**
* Get a Int property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetIntProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
);

/************************************************************************************//**
* Get a Double property from a wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetDoubleProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
double* pDouble
);

/************************************************************************************//**
* Get a Long property from a non-wsg buffer
****************************************************************************************/
CallStatus NonWSG_DataBufferGetLongProperty
(
LPCWSCC api,
HCWSCCBUFFER buf,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
);

