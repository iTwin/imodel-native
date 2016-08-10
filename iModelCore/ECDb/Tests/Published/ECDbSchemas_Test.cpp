/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemas_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbSchemaTests : public ECDbTestFixture
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, OrderOfPropertyIsPreservedInTableColumns)
    {
    SetupECDb("propertyOrderTest.ecdb", SchemaItem("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                                   "<ECSchema schemaName=\"OrderSchema\" nameSpacePrefix=\"os\" version=\"1.0\" xmlns = \"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                                   "  <ECStructClass typeName=\"OrderedStruct\">"
                                                   "   <ECProperty propertyName=\"a\" typeName=\"string\"/>"
                                                   "	 <ECProperty propertyName=\"g\" typeName=\"int\"/>"
                                                   "	 <ECProperty propertyName=\"c\" typeName=\"dateTime\"/>"
                                                   "   <ECProperty propertyName=\"z\" typeName=\"point3d\"/>"
                                                   "	 <ECProperty propertyName=\"y\" typeName=\"point2d\"/>"
                                                   "	 <ECProperty propertyName=\"t\" typeName=\"boolean\"/>"
                                                   "   <ECProperty propertyName=\"u\" typeName=\"double\"/>"
                                                   "	 <ECProperty propertyName=\"k\" typeName=\"string\"/>"
                                                   "	 <ECProperty propertyName=\"r\" typeName=\"string\"/>"
                                                   "  </ECStructClass>"
                                                   "  <ECEntityClass typeName=\"PropertyOrderTest\" >"
                                                   "   <ECProperty propertyName=\"x\" typeName=\"string\"/>"
                                                   "	 <ECProperty propertyName=\"h\" typeName=\"int\"/>"
                                                   "	 <ECProperty propertyName=\"i\" typeName=\"dateTime\"/>"
                                                   "   <ECProperty propertyName=\"d\" typeName=\"point3d\"/>"
                                                   "	 <ECProperty propertyName=\"u\" typeName=\"point2d\"/>"
                                                   "	 <ECProperty propertyName=\"f\" typeName=\"boolean\"/>"
                                                   "	 <ECStructArrayProperty propertyName=\"sarray\" typeName=\"OrderedStruct\"/>"
                                                   "   <ECProperty propertyName=\"e\" typeName=\"double\"/>"
                                                   "	 <ECProperty propertyName=\"p\" typeName=\"string\"/>"
                                                   "	 <ECStructProperty propertyName=\"o\" typeName=\"OrderedStruct\"/>"
                                                   "	 <ECProperty propertyName=\"z\" typeName=\"long\"/>"
                                                   "  </ECEntityClass>"
                                                   "</ECSchema>"));

    ASSERT_TRUE(GetECDb().IsDbOpen());

    Statement statement;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, statement.Prepare(GetECDb(), "PRAGMA table_info('os_PropertyOrderTest')"));
    Utf8String order_PropertyOrderTest;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        order_PropertyOrderTest.append(statement.GetValueText(1)).append(" ");
        }

    ASSERT_STREQ("ECInstanceId x h i d_X d_Y d_Z u_X u_Y f sarray e p o_a o_g o_c o_z_X o_z_Y o_z_Z o_y_X o_y_Y o_t o_u o_k o_r z ", order_PropertyOrderTest.c_str());
    ASSERT_FALSE(GetECDb().TableExists("os_OrderedStruct"));
    }

