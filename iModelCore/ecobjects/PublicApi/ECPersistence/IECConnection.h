/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPersistence/IECConnection.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECPersistence/ECPersistence.h"
#include "ECPersistence/IECSchemaManager.h"
#include "ECPersistence/IECStatement.h"

BEGIN_BENTLEY_EC_NAMESPACE

//=======================================================================================    
//! @ingroup ECPersistence
//! IECConnection represents the connection to a data source. It serves as context for the
//! ECPersistence API.
//! @bsiclass                                                 Krischan.Eberle      08/2012
//=======================================================================================    
struct IECConnection
    {
private:
    ECPERSISTENCE_EXPORT virtual WCharCP _GetProviderId () const = 0;
    ECPERSISTENCE_EXPORT virtual bool _IsOpen () const = 0;
    ECPERSISTENCE_EXPORT virtual IECSchemaManagerCR _GetSchemaManager () const = 0;
    ECPERSISTENCE_EXPORT virtual IECStatementPtr _CreateStatement () = 0;
    ECPERSISTENCE_EXPORT virtual int _GetLastError (WStringP lastErrorMessage = NULL) const = 0;

public:
    ECPERSISTENCE_EXPORT virtual ~IECConnection () {};
    
    //! Gets an ID representing the ECPersistence provider.
    //! The memory is managed by the provider. Clients do not have
    //! to care for freeing the string.
    //! @return ECPersistence provider ID
    ECPERSISTENCE_EXPORT WCharCP GetProviderId () const;

    //! Gets a value indicating whether the connection is open.
    //! Depending on the repository type, connections might already be open when clients
    //! obtain them. Therefore opening and closing a connection is handled outside of 
    //! ECPersistence (this may be revisited while going)
    //! @return true if the connection is open. false otherwise
    ECPERSISTENCE_EXPORT bool IsOpen () const;

    //! Gets the schema manager for this connection.
    //! @return Schema manager for this connection
    ECPERSISTENCE_EXPORT IECSchemaManagerCR GetSchemaManager () const;

    //! Creates a new ECStatement which can be used to execute an ECSQL statement.
    //! @return ECStatement
    ECPERSISTENCE_EXPORT IECStatementPtr CreateStatement ();

    //! Gets information about the last error that occurred within the scope of this connection. last error information
    //! Notes to implementors: This method needs to be thread-safe.
    //! @param[out] lastErrorMessage The error message corresponding to the last error code returned.
    //! @return The last error code (provider specific)
    //TODOs: Using Int64 instead in case provider specific error code doesn't fit in int?
    //Or even use void*?
    ECPERSISTENCE_EXPORT int GetLastError (WStringP lastErrorMessage = NULL) const;
    };

END_BENTLEY_EC_NAMESPACE
