/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/NestedStructArrayTestSchemaHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "NestedStructArrayTestSchemaHelper.h"

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::setContactDetailsValues (StandaloneECInstancePtr instance, Utf8CP contactType, Utf8CP notes)
    {
    instance->SetValue ("ContactType", ECValue (contactType));
    instance->SetValue ("Notes", ECValue (notes));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::setEmployeeValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, bool isContractual, std::vector<StandaloneECInstancePtr> contactDetailsArray)
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
void NestedStructArrayTestSchemaHelper::setCustomerValues (StandaloneECInstancePtr instance, int64_t phone, Utf8CP city, Utf8CP country, Utf8CP address, Utf8CP FirstName, Utf8CP LastName, Utf8CP contactTitle, Utf8CP company, bool isRegular, std::vector<StandaloneECInstancePtr> contactDetailsArray)
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
void NestedStructArrayTestSchemaHelper::setOrderValues (StandaloneECInstancePtr instance, Utf8CP orderDate, Utf8CP releaseDate, Utf8CP shipCity, Utf8CP shipPostalCode)
    {
    instance->SetValue ("OrderDate", ECValue (orderDate));
    instance->SetValue ("ReleaseDate", ECValue (releaseDate));
    instance->SetValue ("ShipCity", ECValue (shipCity));
    instance->SetValue ("ShipPostalCode", ECValue (shipPostalCode));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::setShipperValues (StandaloneECInstancePtr instance, Utf8CP companyName, int64_t Phone)
    {
    instance->SetValue ("CompanyName", ECValue (companyName));
    instance->SetValue ("Phone", ECValue (Phone));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::setSupplierValues (StandaloneECInstancePtr instance, Utf8CP companyName, Utf8CP contactName, Utf8CP contactTitle, Utf8CP City, Utf8CP Country, Utf8CP Address, int64_t phone)
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
void NestedStructArrayTestSchemaHelper::setProductsValues (StandaloneECInstancePtr instance, int ProductId, Utf8CP productName, double unitPrice, bool productAvailable)
    {
    instance->SetValue ("ProductName", ECValue (productName));
    instance->SetValue ("UnitPrice", ECValue (unitPrice));
    instance->SetValue ("ProductAvailable", ECValue (productAvailable));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb (BeSQLite::EC::ECDbR ecdb)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP const NestedStructArrayTestSchemaHelper::s_testSchemaXml =
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='NestedStructArrayTest' nameSpacePrefix='nsat' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "<ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca'/>"
    "   <ECEntityClass typeName='ClassA' >"
    "       <ECProperty propertyName='I' typeName='int' readOnly='false' />"
    "       <ECProperty propertyName='T' typeName='string' readOnly='false' />"
    "   </ECEntityClass>"
    "   <ECStructClass typeName = 'S4' modifier = 'None'>"
    "       <ECProperty propertyName = 'T' typeName = 'string' readOnly = 'false' />"
    "       <ECProperty propertyName = 'I' typeName = 'int' readOnly = 'false' />"
    "   </ECStructClass>"
    "   <ECStructClass typeName = 'S3' modifier = 'None'>"
    "       <ECProperty propertyName = 'T' typeName = 'string' readOnly = 'false' />"
    "       <ECProperty propertyName = 'I' typeName = 'int' readOnly = 'false' />"
    "       <ECStructArrayProperty propertyName = 'S4ARRAY' typeName = 'S4' readOnly = 'false' minOccurs = '0' maxOccurs = 'unbounded' />"
    "   </ECStructClass>"
    "   <ECStructClass typeName = 'S2' modifier = 'None'>"
    "       <ECProperty propertyName = 'T' typeName = 'string' readOnly = 'false' />"
    "       <ECProperty propertyName = 'I' typeName = 'int' readOnly = 'false' />"
    "       <ECStructArrayProperty propertyName = 'S3ARRAY' typeName = 'S3' readOnly = 'false' minOccurs = '0' maxOccurs = 'unbounded' />"
    "   </ECStructClass>"
    "   <ECStructClass typeName = 'S1' modifier = 'None'>"
    "       <ECProperty propertyName = 'T' typeName = 'string' readOnly = 'false' />"
    "       <ECProperty propertyName = 'I' typeName = 'int' readOnly = 'false' />"
    "       <ECStructArrayProperty propertyName = 'S2ARRAY' typeName = 'S2' readOnly = 'false' minOccurs = '0' maxOccurs = 'unbounded' />"
    "   </ECStructClass>"
    "   <ECEntityClass typeName = 'DerivedA' modifier = 'None'>"
    "       <BaseClass>ClassA</BaseClass>"
    "       <ECProperty propertyName = 'PropDerivedA' typeName = 'int' readOnly = 'false' />"
    "       <ECStructArrayProperty propertyName = 'S1ARRAY' typeName = 'S1' readOnly = 'false' minOccurs = '0' maxOccurs = 'unbounded' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName = 'DerivedB' modifier = 'None'>"
    "       <BaseClass>ClassA</BaseClass>"
    "       <ECProperty propertyName = 'PropDerivedB' typeName = 'int' readOnly = 'false' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName = 'DoubleDerivedA' modifier = 'None'>"
    "       <BaseClass>DerivedB</BaseClass>"
    "       <ECProperty propertyName = 'PropDoubleDerivedA' typeName = 'int' readOnly = 'false' />"
    "       <ECStructArrayProperty propertyName = 'S1ARRAY' typeName = 'S1' readOnly = 'false' minOccurs = '0' maxOccurs = 'unbounded' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName = 'DoubleDerivedB' modifier = 'None'>"
    "       <BaseClass>DerivedB</BaseClass>"
    "       <ECProperty propertyName = 'PropDoubleDerivedB' typeName = 'int' readOnly = 'false' />"
    "   </ECEntityClass>"
    "   <ECEntityClass typeName = 'DoubleDerivedC' modifier = 'None'>"
    "       <BaseClass>DerivedA</BaseClass>"
    "       <ECProperty propertyName = 'PropDoubleDerivedC' typeName = 'int' readOnly = 'false' />"
    "   </ECEntityClass>"
    "</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstance_S4 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    ECClassCP s4 = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "S4");
    EXPECT_TRUE (s4 != nullptr);

    Utf8String stringValue;
    stringValue.Sprintf ("testData_S4_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = s4->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;
        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstance_S3 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    int m = n + 1;
    ECClassCP s3 = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "S3");
    EXPECT_TRUE (s3 != nullptr);

    Utf8String stringValue;
    stringValue.Sprintf ("testData_S3_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = s3->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->AddArrayElements ("S4ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S4 (ecdb, m, className))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("S4ARRAY", elmV, v++)) << "Set Struct Value failed for " << className;
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstance_S2 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    int m = n + 1;
    ECClassCP s2 = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "S2");
    EXPECT_TRUE (s2 != nullptr);

    Utf8String stringValue;
    stringValue.Sprintf ("testData_S2_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = s2->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->AddArrayElements ("S3ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S3 (ecdb, m, className))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("S3ARRAY", elmV, v++)) << "Set Struct Value failed for " << className;
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstance_S1 (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    int m = n + 1;
    ECClassCP s1 = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "S1");
    EXPECT_TRUE (s1 != nullptr);

    Utf8String stringValue;
    stringValue.Sprintf ("testData_S1_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = s1->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->AddArrayElements ("S2ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S2 (ecdb, m, className))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("S2ARRAY", elmV, v++)) << "Set Struct Value failed for " << className;
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstances (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    int m = n + 1;
    ECClassCP ecClassCP = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", className);
    EXPECT_TRUE (ecClassCP != nullptr);
    Utf8String stringValue;
    stringValue.Sprintf ("testData_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = ecClassCP->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->AddArrayElements ("S1ARRAY", m));
        int v = 0;
        for (auto elm : CreateECInstance_S1 (ecdb, m, className))
            {
            ECValue elmV;
            elmV.SetStruct (elm.get ());
            EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("S1ARRAY", elmV, v++)) << "Set Struct Value failed for " << className;
            }

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECInstancePtr> NestedStructArrayTestSchemaHelper::CreateECInstanceWithOutStructArrayProperty (BeSQLite::EC::ECDbR ecdb, int n, Utf8CP className)
    {
    ECClassCP ecClassCP = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", className);
    EXPECT_TRUE (ecClassCP != nullptr);
    Utf8String stringValue;
    stringValue.Sprintf ("testData_%s", className);

    bvector<IECInstancePtr> vect;
    for (int j = 0; j < n; j++)
        {
        StandaloneECInstancePtr inst = ecClassCP->GetDefaultStandaloneEnabler ()->CreateInstance ();
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("I", ECValue (j))) << "Set Int Value failed for " << className;
        EXPECT_TRUE (ECObjectsStatus::Success == inst->SetValue ("T", ECValue (stringValue.c_str ()))) << "Set String Value failed for " << className;

        vect.push_back (inst);
        }

    return vect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::InsertRelationshipInstance (BeSQLite::EC::ECDbR ecdb, IECInstancePtr sourceInstance, IECInstancePtr targetInstance, ECRelationshipClassCP relClass)
    {
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass)->CreateRelationshipInstance ();
    BeSQLite::EC::ECInstanceInserter relationshipInserter (ecdb, *relClass);
    relationshipInstance->SetSource (sourceInstance.get ());
    relationshipInstance->SetTarget (targetInstance.get ());
    relationshipInstance->SetInstanceId ("source->target");
    EXPECT_EQ (SUCCESS, relationshipInserter.Insert (*relationshipInstance, true));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb (BeSQLite::EC::ECDbR ecdb, bool insertRelationships)
    {
    //Insert Instances for each class in the Hierarchy Seperately.
    bvector<IECInstancePtr> instances = CreateECInstanceWithOutStructArrayProperty (ecdb, 1, "ClassA");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter sourceInserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (sourceInserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, sourceInserter.Insert (*instance, true));
        }

    instances = CreateECInstances (ecdb, 1, "DerivedA");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter inserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (inserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, inserter.Insert (*instance, true));
        }

    instances = CreateECInstanceWithOutStructArrayProperty (ecdb, 1, "DerivedB");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter inserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (inserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, inserter.Insert (*instance, true));
        }

    instances = CreateECInstanceWithOutStructArrayProperty (ecdb, 1, "DoubleDerivedB");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter inserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (inserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, inserter.Insert (*instance, true));
        }

    instances = CreateECInstances (ecdb, 1, "DoubleDerivedA");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter inserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (inserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, inserter.Insert (*instance, true));
        }

    instances = CreateECInstances (ecdb, 1, "DoubleDerivedC");
    for (auto instance : instances)
        {
        BeSQLite::EC::ECInstanceInserter inserter (ecdb, instance->GetClass ());
        ASSERT_TRUE (inserter.IsValid ());
        ASSERT_EQ (BentleyStatus::SUCCESS, inserter.Insert (*instance, true));
        }

    //Create and Insert Constraint Classes Instances and then for relationship class BaseHasDerivedA
    if (insertRelationships)
        {
        ECRelationshipClassCP baseHasDerivedA = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "BaseHasDerivedA")->GetRelationshipClassCP ();

        instances = CreateECInstanceWithOutStructArrayProperty (ecdb, 1, "ClassA");
        for (auto sourceInstance : instances)
            {
            BeSQLite::EC::ECInstanceInserter sourceInserter (ecdb, sourceInstance->GetClass ());
            ASSERT_TRUE (sourceInserter.IsValid ());
            ASSERT_EQ (BentleyStatus::SUCCESS, sourceInserter.Insert (*sourceInstance, true));

            bvector<IECInstancePtr> targetInstances = CreateECInstances (ecdb, 2, "DerivedA");
            for (auto targetInstance : targetInstances)
                {
                BeSQLite::EC::ECInstanceInserter targetInserter (ecdb, targetInstance->GetClass ());
                ASSERT_TRUE (targetInserter.IsValid ());
                ASSERT_EQ (BentleyStatus::SUCCESS, targetInserter.Insert (*targetInstance, true));
                InsertRelationshipInstance (ecdb, sourceInstance, targetInstance, baseHasDerivedA);
                }
            }

        //Create and Insert Constraint Classes Instances and then for relationship class DerivedBOwnsChilds, relationship contains multiple target classes also containing structArray properties.
        ECRelationshipClassCP derivedBHasChildren = ecdb.Schemas ().GetECClass ("NestedStructArrayTest", "DerivedBHasChildren")->GetRelationshipClassCP ();
        instances = CreateECInstanceWithOutStructArrayProperty (ecdb, 1, "DerivedB");
        for (IECInstancePtr sourceInstance : instances)
            {
            BeSQLite::EC::ECInstanceInserter sourceInserter (ecdb, sourceInstance->GetClass ());
            ASSERT_TRUE (sourceInserter.IsValid ());
            ASSERT_EQ (BentleyStatus::SUCCESS, sourceInserter.Insert (*sourceInstance, true));

            bvector<IECInstancePtr> targetInstances = CreateECInstances (ecdb, 2, "DoubleDerivedA");
            for (IECInstancePtr targetInstance : targetInstances)
                {
                BeSQLite::EC::ECInstanceInserter targetInserter (ecdb, targetInstance->GetClass ());
                ASSERT_TRUE (targetInserter.IsValid ());
                ASSERT_EQ (BentleyStatus::SUCCESS, targetInserter.Insert (*targetInstance, true));
                InsertRelationshipInstance (ecdb, sourceInstance, targetInstance, derivedBHasChildren);
                }
            }
        }

    ecdb.SaveChanges ();
    }
