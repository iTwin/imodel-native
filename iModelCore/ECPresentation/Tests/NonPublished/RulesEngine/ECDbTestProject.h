/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ECDbTestProject.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/BackDoor/ECPresentation/ECPresentationTest.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

typedef bmap<BeSQLite::EC::ECInstanceId, ECN::IECInstancePtr> ECInstanceMap;
typedef const ECInstanceMap& ECInstanceMapCR;

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestProject
{
public:

private:
    BeSQLite::EC::ECDb*    m_ecdb;
    ECN::ECSchemaReadContextPtr m_schemaContext;
    std::map<ECN::ECClassCP, std::unique_ptr<BeSQLite::EC::ECInstanceInserter>> m_inserterCache;

    static bool            s_isInitialized;

    void                   CreateEmpty (Utf8CP ecdbFileName);
    BentleyStatus          ImportECSchema(ECN::ECSchemaPtr& schema, Utf8CP testSchemaXmlFileName);

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when constructing an ECDbTestProject, too. Tests that don't use
    //! the ECDbTestProject can call this method statically.
    static void            Initialize();
    static bool            IsInitialized();

public:
                           ECDbTestProject ();
                           ~ECDbTestProject ();
    
    Utf8CP                 GetECDbPath () const;

    //! Creates an empty DgnDb file without importing a test schema and without inserting any instances.
    //! @return ECDb pointing to the creating DgnDb file
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName);
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName, Utf8CP testSchemaXmlFileName);
    BeSQLite::DbResult     Open (Utf8CP ecdbFileName, BeSQLite::EC::ECDb::OpenParams openParams = BeSQLite::EC::ECDb::OpenParams (BeSQLite::EC::ECDb::OpenMode::Readonly));
    BeSQLite::EC::ECDbR    GetECDb ();
    BeSQLite::EC::ECDbCR   GetECDbCR () const;
    BentleyStatus          GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className, bool polymorphic = false);
    BentleyStatus InsertECInstance (BeSQLite::EC::ECInstanceKey& ecInstanceKey, ECN::IECInstancePtr ecInstance);
    ECN::IECInstancePtr CreateECInstance(ECN::ECClassCR ecClass);
    ECN::ECSchemaReadContextP GetSchemaContext() const {return m_schemaContext.get();}
    };


END_ECPRESENTATIONTESTS_NAMESPACE
