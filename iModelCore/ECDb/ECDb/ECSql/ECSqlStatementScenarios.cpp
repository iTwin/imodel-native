/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementScenarios.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//*******************************************************************
// This shows sample code which illustrates how the new ECSqlStatement API is used
//
//*******************************************************************
#include <ECDb/ECDbApi.h>

using namespace Bentley::BeSQLite;
using namespace Bentley::BeSQLite::EC;
using namespace Bentley::ECN;

//**************************************************************************************
//Scenario: Get certain primitive, struct and array values
//**************************************************************************************
void OutputNameValuePair (IECSqlValue const& value);

void Scenario_GetCertainPrimitiveAndStructAndArrayValuesFromStatement_Variation1_AccessStructMembersByIteration (ECDbR ecdb)
    {
    //ECClass Company:
    //   Name : String
    //   Headquarter: Location
    //   Affiliates: Array of Location
    //   Acronyms: Array of String
    //
    //ECClass Location (ECStruct)
    //   StreetName : Street
    //   Zip : Integer
    //   Town : String
    //
    //ECClass Street (ECStruct)
    //   HouseNumber: Integer
    //   Name : String

    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT Name, Headquarter, Affiliates, Acronyms FROM Company");

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //1) print out company name
        //API_COMPARISON #1
        Utf8CP companyName = statement.GetValueText (0);
        printf ("Company: %s\r\n", companyName);

        //2) print out headquarter address
        //API_COMPARISON #2
        IECSqlValue const& headquarterValue = statement.GetValue (1);
        printf ("Headquarter:\r\n");
        OutputNameValuePair (headquarterValue);

        //3) print out affiliates addresses
        printf ("Affiliates\r\n");
        IECSqlArrayValue const& affiliateArrayValue = statement.GetValueArray (2);
        for (IECSqlValue const& arrayElement : affiliateArrayValue)
            {
            OutputNameValuePair (arrayElement);
            }

        //4) print out acronyms
        printf ("Acronyms:\r\n");
        IECSqlArrayValue const& acronymsArrayValue = statement.GetValueArray (3);
        for (IECSqlValue const& arrayElement : acronymsArrayValue)
            {
            OutputNameValuePair (arrayElement);
            }
        }
    }

void OutputNameValuePair (IECSqlValue const& value)
    {
    auto prop = value.GetColumnInfo ().GetProperty ();
    auto propName = prop->GetName ().c_str ();
    if (prop->GetIsPrimitive ())
        {
        auto primitiveType = prop->GetAsPrimitiveProperty ()->GetType ();
        switch (primitiveType)
            {
                case PRIMITIVETYPE_Integer:
                    auto intValue = value.GetInt ();
                    printf ("%s: %d\t", propName, intValue);
                    break;
            }
        }
    else if (prop->GetIsStruct ())
        {
        IECSqlStructValue const& structValue = value.GetStruct ();
        for (int i = 0; i < structValue.GetMemberCount (); i++)
            OutputNameValuePair (structValue.GetValue (i));
        }
    else //Array
        {
        IECSqlArrayValue const& arrayValue = value.GetArray ();
        for (IECSqlValue const* arrayElement : arrayValue)
            {
            //now recurse into array element reader
            OutputNameValuePair (*arrayElement);
            }
        }
    }

//**************************************************************************************
//Scenario: Get all values from statement
//**************************************************************************************
//find impl below scenario code
void OutputHeader (IECSqlValue const& value);
void OutputValue (IECSqlValue const& value);

void Scenario_GetAllValuesFromStatement (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT * FROM Company");

    const int columnCount = statement.GetColumnCount ();

    for (int i = 0; i < columnCount; i++)
        OutputHeader (statement.GetValue (i));
    bool isFirstRow = true;
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        for (int i = 0; i < columnCount; i++)
            {
            IECSqlValue const& value = statement.GetValue (i);
            if (isFirstRow)
                {
                OutputHeader (value);
                isFirstRow = false;
                }

            OutputValue (value);
            }
        }
    }

void OutputHeader (IECSqlValue const& value)
    {
    auto prop = value.GetColumnInfo ().GetProperty ();

    if (prop->GetIsPrimitive ())
        {
        printf ("%s\t", Utf8String (reader.GetColumnInfo (i).GetPropertyPath ().ToString ().c_str ()).c_str ());
        }
    else if (prop->GetIsStruct ())
        {
        IECSqlStructValue const& structValue = value.GetStruct ();
        for (int i = 0; i < structValue.GetMemberCount (); i++)
            {
            OutputHeader (structValue.GetValue (i));
            }
        }
    else //Array
        {
        IECSqlArrayValue const& arrayValue = value.GetArray ();
        auto arrayIt = arrayValue.begin ();
        OutputHeader (*arrayIt);
        }

    printf ("\r\n");
    }

