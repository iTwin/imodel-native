/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

typedef bmap<ECInstanceId, IECInstancePtr> ECInstanceMap;
typedef const ECInstanceMap& ECInstanceMapCR;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECDbTestProject
{
private:
    ECDb* m_ecdb;
    ECSchemaReadContextPtr m_schemaContext;
    std::map<ECClassCP, std::unique_ptr<ECInstanceInserter>> m_inserterCache;

private:
    void CreateEmpty (Utf8CP ecdbFileName);

public:
    ECDbTestProject(ECDb* = nullptr);
    ~ECDbTestProject();

    Utf8CP GetECDbPath() const {return GetECDbCR().GetDbFileName();}
    ECDb& GetECDb() {return *m_ecdb;}
    ECDb const& GetECDbCR() const {return *m_ecdb;}

    //! Creates an empty DgnDb file without importing a test schema and without inserting any instances.
    //! @return ECDb pointing to the creating DgnDb file
    ECDb& Create (Utf8CP ecdbFileName);
    ECDb& Create (Utf8CP ecdbFileName, Utf8CP testSchemaXmlFileName);
    DbResult Open (Utf8CP ecdbFileName, ECDb::OpenParams openParams = ECDb::OpenParams (ECDb::OpenMode::Readonly));
    DbResult ReOpen();
    BentleyStatus ImportECSchema(ECSchemaPtr& schema, Utf8CP testSchemaXmlFileName);
    BentleyStatus GetInstances (bvector<IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className, bool polymorphic = false);
    BentleyStatus InsertECInstance (ECInstanceKey& ecInstanceKey, IECInstancePtr ecInstance);
    IECInstancePtr CreateECInstance(ECClassCR ecClass);
    ECSchemaReadContextP GetSchemaContext() const {return m_schemaContext.get();}
    };


END_ECPRESENTATIONTESTS_NAMESPACE