//---------------------------------------------------------------------------------------
// @bsimethods                                     Affan.Khan                  05/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, ValidateSchemaUsingSqliteQuery)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_OK, stat);

    Utf8CP sql = "SELECT [Element].[ElementId] FROM (SELECT NULL ECClassId, NULL ECInstanceId, NULL [ElementId] LIMIT 0) Element ";
    // Validate the expected ECSchemas in the project
    Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(db, sql));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, LoadECSchemas)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    saveTestProject.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    // Reopen the test project
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(saveTestProject.GetECDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly));
    EXPECT_EQ(BE_SQLITE_OK, stat);

    std::vector<Utf8CP> expectedSchemas;
    expectedSchemas.push_back("Bentley_Standard_CustomAttributes");
    expectedSchemas.push_back("CoreCustomAttributes");
    expectedSchemas.push_back("ECDb_FileInfo");
    expectedSchemas.push_back("ECDb_System");
    expectedSchemas.push_back("ECDbMap");
    expectedSchemas.push_back("EditorCustomAttributes");
    expectedSchemas.push_back("MetaSchema");
    expectedSchemas.push_back("StartupCompany");

    // Validate the expected ECSchemas in the project
    Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(db, "SELECT Name FROM ec_Schema ORDER BY Name"));
    int i = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_STREQ(expectedSchemas[i], stmt.GetValueText(0));
        i++;
        }

    ASSERT_EQ(expectedSchemas.size(), i);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetColumnCount(DbR db, Utf8CP table)
    {
    Statement stmt;
    stmt.Prepare(db, SqlPrintfString("SELECT * FROM %s LIMIT 1", table));
    return stmt.GetColumnCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, VerifyDatabaseSchemaAfterImport)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR db = test.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
    //========================[sc_ClassWithPrimitiveProperties===================================

    Utf8CP tblClassWithPrimitiveProperties = "sc_ClassWithPrimitiveProperties";
    EXPECT_TRUE(db.TableExists(tblClassWithPrimitiveProperties));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "ECInstanceId"));
    EXPECT_EQ(13, GetColumnCount(db, tblClassWithPrimitiveProperties));
    //Verify columns columns in this class is renamed to 
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_intProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_longProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_doubleProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_stringProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_dateTimeProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_binaryProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_booleanProp"));
    //point2Prop is stored as x,y 2 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point2dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point2dProp_Y"));
    //point3Prop is stored as x,y,z 3 columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_X"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_Y"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveProperties, "myColumn_point3dProp_Z"));

    //========================[StructWithPrimitiveProperties==================================
    EXPECT_FALSE(db.TableExists("sc_StructWithPrimitiveProperties"));

    //========================[sc_ClassWithPrimitiveArrayProperties==============================
    //Array properties doesnt take any column currently it will take in case of embeded senario but
    //we need to make sure it doesnt exist right now. They uses special System arrray tables 
    Utf8CP tblClassWithPrimitiveArrayProperties = "sc_ClassWithPrimitiveArrayProperties";
    EXPECT_TRUE(db.TableExists(tblClassWithPrimitiveArrayProperties));
    EXPECT_EQ(10, GetColumnCount(db, tblClassWithPrimitiveArrayProperties));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "ECInstanceId"));

    //Verify columns
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "intArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "longArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "doubleArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "stringArrayProp"));// MapStrategy=Blob
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "dateTimeArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "binaryArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "booleanArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point2dArrayProp"));
    EXPECT_TRUE(db.ColumnExists(tblClassWithPrimitiveArrayProperties, "point3dArrayProp")); // MapStrategy=Blob

    //========================[StructWithPrimitiveArrayProperties=============================
    EXPECT_FALSE(db.TableExists("sc_StructWithPrimitiveArrayProperties"));

    //verify system array tables. They are created if  a primitive array property is ecounter in schema
    //========================[sc_Asset]=========================================================
    //baseClass
    Utf8CP tblAsset = "sc_Asset";
    EXPECT_TRUE(db.TableExists(tblAsset));
    EXPECT_EQ(34, GetColumnCount(db, tblAsset));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ECInstanceId"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetID"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetOwner"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "BarCode"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetUserID"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Cost"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Room"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "AssetRecordKey"));
    //Local properties of Furniture   
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Condition"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Material"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    // Properties of Chair which is derived from Furniture
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ChairFootPrint"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Color"));

    // Properties of Desk which is derived from Furniture    
    EXPECT_TRUE(db.ColumnExists(tblAsset, "DeskFootPrint"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "NumberOfCabinets"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Size"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Breadth"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Length"));
    //relation keys
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ForeignECInstanceId_stco_EmployeePhone"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "ForeignECInstanceId_stco_EmployeeFurniture"));

    EXPECT_TRUE(db.ColumnExists(tblAsset, "HasWarranty"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "IsCompanyProperty"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Make"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Model"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "WarrantyExpiryDate"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Vendor"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Size"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Type"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Vendor"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Weight"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Number"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "Owner"));
    EXPECT_TRUE(db.ColumnExists(tblAsset, "User"));

    //========================[sc_Employee]======================================================
    //Related to Furniture. Employee can have one or more furniture
    Utf8CP tblEmployee = "sc_Employee";
    EXPECT_TRUE(db.TableExists(tblEmployee));
    EXPECT_EQ(32, GetColumnCount(db, tblEmployee));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "ECInstanceId"));

    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeID"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "FirstName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "JobTitle"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "LastName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "ManagerID"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Room"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "SSN"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Project"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "FullName"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeType"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "EmployeeRecordKey"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Company__trg_11_id"));
    EXPECT_TRUE(db.ColumnExists(tblEmployee, "Certifications"));

    //========================[sc_Company]=======================================================
    Utf8CP tblCompany = "sc_Company";
    EXPECT_TRUE(db.TableExists(tblCompany));
    EXPECT_EQ(15, GetColumnCount(db, tblCompany));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "ECInstanceId"));

    EXPECT_TRUE(db.ColumnExists(tblCompany, "Name"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "NumberOfEmployees"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "ContactAddress"));
    EXPECT_TRUE(db.ColumnExists(tblCompany, "RecordKey"));

    //======================== EmployeeCertifications========================================
    EXPECT_FALSE(db.TableExists("sc_EmployeeCertification")) << "struct don't get a table";

    //========================[sc_Widget]========================================================
    Utf8CP tblWidget = "sc_Widget";
    EXPECT_TRUE(db.TableExists(tblWidget));
    EXPECT_EQ(3, GetColumnCount(db, tblWidget));

    EXPECT_TRUE(db.ColumnExists(tblWidget, "ECInstanceId"));
    EXPECT_TRUE(db.ColumnExists(tblWidget, "stringOfWidget"));

    //========================[sc_Project]=======================================================
    Utf8CP tblProject = "sc_Project";
    EXPECT_TRUE(db.TableExists(tblProject));
    EXPECT_EQ(14, GetColumnCount(db, tblProject));

    EXPECT_TRUE(db.ColumnExists(tblProject, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblProject, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblProject, "CompletionDate"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "EstimatedCost"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectName"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectDescription"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectState"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "StartDate"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "InProgress"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "TeamSize"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "Logo"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "Manager"));
    EXPECT_TRUE(db.ColumnExists(tblProject, "ProjectRecordKey"));
    //struct/arrays mapped to table
    EXPECT_TRUE(db.ColumnExists(tblProject, "TeamMemberList"));  //int array
    //relation
    EXPECT_TRUE(db.ColumnExists(tblProject, "Company__src_11_id"));

    //========================[sc_Building]======================================================
    Utf8CP tblBuilding = "sc_Building";
    EXPECT_TRUE(db.TableExists(tblBuilding));
    EXPECT_EQ(14, GetColumnCount(db, tblBuilding));

    EXPECT_TRUE(db.ColumnExists(tblBuilding, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblBuilding, "Number"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "Name"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "NumberOfFloors"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuilding, "RecordKey"));
    //struct array
    EXPECT_FALSE(db.ColumnExists(tblBuilding, "Location"));

    //========================[Location]======================================================
    EXPECT_FALSE(db.TableExists("sc_Location")) << "no tables for structs";

    //========================[sc_BuildingFloor]=================================================
    Utf8CP tblBuildingFloor = "sc_BuildingFloor";
    EXPECT_TRUE(db.TableExists(tblBuildingFloor));
    EXPECT_EQ(8, GetColumnCount(db, tblBuildingFloor));

    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblBuildingFloor, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "FloorNumber"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "NumberOfOffices"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "Area"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "FloorCode"));
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "RecordKey"));
    //relation
    EXPECT_TRUE(db.ColumnExists(tblBuildingFloor, "Building__src_11_Id"));

    //========================[sc_Cubicle]=================================================
    Utf8CP tblCubicle = "sc_Cubicle";
    EXPECT_TRUE(db.TableExists(tblCubicle));
    EXPECT_EQ(13, GetColumnCount(db, tblCubicle));

    EXPECT_TRUE(db.ColumnExists(tblCubicle, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblCubicle, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Bay"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "IsOccupied"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "BuildingFloor"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Length"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Breadth"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "NumberOfOccupants"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "BuildingCode"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "OfficeCode"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "Area"));
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "RecordKey"));
    //array    
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "OccupiedBy"));
    //relation
    EXPECT_TRUE(db.ColumnExists(tblCubicle, "BuildingFloor__src_11_id"));

    //========================AnglesStruct======================================================
    EXPECT_FALSE(db.TableExists("sc_AnglesStruct")) << "structs are not mapped to any tables";

    //========================[sc_ABFoo]======================================================
    Utf8CP tblABFoo = "sc_ABFoo";
    EXPECT_TRUE(db.TableExists(tblABFoo));
    EXPECT_EQ(2, GetColumnCount(db, tblABFoo));

    EXPECT_TRUE(db.ColumnExists(tblABFoo, "ECInstanceId"));
    //It must not have ECClassId to differentiate each row to see which class it belong to.
    EXPECT_FALSE(db.ColumnExists(tblABFoo, "ECClassId"));

    EXPECT_TRUE(db.ColumnExists(tblABFoo, "stringABFoo"));

    //========================[sc_AAFoo]=========================================================
    Utf8CP tblAAFoo = "sc_AAFoo";
    EXPECT_TRUE(db.TableExists(tblAAFoo));
    EXPECT_FALSE(db.TableExists("AFooChild")); //This child class of AAFoo which have TablePerHierarchy so table for its child classes should not be created
    EXPECT_EQ(26, GetColumnCount(db, tblAAFoo));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "ECClassId"));

    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "FooTag"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "intAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "longAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "stringAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "doubleAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "datetimeAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "binaryAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "booleanAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point2dAAFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point2dAAFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "point3dAAFoo_Z"));
    //EXPECT_TRUE (db.ColumnExists(tblAAFoo, "anglesAAFoo")); // we are no longer stuffing structs into blobs
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "commonGeometryAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "colorAAFoo"));

    // arrays
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfIntsAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfpoint2dAAFoo"));
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "arrayOfpoint3dAAFoo"));

    //From ABFoo since its one of the base class of child class AFooChild
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "stringABFoo"));
    //From AFooChild which is child of AAFoo
    EXPECT_TRUE(db.ColumnExists(tblAAFoo, "binaryAFooChild"));

    //========================[sc_Bar]===========================================================
    Utf8CP tblBar = "sc_Bar";
    EXPECT_TRUE(db.TableExists(tblBar));
    EXPECT_EQ(4, GetColumnCount(db, tblBar));
    EXPECT_TRUE(db.ColumnExists(tblBar, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_FALSE(db.ColumnExists(tblBar, "ECClassId"));
    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblBar, "stringBar"));
    //Relations
    EXPECT_TRUE(db.ColumnExists(tblBar, "ForeignECInstanceId_stco_Foo_has_Bars"));
    EXPECT_TRUE(db.ColumnExists(tblBar, "ForeignECInstanceId_stco_Foo_has_Bars_hint"));

    //========================[sc_Foo]===========================================================
    Utf8CP tblFoo = "sc_Foo";
    EXPECT_TRUE(db.TableExists(tblFoo));
    EXPECT_EQ(20, GetColumnCount(db, tblFoo));

    EXPECT_TRUE(db.ColumnExists(tblFoo, "ECInstanceId"));
    //This a TablePerHieracrchy
    EXPECT_TRUE(db.ColumnExists(tblFoo, "ECClassId"));

    //Local properties
    EXPECT_TRUE(db.ColumnExists(tblFoo, "intFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "longFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "stringFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "doubleFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "datetimeFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "binaryFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "booleanFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point2dFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point2dFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_X"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_Y"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "point3dFoo_Z"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "commonGeometryFoo"));

    // arrays/struct
    EXPECT_TRUE(db.ColumnExists(tblFoo, "arrayOfIntsFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "arrayOfAnglesStructsFoo"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Alpha"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Beta"));
    EXPECT_TRUE(db.ColumnExists(tblFoo, "anglesFoo_Theta"));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                            Muhammad Hassan                        10/15
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, ImportECSchemaWithSameVersionAndSameContentTwice)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    auto schemaStatus = db.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Muhammad Hassan                        10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, ImportMultipleSchemasInSameECDb)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("MultipleSchemas.ecdb", L"BaseSchemaA.01.00.ecschema.xml", false);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"SchoolSchema.01.00.ecschema.xml");
    auto schemaStatus = db.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"TestSchema.01.00.ecschema.xml");
    schemaStatus = db.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, IntegrityCheck)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("IntegrityCheck.ecdb", L"IntegrityCheck.01.00.ecschema.xml", true);
    Statement stmt;
    std::map<Utf8String, Utf8String> expected;
    expected["ic_TargetBase"] = "CREATE TABLE [ic_TargetBase]([ECInstanceId] INTEGER PRIMARY KEY, [ECClassId] INTEGER NOT NULL, [I] INTEGER, [S] TEXT, [SourceECInstanceId] INTEGER NOT NULL, FOREIGN KEY([SourceECInstanceId]) REFERENCES [ic_SourceBase]([ECInstanceId]) ON DELETE CASCADE ON UPDATE NO ACTION)";
    stmt.Prepare(db, "select name, sql from sqlite_master Where type='table' AND tbl_name = 'ic_TargetBase'");
    int nRows = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        nRows = nRows + 1;
        Utf8String name = stmt.GetValueText(0);
        Utf8String sql = stmt.GetValueText(1);
        auto itor = expected.find(name);
        if (itor == expected.end())
            {
            ASSERT_FALSE(true) << "Failed to find expected value [name=" << name << "]";
            }
        if (itor->second != sql)
            {
            ASSERT_FALSE(true) << "SQL def for  [name=" << name << "] has changed \r\n Expected :" << itor->second.c_str() << "\r\n Actual : " << sql.c_str();
            }
        }

    ASSERT_EQ(nRows, expected.size()) << "Number of SQL definitions are not same";
    }

