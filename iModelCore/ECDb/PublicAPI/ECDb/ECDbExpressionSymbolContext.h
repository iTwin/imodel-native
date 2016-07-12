/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbExpressionSymbolContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDb.h>
#include <ECObjects/ECExpressions.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Sets up ECDb ECExpression symbol provider that's available for the lifetime of this 
//! class object. The registered symbol provider provides such ECExpression symbols:
//! - ECDb.Path - Returns the path to ECDb.
//! - ECDb.Name - Returns the name of the ECDb.
//! - ECInstance ECExpression context methods:
//!   - GetRelatedInstance("RelationshipName:0|1:RelatedClassName") - Returns related ECInstance
//!     ECExpression context.
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolContext
{
private:
    ECN::IECSymbolProvider* m_provider;
public:
    //! Constructor. Registers the symbol provider for the specified ECDb.
    ECDB_EXPORT explicit ECDbExpressionSymbolContext(ECDbCR ecdb);

    //! Destructor. Unregisters the registered symbol provider.
    ~ECDbExpressionSymbolContext() { LeaveContext(); }

    //! Unregisters the registered symbol provider.
    ECDB_EXPORT void LeaveContext();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
