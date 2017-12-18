/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/ConnectC/CWSCCBufferPublic.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/ConnectC/CWSCC.h>

/****************************************************************************************
* \defgroup BufferFunctions ConnectWebServicesClientC Data Buffer Functions
* \{
****************************************************************************************/
/****************************************************************************************
* \brief Free an allocated data buffer
* \param[in] apiHandle Handle to api
* \param[in] dataBuffer Data buffer
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferFree
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer
);
/****************************************************************************************
* \brief Get a count of the number of items in a data buffer
* \param[in] dataBuffer Data buffer
* \return Object count
****************************************************************************************/
CWSCC_EXPORT uint64_t ConnectWebServicesClientC_DataBufferGetCount
(
CWSCCDATABUFHANDLE dataBuffer
);
/****************************************************************************************
* \brief Get a String property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[in] strLength buffer length
* \param[out] str Pointer to buffer to store string property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetStringProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP str
);

/****************************************************************************************
* \brief Get a Stringlength property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[out] outStringSize Pointer to store the string length
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetStringLength
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
size_t* outStringSize
);

/****************************************************************************************
* \brief Get a Datetime property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[in] strLength buffer length
* \param[out] dateTime Pointer to buffer to store dateTime property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetDatetimeProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP dateTime
);

/****************************************************************************************
* \brief Get a Guid property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[in] strLength guid-buffer length
* \param[out] guid Pointer to buffer to store GUID property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetGuidProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
uint32_t strLength,
WCharP guid
);

/****************************************************************************************
* \brief Get a Boolean property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[out] boolean Pointer to bool to store property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetBooleanProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
bool* boolean
);

/****************************************************************************************
* \brief Get a Int property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[out] integer Pointer to int to store property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetIntProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
int32_t* integer
);

/****************************************************************************************
* \brief Get a Double property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[out] pDouble Pointer to double to store property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetDoubleProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
double* pDouble
);

/****************************************************************************************
* \brief Get a Long property from a data buffer
* \param[in] apiHandle handle to api
* \param[in] dataBuffer Data buffer
* \param[in] bufferProperty buffer property
* \param[in] index buffer index
* \param[out] pLong Pointer to long to store property
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_DataBufferGetLongProperty
(
CWSCCHANDLE apiHandle,
CWSCCDATABUFHANDLE dataBuffer,
int16_t bufferProperty,
uint32_t index,
int64_t* pLong
);

/** \} */
