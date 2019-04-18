/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <cmath> // for std::pow

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelect()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelect.sampleCode

    // Prepare statement
    Utf8CP ecsql = "SELECT FirstName, LastName, Birthday FROM stco.Employee WHERE LastName LIKE 'S%' ORDER BY LastName, FirstName";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, ecsql);
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    // Execute statement and step over each row of the result set
    while (BE_SQLITE_ROW == statement.Step())
        {
        // Property indices in result set are 0-based -> 0 refers to first property in result set
        Utf8CP firstName = statement.GetValueText(0);
        Utf8CP lastName = statement.GetValueText(1);
        DateTime birthday = statement.GetValueDateTime(2);
        // do something with the retrieved data of this row
        printf("%s, %s - Born %s\n", lastName, firstName, birthday.ToString().c_str());
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectStructProps()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructProps.sampleCode

    Utf8CP ecsql = "SELECT FirstName, Address FROM stco.Employee";

    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP firstName = statement.GetValueText(0);
        //second item in select clause is a struct prop, so call GetStructReader
        IECSqlValue const& addressStructValue = statement.GetValue(1);

        // Print out the address for each employee
        printf("Employee %s:\n", firstName);
        for (IECSqlValue const& addressMemberValue : addressStructValue.GetStructIterable())
            {
            ECSqlColumnInfoCR addressMemberColumnInfo = addressMemberValue.GetColumnInfo();

            //print out name of address member
            Utf8StringCR addressMemberColumnName = addressMemberColumnInfo.GetProperty()->GetName();
            printf("   %s: ", addressMemberColumnName.c_str());
            //print out value of address member
            PrimitiveType type = addressMemberColumnInfo.GetDataType().GetPrimitiveType();
            switch (type)
                {
                    case PRIMITIVETYPE_String:
                        printf("%s", addressMemberValue.GetText());
                        break;
                    case PRIMITIVETYPE_Integer:
                        printf("%d", addressMemberValue.GetInt());
                        break;
                    default:
                        //For the sake of simplicity the struct in this example has only properties of type String and Integer. 
                        //In other cases you might have to switch on the other data types, too 
                        break;
                }

            printf("\n");
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectStructPropMembers()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructPropMembers.sampleCode

    //Only interested in street, city and zip although Location struct has more properties
    Utf8CP ecsql = "SELECT FirstName, Address.Street, Address.City, Address.Zip FROM stco.Employee";

    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP firstName = statement.GetValueText(0);

        Utf8CP streetName = statement.GetValueText(1);
        Utf8CP city = statement.GetValueText(2);
        int zip = statement.GetValueInt(3);

        // Print out the address for each employee
        printf("Employee %s\n", firstName);
        printf("   %s\n", streetName);
        printf("   %s %d\n", city, zip);
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectPrimArrayProps()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectPrimArrayProps.sampleCode

    //Let MobilePhones be a string array property of the Employee class
    Utf8CP ecsql = "SELECT FirstName, LastName, MobilePhones FROM myschema.Employee";

    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP firstName = statement.GetValueText(0);
        Utf8CP lastName = statement.GetValueText(1);
        IECSqlValue const& mobilePhoneArrayValue = statement.GetValue(2);

        printf("Mobile numbers for %s %s:\n", firstName, lastName);
        int arrayIndex = 0;
        for (IECSqlValue const& arrayMemberValue : mobilePhoneArrayValue.GetArrayIterable())
            {
            //Primitive arrays always have one column. Therefore always pass 0 to the reader when retrieving array values.
            Utf8CP mobileNumber = arrayMemberValue.GetText();
            printf("#%d: %s\n", arrayIndex + 1, mobileNumber);
            arrayIndex++;
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectStructArrayProps()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructArrayProps.sampleCode

    Utf8CP ecsql = "SELECT FirstName, Certifications FROM stco.Employee";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8CP firstName = statement.GetValueText(0);
        IECSqlValue const& certStructArray = statement.GetValue(1);
        //now iterate over struct array elements. Each steps moves one struct element further in the array.
        // Print out the certifications for each employee
        printf("Employee %s\n", firstName);
        printf("  Certifications:\n");
        int arrayIndex = 0;
        for (IECSqlValue const& certStructVal : certStructArray.GetArrayIterable())
            {
            printf("  #%d:\n", arrayIndex + 1);
            for (IECSqlValue const& certMemberVal : certStructVal.GetStructIterable())
                {
                ECSqlColumnInfoCR certMemberColumnInfo = certMemberVal.GetColumnInfo();

                //print out name of cert member
                Utf8StringCR certMemberColumnName = certMemberColumnInfo.GetProperty()->GetName();
                printf("   %s: ", certMemberColumnName.c_str());
                //print out value of cert member
                PrimitiveType type = certMemberColumnInfo.GetDataType().GetPrimitiveType();
                switch (type)
                    {
                        case PRIMITIVETYPE_String:
                            printf("%s", certMemberVal.GetText());
                            break;
                        case PRIMITIVETYPE_Integer:
                            printf("%d", certMemberVal.GetInt());
                            break;
                        case PRIMITIVETYPE_DateTime:
                            printf("%s", certMemberVal.GetDateTime().ToString().c_str());
                            break;
                        default:
                            //For the sake of simplicity the struct in this example has only properties of type String, Integer,
                            //and DateTime. In other cases you might have to switch on the other data types, too 
                            break;
                    }
                printf("\n");
                arrayIndex++;
                }
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementAndDateTime()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementAndDateTime.sampleCode

    Utf8CP ecsql = "SELECT Birthday, LastModified FROM stco.Employee";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    while (BE_SQLITE_ROW == statement.Step())
        {
        DateTime birthday = statement.GetValueDateTime(0); //birthday.GetInfo().GetComponent() will amount to DateTime::Component::Date
        DateTime lastModified = statement.GetValueDateTime(1); //lastModified.GetInfo().GetKind() will amount to DateTime::Kind::Utc
        printf("Birthday: %s\n", birthday.ToString().c_str());
        printf("LastModified: %s\n", lastModified.ToString().c_str());
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectWithJoin()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectWithJoin.sampleCode

    // Task: Perform a query that finds the company John Smith works for.
    Utf8CP candidateEmployeeFirstName = "John";
    Utf8CP candidateEmployeeLastName = "Smith";

    // Prepare statement
    Utf8CP ecsql = "SELECT c.Name FROM stco.Company c JOIN stco.Employee e USING stco.CompanyHasEmployees"
        "WHERE e.FirstName = ? AND e.LastName = ?";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    statement.BindText(1, candidateEmployeeFirstName, IECSqlBinder::MakeCopy::No);
    statement.BindText(2, candidateEmployeeLastName, IECSqlBinder::MakeCopy::No);
    // Execute statement
    DbResult stat = statement.Step();
    switch (stat)
        {
            case BE_SQLITE_ROW:
            {
            // Note: GetValue parameter indices are 0-based!
            Utf8CP companyName = statement.GetValueText(0);
            printf("%s %s works for company '%s'.\n", candidateEmployeeFirstName, candidateEmployeeLastName, companyName);
            break;
            }
            case BE_SQLITE_DONE:
                printf("No company found for which %s %s works.\n", candidateEmployeeFirstName, candidateEmployeeLastName);
                break;

            default:
                //do error handling
                return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingPrimitives()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingPrimitives.sampleCode

    // Prepare statement
    Utf8CP ecsql = "INSERT INTO stco.Employee (FirstName, LastName, Birthday) VALUES (?, ?, ?)";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    //*** #1 Insert first employee 
    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    statement.BindText(1, "Joan", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(2, "Rieck", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime(3, DateTime(1965, 4, 12));

    // Execute statement
    ECInstanceKey ecInstanceKey;
    if (BE_SQLITE_DONE != statement.Step(ecInstanceKey))
        {
        //INSERT statements are expected to return BE_SQLITE_DONE if successful.
        return ERROR;
        }

    //*** #2 Insert second employee 
    statement.Reset(); //reset statement, so that it can be reused
    statement.ClearBindings(); //clear previous bindings

    // Bind values to parameters
    statement.BindText(1, "Bill", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(2, "Becker", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime(3, DateTime(1959, 1, 31));

    // Execute statement
    if (BE_SQLITE_DONE != statement.Step(ecInstanceKey))
        {
        //INSERT statements are expected to return BE_SQLITE_DONE if successful.
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingWithNamedParameters()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingWithNamedParameters.sampleCode

    // Prepare statement
    Utf8CP ecsql = "SELECT LastName FROM stco.Employee LIMIT :pagesize OFFSET (:pageno + 1) * :pagesize";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    const int pageSize = 50;
    int pageNumber = 0;

    const int pageSizeParameterIndex = statement.GetParameterIndex("pagesize");
    const int pageNoParameterIndex = statement.GetParameterIndex("pageno");

    //*** #1 Read first page
    statement.BindInt(pageSizeParameterIndex, pageSize);
    statement.BindInt(pageNoParameterIndex, pageNumber);

    while (BE_SQLITE_ROW == statement.Step())
        {
        //process first page
        }

    //*** #2 Read second page
    statement.Reset(); //reset statement, so that it can be reused
    statement.ClearBindings(); //clear previous bindings

    pageNumber++;
    statement.BindInt(pageSizeParameterIndex, pageSize);
    statement.BindInt(pageNoParameterIndex, pageNumber);

    while (BE_SQLITE_ROW == statement.Step())
        {
        //process second page
        }

    //__PUBLISH_EXTRACT_END__

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingStructs()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingStructs.sampleCode

    // Task: Insert a employee where the property Address is of a struct type called Location

    // Note: this example also shows complete error handling.

    // Prepare statement
    Utf8CP ecsql = "INSERT INTO stco.Employee (FirstName, LastName, Address) VALUES (?, ?, ?)";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, ecsql);
    if (!stat.IsSuccess())
        return ERROR;

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    stat = statement.BindText(1, "Joan", IECSqlBinder::MakeCopy::Yes);
    if (!stat.IsSuccess())
        return ERROR;

    stat = statement.BindText(2, "Smith", IECSqlBinder::MakeCopy::Yes);
    if (!stat.IsSuccess())
        return ERROR;

    IECSqlBinder& addressBinder = statement.GetBinder(3);

    stat = addressBinder["Street"].BindText("2000 Main Street", IECSqlBinder::MakeCopy::Yes);
    if (!stat.IsSuccess())
        return ERROR;

    stat = addressBinder["Zip"].BindInt(10014);
    if (!stat.IsSuccess())
        return ERROR;

    stat = addressBinder["City"].BindText("New York", IECSqlBinder::MakeCopy::Yes);
    if (!stat.IsSuccess())
        return ERROR;

    //Note: all other members of the struct Address which are not set through the binding API are set to DB NULL.

    // Execute statement
    ECInstanceKey ecInstanceKey;
    if (BE_SQLITE_DONE != statement.Step(ecInstanceKey))
        {
        //INSERT statements are expected to return BE_SQLITE_DONE if successful.
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingPrimArrays()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingPrimArrays.sampleCode
    // Task: Add a list of mobile phone numbers to an employee. The phone numbers are held
    // in a string array property called MobilePhones

    // Prepare statement
    Utf8CP ecsql = "UPDATE ONLY stco.Employee SET MobilePhones = ? WHERE FirstName = ? AND LastName = ?";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    IECSqlBinder& phoneNumbersBinder = statement.GetBinder(1);
    std::vector<Utf8String> phoneNumbers = {"+1 (610) 726-4312", "+1 (610) 726-4444", "+1 (610) 726-4112"};
    for (Utf8StringCR phoneNumber : phoneNumbers)
        {
        IECSqlBinder& arrayElementBinder = phoneNumbersBinder.AddArrayElement();
        ECSqlStatus stat = arrayElementBinder.BindText(phoneNumber.c_str(), IECSqlBinder::MakeCopy::Yes);
        if (!stat.IsSuccess())
            return ERROR;
        }

    //now bind to parameter 2 and 3 (primitive binding) (skipping error handling for better readability)
    statement.BindText(2, "John", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(3, "Smith", IECSqlBinder::MakeCopy::Yes);

    // Execute statement
    if (BE_SQLITE_DONE != statement.Step())
        {
        //UPDATE statements are expected to return BE_SQLITE_DONE if successful.
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingStructArrays()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingStructArrays.sampleCode
    // Task: Add a list of certifications for an employee. The certifications are held
    // in a struct array property called Certifications where the array elements are of
    // type Certification.

    // Prepare statement
    Utf8CP ecsql = "UPDATE ONLY stco.Employee SET Certifications = ? WHERE FirstName = ? AND LastName = ?";
    ECSqlStatement statement;
    statement.Prepare(ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    IECSqlBinder& certificationsBinder = statement.GetBinder(1);

    //AddArrayElement gets a binder for the array element.
    IECSqlBinder& arrayElementBinder = certificationsBinder.AddArrayElement();
    arrayElementBinder["Name"].BindText("Certified Engineer Assoc.", IECSqlBinder::MakeCopy::Yes);
    arrayElementBinder["StartsOn"].BindDateTime(DateTime(2014, 01, 28));

    IECSqlBinder& arrayElementBinder2 = certificationsBinder.AddArrayElement();
    arrayElementBinder2["Name"].BindText("Professional Engineers", IECSqlBinder::MakeCopy::Yes);
    arrayElementBinder2["StartsOn"].BindDateTime(DateTime(2011, 05, 14));

    //now bind to parameter 2 and 3 (primitive binding) (skipping error handling for better readability)
    statement.BindText(2, "Patty", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(3, "Rieck", IECSqlBinder::MakeCopy::Yes);

    // Execute statement
    if (BE_SQLITE_DONE != statement.Step())
        //UPDATE statements are expected to return BE_SQLITE_DONE if successful.
        return ERROR;


    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   05/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingVirtualSets()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingVirtualSets.sampleCode
    // Task: Repeatedly retrieve a list of Assets by their ECInstanceIds. The list of ids varies in size from call to call.

    ECSqlStatement statement;
    statement.Prepare(ecdb, "SELECT ECInstanceId AS Id, ECClassId AS AssetSubclassId, BarCode FROM stco.Asset WHERE InVirtualSet(?,ECInstanceId)");

    //First list
    ECInstanceIdSet idSet;
    idSet.insert(ECInstanceId(UINT64_C(1)));
    idSet.insert(ECInstanceId(UINT64_C(2)));
    idSet.insert(ECInstanceId(UINT64_C(3)));

    statement.BindVirtualSet(1, idSet);

    while (BE_SQLITE_ROW == statement.Step())
        {
        printf("Id: %s | SubclassId: %s | BarCode: %s\r\n",
               statement.GetValueId<ECInstanceId>(0).ToString().c_str(), statement.GetValueId<ECClassId>(1).ToString().c_str(),
               statement.GetValueText(2));
        }

    statement.Reset();
    statement.ClearBindings();

    //Another list
    ECInstanceIdSet anotherIdSet;
    anotherIdSet.insert(ECInstanceId(UINT64_C(100)));
    anotherIdSet.insert(ECInstanceId(UINT64_C(200)));

    statement.BindVirtualSet(1, anotherIdSet);

    while (BE_SQLITE_ROW == statement.Step())
        {
        printf("Id: %s | SubclassId: %s | BarCode: %s\r\n",
               statement.GetValueId<ECInstanceId>(0).ToString().c_str(), statement.GetValueId<ECClassId>(1).ToString().c_str(),
               statement.GetValueText(2));
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlColumnInfoOnNestedLevels()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlColumnInfoOnNestedLevels.sampleCode

    ECSqlStatement statement;
    statement.Prepare(ecdb, "SELECT FirstName, Certifications FROM stco.Employee WHERE ECInstanceId = 123");

    auto printColumnInfoCategoryLambda = [] (ECSqlColumnInfoCR columnInfo)
        {
        //Print property path
        Utf8String propPath = columnInfo.GetPropertyPath().ToString();
        printf("%s: ", propPath.c_str());
        //append type IECSqlBinder
        ECTypeDescriptor const& dataType = columnInfo.GetDataType();
        if (dataType.IsPrimitive())
            printf("Primitive type\n");
        else if (dataType.IsStruct())
            printf("ECStruct type\n");
        else if (dataType.IsPrimitiveArray())
            printf("Primitive array type\n");
        else
            printf("ECStruct array type\n");
        };

    while (BE_SQLITE_ROW == statement.Step())
        {
        printf("Top level\n");
        ECSqlColumnInfoCR firstNameColumnInfo = statement.GetColumnInfo(0);
        printColumnInfoCategoryLambda(firstNameColumnInfo);

        ECSqlColumnInfoCR certArrayColumnInfo = statement.GetColumnInfo(1);
        printColumnInfoCategoryLambda(certArrayColumnInfo);

        printf("\nStruct array element level\n");
        IECSqlValue const& certArray = statement.GetValue(1);
        for (IECSqlValue const& certElement : certArray.GetArrayIterable())
            {
            for (IECSqlValue const& certMember : certElement.GetStructIterable())
                {
                ECSqlColumnInfoCR certMemberColumnInfo = certMember.GetColumnInfo();
                printColumnInfoCategoryLambda(certMemberColumnInfo);
                }

            printf("\n");
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlInsert()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlInsert.sampleCode

    // ECSQL
    // Prepare statement
    // Use binding parameters so that the prepared statement can be reused with different values
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "INSERT INTO stco.Employee(FirstName,LastName,Birthday) VALUES (?,?,?)");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    //*** Insert first employee ***
    // Bind first address values to parameters of the ECSQL statement
    // Note: parameter indices are 1-bound!
    statement.BindText(1, "John", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(2, "Smith", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime(3, DateTime(1984, 5, 16));

    // Execute statement
    ECInstanceKey newECInstanceKey;
    // Note: For non-select statements BE_SQLITE_DONE indicates successful execution
    if (BE_SQLITE_DONE != statement.Step(newECInstanceKey))
        {
        // do error handling here...
        return ERROR;
        }

    //*** Insert second employee ***
    // Reset statement so that it can be re-executed.
    statement.Reset();
    // Reset bound values
    statement.ClearBindings();

    // Now bind second address values to parameters of the ECSQL statement
    statement.BindText(1, "Anne", IECSqlBinder::MakeCopy::Yes);
    statement.BindText(2, "Tyler", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime(3, DateTime(1979, 12, 1));

    // Execute statement
    if (BE_SQLITE_DONE != statement.Step(newECInstanceKey))
        {
        // do error handling here...
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlInsertRelationViaNavigationProperty()
    {
    ECDb ecdb;
    ECInstanceId companyECInstanceId;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlInsertRelationViaNavigationProperty.sampleCode

    // ECSQL
    // Use binding parameters so that the prepared statement can be reused with different values.
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "INSERT INTO stco.Employee(FirstName, LastName, Company) VALUES (?,?,?)");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate first employee to company***
    // Note: parameter indices are 1-bound!
    statement.BindText(1, "Laura", IECSqlBinder::MakeCopy::No);
    statement.BindText(2, "Miller", IECSqlBinder::MakeCopy::No);
    ECClassId companyHasEmployeesRelClassId = ecdb.Schemas().GetClassId("stco", "CompanyHasEmployees", SchemaLookupMode::ByAlias);
    statement.BindNavigationValue(3, companyECInstanceId, companyHasEmployeesRelClassId);

    // Execute statement
    ECInstanceKey employee1ECInstanceId;
    // Note: For non-select statements BE_SQLITE_DONE indicates successful execution
    if (BE_SQLITE_DONE != statement.Step(employee1ECInstanceId))
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate second employee to company***
    // Reset statement so that it can be re-executed.
    statement.Reset();
    // Reset bound values
    statement.ClearBindings();

    statement.BindText(1, "John", IECSqlBinder::MakeCopy::No);
    statement.BindText(2, "Smith", IECSqlBinder::MakeCopy::No);
    statement.BindNavigationValue(3, companyECInstanceId, companyHasEmployeesRelClassId);

    // Execute statement
    ECInstanceKey employee2ECInstanceId;
    if (BE_SQLITE_DONE != statement.Step(employee2ECInstanceId))
        {
        // do error handling here...
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlInsertRelation()
    {
    ECDb ecdb;
    ECInstanceId monitorECInstanceId;
    ECInstanceId laptopECInstanceId;
    ECInstanceId employee1ECInstanceId;
    ECInstanceId employee2ECInstanceId;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlInsertRelation.sampleCode

    // ECSQL
    // In the ECRelationship HardwareUsedByEmployee the Employee class is the source end and the Hardware class is
    // the target end.
    // Use binding parameters so that the prepared statement can be reused with different values.
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb, "INSERT INTO stco.HardwareUsedByEmployee (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate a monitor to employee 1***
    ECClassId employeeClassId = ecdb.Schemas().GetClassId("stco", "Employee", SchemaLookupMode::ByAlias);
    ECClassId monitorClassId = ecdb.Schemas().GetClassId("stco", "Monitor", SchemaLookupMode::ByAlias);

    // Note: parameter indices are 1-bound!
    statement.BindId(1, employee1ECInstanceId);
    statement.BindId(2, employeeClassId);
    statement.BindId(3, monitorECInstanceId);
    statement.BindId(4, monitorClassId);

    // Execute statement
    ECInstanceKey newRelationshipECInstanceKey;
    // Note: For non-select statements BE_SQLITE_DONE indicates successful execution
    if (BE_SQLITE_DONE != statement.Step(newRelationshipECInstanceKey))
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate a laptop to employee 2***
    // Reset statement so that it can be re-executed.
    statement.Reset();
    // Reset bound values
    statement.ClearBindings();

    ECClassId laptopClassId = ecdb.Schemas().GetClassId("stco", "Laptop", SchemaLookupMode::ByAlias);

    statement.BindId(1, employee2ECInstanceId);
    statement.BindId(2, employeeClassId);
    statement.BindId(3, laptopECInstanceId);
    statement.BindId(4, laptopClassId);

    // Execute statement
    if (BE_SQLITE_DONE != statement.Step(newRelationshipECInstanceKey))
        {
        // do error handling here...
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlAndCustomSQLiteFunctions()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlAndCustomSQLiteFunctions.sampleCode

    // SQLite custom function that computes the power of a given number
    struct PowSqlFunction final : BeSQLite::ScalarFunction
        {
        private:

            void _ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args) override
                {
                if (args[0].IsNull() || args[1].IsNull())
                    {
                    ctx.SetResultError("Arguments to POW must not be NULL", -1);
                    return;
                    }

                double base = args[0].GetValueDouble();
                double exp = args[1].GetValueDouble();

                double res = std::pow(base, exp);
                ctx.SetResultDouble(res);
                }

        public:
            PowSqlFunction() : BeSQLite::ScalarFunction("POW", 2, BeSQLite::DbValueType::FloatVal) {}
        };


    //instantiate the function object. Must remain valid until it is unregistered or the ECDb file is closed.
    PowSqlFunction powFunc;
    if (ecdb.AddFunction(powFunc) != 0)
        return ERROR;

    // ECSQL that computes the area of round tables based on the table radius
    // This illustrates the use of the custom function POW
    ECSqlStatement statement;
    statement.Prepare(ecdb, "SELECT Radius, (3.1415 * POW(Radius,2)) AS Area from stco.RoundTable");

    while (BE_SQLITE_ROW == statement.Step())
        {
        double radius = statement.GetValueDouble(0);
        double area = statement.GetValueDouble(1);
        // do something with the retrieved data of this row
        printf("Radius: %f cm - Area: %f cm^2\n", radius, area);
        }

    ecdb.RemoveFunction(powFunc);

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectMultiThreaded()
    {
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectMultiThreaded.sampleCode
    BeFileName filePath(L"C:\\mypath\\myfile.ecdb");

    // in Thread 1...
    ECDb ecdb;
    ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::Readonly));

    // in Thread 2... 
    // Data source connection must point to very same file and must be opened readonly!
    Db dataSourceECDb;
    dataSourceECDb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::Readonly));

    // in Thread 1 or 2...
    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare(ecdb.Schemas(), dataSourceECDb, "SELECT FirstName, LastName FROM stco.Employee WHERE LastName LIKE 'S%' ORDER BY LastName, FirstName");
    if (!stat.IsSuccess())
        {
        // do error handling here...
        return ERROR;
        }

    // in Thread 2...
    // Execute statement and step over each row of the result set
    while (BE_SQLITE_ROW == statement.Step())
        {
        // do something with the retrieved data of this row
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
