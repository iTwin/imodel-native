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

std::ostream& operator << (std::ostream &o, CredentialsCR creds)
    {
    o << creds.GetUsername () + ":" + creds.GetPassword ();
    return o;
    }

std::ostream& operator << (std::ostream &o, ConnectionStatus status)
    {
    static std::map<ConnectionStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR (ConnectionStatus::None),
        TO_VALUE_STRING_PAIR (ConnectionStatus::OK),
        TO_VALUE_STRING_PAIR (ConnectionStatus::Canceled),
        TO_VALUE_STRING_PAIR (ConnectionStatus::CouldNotConnect),
        TO_VALUE_STRING_PAIR (ConnectionStatus::ConnectionLost),
        TO_VALUE_STRING_PAIR (ConnectionStatus::Timeout),
        TO_VALUE_STRING_PAIR (ConnectionStatus::UnknownError),
        };

    Utf8String name = names[status];
    BeAssert (!name.empty () && "Add missing value");
    o << name;

    return o;
    }

std::ostream& operator << (std::ostream &o, HttpStatus status)
    {
    o << static_cast<int>(status);
    return o;
    }

std::ostream& operator << (std::ostream &o, BeVersionCR version)
    {
    o << version.ToString ().c_str ();
    return o;
    }

namespace Json
    {
    void PrintTo (const Value& value, ::std::ostream* os)
        {
        *os << value.toStyledString ();
        }
    }

BEGIN_BENTLEY_NAMESPACE

void PrintTo (const Utf8String& value, ::std::ostream* os)
    {
    *os << '"' << value << '"';
    }

void PrintTo (const WString& value, ::std::ostream* os)
    {
    PrintTo (Utf8String (value), os);
    }

void PrintTo (BentleyStatus value, ::std::ostream* os)
    {
    static std::map<BentleyStatus, Utf8String> names
        {
        TO_VALUE_STRING_PAIR (ERROR),
        TO_VALUE_STRING_PAIR (SUCCESS)
    };

    Utf8String name = names[value];
    BeAssert (!name.empty () && "Add missing value");
    *os << name;
    }

END_BENTLEY_NAMESPACE