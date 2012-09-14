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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP IECConnection::GetProviderId () const
    {
    return _GetProviderId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECConnection::IsOpen () const
    {
    return _IsOpen ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECSchemaManagerCR IECConnection::GetSchemaManager () const
    {
    return _GetSchemaManager ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECStatementPtr IECConnection::CreateStatement ()
    {
    return _CreateStatement ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int IECConnection::GetLastError (WStringP lastErrorMessage) const
    {
    return _GetLastError (lastErrorMessage);
    }

END_BENTLEY_EC_NAMESPACE
