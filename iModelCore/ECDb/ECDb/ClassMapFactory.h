/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapFactory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

public:
    static ClassMapPtr Load (MapStatus&, ClassMapLoadContext& ctx, ECN::ECClassCR, ECDbMapCR);
    static ClassMapPtr Create (MapStatus&, SchemaImportContext&, ECN::ECClassCR, ECDbMapCR );
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