void OutputValue (IECSqlValue const& value)
    {
    auto  prop = value.GetColumnInfo ().GetProperty ();

    if (prop->GetIsPrimitive ())
        {
        auto primitiveType = prop->GetAsPrimitiveProperty ()->GetType ();
        switch (primitiveType)
            {
                case PRIMITIVETYPE_Integer:
                    auto intValue = value.GetInt ();
                    printf ("%d\t", intValue);
                    break;
            }
        }
    else if (prop->GetIsStruct ())
        {
        IECSqlStructValue const& structValue = value.GetStruct ();
        for (int i = 0; i < structValue.GetMemberCount (); i++)
            OutputValue (structValue.GetValue (i));
        }
    else //Array
        {
        IECSqlArrayValue const& arrayValue = reader.GetArray ();
        for (IECSqlValue const& arrayElement : arrayValue)
            {
            //now recurse into array element reader
            OutputValue (arrayElement);
            }
        }
    }


//**************************************************************************************
//Scenario: Get metadata for empty-result query
//**************************************************************************************
void Scenario_GetMetadataForQueryWithNoResults (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT Name, Headquarter FROM Company WHERE 1 = 0");
    
    const auto columnCount = statement.GetColumnCount ();
    for (auto i = 0; i < columnCount; i++)
        {
        auto propName = statement.GetColumnInfo (i).GetProperty ()->GetName ().c_str ();
        printf ("%s\t", propName);
        }
        
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //do something...
        }
    }

//**************************************************************************************
//Scenario: Get value for incorrect property index of if statement is not in valid state
//**************************************************************************************
void Scenario_GetValueForOutOfBoundIndex (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT Name FROM Company");

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //GetInt32 would return 0 in case of error. This is similar to SQLite where it even returns an undefined value
        //in case of error.
        int val = statement.GetValueInt (2);
        //Would return an empty column info.
        auto const columnInfo = statement.GetColumnInfo (2);
        }

    //the same accounts for cases when GetValue is called on a statement which is in an invalid state (e.g. no more rows)
    }


//**************************************************************************************
//Scenario: Select ECInstanceId
//**************************************************************************************
void Scenario_SelectECInstanceId (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT ECInstanceId FROM Foo");
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        ECInstanceId ecinstanceid = statement.GetValueId<ECInstanceId> (0));
        printf ("ECInstanceId: %lld\r\n", ecinstanceid.GetValue ());
        }
    }

//**************************************************************************************
//Scenario: Select ECInstanceId with joins
//**************************************************************************************
void Scenario_SelectECInstanceIdWithJoins (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT parentFolder.ECInstanceId, subfolder.ECInstanceId FROM Folder parentFolder JOIN Folder subfolder USING FolderHasSubfolders FORWARD WHERE parentFolder.Name = ?");
    statement.BindText (1, "My Documents", IECSqlBinder::MakeCopy::Yes);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        ECInstanceId parentFolderECInstanceId = statement.GetValueId<ECInstanceId> (0);
        ECInstanceId subfolderECInstanceId = statement.GetValueId<ECInstanceId> (1);
        printf ("Parent folder: %lld  Subfolder: %lld\r\n", parentFolderECInstanceId.GetValue (), subfolderECInstanceId.GetValue ());
        }
    }

