/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapFactory.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECDbMap.h"
#include "ClassMap.h"
#include "ClassMapInfo.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2014
//+===============+===============+===============+===============+===============+======
struct ClassMapFactory
    {
private:
    ClassMapFactory ();
    ~ClassMapFactory ();

    static ClassMapPtr CreateInstance (MapStatus& mapStatus, ClassMapInfoCR mapInfo, bool setIsDirty);

public:
    static ClassMapPtr Load (MapStatus& mapStatus, ECN::ECClassCR ecClass, ECDbMapCR ecdbMap);
    static ClassMapPtr Create (MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecdbMap);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
