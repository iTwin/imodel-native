/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlDeleteTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlDeleteTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlDeleteTestDataset::FromTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    //Delete classes with base classes
    //*******************************************************
    Utf8CP ecsql = "DELETE FROM ONLY ecsql.TH5";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Delete structs
    //*******************************************************
    //structs which are domain classes
    ecsql = "DELETE FROM ONLY ecsql.SAStruct";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //structs which are not domain classes. They cannot be updated, so this always returns 0 rows affected.
    ecsql = "DELETE FROM ONLY ecsql.PStruct";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Updating CAs
    //*******************************************************
    ecsql = "DELETE FROM ONLY bsca.DateTimeInfo";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Custom Attributes classes are invalid in DELETE statements.");


    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql.PUnmapped";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in DELETE statements.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    //by contract non-polymorphic deletes on abstract classes are valid, but are a no-op
    ecsql = "DELETE FROM ONLY ecsql.Abstract";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql.Sub1";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);


    //*******************************************************
    // Unsupported classes
    //*******************************************************
    ecsql = "DELETE FROM ONLY bsm.AnyClass";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY bsm.InstanceCount";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "DELETE FROM ONLY P SET I=123, L=100000";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "DELETE FROM ONLY ecsql.BlaBla SET I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY blabla.P SET I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY ecsql.P SET Garbage='bla', I=123";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid, "One of the properties does not exist in the target class.");

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlDeleteTestDataset::MiscTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    //*******************************************************
    // Syntactically incorrect statements 
    //*******************************************************
    Utf8CP ecsql = "";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE ONLY ecsql.P";
    ECSqlTestFrameworkHelper::AddPrepareFailing (dataset, ecsql, ECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ecsql = "DELETE FROM ONLY ecsql.P t WHERE t.D > 0.0";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.P t WHERE t.S = ?";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    //*******************************************************
    // Delete clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql._UnderBar u WHERE u.ABC_ = ?";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);
    //Delete clause with square brackets
    ecsql = "DELETE FROM ONLY ecsql.[_UnderBar] u WHERE u.[ABC_] = ?";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlDeleteTestDataset::PolymorphicTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "DELETE FROM ecsql.P";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ecsql.Abstract";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.Abstract";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ecsql.AbstractTablePerHierarchy";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractTablePerHierarchy";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ecsql.THBase";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.THBase";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    ecsql = "DELETE FROM ecsql.TCBase";
    ECSqlTestFrameworkHelper::AddNonSelect (dataset, ecsql, true);

    ecsql = "DELETE FROM ONLY ecsql.TCBase";
    ECSqlTestFrameworkHelper::AddNonSelect(dataset, ecsql, true);

    return dataset;
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE