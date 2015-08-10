/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/docs/samplecode/ECDbCRUD.sample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <cmath> // for std::pow

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelect ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelect.sampleCode

    // Prepare statement
    Utf8CP ecsql = "SELECT FirstName, LastName, Birthday FROM stco.Employee WHERE LastName LIKE 'S%' ORDER BY LastName, FirstName";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        // do error handling here...
        return ERROR;
        }

    // Execute statement and step over each row of the result set
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        // Property indices in result set are 0-based -> 0 refers to first property in result set
        Utf8CP firstName = statement.GetValueText (0);
        Utf8CP lastName = statement.GetValueText (1);
        DateTime birthday = statement.GetValueDateTime (2);
        // do something with the retrieved data of this row
        printf ("%s, %s - Born %s\n", lastName, firstName, birthday.ToUtf8String ().c_str ());
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectBuilder ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectBuilder.sampleCode
    
    // Get class to perform ECSQL statement against
    ECClassCP employeeClass = ecdb.Schemas ().GetECClass ("StartupCompany", "Employee");
    if (employeeClass == nullptr)
        {
        // class not found. do error handling here...
        return ERROR;
        }
    
    // Define ECSQL via ECSqlBuilder
    ECSqlSelectBuilder ecsqlBuilder;
    ecsqlBuilder.Select ("FirstName, LastName, Birthday").From (*employeeClass).Where ("LastName LIKE 'S%'").OrderBy ("LastName, FirstName");
    
    Utf8String ecsql = ecsqlBuilder.ToString ();

    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, ecsql.c_str ());
    if (stat != ECSqlStatus::Success)
        {
        // do error handling here...
        return ERROR;
        }

    // Execute statement and step over each row of the result set
    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        // Property indices in result set are 0-based -> 0 refers to first property in result set
        Utf8CP firstName = statement.GetValueText (0);
        Utf8CP lastName = statement.GetValueText (1);
        DateTime birthday = statement.GetValueDateTime (2);
        // do something with the retrieved data of this row
        printf ("%s, %s - Born %s\n", lastName, firstName, birthday.ToUtf8String ().c_str ());
        }