//-------------------------------------------------------------------------------------
// <author>Carole.MacDonald</author>                     <date>06/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
TEST_F(ECDbSchemaTests, CreateCloseOpenImport)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create("importecschema.ecdb");
    Utf8String filename = ecdb.GetDbFileName();
    ecdb.CloseDb();

    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(filename.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ(BE_SQLITE_OK, stat);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(ecSchema, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(schemaContext->GetCache())) << "ImportECSchema should have imported successfully after closing and re-opening the database.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaCachePtr CreateImportSchemaAgainstExistingTablesTestSchema()
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchema::CreateSchema(testSchema, "test", 1, 0);
    testSchema->SetAlias("t");
    ECEntityClassP fooClass = nullptr;
    testSchema->CreateEntityClass(fooClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    fooClass->CreatePrimitiveProperty(prop, "Name", PRIMITIVETYPE_String);

    ECEntityClassP gooClass = nullptr;
    testSchema->CreateEntityClass(gooClass, "Goo");
    prop = nullptr;
    gooClass->CreatePrimitiveProperty(prop, "Price", PRIMITIVETYPE_Double);

    ECRelationshipClassP oneToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass(oneToManyRelClass, "FooHasGoo");
    oneToManyRelClass->SetStrength(StrengthType::Holding);
    oneToManyRelClass->GetSource().AddClass(*fooClass);
    oneToManyRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    oneToManyRelClass->GetTarget().AddClass(*gooClass);
    oneToManyRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());

    ECRelationshipClassP manyToManyRelClass = nullptr;
    testSchema->CreateRelationshipClass(manyToManyRelClass, "RelFooGoo");
    manyToManyRelClass->SetStrength(StrengthType::Referencing);
    manyToManyRelClass->GetSource().AddClass(*fooClass);
    manyToManyRelClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    manyToManyRelClass->GetTarget().AddClass(*gooClass);
    manyToManyRelClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());

    auto schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*testSchema);

    return schemaCache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertImportedSchema(DbR ecdb, Utf8CP expectedSchemaName, Utf8CP expectedClassName, Utf8CP expectedPropertyName)
    {
    CachedStatementPtr findClassStmt = nullptr;
    ecdb.GetCachedStatement(findClassStmt, "SELECT NULL FROM ec_Class c, ec_Schema s WHERE c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? LIMIT 1");
    findClassStmt->BindText(1, expectedSchemaName, Statement::MakeCopy::No);
    findClassStmt->BindText(2, expectedClassName, Statement::MakeCopy::No);
    EXPECT_EQ(BE_SQLITE_ROW, findClassStmt->Step()) << "ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Class table.";

    if (expectedPropertyName != nullptr)
        {
        CachedStatementPtr findPropertyStmt = nullptr;
        ecdb.GetCachedStatement(findPropertyStmt, "SELECT NULL FROM ec_Property p, ec_Class c, ec_Schema s WHERE p.ClassId = c.Id AND c.SchemaId = s.Id AND s.Name = ? AND c.Name = ? AND p.Name = ? LIMIT 1");
        findPropertyStmt->BindText(1, expectedSchemaName, Statement::MakeCopy::No);
        findPropertyStmt->BindText(2, expectedClassName, Statement::MakeCopy::No);
        findPropertyStmt->BindText(3, expectedPropertyName, Statement::MakeCopy::No);
        EXPECT_EQ(BE_SQLITE_ROW, findPropertyStmt->Step()) << "ECProperty " << expectedPropertyName << " in ECClass " << expectedClassName << " of ECSchema " << expectedSchemaName << " is expected to be found in ec_Property table.";;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, ImportSchemaAgainstExistingTableWithoutECInstanceIdColumn)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ExecuteSql("CREATE TABLE t_Foo (Name TEXT)"));

    ECSchemaCachePtr testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema();
    //now import test schema where the table already exists for the ECClass. This is expected to fail.
    BeTest::SetFailOnAssert(false);
    {
    ASSERT_EQ(ERROR, ecdb.Schemas().ImportECSchemas(*testSchemaCache)) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";
    }
    BeTest::SetFailOnAssert(true);

    EXPECT_TRUE(ecdb.ColumnExists("t_Foo", "Name")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_FALSE(ecdb.ColumnExists("t_Foo", "ECInstanceId")) << "ECInstanceId column not expected to be in the table after ImportECSchemas as ImportECSchemas is not expected to modify existing tables.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, ImportSchemaAgainstExistingTableWithECInstanceIdColumn)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ExecuteSql("CREATE TABLE t_Foo (ECInstanceId INTEGER PRIMARY KEY, Name TEXT)"));

    ECSchemaCachePtr testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema();
    //now import test schema where the table already exists for the ECClass
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*testSchemaCache)) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    //ImportSchema does not (yet) modify the existing tables. So it is expected that the ECInstanceId column is not added
    AssertImportedSchema(ecdb, "test", "Foo", "Name");
    EXPECT_TRUE(ecdb.ColumnExists("t_Foo", "Name")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE(ecdb.ColumnExists("t_Foo", "ECInstanceId")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, DiegoRelationshipTest)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create("importecschema.ecdb");

    ECSchemaPtr s1, s2;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromDisk(s1, ctx, L"DiegoSchema1.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(s1.IsValid());
    ECDbTestUtility::ReadECSchemaFromDisk(s2, ctx, L"DiegoSchema2.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(s2.IsValid());

    //now import test schema where the table already exists for the ECClass
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(ctx->GetCache())) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    ECClassCP civilModelClass = ecdb.Schemas().GetECClass("DiegoSchema1", "CivilModel");
    ASSERT_TRUE(civilModelClass != nullptr);
    ECClassCP datasetModelClass = ecdb.Schemas().GetECClass("DiegoSchema1", "DataSetModel");
    ASSERT_TRUE(datasetModelClass != nullptr);
    ECClassCP relClass = ecdb.Schemas().GetECClass("DiegoSchema1", "CivilModelHasDataSetModel");
    ASSERT_TRUE(relClass != nullptr);
    ECClassCP geometricModelClass = ecdb.Schemas().GetECClass("DiegoSchema2", "GeometricModel");
    ASSERT_TRUE(geometricModelClass != nullptr);

    IECInstancePtr civilModel1 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
    IECInstancePtr civilModel2 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
    IECInstancePtr geometricModel = ECDbTestUtility::CreateArbitraryECInstance(*geometricModelClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*(relClass->GetRelationshipClassCP()));
    StandaloneECRelationshipInstancePtr rel1 = relationshipEnabler->CreateRelationshipInstance();

    rel1->SetSource(civilModel2.get());
    rel1->SetTarget(geometricModel.get());

    ECInstanceInserter civilModelInserter(ecdb, *civilModelClass);
    ASSERT_TRUE(civilModelInserter.IsValid());
    ASSERT_EQ(SUCCESS, civilModelInserter.Insert(*civilModel1));
    ASSERT_EQ(SUCCESS, civilModelInserter.Insert(*civilModel2));

    ECInstanceInserter geometricModelInserter(ecdb, *geometricModelClass);
    ASSERT_TRUE(geometricModelInserter.IsValid());
    ASSERT_EQ(SUCCESS, geometricModelInserter.Insert(*geometricModel));

    ECInstanceInserter relInserter(ecdb, *relClass);
    ASSERT_TRUE(relInserter.IsValid());
    ASSERT_EQ(SUCCESS, relInserter.Insert(*rel1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, ImportSchemaWithRelationshipAgainstExistingTable)
    {
    // Create a sample project
    ECDbTestProject test;
    ECDbR ecdb = test.Create("importecschema.ecdb");

    //create ec table bypassing ECDb API, but don't add it to the ec_ profile tables
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ExecuteSql("CREATE TABLE t_Foo (ECInstanceId INTEGER PRIMARY KEY, Name TEXT)"));
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ExecuteSql("CREATE TABLE t_Goo (ECInstanceId INTEGER PRIMARY KEY, Price REAL)"));

    ECSchemaCachePtr testSchemaCache = CreateImportSchemaAgainstExistingTablesTestSchema();
    //now import test schema where the table already exists for the ECClass
    //missing link tables are created if true is passed for createTables
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*testSchemaCache)) << "ImportECSchema is expected to return success for schemas with classes that map to an existing table.";

    //ImportSchema does not (yet) modify the existing tables. So it is expected that the ECInstanceId column is not added
    EXPECT_TRUE(ecdb.ColumnExists("t_Goo", "ECInstanceId")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE(ecdb.ColumnExists("t_Goo", "Price")) << "Existing column is expected to still be in the table after ImportECSchemas.";
    EXPECT_TRUE(ecdb.ColumnExists("t_Goo", "ForeignECInstanceId_t_FooHasGoo")) << "ForeignECInstanceId_t_FooHasGoo column not expected to be in the table after ImportECSchemas as ImportECSchemas is not expected to modify existing tables.";
    EXPECT_TRUE(ecdb.TableExists("t_RelFooGoo")) << "Existence of Link table not as expected.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void CreateCustomAttributeTestSchema(ECSchemaPtr& testSchema, ECSchemaCachePtr& testSchemaCache)
    {
    ECSchemaPtr schema = nullptr;
    ECObjectsStatus stat = ECSchema::CreateSchema(schema, "foo", 1, 0);
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating test schema failed";
    schema->SetAlias("f");

    ECEntityClassP domainClass = nullptr;
    stat = schema->CreateEntityClass(domainClass, "domain1");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating domain class 1 in schema failed";

    ECEntityClassP domainClass2 = nullptr;
    stat = schema->CreateEntityClass(domainClass2, "domain2");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating domain class 2 in schema failed";

    ECCustomAttributeClassP caClass = nullptr;
    stat = schema->CreateCustomAttributeClass(caClass, "MyCA");
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Creating CA class in schema failed";

    PrimitiveECPropertyP dateProp = nullptr;
    caClass->CreatePrimitiveProperty(dateProp, "dateprop", PRIMITIVETYPE_DateTime);

    PrimitiveECPropertyP stringProp = nullptr;
    caClass->CreatePrimitiveProperty(stringProp, "stringprop", PRIMITIVETYPE_String);

    PrimitiveECPropertyP doubleProp = nullptr;
    caClass->CreatePrimitiveProperty(doubleProp, "doubleprop", PRIMITIVETYPE_Double);

    PrimitiveECPropertyP pointProp = nullptr;
    caClass->CreatePrimitiveProperty(pointProp, "pointprop", PRIMITIVETYPE_Point3D);

    ECSchemaCachePtr cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);

    testSchema = schema;
    testSchemaCache = cache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
void AssignCustomAttribute(IECInstancePtr& caInstance, ECSchemaPtr schema, Utf8CP containerClassName, Utf8CP caClassName, Utf8CP instanceId, bmap<Utf8String, ECValue> const& caPropValues)
    {
    ECClassP caClass = schema->GetClassP(caClassName);
    IECInstancePtr ca = caClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(ca.IsValid());

    ECObjectsStatus stat;
    if (instanceId != nullptr)
        {
        stat = ca->SetInstanceId(instanceId);
        ASSERT_EQ(ECObjectsStatus::Success, stat) << "Setting instance id in CA instance failed";
        }

    typedef bpair<Utf8String, ECValue> T_PropValuePair;

    for (T_PropValuePair const& pair : caPropValues)
        {
        stat = ca->SetValue(pair.first.c_str(), pair.second);
        ASSERT_EQ(ECObjectsStatus::Success, stat) << "Assigning property value to CA instance failed";
        }

    ECClassP containerClass = schema->GetClassP(containerClassName);
    stat = containerClass->SetCustomAttribute(*ca);
    ASSERT_EQ(ECObjectsStatus::Success, stat) << "Assigning CA instance to container class failed";

    caInstance = ca;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr CreateAndAssignRandomCAInstance(ECSchemaPtr testSchema)
    {
    //assign CA with instance id and all props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String("dateprop")] = ECValue(DateTime(DateTime::Kind::Unspecified, 1971, 4, 30, 21, 9, 0, 0));
    propValueMap[Utf8String("stringprop")] = ECValue("hello world", true);
    propValueMap[Utf8String("doubleprop")] = ECValue(3.14);
    DPoint3d point;
    point.x = 1.0;
    point.y = -2.0;
    point.z = 3.0;
    propValueMap[Utf8String("pointprop")] = ECValue(point);

    IECInstancePtr ca = nullptr;
    AssignCustomAttribute(ca, testSchema, "domain1", "MyCA", "bla bla", propValueMap);

    return ca;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, ReadCustomAttributesTest)
    {
    Utf8CP const CAClassName = "MyCA";

    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema(testSchema, testSchemaCache);

    //assign CA with instance id and all props populated
    IECInstancePtr expectedCAInstanceWithInstanceId = CreateAndAssignRandomCAInstance(testSchema);

    //assign CA without instance id and only a few props populated
    bmap<Utf8String, ECValue> propValueMap;
    propValueMap[Utf8String("doubleprop")] = ECValue(3.14);
    IECInstancePtr expectedCAInstanceWithoutInstanceId = nullptr;
    AssignCustomAttribute(expectedCAInstanceWithoutInstanceId, testSchema, "domain2", CAClassName, nullptr, propValueMap);

    //create test db and close it again
    Utf8String dbPath;
    {
    ECDbTestProject testProject;
    ECDbR db = testProject.Create("customattributestest.ecdb");
    auto importStat = db.Schemas().ImportECSchemas(*testSchemaCache);
    ASSERT_EQ(SUCCESS, importStat) << "Could not import test schema into ECDb file";

    dbPath = testProject.GetECDbPath();
    }

    //reopen test ECDb file (to make sure that the stored schema is read correctly)
    ECDb db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Could not open test ECDb file";

    ECSchemaCP readSchema = db.Schemas().GetECSchema(testSchema->GetName().c_str());
    ASSERT_TRUE(readSchema != nullptr) << "Could not read test schema from reopened ECDb file.";
    //*** assert custom attribute instance with instance id
    ECClassCP domainClass1 = readSchema->GetClassCP("domain1");
    ASSERT_TRUE(domainClass1 != nullptr) << "Could not retrieve domain class 1 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithInstanceId = domainClass1->GetCustomAttribute(CAClassName);
    ASSERT_TRUE(actualCAInstanceWithInstanceId.IsValid()) << "Test custom attribute instance not found on domain class 1.";

    //compare instance ids
    ASSERT_STREQ(expectedCAInstanceWithInstanceId->GetInstanceId().c_str(), actualCAInstanceWithInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance doesn't match.";

    //compare rest of instance
    bool equal = ECDbTestUtility::CompareECInstances(*expectedCAInstanceWithInstanceId, *actualCAInstanceWithInstanceId);
    ASSERT_TRUE(equal) << "Read custom attribute instance with instance id differs from expected.";

    //*** assert custom attribute instance without instance id
    ECClassCP domainClass2 = readSchema->GetClassCP("domain2");
    ASSERT_TRUE(domainClass2 != nullptr) << "Could not retrieve domain class 2 from re-read test schema.";
    IECInstancePtr actualCAInstanceWithoutInstanceId = domainClass2->GetCustomAttribute(CAClassName);
    ASSERT_TRUE(actualCAInstanceWithoutInstanceId.IsValid()) << "Test custom attribute instance not found on domain class 2.";

    //compare instance ids
    ASSERT_STREQ(expectedCAInstanceWithoutInstanceId->GetInstanceId().c_str(), actualCAInstanceWithoutInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance doesn't match.";
    ASSERT_STREQ("", actualCAInstanceWithoutInstanceId->GetInstanceId().c_str()) << "Instance Ids of retrieved custom attribute instance is expected to be empty";

    //compare rest of instance
    equal = ECDbTestUtility::CompareECInstances(*expectedCAInstanceWithoutInstanceId, *actualCAInstanceWithoutInstanceId);
    ASSERT_TRUE(equal) << "Read custom attribute instance without instance id differs from expected.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaTests, CheckCustomAttributesXmlFormatTest)
    {
    ECSchemaPtr testSchema = nullptr;
    ECSchemaCachePtr testSchemaCache = nullptr;
    CreateCustomAttributeTestSchema(testSchema, testSchemaCache);

    //assign CA with instance id
    CreateAndAssignRandomCAInstance(testSchema);

    ECDbTestProject testProject;
    ECDbR db = testProject.Create("customattributestest.ecdb");
    auto importStat = db.Schemas().ImportECSchemas(*testSchemaCache);
    ASSERT_EQ(SUCCESS, importStat) << "Could not import test schema into ECDb file";

    //now retrieve the persisted CA XML from ECDb directly
    Statement stmt;
    DbResult stat = stmt.Prepare(db, "SELECT Instance from ec_CustomAttribute ca, ec_Class c where ca.ClassId = c.Id AND c.Name = 'MyCA'");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Preparing the SQL statement to fetch the persisted CA XML string failed.";

    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP caXml = stmt.GetValueText(0);
        ASSERT_TRUE(caXml != nullptr) << "Retrieved custom attribute XML string is expected to be not null.";
        Utf8String caXmlString(caXml);
        EXPECT_LT(0, (int) caXmlString.length()) << "Retrieved custom attribute XML string is not expected to be empty.";

        //It is expected that the XML string doesn't contain the XML descriptor.
        size_t found = caXmlString.find("<?xml");
        EXPECT_EQ(Utf8String::npos, found) << "The custom attribute XML string is expected to not contain the XML description tag.";

        //It is expected that the XML string does contain the instance id if the original CA was assigned one
        found = caXmlString.find("instanceID=");
        EXPECT_NE(Utf8String::npos, found) << "The custom attribute XML string is expected to contain the instance id for the given custom attribute instance.";
        }

    ASSERT_EQ(1, rowCount) << "Only one test custom attribute instance had been created.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, ImportSupplementalSchemas)
    {
    Utf8CP dbFileName = "supplementalschematest.ecdb";
    ECDbR ecdb = SetupECDb(dbFileName);

    ECSchemaPtr ecSchema = nullptr;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECSchemaPtr startup;
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(startup, schemaContext, L"StartupCompany.02.00.ecschema.xml");
    ECDbTestUtility::ReadECSchemaFromDisk(supple, schemaContext, L"StartupCompany_Supplemental_ECDbTest.01.00.ecschema.xml");
    SchemaKey key("StartupCompany", 2, 0);

    bvector<ECSchemaP> supplementalSchemas;
    supplementalSchemas.push_back(supple.get());
    SupplementedSchemaBuilder builder;

    BentleyStatus schemaStatus = ecdb.Schemas().ImportECSchemas(schemaContext->GetCache());
    ASSERT_EQ(SUCCESS, schemaStatus);

    ecdb.SaveChanges();
    ecdb.CloseDb();

    BeFileName dbPath;
    BeTest::GetHost().GetOutputRoot(dbPath);
    dbPath.AppendToPath(BeFileName(dbFileName));

    ASSERT_EQ(DbResult::BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(dbPath, Db::OpenParams(Db::OpenMode::Readonly)));
    ECSchemaCP startupCompanySchema = ecdb.Schemas().GetECSchema("StartupCompany");
    ASSERT_TRUE(startupCompanySchema != nullptr);
    ECClassCP aaa2 = startupCompanySchema->GetClassCP("AAA");

    ECCustomAttributeInstanceIterable allCustomAttributes2 = aaa2->GetCustomAttributes(false);
    uint32_t allCustomAttributesCount2 = 0;
    for (IECInstancePtr attribute : allCustomAttributes2)
        {
        allCustomAttributesCount2++;
        }
    ASSERT_EQ(2, allCustomAttributesCount2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, ArrayPropertyTest)
    {
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("StartupCompany.ecdb", L"StartupCompany.02.00.ecschema.xml", false);
    db.ClearECDbCache();

    ECSchemaCP startupCompanySchema = db.Schemas().GetECSchema("StartupCompany", true);
    ASSERT_TRUE(startupCompanySchema != nullptr);

    ECClassCP arrayTestClass = startupCompanySchema->GetClassCP("ArrayTestclass");
    ASSERT_TRUE(arrayTestClass != nullptr);

    ArrayECPropertyCP p0_unbounded = arrayTestClass->GetPropertyP("p0_unbounded")->GetAsArrayProperty();
    ASSERT_TRUE(p0_unbounded != nullptr);
    ASSERT_EQ(p0_unbounded->GetMinOccurs(), 0);
    ASSERT_EQ(p0_unbounded->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_unbounded = arrayTestClass->GetPropertyP("p1_unbounded")->GetAsArrayProperty();
    ASSERT_TRUE(p1_unbounded != nullptr);
    ASSERT_EQ(p1_unbounded->GetMinOccurs(), 1);
    ASSERT_EQ(p1_unbounded->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p0_1 = arrayTestClass->GetPropertyP("p0_1")->GetAsArrayProperty();
    ASSERT_TRUE(p0_1 != nullptr);
    ASSERT_EQ(p0_1->GetMinOccurs(), 0);
    ASSERT_EQ(p0_1->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_1 = arrayTestClass->GetPropertyP("p1_1")->GetAsArrayProperty();
    ASSERT_TRUE(p1_1 != nullptr);
    ASSERT_EQ(p1_1->GetMinOccurs(), 1);
    ASSERT_EQ(p1_1->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p1_10000 = arrayTestClass->GetPropertyP("p1_10000")->GetAsArrayProperty();
    ASSERT_TRUE(p1_10000 != nullptr);
    ASSERT_EQ(p1_10000->GetMinOccurs(), 1);
    ASSERT_EQ(p1_10000->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p100_10000 = arrayTestClass->GetPropertyP("p100_10000")->GetAsArrayProperty();
    ASSERT_TRUE(p100_10000 != nullptr);
    ASSERT_EQ(p100_10000->GetMinOccurs(), 100);
    ASSERT_EQ(p100_10000->GetMaxOccurs(), UINT32_MAX);

    ArrayECPropertyCP p123_12345 = arrayTestClass->GetPropertyP("p123_12345")->GetAsArrayProperty();
    ASSERT_TRUE(p123_12345 != nullptr);
    ASSERT_EQ(p123_12345->GetMinOccurs(), 123);
    ASSERT_EQ(p123_12345->GetMaxOccurs(), UINT32_MAX);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/13
! This test need to be moved to ECF test suit
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbSchemaTests, DynamicSchemaTest)
    {
    ECSchemaPtr testSchema;
    ASSERT_EQ(ECSchema::CreateSchema(testSchema, "TestSchema", 1, 1), ECObjectsStatus::Success);
    ASSERT_EQ(testSchema->IsDynamicSchema(), false);
    ASSERT_EQ(testSchema->SetIsDynamicSchema(true), ECObjectsStatus::DynamicSchemaCustomAttributeWasNotFound);
    //reference BCSA, DynamicSchema CA introduce in 1.6
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    SchemaKey bscaKey("Bentley_Standard_CustomAttributes", 1, 6);
    ECSchemaPtr bscaSchema = ctx->LocateSchema(bscaKey, SchemaMatchType::Latest);
    ASSERT_TRUE(bscaSchema.IsValid());
    ASSERT_EQ(testSchema->AddReferencedSchema(*bscaSchema), ECObjectsStatus::Success);
    ASSERT_EQ(testSchema->SetIsDynamicSchema(true), ECObjectsStatus::Success);
    ASSERT_TRUE(testSchema->IsDynamicSchema());
    ASSERT_TRUE(StandardCustomAttributeHelper::IsDynamicSchema(*testSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaDiffTests : public ECDbTestFixture
    {
    protected:
        void WriteECSchemaDiffToLog(ECDiffR diff, NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO)
            {
            Utf8String diffString;
            ASSERT_EQ(diff.WriteToString(diffString, 2), DiffStatus::Success);
            LOG.message(severity, "ECDiff: Legend [L] Added from left schema, [R] Added from right schema, [!] conflicting value");
            LOG.message(severity, "=====================================[ECDiff Start]=====================================");
            //LOG doesnt allow single large string
            Utf8String eol = "\r\n";
            Utf8String::size_type i = 0;
            Utf8String::size_type j = diffString.find(eol, i);
            while (j > i && j != Utf8String::npos)
                {
                Utf8String line = diffString.substr(i, j - i);
                LOG.messagev(severity, "> %s", line.c_str()); //print out the difference
                i = j + eol.size();
                j = diffString.find(eol, i);
                }
            LOG.message(severity, "=====================================[ECDiff End]=====================================");
            }

        void VerifyRelationshipConstraint(ECN::ECSchemaCR schema, Utf8CP relationName, Utf8CP sourceClass, Utf8CP targetClass)
            {
            ECClassCP ecClass = schema.GetClassCP(relationName);
            ASSERT_TRUE(ecClass != nullptr);
            ECRelationshipClassCP ecRelationshipClass = ecClass->GetRelationshipClassCP();
            ASSERT_TRUE(ecRelationshipClass != nullptr);
            ASSERT_EQ(ecRelationshipClass->GetSource().GetClasses().size(), 1);
            ASSERT_EQ(ecRelationshipClass->GetTarget().GetClasses().size(), 1);
            ASSERT_TRUE(ecRelationshipClass->GetSource().GetClasses().at(0)->GetName().Equals(sourceClass));
            ASSERT_TRUE(ecRelationshipClass->GetTarget().GetClasses().at(0)->GetName().Equals(targetClass));
            }
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDbTestFixture, CheckClassHasCurrentTimeStamp)
    {
    SchemaItem schema(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"SimpleSchema\" nameSpacePrefix=\"adhoc\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "<ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.11\" prefix=\"besc\" />"
        "<ECEntityClass typeName=\"SimpleClass\" >"
        "<ECProperty propertyName = \"DateTimeProperty\" typeName=\"dateTime\" readOnly=\"True\" />"
        "<ECProperty propertyName = \"testprop\" typeName=\"int\" />"
        "<ECCustomAttributes>"
        "<ClassHasCurrentTimeStampProperty xmlns=\"Bentley_Standard_CustomAttributes.01.11\">"
        "<PropertyName>DateTimeProperty</PropertyName>"
        "</ClassHasCurrentTimeStampProperty>"
        "</ECCustomAttributes>"
        "</ECEntityClass>"
        "</ECSchema>");

    SetupECDb("checkClassHasCurrentTimeStamp.ecdb", schema);

    ECInstanceKey key;
    {
    ECSqlStatement insertStatement;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(GetECDb(), "INSERT INTO adhoc.SimpleClass(testprop) VALUES(12)"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(key));
    }

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT DateTimeProperty FROM adhoc.SimpleClass WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetECInstanceId()));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_FALSE(statement.IsValueNull(0));
    DateTime lastMod1 = statement.GetValueDateTime(0);
    statement.Reset();
    statement.ClearBindings();

    {
    BeThreadUtilities::BeSleep(100); // make sure the time is different by more than the resolution of the timestamp
    ECSqlStatement updateStatement;
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.Prepare(GetECDb(), "UPDATE adhoc.SimpleClass SET testprop = 23 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, updateStatement.BindId(1, key.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, updateStatement.Step());
    }

    ASSERT_EQ(ECSqlStatus::Success, statement.BindId(1, key.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_FALSE(statement.IsValueNull(0));
    DateTime lastMod2 = statement.GetValueDateTime(0);

    ASSERT_NE(lastMod1, lastMod2) << "LastMod date should have been updated after the last UPDATE statement";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                       10/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECSqlParseTest, VerifyECSqlParsingOnAndroid)
    {
    auto AssertParseECSql = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        Utf8String parseTree;
        ASSERT_EQ(SUCCESS, ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, ecdb, ecsql)) << "Failed to parse ECSQL";
        };

    ECDb ecdb; // only needed for issue listener, doesn't need to represent a file on disk
    AssertParseECSql(ecdb, "SELECT '''' FROM stco.Hardware");
    AssertParseECSql(ecdb, "SELECT 'aa', '''', b FROM stco.Hardware WHERE Name = 'a''b'");
    AssertParseECSql(ecdb, "SELECT _Aa, _bC, _123, Abc, a123, a_123, a_b, _a_b_c FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT * FROM stco.Hardware WHERE Name = 'Fusion'");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo]");
    AssertParseECSql(ecdb, "SELECT [Foo].[Name] FROM stco.[Hardware] [Foo] WHERE [Name] = 'HelloWorld'");
    AssertParseECSql(ecdb, "Select EQUIP_NO From only appdw.Equipment where EQUIP_NO = '50E-101A' ");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[STRUCTURE_IL1] ([VarFixedStartZ], [DeviceID1], [ObjectType], [PlaceMethod], [CopyConstrDrwToProj]) VALUES ('?', '-E1-1', 'SGL', '1', 'Y')");
    AssertParseECSql(ecdb, "INSERT INTO [V8TagsetDefinitions].[grid__x0024__0__x0024__CB_1] ([CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457],[CB_1_8456], [CB_1_8455], [CB_1_8454], [CB_1_8457], [CB_1_8456], [CB_1_8455], [CB_1_8454]) VALUES ('', '1.1', '', '', '', '2.2', '', '', '', '2.5', '', '', '', '2.5', '', '', '', '2.1', '', '', '', 'E.3', '', '', '', 'B.4', '', '', '', 'D.4', '', '')");
    }

END_ECDBUNITTESTS_NAMESPACE
