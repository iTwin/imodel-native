/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader
    {
private:
    virtual DbResult _Upgrade(ECDbCR) const = 0;

public:
    virtual ~ECDbProfileUpgrader() {}
    DbResult Upgrade(ECDbCR ecdb) const { return _Upgrade(ecdb); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileECSchemaUpgrader
    {
private:
    ECDbProfileECSchemaUpgrader();
    ~ECDbProfileECSchemaUpgrader();

    static Utf8CP GetECDbSystemECSchemaXml();

    static BentleyStatus ReadECDbSystemSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadSchemaFromDisk(ECN::ECSchemaReadContextR readContext, ECN::SchemaKey&, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
