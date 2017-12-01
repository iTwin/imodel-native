/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ProfileUpgrader.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader
    {
private:
    virtual DbResult _Upgrade(ECDbCR) const = 0;

protected:
    ProfileUpgrader() {}

public:
    virtual ~ProfileUpgrader() {}
    DbResult Upgrade(ECDbCR ecdb) const { return _Upgrade(ecdb); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle     10/2017
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader_4001 final : ProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileSchemaUpgrader final
    {
private:
    ProfileSchemaUpgrader();
    ~ProfileSchemaUpgrader();

    static Utf8CP GetECDbSystemSchemaXml();

    static BentleyStatus ReadECDbSystemSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadSchemaFromDisk(ECN::ECSchemaReadContextR readContext, ECN::SchemaKey&, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
