/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPersistence/IECSchemaManager.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "Bentley/NonCopyableClass.h"
#include "ECObjects/ECSchema.h"
#include "ECPersistence/ECPersistence.h"

BEGIN_BENTLEY_EC_NAMESPACE

//*** WIP: Content of the IECSChemaManager is not finalized. The class added for the purpose
//of showing its integration in ECPersistence.

//=======================================================================================    
//! @ingroup ECPersistence
//! The IECSchemaManager is the central API for managing ECSchema information of an ECRepository.
//! @bsiclass                                                 Krischan.Eberle      08/2012
//=======================================================================================    
struct IECSchemaManager : NonCopyableClass
    {
    private:
        ECPERSISTENCE_EXPORT virtual BentleyStatus _ImportSchema (ECSchemaCR ecSchema) = 0;
        ECPERSISTENCE_EXPORT virtual BentleyStatus _GetClass (WCharCP schemaFullName, WCharCP className, ECClassP& ecClass) const = 0;
    public:
        ECPERSISTENCE_EXPORT virtual ~IECSchemaManager () {};

        ECPERSISTENCE_EXPORT BentleyStatus ImportSchema (ECSchemaCR ecSchema);

        //logical correctness (internal cache might have to be made mutable)
        ECPERSISTENCE_EXPORT BentleyStatus GetClass (WCharCP schemaFullName, WCharCP className, ECClassP& ecClass) const;
    };

END_BENTLEY_EC_NAMESPACE

