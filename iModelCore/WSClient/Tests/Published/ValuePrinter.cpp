/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ValuePrinter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ValuePrinter.h"
#include <map>

#define TO_VALUE_STRING_PAIR(value) {value, #value}

std::ostream& operator << (std::ostream &o, WSError::Status status)
    {
    static std::map<WSError::Status, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(WSError::Status::None),
        TO_VALUE_STRING_PAIR(WSError::Status::Canceled),
        TO_VALUE_STRING_PAIR(WSError::Status::ConnectionError),
        TO_VALUE_STRING_PAIR(WSError::Status::ServerNotSupported),
        TO_VALUE_STRING_PAIR(WSError::Status::ReceivedError),
    };

    Utf8String name = names[status];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, WSError::Id errorId)
    {
    static std::map<WSError::Id, Utf8String> names
        {
        TO_VALUE_STRING_PAIR(WSError::Id::Unknown),
        TO_VALUE_STRING_PAIR(WSError::Id::LoginFailed),
        TO_VALUE_STRING_PAIR(WSError::Id::SslRequired),
        TO_VALUE_STRING_PAIR(WSError::Id::NotEnoughRights),
        TO_VALUE_STRING_PAIR(WSError::Id::RepositoryNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::SchemaNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::ClassNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::PropertyNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::InstanceNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::FileNotFound),
        TO_VALUE_STRING_PAIR(WSError::Id::NotSupported),
        TO_VALUE_STRING_PAIR(WSError::Id::NoServerLicense),
        TO_VALUE_STRING_PAIR(WSError::Id::NoClientLicense),
        TO_VALUE_STRING_PAIR(WSError::Id::TooManyBadLoginAttempts),
        TO_VALUE_STRING_PAIR(WSError::Id::ServerError),
        TO_VALUE_STRING_PAIR(WSError::Id::BadRequest),
        TO_VALUE_STRING_PAIR(WSError::Id::Conflict),
    };

    Utf8String name = names[errorId];
    BeAssert(!name.empty() && "Add missing value");
    o << name;

    return o;
    }

//std::ostream& operator << (std::ostream &o, CredentialsCR creds)
//    {
//    o << creds.GetUsername () + ":" + creds.GetPassword ();
//    return o;
//    }
//
//std::ostream& operator << (std::ostream &o, ConnectionStatus status)
//    {
//    static std::map<ConnectionStatus, Utf8String> names
//        {
//        TO_VALUE_STRING_PAIR (ConnectionStatus::None),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::OK),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::Canceled),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::CouldNotConnect),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::ConnectionLost),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::Timeout),
//        TO_VALUE_STRING_PAIR (ConnectionStatus::UnknownError),
//        };
//
//    Utf8String name = names[status];
//    BeAssert (!name.empty () && "Add missing value");
//    o << name;
//
//    return o;
//    }
//
//std::ostream& operator << (std::ostream &o, HttpStatus status)
//    {
//    o << static_cast<int>(status);
//    return o;
//    }
//
//std::ostream& operator << (std::ostream &o, BeVersionCR version)
//    {
//    o << version.ToString ().c_str ();
//    return o;
//    }
//
//namespace Json
//    {
//    void PrintTo (const Value& value, ::std::ostream* os)
//        {
//        *os << value.toStyledString ();
//        }
//    }
//
//BEGIN_BENTLEY_NAMESPACE
//
//void PrintTo (const Utf8String& value, ::std::ostream* os)
//    {
//    *os << '"' << value << '"';
//    }
//
//void PrintTo (const WString& value, ::std::ostream* os)
//    {
//    PrintTo (Utf8String (value), os);
//    }
//
//void PrintTo (BentleyStatus value, ::std::ostream* os)
//    {
//    static std::map<BentleyStatus, Utf8String> names
//        {
//        TO_VALUE_STRING_PAIR (ERROR),
//        TO_VALUE_STRING_PAIR (SUCCESS)
//    };
//
//    Utf8String name = names[value];
//    BeAssert (!name.empty () && "Add missing value");
//    *os << name;
//    }
//
//END_BENTLEY_NAMESPACE