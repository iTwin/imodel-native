/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/ValuePrinter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// GTEST value printers

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeVersion.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/HttpStatus.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSInfo.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <iostream>
#include <ostream>

USING_NAMESPACE_BENTLEY_WEBSERVICES

std::ostream& operator << (std::ostream &o, WSError::Status status);
std::ostream& operator << (std::ostream &o, WSError::Id errorId);
std::ostream& operator << (std::ostream &o, ObjectIdCR id);

// Caching
std::ostream& operator << (std::ostream &o, ICachingDataSource::Status status);
std::ostream& operator << (std::ostream &o, ICachingDataSource::DataOrigin origin);
std::ostream& operator << (std::ostream &o, CacheStatus status);
std::ostream& operator << (std::ostream &o, FileCache location);
std::ostream& operator << (std::ostream &o, IChangeManager::ChangeStatus status);
std::ostream& operator << (std::ostream &o, IChangeManager::SyncStatus status);
std::ostream& operator << (std::ostream &o, ECInstanceKeyCR instance);
std::ostream& operator << (std::ostream &o, DateTimeCR date);

// EC
std::ostream& operator << (std::ostream &o, ECClassCR value);
std::ostream& operator << (std::ostream &o, ECValueCR value);
std::ostream& operator << (std::ostream &o, DbResult value);
std::ostream& operator << (std::ostream &o, ECSqlStatus status);
std::ostream& operator << (std::ostream &o, const ECInstanceKeyMultiMap::value_type& pair);
std::ostream& operator << (std::ostream &o, SchemaReadStatus value);

std::ostream& operator << (std::ostream &o, BeFileNameStatus value);
std::ostream& operator << (std::ostream &o, BeFileStatus value);

namespace rapidjson
    {
    void PrintTo(const Value& value, ::std::ostream* os);
    void PrintTo(const Document& value, ::std::ostream* os);
    }

// DEPRECATED: Duplicating symbols with DgnClientFx SDK UnitTests
// Now tests are build seperately for each library so no duplication should occur
// In case of Aggregate build set Environment Variable (BUILD_AGGREGATE_TESTS) which will make (#define WSCLIENT_ENABLE_DUPLICATING_SYMBOLS) in active

#ifndef BUILD_FOR_AGGREGATE_TESTS
#define WSCLIENT_ENABLE_DUPLICATING_SYMBOLS
#endif

#ifdef WSCLIENT_ENABLE_DUPLICATING_SYMBOLS

std::ostream& operator << (std::ostream &o, CredentialsCR creds);
std::ostream& operator << (std::ostream &o, ConnectionStatus status);
std::ostream& operator << (std::ostream &o, HttpStatus status);
std::ostream& operator << (std::ostream &o, BeVersionCR version);

BEGIN_BENTLEY_NAMESPACE
namespace Json
    {
    void PrintTo(const Value& value, ::std::ostream* os);
    }

void PrintTo(const Utf8String& value, ::std::ostream* os);
void PrintTo(const WString& value, ::std::ostream* os);
void PrintTo(const BeFileName& value, ::std::ostream* os);
void PrintTo(BentleyStatus value, ::std::ostream* os);
END_BENTLEY_NAMESPACE

#endif
