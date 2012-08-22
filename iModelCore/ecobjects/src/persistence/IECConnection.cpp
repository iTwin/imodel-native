/*--------------------------------------------------------------------------------------+
|
|     $Source: src/persistence/IECConnection.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "ECPersistence/IECConnection.h"

BEGIN_BENTLEY_EC_NAMESPACE

WCharCP IECConnection::GetProviderId () const
    {
    return _GetProviderId ();
    }

IECStatementPtr IECConnection::CreateStatement ()
    {
    return _CreateStatement ();
    }

int IECConnection::GetLastError (WStringP lastErrorMessage) const
    {
    return _GetLastError (lastErrorMessage);
    }

END_BENTLEY_EC_NAMESPACE