//__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectStructProps ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructProps.sampleCode

    Utf8CP ecsql = "SELECT FirstName, Address FROM stco.Employee";

    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        Utf8CP firstName = statement.GetValueText (0);
        //second item in select clause is a struct prop, so call GetStructReader
        IECSqlStructValue const& addressStructValue = statement.GetValueStruct (1);
        
        // Print out the address for each employee
        printf ("Employee %s:\n", firstName);
        const int addressMemberCount = addressStructValue.GetMemberCount ();
        for (int i = 0; i < addressMemberCount; i++)
            {
            IECSqlValue const& addressMemberValue = addressStructValue.GetValue (i);
            ECSqlColumnInfoCR addressMemberColumnInfo = addressMemberValue.GetColumnInfo ();
            
            //print out name of address member
            Utf8StringCR addressMemberColumnName = addressMemberColumnInfo.GetProperty ()->GetName ();
            printf ("   %s: ", Utf8String (addressMemberColumnName).c_str ());
            //print out value of address member
            PrimitiveType type = addressMemberColumnInfo.GetDataType ().GetPrimitiveType ();
            switch (type)
                {
                case PRIMITIVETYPE_String:
                    printf ("%s", addressMemberValue.GetText ());
                    break;
                case PRIMITIVETYPE_Integer:
                    printf ("%d", addressMemberValue.GetInt ());
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
BentleyStatus ECDb_ECSqlSelectStructPropMembers ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructPropMembers.sampleCode

    //Only interested in street, city and zip although Location struct has more properties
    Utf8CP ecsql = "SELECT FirstName, Address.Street, Address.City, Address.Zip FROM stco.Employee";

    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        Utf8CP firstName = statement.GetValueText (0);

        Utf8CP streetName = statement.GetValueText (1);
        Utf8CP city = statement.GetValueText (2);
        int zip = statement.GetValueInt (3);

        // Print out the address for each employee
        printf ("Employee %s\n", firstName);
        printf ("   %s\n", streetName);
        printf ("   %s %d\n", city, zip);
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectPrimArrayProps ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectPrimArrayProps.sampleCode

    //Let MobilePhones be a string array property of the Employee class
    Utf8CP ecsql = "SELECT FirstName, LastName, MobilePhones FROM myschema.Employee";

    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        Utf8CP firstName = statement.GetValueText (0);
        Utf8CP lastName = statement.GetValueText (1);
        IECSqlArrayValue const& mobilePhoneArrayValue = statement.GetValueArray (2);

        printf ("Mobile numbers for %s %s:\n", firstName, lastName);
        int arrayIndex = 0;
        for (IECSqlValue const* arrayMemberValue : mobilePhoneArrayValue)
            {
            //Primitive arrays always have one column. Therefore always pass 0 to the reader when retrieving array values.
            Utf8CP mobileNumber = arrayMemberValue->GetText ();
            printf ("#%d: %s\n", arrayIndex + 1, mobileNumber);
            arrayIndex++;
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectStructArrayProps ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlSelectStructArrayProps.sampleCode

    Utf8CP ecsql = "SELECT FirstName, Certifications FROM stco.Employee";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        Utf8CP firstName = statement.GetValueText (0);
        IECSqlArrayValue const& certStructArray = statement.GetValueArray (1);
        //now iterate over struct array elements. Each steps moves one struct element further in the array.
        // Print out the certifications for each employee
        printf ("Employee %s\n", firstName);
        printf ("  Certifications:\n");
        int arrayIndex = 0;
        for (IECSqlValue const* arrayElement : certStructArray)
            {
            printf ("  #%d:\n", arrayIndex + 1);
            IECSqlStructValue const& certStructArrayElement = arrayElement->GetStruct ();
            const int memberCount = certStructArrayElement.GetMemberCount ();
            for (int i = 0; i < memberCount; i++)
                {
                IECSqlValue const& certMember = certStructArrayElement.GetValue (i);
                ECSqlColumnInfoCR certMemberColumnInfo = certMember.GetColumnInfo ();

                //print out name of cert member
                Utf8StringCR certMemberColumnName = certMemberColumnInfo.GetProperty ()->GetName ();
                printf ("   %s: ", Utf8String (certMemberColumnName).c_str ());
                //print out value of cert member
                PrimitiveType type = certMemberColumnInfo.GetDataType ().GetPrimitiveType ();
                switch (type)
                    {
                    case PRIMITIVETYPE_String:
                        printf ("%s", certMember.GetText ());
                        break;
                    case PRIMITIVETYPE_Integer:
                        printf ("%d", certMember.GetInt ());
                        break;
                    case PRIMITIVETYPE_DateTime:
                        printf ("%s", certMember.GetDateTime ().ToUtf8String ().c_str ());
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
BentleyStatus ECDb_ECSqlStatementAndDateTime ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementAndDateTime.sampleCode

    Utf8CP ecsql = "SELECT Birthday, LastModified FROM stco.Employee";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        DateTime birthday = statement.GetValueDateTime (0); //dt.GetInfo ().GetComponent () will amount to DateTime::Component::Date
        DateTime lastModified = statement.GetValueDateTime (1); //dt.GetInfo ().GetKind () will amount to DateTime::Kind::Utc

        DateTime::Info const& birthdayInfo = birthday.GetInfo ();
        DateTime::Info const& lastModifiedInfo = lastModified.GetInfo ();
        printf ("Birthday: Kind %s, Component %s\n", 
        DateTime::Info::KindToString (birthdayInfo.GetKind ()).c_str (), 
        DateTime::Info::ComponentToString (birthdayInfo.GetComponent ()).c_str ());
        printf ("LastModified: Kind %s, Component %s\n", 
        DateTime::Info::KindToString (lastModifiedInfo.GetKind ()).c_str (), 
        DateTime::Info::ComponentToString (lastModifiedInfo.GetComponent ()).c_str ());
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlSelectWithJoin ()
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
    statement.Prepare (ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    statement.BindText (1, candidateEmployeeFirstName, IECSqlBinder::MakeCopy::No);
    statement.BindText (2, candidateEmployeeLastName, IECSqlBinder::MakeCopy::No);
    // Execute statement
    ECSqlStepStatus stat = statement.Step ();
    switch (stat)
        {
        case ECSqlStepStatus::HasRow:
            {
            // Note: GetValue parameter indices are 0-based!
            Utf8CP companyName = statement.GetValueText (0);
            printf ("%s %s works for company '%s'.\n", candidateEmployeeFirstName, candidateEmployeeLastName, companyName);
            break;
            }
        case ECSqlStepStatus::Done:
            printf ("No company found for which %s %s works.\n", candidateEmployeeFirstName, candidateEmployeeLastName);
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
BentleyStatus ECDb_ECSqlStatementBindingPrimitives ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingPrimitives.sampleCode

    // Prepare statement
    Utf8CP ecsql = "INSERT INTO stco.Employee (FirstName, LastName, Birthday) VALUES (?, ?, ?)";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    //*** #1 Insert first employee 
    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    statement.BindText (1, "Joan", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (2, "Rieck", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime (3, DateTime (1965, 4, 12));

    // Execute statement
    ECInstanceKey ecInstanceKey;
    ECSqlStepStatus stepStat = statement.Step (ecInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
        //INSERT statements are expected to return ECSqlStepStatus::Done if successful.
        return ERROR;

    //*** #2 Insert second employee 
    statement.Reset (); //reset statement, so that it can be reused
    statement.ClearBindings (); //clear previous bindings

    // Bind values to parameters
    statement.BindText (1, "Bill", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (2, "Becker", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime (3, DateTime (1959, 1, 31));

    // Execute statement
    stepStat = statement.Step (ecInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
        //INSERT statements are expected to return ECSqlStepStatus::Done if successful.
        return ERROR;

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingWithNamedParameters ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingWithNamedParameters.sampleCode

    // Prepare statement
    Utf8CP ecsql = "SELECT LastName FROM stco.Employee LIMIT :pagesize OFFSET (:pageno + 1) * :pagesize";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    const int pageSize = 50;
    int pageNumber = 0;

    const int pageSizeParameterIndex = statement.GetParameterIndex ("pagesize");
    const int pageNoParameterIndex = statement.GetParameterIndex ("pageno");

    //*** #1 Read first page
    statement.BindInt (pageSizeParameterIndex, pageSize);
    statement.BindInt (pageNoParameterIndex, pageNumber);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //process first page
        }

    //*** #2 Read second page
    statement.Reset (); //reset statement, so that it can be reused
    statement.ClearBindings (); //clear previous bindings

    pageNumber++;
    statement.BindInt (pageSizeParameterIndex, pageSize);
    statement.BindInt (pageNoParameterIndex, pageNumber);

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //process second page
        }

    //__PUBLISH_EXTRACT_END__

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingStructs ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingStructs.sampleCode
    
    // Task: Insert a employee where the property Address is of a struct type called Location

    // Note: this example also shows complete error handling.

    // Prepare statement
    Utf8CP ecsql = "INSERT INTO stco.Employee (FirstName, LastName, Address) VALUES (?, ?, ?)";
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        return ERROR;

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    stat = statement.BindText (1, "Joan", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        return ERROR;
    
    stat = statement.BindText (2, "Smith", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        return ERROR;

    IECSqlStructBinder& addressBinder = statement.BindStruct (3);
    //For methods not returning a status, ECSqlStatement::GetLastStatus can be called.
    if (statement.GetLastStatus () != ECSqlStatus::Success)
        return ERROR;

    stat = addressBinder.GetMember ("Street").BindText ("2000 Main Street", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        return ERROR;

    stat = addressBinder.GetMember ("Zip").BindInt (10014);
    if (stat != ECSqlStatus::Success)
        return ERROR;

    stat = addressBinder.GetMember ("City").BindText ("New York", IECSqlBinder::MakeCopy::Yes);
    if (stat != ECSqlStatus::Success)
        return ERROR;

    //Note: all other members of the struct Address which are not set through the binding API are set to DB NULL.

    // Execute statement
    ECInstanceKey ecInstanceKey;
    ECSqlStepStatus stepStat = statement.Step (ecInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
        //INSERT statements are expected to return ECSqlStepStatus::Done if successful.
        return ERROR;

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingPrimArrays ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingPrimArrays.sampleCode
    // Task: Add a list of mobile phone numbers to an employee. The phone numbers are held
    // in a string array property called MobilePhones

    // Prepare statement
    Utf8CP ecsql = "UPDATE ONLY stco.Employee SET MobilePhones = ? WHERE FirstName = ? AND LastName = ?";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    IECSqlArrayBinder& phoneNumbersBinder = statement.BindArray (1, //parameter index
                                                                3); //initial array capacity
    //For methods not returning a status, ECSqlStatement::GetLastStatus can be called.
    if (statement.GetLastStatus () != ECSqlStatus::Success)
        return ERROR;

    std::vector<Utf8String> phoneNumbers = { "+1 (610) 726-4312", "+1 (610) 726-4444", "+1 (610) 726-4112" };
    for (Utf8StringCR phoneNumber : phoneNumbers)
        {
        IECSqlBinder& phoneNumberBinder = phoneNumbersBinder.AddArrayElement ();
        //For methods not returning a status, ECSqlStatement::GetLastStatus can be called.
        if (statement.GetLastStatus () != ECSqlStatus::Success)
            return ERROR;

        ECSqlStatus stat = phoneNumberBinder.BindText (phoneNumber.c_str (), IECSqlBinder::MakeCopy::Yes);
        if (stat != ECSqlStatus::Success)
            return ERROR;
        }

    //now bind to parameter 2 and 3 (primitive binding) (skipping error handling for better readability)
    statement.BindText (2, "John", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (3, "Smith", IECSqlBinder::MakeCopy::Yes);

    // Execute statement
    ECSqlStepStatus stepStat = statement.Step ();
    if (stepStat != ECSqlStepStatus::Done)
        //UPDATE statements are expected to return ECSqlStepStatus::Done if successful.
        return ERROR;

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementBindingStructArrays ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementBindingStructArrays.sampleCode
    // Task: Add a list of certifications for an employee. The certifications are held
    // in a struct array property called Certifications where the array elements are of
    // type Certification.

    // Prepare statement
    Utf8CP ecsql = "UPDATE ONLY stco.Employee SET Certifications = ? WHERE FirstName = ? AND LastName = ?";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    // Bind values to parameters
    // Note: bind parameter indices are 1-based!
    IECSqlArrayBinder& certificationsBinder = statement.BindArray (1, //parameter index
                                                                2); //initial array capacity

    //AddArrayElement gets a binder for the array element. As the array element is a struct, we need to call
    //BindStruct (just as with other struct properties)
    IECSqlStructBinder& firstCertBinder = certificationsBinder.AddArrayElement ().BindStruct ();
    firstCertBinder.GetMember ("Name").BindText ("Certified Engineer Assoc.", IECSqlBinder::MakeCopy::Yes);
    firstCertBinder.GetMember ("StartsOn").BindDateTime (DateTime (2014, 01, 28));

    IECSqlStructBinder& secondCertBinder = certificationsBinder.AddArrayElement ().BindStruct ();
    secondCertBinder.GetMember ("Name").BindText ("Professional Engineers", IECSqlBinder::MakeCopy::Yes);
    secondCertBinder.GetMember ("StartsOn").BindDateTime (DateTime (2011, 05, 14));

    //now bind to parameter 2 and 3 (primitive binding) (skipping error handling for better readability)
    statement.BindText (2, "Patty", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (3, "Rieck", IECSqlBinder::MakeCopy::Yes);

    // Execute statement
    ECSqlStepStatus stepStat = statement.Step ();
    if (stepStat != ECSqlStepStatus::Done)
        //UPDATE statements are expected to return ECSqlStepStatus::Done if successful.
        return ERROR;


    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlColumnInfoOnNestedLevels ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlColumnInfoOnNestedLevels.sampleCode

    Utf8CP ecsql = "SELECT FirstName, Certifications FROM stco.Employee WHERE ECInstanceId = 123";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    auto printColumnInfoCategoryLambda = [] (ECSqlColumnInfoCR columnInfo)
        {
        //Print property path
        Utf8String propPath = columnInfo.GetPropertyPath ().ToString ();
        printf ("%s: ", propPath.c_str ());
        //append type IECSqlBinder
        ECTypeDescriptor const& dataType = columnInfo.GetDataType ();
        if (dataType.IsPrimitive ())
            printf ("Primitive type\n");
        else if (dataType.IsStruct ())
            printf ("ECStruct type\n");
        else if (dataType.IsPrimitiveArray ())
            printf ("Primitive array type\n");
        else
            printf ("ECStruct array type\n");
        };

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        printf ("Top level\n");
        ECSqlColumnInfoCR firstNameColumnInfo = statement.GetColumnInfo (0);
        printColumnInfoCategoryLambda (firstNameColumnInfo);
        
        ECSqlColumnInfoCR certArrayColumnInfo = statement.GetColumnInfo (1);
        printColumnInfoCategoryLambda (certArrayColumnInfo);

        printf ("\nStruct array element level\n");
        IECSqlArrayValue const& certArray = statement.GetValueArray (1);
        for (IECSqlValue const* arrayElement : certArray)
            {
            IECSqlStructValue const& certElement = arrayElement->GetStruct ();
            int arrayColumnCount = certElement.GetMemberCount ();
            for (int i = 0; i < arrayColumnCount; i++)
                {
                ECSqlColumnInfoCR certMemberColumnInfo = certElement.GetValue (i).GetColumnInfo ();
                printColumnInfoCategoryLambda (certMemberColumnInfo);
                }

            printf ("\n");
            }
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

void DoSomething (ECSqlColumnInfoCR);
void DoSomething (Utf8CP);
void DoSomething (IECSqlStructValue const&);

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlStatementGetValueErrorHandling ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlStatementGetValueErrorHandling.sampleCode
    Utf8CP ecsql = "SELECT FirstName, Address FROM stco.Employee";
    ECSqlStatement statement;
    statement.Prepare (ecdb, ecsql);

    //define a lambda for the error checking to keep the sample code more concise
    auto checkErrorLambda = [&statement] ()
        {
        ECSqlStatus stat = statement.GetLastStatus ();
        if (stat != ECSqlStatus::Success)
            {
            printf ("Error when retrieving value from ECSqlStatement: %s\n", statement.GetLastStatusMessage ().c_str ());
            return false;
            }

        return true;
        };

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        //valid call
        ECSqlColumnInfoCR validColumnInfo = statement.GetColumnInfo (0);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (validColumnInfo);

        //invalid call: column index too large
        ECSqlColumnInfoCR invalidColumnInfo = statement.GetColumnInfo (10);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (invalidColumnInfo);

        //valid call
        Utf8CP validStringVal = statement.GetValueText (0);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (validStringVal);

        //invalid call: GetValueText with too large column index:
        Utf8CP invalidStringVal = statement.GetValueText (10);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (invalidStringVal);

        //invalid call: GetValueText with too large column index:
        invalidStringVal = statement.GetValueText (10);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (invalidStringVal);

        //invalid call: GetValueText on struct property:
        invalidStringVal = statement.GetValueText (1);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (invalidStringVal);

        //valid call
        IECSqlStructValue const& validStructValue = statement.GetValueStruct (1);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (validStructValue);

        //invalid call: GetStructReader on string property
        IECSqlStructValue const& invalidStructValue = statement.GetValueStruct (0);
        if (!checkErrorLambda ())
            return ERROR;
        else
            DoSomething (invalidStructValue);
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlInsert ()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlInsert.sampleCode

    // ECSQL
    // Use binding parameters so that the prepared statement can be reused with different values
    Utf8CP ecsql = "INSERT INTO stco.Employee (FirstName, LastName, Birthday) VALUES (?, ?, ?)";

    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        // do error handling here...
        return ERROR;
        }

    //*** Insert first employee ***
    // Bind first address values to parameters of the ECSQL statement
    // Note: parameter indices are 1-bound!
    statement.BindText (1, "John", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (2, "Smith", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime (3, DateTime (1984, 5, 16));

    // Execute statement
    ECInstanceKey newECInstanceKey;
    ECSqlStepStatus stepStat = statement.Step (newECInstanceKey);
    // Note: For non-select statements BE_SQLITE_DONE indicates successful execution
    if (stepStat != ECSqlStepStatus::Done)
        {
        // do error handling here...
        return ERROR;
        }

    //*** Insert second employee ***
    // Reset statement so that it can be re-executed.
    statement.Reset ();
    // Reset bound values
    statement.ClearBindings ();
    
    // Now bind second address values to parameters of the ECSQL statement
    statement.BindText (1, "Anne", IECSqlBinder::MakeCopy::Yes);
    statement.BindText (2, "Tyler", IECSqlBinder::MakeCopy::Yes);
    statement.BindDateTime (3, DateTime (1979, 12, 1));

    // Execute statement
    stepStat = statement.Step (newECInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
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
BentleyStatus ECDb_ECSqlInsertRelation ()
    {
    ECDb ecdb;
    ECClassCP companyClass = nullptr;
    ECInstanceId companyECInstanceId;
    ECClassCP employeeClass = nullptr;
    ECInstanceId employee1ECInstanceId;
    ECInstanceId employee2ECInstanceId;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlInsertRelation.sampleCode

    // ECSQL
    // In the ECRelationship CompanyHasEmployees the Company class is the source end and the Employee class is
    // the target end.
    // Use binding parameters so that the prepared statement can be reused with different values.
    Utf8CP ecsql = "INSERT INTO stco.CompanyHasEmployees (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)";

    // Prepare statement
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate first employee to company***
    // Preparational step not shown in the example: Retrieve the ECInstanceIds and ECClassIds of the instances to relate

    // Note: parameter indices are 1-bound!
    statement.BindId (1, companyECInstanceId);
    statement.BindInt64 (2, companyClass->GetId ());
    statement.BindId (3, employee1ECInstanceId);
    statement.BindInt64 (4, employeeClass->GetId ());

    // Execute statement
    ECInstanceKey newRelationshipECInstanceKey;
    ECSqlStepStatus stepStat = statement.Step (newRelationshipECInstanceKey);
    // Note: For non-select statements BE_SQLITE_DONE indicates successful execution
    if (stepStat != ECSqlStepStatus::Done)
        {
        // do error handling here...
        return ERROR;
        }

    //*** Relate second employee to company***
    // Reset statement so that it can be re-executed.
    statement.Reset ();
    // Reset bound values
    statement.ClearBindings ();

    statement.BindId (1, companyECInstanceId);
    statement.BindInt64 (2, companyClass->GetId ());
    statement.BindId (3, employee2ECInstanceId);
    statement.BindInt64 (4, employeeClass->GetId ());

    // Execute statement
    stepStat = statement.Step (newRelationshipECInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
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
    struct PowSqlFunction : BeSQLite::ScalarFunction
        {
        private:

            virtual void _ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args) override
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

    while (statement.Step() == ECSqlStepStatus::HasRow)
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

#ifdef WIP_MERGE
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   04/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb_ECSqlAndCustomSQLiteFunctions()
    {
    ECDb ecdb;

    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ECSqlAndCustomSQLiteFunctions.sampleCode

    // SQLite custom function that computes the power of a given number
    struct PowSqlFunction : BeSQLite::ScalarFunction, BeSQLite::ScalarFunction::IScalar
        {
        private:

            virtual void _ComputeScalar(BeSQLite::ScalarFunction::Context* ctx, int nArgs, BeSQLite::DbValue* args) override
                {
                if (nArgs != GetNumArgs())
                    {
                    ctx->SetResultError("Function POW expects 2 arguments.", -1);
                    return;
                    }

                if (args[0].IsNull() || args[1].IsNull())
                    {
                    ctx->SetResultError("Arguments to POW must not be NULL", -1);
                    return;
                    }

                double base = args[0].GetValueDouble();
                double exp = args[1].GetValueDouble();

                double res = std::pow(base, exp);
                ctx->SetResultDouble(res);
                }

        public:
            PowSqlFunction() : BeSQLite::ScalarFunction("POW", 2, BeSQLite::DbValueType::FloatVal, this) {}
        };


    //instantiate the function object. Must remain valid until it is unregistered or the ECDb file is closed.
    PowSqlFunction powFunc;
    if (ecdb.AddScalarFunction(powFunc) != 0)
        return ERROR;

    // ECSQL that computes the area of round tables based on the table radius
    // This illustrates the use of the custom function POW
    ECSqlStatement statement;
    statement.Prepare(ecdb, "SELECT Radius, (3.1415 * POW(Radius,2)) AS Area from stco.RoundTable");

    while (statement.Step() == ECSqlStepStatus::HasRow)
        {
        double radius = statement.GetValueDouble(0);
        double area = statement.GetValueDouble(1);
        // do something with the retrieved data of this row
        printf("Radius: %f cm - Area: %f cm^2\n", radius, area);
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }
#endif
    
END_BENTLEY_SQLITE_EC_NAMESPACE
