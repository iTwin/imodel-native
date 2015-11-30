/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

typedef bmap<ECInstanceId, ECN::IECInstancePtr> ECInstanceMap;
typedef const ECInstanceMap& ECInstanceMapCR;

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      04/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbTestProject
{
public:

private:
    BeSQLite::EC::ECDb*    m_ecdb;
    ECInstanceMap          m_ecInstances;
    std::map<ECN::ECClassCP, std::unique_ptr<ECInstanceInserter>> m_inserterCache;

    static bool            s_isInitialized;

    void                   CreateEmpty (Utf8CP ecdbFileName);
    BentleyStatus          ImportECSchema(ECN::ECSchemaPtr& schema, WCharCP testSchemaXmlFileName);
    void                   ImportArbitraryECInstances (ECN::ECSchemaCR, int numberOfECInstances);
    void                   ImportECInstance (ECN::IECInstancePtr ecInstance);

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
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, int numberOfArbitraryECInstancesToImport);
    BeSQLite::EC::ECDbR    Create (Utf8CP ecdbFileName, WCharCP testSchemaXmlFileName, bool importArbitraryECInstances);
    DbResult               ReOpen(BeSQLite::EC::ECDb::OpenParams openParams = BeSQLite::EC::ECDb::OpenParams(BeSQLite::EC::ECDb::OpenMode::Readonly));
    DbResult               Open (Utf8CP ecdbFilePath, BeSQLite::EC::ECDb::OpenParams openParams = BeSQLite::EC::ECDb::OpenParams (BeSQLite::EC::ECDb::OpenMode::Readonly));
    BeSQLite::EC::ECDbR    GetECDb ();
    BeSQLite::EC::ECDbCR   GetECDbCR () const;
    ECInstanceMapCR        GetImportedECInstances() const   {return m_ecInstances;}
    BentleyStatus          GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);
    BentleyStatus InsertECInstance (BeSQLite::EC::ECInstanceKey& ecInstanceKey, ECN::IECInstancePtr ecInstance);
 
    };


END_ECDBUNITTESTS_NAMESPACE