/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ValuePrinter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// GTEST value printers

#include <Bentley/BeVersion.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <MobileDgn/Utils/Http/Credentials.h>
#include <MobileDgn/Utils/Http/HttpStatus.h>
#include <MobileDgn/Utils/Http/HttpResponse.h>
#include <WebServices/Client/WSError.h>
#include <ostream>
#include <iostream>

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_BENTLEY_WEBSERVICES

std::ostream& operator << (std::ostream &o, WSError::Status status);
std::ostream& operator << (std::ostream &o, WSError::Id errorId);

// TODO: resolve duplicating symbols with MobileDgnUnitTests
//// Web
//std::ostream& operator << (std::ostream &o, CredentialsCR creds);
//std::ostream& operator << (std::ostream &o, ConnectionStatus status);
//std::ostream& operator << (std::ostream &o, HttpStatus status);
//std::ostream& operator << (std::ostream &o, BeVersionCR version);
//
//// Misc
//namespace Json
//    {
//    void PrintTo (const Value& value, ::std::ostream* os);
//    }
//
//BEGIN_BENTLEY_NAMESPACE
//void PrintTo (const WString& value, ::std::ostream* os);
//void PrintTo (const Utf8String& value, ::std::ostream* os);
//void PrintTo (BentleyStatus value, ::std::ostream* os);
//END_BENTLEY_NAMESPACE
