/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlDeleteStatementCrudTestDataset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlDeleteStatementCrudTestDataset.h"

//Note: Please keep methods for a given class alphabetized
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

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
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, rowCountPerClass, true);

    //*******************************************************
    // Delete structs
    //*******************************************************
    //structs which are domain classes
    ecsql = "DELETE FROM ONLY ecsql.SAStruct";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, rowCountPerClass, true);
    //ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::NotYetSupported, "Instances with struct array props cannot be deleted yet");

    //structs which are not domain classes. They cannot be updated, so this always returns 0 rows affected.
    ecsql = "DELETE FROM ONLY ecsql.PStruct";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    //*******************************************************
    // Updating CAs
    //*******************************************************
    ecsql = "DELETE FROM ONLY bsca.DateTimeInfo";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Custom Attributes classes are invalid in DELETE statements.");


    //*******************************************************
    // Unmapped classes
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql.PUnmapped";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Unmapped classes cannot be used in DELETE statements.");

    //*******************************************************
    // Abstract classes
    //*******************************************************
    //by contract non-polymorphic deletes on abstract classes are valid, but are a no-op
    ecsql = "DELETE FROM ONLY ecsql.Abstract";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    //*******************************************************
    // Subclasses of abstract class
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql.Sub1";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, rowCountPerClass, true);


    //*******************************************************
    // Unsupported classes
    //*******************************************************
    ecsql = "DELETE FROM ONLY bsm.AnyClass";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY bsm.InstanceCount";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //Class which has a relationship subclass. The latter is unmapped, so testing here that the base class can still be used
    ecsql = "DELETE FROM ONLY ecsql.NonRelBase";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, rowCountPerClass, true);

    //Relationship class which has a class as base class which is not supported
    ecsql = "DELETE FROM ONLY ecsql.RelSubclassOfNonRel";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY ecsql.RelWithExplicitRelConstraint";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY ecsql.RelWithImplicitRelConstraint";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Missing schema prefix / not existing ECClasses / not existing ECProperties
    //*******************************************************
    ecsql = "DELETE FROM ONLY P SET I=123, L=100000";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "Class name needs to be prefixed by schema prefix.");

    ecsql = "DELETE FROM ONLY ecsql.BlaBla SET I=123";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY blabla.P SET I=123";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM ONLY ecsql.P SET Garbage='bla', I=123";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid, "One of the properties does not exist in the target class.");

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
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE FROM";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    ecsql = "DELETE ONLY ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddPrepareFailing (dataset, ecsql, IECSqlExpectedResult::Category::Invalid);

    //*******************************************************
    // Class aliases
    //*******************************************************
    //In SQLite they are not allowed, but ECSQL allows them. So test that ECDb properly ommits them
    //during preparation
    ecsql = "DELETE FROM ONLY ecsql.P t WHERE t.D > 0.0";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, rowCountPerClass, true);

    ecsql = "DELETE FROM ONLY ecsql.P t WHERE t.S = ?";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    //*******************************************************
    // Delete clause in which the class name and the properties name contain, start with or end with under bar
    //*******************************************************
    ecsql = "DELETE FROM ONLY ecsql._UnderBar u WHERE u.ABC_ = ?";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);
    //Delete clause with square brackets
    ecsql = "DELETE FROM ONLY ecsql.[_UnderBar] u WHERE u.[ABC_] = ?";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);

    return dataset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset ECSqlDeleteTestDataset::PolymorphicTests (int rowCountPerClass)
    {
    ECSqlTestDataset dataset;

    Utf8CP ecsql = "DELETE FROM ecsql.P";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, rowCountPerClass, true);

    ecsql = "DELETE FROM ecsql.Abstract";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.Abstract";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractNoSubclasses";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ecsql.AbstractTablePerHierarchy";
    //ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, rowCountPerClass * 2, true);
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.AbstractTablePerHierarchy";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ecsql.THBase";
    //ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, 6 * rowCountPerClass, true);
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.THBase";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, rowCountPerClass, true);

    ecsql = "DELETE FROM ecsql.TCBase";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect (dataset, ecsql, 0, true);

    ecsql = "DELETE FROM ONLY ecsql.TCBase";
    ECSqlStatementCrudTestDatasetHelper::AddNonSelect(dataset, ecsql, rowCountPerClass, true);

    return dataset;
    }



END_ECDBUNITTESTS_NAMESPACE
