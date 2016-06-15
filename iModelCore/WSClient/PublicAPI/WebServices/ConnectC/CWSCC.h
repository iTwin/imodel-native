/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/ConnectC/CWSCC.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

using namespace std;

// define __CWSCC_DLL_BUILD__ when consuming header.
#ifdef __CWSCC_DLL_BUILD__
#define CWSCC_EXPORT EXPORT_ATTRIBUTE
#else
#define CWSCC_EXPORT IMPORT_ATTRIBUTE
#endif

/************************************************************************************//**
* \defgroup ConnectWebServicesClientCStatusCodes ConnectWebServicesClientC Status Codes
* \\ NOTE: These status codes are used in pyCApiGen tool. If edited here, update there as well.
* \{
****************************************************************************************/

/*!
\def ERROR500
The response from the server contained a 500, Internal Server, http error
*/

/*!
\def ERROR409
The response from the server contained a 409, Conflict, http error
*/

/*!
\def ERROR404
The response from the server contained a 404, Not Found, http error
*/

/*!
\def ERROR403
The response from the server contained a 403, Forbidden, http error
*/

/*!
\def ERROR401
The response from the server contained a 401, Unauthorized, http error
*/

/*!
\def ERROR400
The response from the server contained a 400, Bad Request, http error
*/

/*!
\def SUCCESS
Successful operation
*/

/*!
\def INVALID_PARAMETER
Invalid parameter passed to function
*/

/*!
\def PROPERTY_HAS_NOT_BEEN_SET
The buffer property passed to function has not been set in the buffer
*/

/*!
\def INTERNAL_MEMORY_ERROR
Memory failed to initialize interally.
*/

/*!
\def LOGIN_FAILED
The login attempt was not completed successfully.
*/

/*!
\def SSL_REQUIRED
SSL is required for communication with the server.
*/

/*!
\def NOT_ENOUGH_RIGHTS
The user does not have the proper rights.
*/

/*!
\def REPOSITORY_NOT_FOUND
The repository you attempted to query was not found.
*/

/*!
\def SCHEMA_NOT_FOUND
The schema you attempted to query was not found.
*/

/*!
\def CLASS_NOT_FOUND
The class you attempted to query was not found.
*/

/*!
\def PROPERTY_NOT_FOUND
The property you attempted to query was not found.
*/

/*!
\def INSTANCE_NOT_FOUND
The instance you attempted to query was not found.
*/

/*!
\def FILE_NOT_FOUND
The file you attempted to query was not found.
*/

/*!
\def NOT_SUPPORTED
The action you attempted is not supported.
*/

/*!
\def NO_SERVER_LICENSE
A server license was not found.
*/

/*!
\def NO_CLIENT_LICENSE
A client license was not found.
*/

/*!
\def TO_MANY_BAD_LOGIN_ATTEMPTS
To many unsuccessful login attempts have happened.
*/

#define ERROR500                                 500
#define ERROR409                                 409
#define ERROR404                                 404
#define ERROR403                                 403
#define ERROR401                                 401
#define ERROR400                                 400
#define SUCCESS                                  0
#define INVALID_PARAMETER                        -100
#define PROPERTY_HAS_NOT_BEEN_SET                -101
#define INTERNAL_MEMORY_ERROR                    -102
#define LOGIN_FAILED                             -200
#define SSL_REQUIRED                             -201
#define NOT_ENOUGH_RIGHTS                        -202
#define REPOSITORY_NOT_FOUND                     -204
#define SCHEMA_NOT_FOUND                         -205
#define CLASS_NOT_FOUND                          -206
#define PROPERTY_NOT_FOUND                       -207
#define INSTANCE_NOT_FOUND                       -210
#define FILE_NOT_FOUND                           -211
#define NOT_SUPPORTED                            -212
#define NO_SERVER_LICENSE                        -213
#define NO_CLIENT_LICENSE                        -214
#define TO_MANY_BAD_LOGIN_ATTEMPTS               -215

/**
* \brief Call status code. See \ref ConnectWebServicesClientCStatusCodes
*/
typedef int16_t CallStatus;
/** \} */

/************************************************************************************//**
* \defgroup PointerTypes ConnectWebServicesClientC Pointer types
* \{
****************************************************************************************/
/**
* \brief API handle 
*/
typedef void* CWSCCHANDLE;

typedef void* CWSCCDATABUFHANDLE;

/**
* \brief IHttpHandlerPtr [forbidden] Pointer to a custom HttpHandler used for testing ONLY
*/
typedef void* IHTTPHANDLERPTR;
/** \} */