//**************************************************************************************
//Scenario: Bind certain primitives, structs, and arrays
//**************************************************************************************
void Scenario_BindCertainPrimitiveAndStructsAndArrays (ECDbR ecdb)
    {
    //ECClass Company:
    //   Name : String
    //   Headquarter: Location
    //   Affiliates: Array of Location
    //   Acronyms: Array of String
    //
    //ECClass Location (ECStruct)
    //   StreetName : Street
    //   Zip : Integer
    //   Town : String
    //
    //ECClass Street (ECStruct)
    //   HouseNumber: Integer
    //   Name : String

    ECSqlStatement statement;
    statement.Prepare (ecdb, "INSERT INTO Company (Name, Headquarter, Affiliates, Acronyms) VALUES (?, ?, ?, ?)");

    // *** Bind primitive value ***
    statement.BindInt (1, "ACME");

    // *** Bind struct value ***
    IECSqlStructBinder& headquarterStructBinder = statement.BindStruct (2);
    auto stat = headquarterStructBinder.GetMember ("Town").BindText ("New York", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        {
        //do error handling
        return;
        }
    stat = headquarterStructBinder.GetBinder ("Zip").BindInt (97652);
    if (stat != ECSqlStatus::Success)
        {
        //do error handling
        return;
        }

    //Bind nested struct value (StreetName is a member of struct Location which is a struct again
    IECSqlStructBinder& streetNameStructBinder = headquarterStructBinder.GetMember ("StreetName").BindStruct ();
    stat = streetNameStructBinder.GetMember ("Name").BindText ("Main Street", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        {
        //do error handling
        return;
        }

    stat = streetNameStructBinder.GetMember ("HouseNumber").BindInt (2001);
    if (stat != ECSqlStatus::Success)
        {
        //do error handling
        return;
        }

    //*** Bind struct array value ***
    IECSqlArrayBinder& affiliateArrayBinder = statement.BindArray (3, 2);
    //first array element
    IECSqlStructBinder& affiliateBinder = affiliateArrayBinder.AddArrayElement ().BindStruct ();
    
    stat = affiliateBinder.GetMember ("Town").BindText ("Philadelphia", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        {
        //do error handling
        return;
        }
    //skipping error handling from here for better readability of sample code
    affiliateBinder.GetMember ("Zip").BindInt (23451);
    //Bind nested struct value (StreetName is a member of struct Location which is a struct again
    streetNameStructBinder = affiliateBinder.GetMember ("StreetName").BindStruct ();
    streetNameStructBinder.GetMember ("Name").BindText ("High Street", IECSqlBinder::MakeCopy::Yes);
    streetNameStructBinder.GetMember ("HouseNumber").BindInt (1100);

    //second array element
    affiliateBinder = affiliateArrayBinder.AddArrayElement ();

    affiliateBinder.GetMember ("Town").BindText ("Washington", IECSqlBinder::MakeCopy::Yes);
    affiliateBinder.GetMember ("Zip").BindInt (43219);
    streetNameStructBinder = affiliateBinder.GetMember ("StreetName").BindStruct ();
    streetNameStructBinder.GetMember ("Name").BindText ("Capitol Hill Rd", IECSqlBinder::MakeCopy::Yes);
    streetNameStructBinder.GetMember ("HouseNumber").BindInt (5000);

    //*** Bind primitive array value ***
    IECSqlArrayBinder& acronymArrayBinder = statement.BindArray (4, 2);
    //first array element
    //Note: For primitive arrays always 1 has to be passed as parameter index
    IECSqlBinder& acronymBinder = acronymArrayBinder.AddArrayElement ();
    acronymBinder.BindText ("ACME", IECSqlBinder::MakeCopy::Yes);
    //second array element
    acronymBinder = acronymArrayBinder.AddArrayElement ();
    acronymBinder.BindText ("A.C.M.E", IECSqlBinder::MakeCopy::Yes);

    //now execute statement
    ECInstanceId generatedECInstanceId = -1LL;
    if (statement.Step (generatedECInstanceId) != ECSqlStepStatus::Done)
        {
        //INSERT succeeded
        }
    }

//**************************************************************************************
//Scenario: Add FileAttribute to File (amounts to binding a struct)
//**************************************************************************************
void Scenario_AddFileAttributeOnFile_BindStruct (ECDbR ecdb)
    {
    //ECClass File:
    //   Name : String
    //   Attributes: Array of FileAttribute
    //
    //ECClass FileAttribute (ECStruct)
    //   Type: String
    //   Value : Boolean
    //

    ECSqlStatement statement;
    //Per SQL-99 the array indexer automatically inserts new array elements up to and including the specified index.
    //So for adding a new element at the end of the array use the cardinality function which returns the length of the array
    statement.Prepare (ecdb, "UPDATE File SET Attributes[Cardinality(Attributes)] = ? WHERE ECInstanceId = ?");

    //Bind FileAttribute struct value
    IECSqlStructBinder& attributeBinder = statement.BindStruct (1);
    attributeBinder.GetMember ("Type").BindText ("readonly", IECSqlBinder::MakeCopy::Yes);
    attributeBinder.GetMember("Value").BindBoolean (true);

    //Bind ECInstanceId value
    statement.BindInt64 (2, 1324353);

    //now execute statement
    int rowsAffected = -1;
    if (statement.Step (rowsAffected) != ECSqlStepStatus::Done)
        {
        //UPDATE succeeded
        }
    }

//**************************************************************************************
//Scenario: Set FileAttributes on File (amounts to binding a struct array)
//**************************************************************************************
void Scenario_SetFileAttributesOnFile_BindStructArray (ECDbR ecdb)
    {
    //ECClass File:
    //   Name : String
    //   Attributes: Array of FileAttribute
    //
    //ECClass FileAttribute (ECStruct)
    //   Type: String
    //   Value : Boolean
    //

    //Update the set of file attributes on a given file, e.g. after a file was made readonly, set the attributes 'readonly'
    //and 'compressed'
    ECSqlStatement statement;
    statement.Prepare (ecdb, "UPDATE File SET Attributes = ? WHERE ECInstanceId = ?");

    //Bind FileAttribute struct value
    IECSqlBinder& attributesBinder = statement.GetBinder (1, 2);
    //First element
    IECSqlBinder& attributeBinder = attributesBinder.AddArrayElement ();
    attributeBinder.BindStruct ().GetMember ("Type").BindText ("readonly", IECSqlBinder::MakeCopy::Yes);
    attributeBinder.BindStruct ().GetMember ("Value").BindBoolean (true);

    //Second element
    attributeBinder = attributesBinder.AddArrayElement ();
    attributeBinder.BindStruct ().GetMember ("Type").BindText ("compressed", IECSqlBinder::MakeCopy::Yes);
    attributeBinder.BindStruct ().GetMember ("Value").BindBoolean (true);

    //Bind ECInstanceId value
    statement.BindInt64 (2, 1324353);

    //now execute statement
    int rowsAffected = -1;
    if (statement.Step (rowsAffected) != ECSqlStepStatus::Done)
        {
        //UPDATE succeeded
        }
    }

//**************************************************************************************
//Scenario: Set FileAttributes on File (amounts to binding a struct array)
//**************************************************************************************
void Scenario_InsertNewProductWithItsAvailableColors_BindPrimitiveArray (ECDbR ecdb)
    {
    //ECClass Product:
    //   Id : String
    //   AvailableColors: Array of Integer

    //Update the set of file attributes on a given file, e.g. after a file was made readonly, set the attributes 'readonly'
    //and 'compressed'
    ECSqlStatement statement;
    statement.Prepare (ecdb, "INSERT INTO Product (Id, AvailableColors) VALUES (?, ?)");

    //Bind Product Id value
    statement.BindText (1, "1342-1222-9834", IECSqlBinder::MakeCopy::Yes);

    //Bind Available color array
    IECSqlArrayBinder& colorArrayBinder = statement.BindArray (2, 3);
    //First color: Color 13
    IECSqlBinder& colorBinder = colorArrayBinder.AddArrayElement ();
    colorBinder.BindInt (13);

    //Second color: Color 4
    colorBinder = colorArrayBinder.AddArrayElement ();
    colorBinder.BindInt (4);

    //Third color: Color 25
    colorBinder = colorArrayBinder.AddArrayElement ();
    colorBinder.BindInt (25);

    //now execute statement
    ECInstanceId generatedECInstanceId = -1LL;
    if (statement.Step (generatedECInstanceId) != ECSqlStepStatus::Done)
        {
        //INSERT succeeded
        }
    }

//**************************************************************************************
// Scenario Insert and bind values from an ECInstance
//**************************************************************************************

void Scenario_InsertWithBindFromECInstance (ECDbR ecdb, IECInstancePtr ecInstance)
    {
    //See ECInstanceInserter which implemented that scenario.
    }



//**************************************************************************************
// Scenario Bind values with out of bounds index
//**************************************************************************************
void Scenario_BindWithOutOfBoundsIndex (ECDbR ecdb)
    {
    ECSqlStatement statement;
    statement.Prepare (ecdb, "SELECT * FROM Company WHERE Headquarter = ?");
    //retrieve struct binder for invalid index. A no-op binder is returned
    IECSqlStructBinder& invalidBinder = statement.BindStruct (1000);
    
    //This is how the no-op binder behaves:
    //The BindXXX methods return an error indicating that the binder was obtained for an index out of bounds
    ECSqlStatus stat = invalidBinder.BindInt64 ("Id", 1000LL);
    if (stat == ECSqlStatus::IndexOutOfBounds)
        {
        //error
        return;
        }
    }

