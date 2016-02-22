/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementTestsSchemaHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlStatementTestsSchemaHelper
    {
    private:
        static void setContactDetailsValues (StandaloneECInstancePtr instance, Utf8CP contactType, Utf8CP notes);

        static void setEmployeeValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, bool isContractual, std::vector<StandaloneECInstancePtr> contactDetailsArray);

        static void setCustomerValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, Utf8CP contactTitle, Utf8CP company, bool isRegular, std::vector<StandaloneECInstancePtr> contactDetailsArray);

        static void setOrderValues (StandaloneECInstancePtr instance, Utf8CP orderDate, Utf8CP releaseDate, Utf8CP shipCity, Utf8CP shipPostalCode);

        static void setShipperValues (StandaloneECInstancePtr instance, Utf8CP companyName, int64_t Phone);

        static void setSupplierValues (StandaloneECInstancePtr instance, Utf8CP companyName, Utf8CP contactName, Utf8CP contactTitle, Utf8CP City, Utf8CP Country, Utf8CP Address, int64_t phone);

        static void setProductsValues (StandaloneECInstancePtr instance, int ProductId, Utf8CP productName, double unitPrice, bool productAvailable);

        //Helper Methods to populate NestedStructArray Db
        static bvector<IECInstancePtr> CreateECInstanceWithOutStructArrayProperty (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);

        static void InsertRelationshipInstance (BeSQLite::EC::ECDbR ecdb, IECInstancePtr sourceInstance, IECInstancePtr targetInstance, ECRelationshipClassCP relClass);

        static bvector<IECInstancePtr> CreateECInstance_S4 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);

        static bvector<IECInstancePtr> CreateECInstance_S3 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);

        static bvector<IECInstancePtr> CreateECInstance_S2 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);

        static bvector<IECInstancePtr> CreateECInstance_S1 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);

    public:

        static void PopulateECSqlStatementTestsDb (BeSQLite::EC::ECDbR ecdb);

        //Populate NestedStructArray Db
        static Utf8CP const s_testSchemaXml;
        static bvector<IECInstancePtr> CreateECInstance (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className);
        static void PopulateNestedStructArrayDb (BeSQLite::EC::ECDbR ecdb, bool insertRelationships);
    };
