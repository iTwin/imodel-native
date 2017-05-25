/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/NestedStructArrayTestSchemaHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
struct NestedStructArrayTestSchemaHelper
    {
public:
    static Utf8CP const s_testSchemaXml;

private:
    static void setContactDetailsValues(ECN::StandaloneECInstancePtr instance, Utf8CP contactType, Utf8CP notes);
    static void setEmployeeValues(ECN::StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, bool isContractual, std::vector<ECN::StandaloneECInstancePtr> contactDetailsArray);
    static void setCustomerValues(ECN::StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, Utf8CP contactTitle, Utf8CP company, bool isRegular, std::vector<ECN::StandaloneECInstancePtr> contactDetailsArray);
    static void setOrderValues(ECN::StandaloneECInstancePtr instance, Utf8CP orderDate, Utf8CP releaseDate, Utf8CP shipCity, Utf8CP shipPostalCode);
    static void setShipperValues(ECN::StandaloneECInstancePtr instance, Utf8CP companyName, int64_t Phone);
    static void setSupplierValues(ECN::StandaloneECInstancePtr instance, Utf8CP companyName, Utf8CP contactName, Utf8CP contactTitle, Utf8CP City, Utf8CP Country, Utf8CP Address, int64_t phone);
    static void setProductsValues(ECN::StandaloneECInstancePtr instance, int ProductId, Utf8CP productName, double unitPrice, bool productAvailable);

    //Helper Methods to populate NestedStructArray Db
    static bvector<ECN::IECInstancePtr> CreateECInstanceWithOutStructArrayProperty(ECDbR ecdb, int n, Utf8CP className);
    static void InsertRelationshipInstance(ECDbR ecdb, ECN::IECInstancePtr sourceInstance, ECN::IECInstancePtr targetInstance, ECN::ECRelationshipClassCP);
    static bvector<ECN::IECInstancePtr> CreateECInstance_S4(ECDbR ecdb, int n, Utf8CP className);
    static bvector<ECN::IECInstancePtr> CreateECInstance_S3(ECDbR ecdb, int n, Utf8CP className);
    static bvector<ECN::IECInstancePtr> CreateECInstance_S2(ECDbR ecdb, int n, Utf8CP className);
    static bvector<ECN::IECInstancePtr> CreateECInstance_S1(ECDbR ecdb, int n, Utf8CP className);

public:

    static void PopulateECSqlStatementTestsDb(ECDbCR);

    //Populate NestedStructArray Db
    static bvector<ECN::IECInstancePtr> CreateECInstances(ECDbR ecdb, int n, Utf8CP className);
    static void PopulateNestedStructArrayDb(ECDbR ecdb, bool insertRelationships);
    };

END_ECDBUNITTESTS_NAMESPACE