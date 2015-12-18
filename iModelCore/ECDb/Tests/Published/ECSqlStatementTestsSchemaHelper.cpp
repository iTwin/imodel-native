#include "ECSqlStatementTestsSchemaHelper.h"

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setContactDetailsValues (StandaloneECInstancePtr instance, Utf8CP contactType, Utf8CP notes)
    {
    instance->SetValue ("ContactType", ECValue (contactType));
    instance->SetValue ("Notes", ECValue (notes));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setEmployeeValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, bool isContractual, std::vector<StandaloneECInstancePtr> contactDetailsArray)
    {
    instance->SetValue ("Phone", ECValue (phone));
    instance->SetValue ("City", ECValue (city));
    instance->SetValue ("Country", ECValue (country));
    instance->SetValue ("Address", ECValue (address));
    instance->SetValue ("PersonName.FirstName", ECValue (FirstName));
    instance->SetValue ("PersonName.LastName", ECValue (LastName));
    instance->SetValue ("IsContractual", ECValue (isContractual));
    instance->AddArrayElements ("ContactDetails", (uint32_t)contactDetailsArray.size ());
    int i = 0;
    for (StandaloneECInstancePtr contactDetailsInstance : contactDetailsArray)
        {
        ECValue structVal1;
        structVal1.SetStruct (contactDetailsInstance.get ());
        instance->SetValue ("ContactDetails", structVal1, i++);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setCustomerValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, Utf8CP contactTitle, Utf8CP company, bool isRegular, std::vector<StandaloneECInstancePtr> contactDetailsArray)
    {
    instance->SetValue ("Phone", ECValue (phone));
    instance->SetValue ("City", ECValue (city));
    instance->SetValue ("Country", ECValue (country));
    instance->SetValue ("Address", ECValue (address));
    instance->SetValue ("PersonName.FirstName", ECValue (FirstName));
    instance->SetValue ("PersonName.LastName", ECValue (LastName));
    instance->SetValue ("ContactTitle", ECValue (contactTitle));
    instance->SetValue ("Company", ECValue (company));
    instance->SetValue ("IsRegular", ECValue (isRegular));
    instance->AddArrayElements ("ContactDetails", (uint32_t)contactDetailsArray.size ());
    int i = 0;
    for (StandaloneECInstancePtr contactDetailsInstance : contactDetailsArray)
        {
        ECValue structVal1;
        structVal1.SetStruct (contactDetailsInstance.get ());
        instance->SetValue ("ContactDetails", structVal1, i++);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setOrderValues (StandaloneECInstancePtr instance, Utf8CP orderDate, Utf8CP releaseDate, Utf8CP shipCity, Utf8CP shipPostalCode)
    {
    instance->SetValue ("OrderDate", ECValue (orderDate));
    instance->SetValue ("ReleaseDate", ECValue (releaseDate));
    instance->SetValue ("ShipCity", ECValue (shipCity));
    instance->SetValue ("ShipPostalCode", ECValue (shipPostalCode));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setShipperValues (StandaloneECInstancePtr instance, Utf8CP companyName, int64_t Phone)
    {
    instance->SetValue ("CompanyName", ECValue (companyName));
    instance->SetValue ("Phone", ECValue (Phone));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setSupplierValues (StandaloneECInstancePtr instance, Utf8CP companyName, Utf8CP contactName, Utf8CP contactTitle, Utf8CP City, Utf8CP Country, Utf8CP Address, int64_t phone)
    {
    instance->SetValue ("CompanyName", ECValue (companyName));
    instance->SetValue ("ContactName", ECValue (contactName));
    instance->SetValue ("ContactTitle", ECValue (contactTitle));
    instance->SetValue ("City", ECValue (City));
    instance->SetValue ("Country", ECValue (Country));
    instance->SetValue ("Address", ECValue (Address));
    instance->SetValue ("Phone", ECValue (phone));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::setProductsValues (StandaloneECInstancePtr instance, int ProductId, Utf8CP productName, double unitPrice, bool productAvailable)
    {
    instance->SetValue ("ProductName", ECValue (productName));
    instance->SetValue ("UnitPrice", ECValue (unitPrice));
    instance->SetValue ("ProductAvailable", ECValue (productAvailable));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlStatementTestsSchemaHelper::Populate (BeSQLite::EC::ECDbR ecdb)
    {
    //Create Instances of ContactDetails class
    ECClassCP contactDetails = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "ContactDetails");
    ASSERT_TRUE (contactDetails != nullptr);

    StandaloneECInstancePtr contactInstance1 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance2 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance3 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance4 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance5 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance6 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance7 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr contactInstance8 = contactDetails->GetDefaultStandaloneEnabler ()->CreateInstance ();
    setContactDetailsValues (contactInstance1, "Email1", "Personal Email address");
    setContactDetailsValues (contactInstance2, "Email2", "My official Email address");
    setContactDetailsValues (contactInstance3, "Permanent Address", "5089 CALERO AVENUE");
    setContactDetailsValues (contactInstance4, "Phone Number", "692514354");
    setContactDetailsValues (contactInstance5, "Fax", "54564");
    setContactDetailsValues (contactInstance6, "Email Address", "my.this@gmail.com");
    setContactDetailsValues (contactInstance7, "Office Address", "5898 AERO AVENUE");
    setContactDetailsValues (contactInstance8, "Address 2", "8 FAYETTEVILLE ROAD");

    //Create instances of Employee and insert into Db
    ECClassCP employeeClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Employee");
    ASSERT_TRUE (employeeClass != nullptr);

    StandaloneECInstancePtr employeeInstance1 = employeeClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr employeeInstance2 = employeeClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr employeeInstance3 = employeeClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    std::vector<StandaloneECInstancePtr> arrayContactDetails;
    arrayContactDetails.push_back (contactInstance1);
    arrayContactDetails.push_back (contactInstance2);
    arrayContactDetails.push_back (contactInstance3);
    setEmployeeValues (employeeInstance1, 1253519, "CA", "USA", "5089 CALERO AVENUE", "Nancy", "Davolio", false, arrayContactDetails);

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance3);
    arrayContactDetails.push_back (contactInstance4);
    arrayContactDetails.push_back (contactInstance5);
    setEmployeeValues (employeeInstance2, 4654647, "NC", "USA", "198 FAYETTEVILLE ROAD", "Andrew", "Fuller", true, arrayContactDetails);

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance6);
    arrayContactDetails.push_back (contactInstance7);
    arrayContactDetails.push_back (contactInstance8);
    setEmployeeValues (employeeInstance3, 1245867, "MD", "USA", "1598 PICCARD DRIVE", "Janet", "Leverling", true, arrayContactDetails);

    BeSQLite::EC::ECInstanceInserter employeeInserter (ecdb, *employeeClass);
    ASSERT_TRUE (employeeInserter.IsValid ());
    ASSERT_EQ (SUCCESS, employeeInserter.Insert (*employeeInstance1, true));
    ASSERT_EQ (SUCCESS, employeeInserter.Insert (*employeeInstance2, true));
    ASSERT_EQ (SUCCESS, employeeInserter.Insert (*employeeInstance3, true));

    //Create instances of Customer and insert into Db
    ECClassCP customerClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Customer");
    ASSERT_TRUE (customerClass != nullptr);

    StandaloneECInstancePtr customerInstance1 = customerClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr customerInstance2 = customerClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr customerInstance3 = customerClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr customerInstance4 = customerClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance1);
    arrayContactDetails.push_back (contactInstance2);
    arrayContactDetails.push_back (contactInstance3);
    setCustomerValues (customerInstance1, 1400, "SAN JOSE", "USA", "44 PRINCESS GATE, HYDE PARK", "Charles", "Baron", "Brathion", "ABC", true, arrayContactDetails);

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance4);
    arrayContactDetails.push_back (contactInstance5);
    arrayContactDetails.push_back (contactInstance6);
    setCustomerValues (customerInstance2, 1500, "AUSTIN", "USA", "5063 RICHMOND MAL", "Gunther", "Spielmann", "SPIELMANN", "DEF", false, arrayContactDetails);

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance6);
    arrayContactDetails.push_back (contactInstance7);
    arrayContactDetails.push_back (contactInstance8);
    setCustomerValues (customerInstance3, 1600, "SAN JOSE", "USA", "3-2-7 ETCHUJMA, KOTO-KU", "A.D.M", "Bryceson", "Adm", "HIJ", true, arrayContactDetails);

    arrayContactDetails.clear ();
    arrayContactDetails.push_back (contactInstance1);
    arrayContactDetails.push_back (contactInstance4);
    arrayContactDetails.push_back (contactInstance8);
    setCustomerValues (customerInstance4, 1700, "ALASKA", "USA", "5063 ETCHUJMA, KT-KU", "A.D.M", "Bryceson", "AM", "KLM", true, arrayContactDetails);

    BeSQLite::EC::ECInstanceInserter customerInserter (ecdb, *customerClass);
    ASSERT_TRUE (customerInserter.IsValid ());
    ASSERT_EQ (SUCCESS, customerInserter.Insert (*customerInstance1));
    ASSERT_EQ (SUCCESS, customerInserter.Insert (*customerInstance2));
    ASSERT_EQ (SUCCESS, customerInserter.Insert (*customerInstance3));
    ASSERT_EQ (SUCCESS, customerInserter.Insert (*customerInstance4));

    //Create and Insert instance of Class Order
    ECClassCP orderClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Order");
    ASSERT_TRUE (orderClass != nullptr);

    StandaloneECInstancePtr orderInstance1 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance2 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance3 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance4 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance5 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance6 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance7 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance8 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr orderInstance9 = orderClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    setOrderValues (orderInstance1, "1/1/2015", "5/1/2015", "WC", "A-123");
    setOrderValues (orderInstance2, "2/1/2015", "5/1/2015", "RAW", "B-123");
    setOrderValues (orderInstance3, "6/1/2015", "10/1/2015", "WC", "C-123");
    setOrderValues (orderInstance4, "30/1/2015", "5/2/2015", "RAW", "D-123");
    setOrderValues (orderInstance5, "1/2/2015", "5/2/2015", "RAW", "A-123");
    setOrderValues (orderInstance6, "6/2/2015", "10/2/2015", "RAW", "A-123");
    setOrderValues (orderInstance7, "9/2/2015", "15/2/2015", "RAW", "B-123");
    setOrderValues (orderInstance8, "15/2/2015", "20/2/2015", "ISB", "C-123");
    setOrderValues (orderInstance9, "29/2/2015", "3/3/2015", "ISB", "D-123");

    BeSQLite::EC::ECInstanceInserter orderInserter (ecdb, *orderClass);
    ASSERT_TRUE (orderInserter.IsValid ());
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance1));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance2));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance3));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance4));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance5));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance6));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance7));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance8));
    ASSERT_EQ (SUCCESS, orderInserter.Insert (*orderInstance9));

    //Insert Relationship Instance CustomerHasOrder
    ECRelationshipClassCP customerHasOrder = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "CustomerHasOrder")->GetRelationshipClassCP ();
    ASSERT_TRUE (customerHasOrder != nullptr);

    StandaloneECRelationshipInstancePtr customerHasOrderInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*customerHasOrder)->CreateRelationshipInstance ();
    BeSQLite::EC::ECInstanceInserter customerHasOrderInserter (ecdb, *customerHasOrder);
    ASSERT_TRUE (customerHasOrderInserter.IsValid ());

    customerHasOrderInstance->SetSource (customerInstance1.get ());
    customerHasOrderInstance->SetTarget (orderInstance1.get ());
    customerHasOrderInstance->SetInstanceId ("source->target");
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance1.get ());
    customerHasOrderInstance->SetTarget (orderInstance2.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance1.get ());
    customerHasOrderInstance->SetTarget (orderInstance3.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance2.get ());
    customerHasOrderInstance->SetTarget (orderInstance4.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance2.get ());
    customerHasOrderInstance->SetTarget (orderInstance5.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance2.get ());
    customerHasOrderInstance->SetTarget (orderInstance6.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance3.get ());
    customerHasOrderInstance->SetTarget (orderInstance7.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance3.get ());
    customerHasOrderInstance->SetTarget (orderInstance8.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    customerHasOrderInstance->SetSource (customerInstance3.get ());
    customerHasOrderInstance->SetTarget (orderInstance9.get ());
    ASSERT_EQ (SUCCESS, customerHasOrderInserter.Insert (*customerHasOrderInstance));

    //Insert Relationship Instance EmployeeHasOrder
    ECRelationshipClassCP employeeHasOrder = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "EmployeeHasOrder")->GetRelationshipClassCP ();
    ASSERT_TRUE (employeeHasOrder != nullptr);

    StandaloneECRelationshipInstancePtr employeeHasOrderInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*employeeHasOrder)->CreateRelationshipInstance ();
    BeSQLite::EC::ECInstanceInserter employeeHasOrderInserter (ecdb, *employeeHasOrder);
    ASSERT_TRUE (employeeHasOrderInserter.IsValid ());

    employeeHasOrderInstance->SetSource (employeeInstance1.get ());
    employeeHasOrderInstance->SetTarget (orderInstance1.get ());
    employeeHasOrderInstance->SetInstanceId ("source->target");
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance1.get ());
    employeeHasOrderInstance->SetTarget (orderInstance2.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance1.get ());
    employeeHasOrderInstance->SetTarget (orderInstance3.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance2.get ());
    employeeHasOrderInstance->SetTarget (orderInstance4.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance2.get ());
    employeeHasOrderInstance->SetTarget (orderInstance5.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance2.get ());
    employeeHasOrderInstance->SetTarget (orderInstance6.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance3.get ());
    employeeHasOrderInstance->SetTarget (orderInstance7.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance3.get ());
    employeeHasOrderInstance->SetTarget (orderInstance8.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    employeeHasOrderInstance->SetSource (employeeInstance3.get ());
    employeeHasOrderInstance->SetTarget (orderInstance9.get ());
    ASSERT_EQ (SUCCESS, employeeHasOrderInserter.Insert (*employeeHasOrderInstance));

    //Create and Insert Instances of Shipper
    ECClassCP shipperClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Shipper");
    ASSERT_TRUE (shipperClass != nullptr);
    StandaloneECInstancePtr shipperInstance1 = shipperClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr shipperInstance2 = shipperClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr shipperInstance3 = shipperClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    setShipperValues (shipperInstance1, "Rio Grand", 19783482);
    setShipperValues (shipperInstance2, "Rue Perisnon", 26422096);
    setShipperValues (shipperInstance3, "Salguero", 39045213);
    BeSQLite::EC::ECInstanceInserter shipperInserter (ecdb, *shipperClass);
    ASSERT_TRUE (shipperInserter.IsValid ());
    ASSERT_EQ (SUCCESS, shipperInserter.Insert (*shipperInstance1, true));
    ASSERT_EQ (SUCCESS, shipperInserter.Insert (*shipperInstance2, true));
    ASSERT_EQ (SUCCESS, shipperInserter.Insert (*shipperInstance3, true));

    //Create and Insert Instances of Supplier
    ECClassCP supplierClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Supplier");
    ASSERT_TRUE (supplierClass != nullptr);
    StandaloneECInstancePtr supplierInstance1 = supplierClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr supplierInstance2 = supplierClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr supplierInstance3 = supplierClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    setSupplierValues (supplierInstance1, "ABCD", "John", "Snow", "CA", "USA", "5089 CALERO AVENUE", 1100);
    setSupplierValues (supplierInstance2, "Rio Grand", "Trion", "Lannistor", "NC", "USA", "198 FAYETTEVILLE ROAD", 1200);
    setSupplierValues (supplierInstance3, "GHIJ", "Stannis", "Brathion", "MD", "USA", "1598 PICCARD DRIVE", 1300);
    BeSQLite::EC::ECInstanceInserter supplierInserter (ecdb, *supplierClass);
    ASSERT_TRUE (supplierInserter.IsValid ());
    ASSERT_EQ (SUCCESS, supplierInserter.Insert (*supplierInstance1, true));
    ASSERT_EQ (SUCCESS, supplierInserter.Insert (*supplierInstance2, true));
    ASSERT_EQ (SUCCESS, supplierInserter.Insert (*supplierInstance3, true));

    //Create and Insert Instances of Products
    ECClassCP productClass = ecdb.Schemas ().GetECClass ("ECSqlStatementTests", "Product");
    ASSERT_TRUE (productClass != nullptr);
    StandaloneECInstancePtr productsInstance1 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance2 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance3 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance4 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance5 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance6 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance7 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance8 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    StandaloneECInstancePtr productsInstance9 = productClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    setProductsValues (productsInstance1, 1, "Pencil", 189.05, true);
    setProductsValues (productsInstance2, 2, "Binder", 999.50, true);
    setProductsValues (productsInstance3, 3, "Pen", 539.73, false);
    setProductsValues (productsInstance4, 4, "Binder", 299.40, true);
    setProductsValues (productsInstance5, 5, "Desk", 150.00, true);
    setProductsValues (productsInstance6, 6, "Pen Set", 255.84, false);
    setProductsValues (productsInstance7, 7, "Pen Set", 479.04, false);
    setProductsValues (productsInstance8, 8, "Pen", 539.73, true);
    setProductsValues (productsInstance9, 9, "Pen", 539.73, true);

    BeSQLite::EC::ECInstanceInserter productInserter (ecdb, *productClass);
    ASSERT_TRUE (productInserter.IsValid ());

    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance1, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance2, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance3, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance4, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance5, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance6, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance7, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance8, true));
    ASSERT_EQ (SUCCESS, productInserter.Insert (*productsInstance9, true));
    }