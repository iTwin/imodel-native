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

    public:

        static void Populate (BeSQLite::EC::ECDbR ecdb);

    };
