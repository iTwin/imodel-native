/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/ValuePrinter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// GTEST value printers

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeVersion.h>
#include <DgnClientFx/Utils/Http/Credentials.h>
#include <DgnClientFx/Utils/Http/HttpClient.h>
#include <DgnClientFx/Utils/Http/HttpResponse.h>
#include <DgnClientFx/Utils/Http/HttpStatus.h>
#include <DgnClientFx/Utils/Threading/AsyncTask.h>
#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Client/ObjectId.h>
#include <WebServices/Client/WSError.h>
#include <WebServices/Client/WSInfo.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <iostream>
#include <ostream>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
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
std::ostream& operator << (std::ostream &o, ECValueCR value);
std::ostream& operator << (std::ostream &o, DbResult value);
std::ostream& operator << (std::ostream &o, ECSqlStatus::Status status);
std::ostream& operator << (std::ostream &o, const ECInstanceKeyMultiMap::value_type& pair);

namespace rapidjson
    {
    void PrintTo(const Value& value, ::std::ostream* os);
    void PrintTo(const Document& value, ::std::ostream* os);
    }

// Duplicating symbols with UnitTests, enable with custom builds only
// #define WSCLIENT_ENABLE_DUPLICATING_SYMBOLS
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

void PrintTo(const WString& value, ::std::ostream* os);
void PrintTo(const Utf8String& value, ::std::ostream* os);
void PrintTo(BentleyStatus value, ::std::ostream* os);
END_BENTLEY_NAMESPACE

#endif
