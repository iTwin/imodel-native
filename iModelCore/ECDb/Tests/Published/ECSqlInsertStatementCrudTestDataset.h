/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlInsertStatementCrudTestDataset.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlStatementCrudTestDatasetHelper.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                           Krischan.Eberle            09/13
//=======================================================================================    
struct ECSqlInsertTestDataset
    {
private:
    static const std::vector<Utf8String> s_invalidClassIdValues;

    ECSqlInsertTestDataset ();
    ~ECSqlInsertTestDataset ();

public:
    static ECSqlTestDataset ArrayTests ();
    static ECSqlTestDataset CommonGeometryTests ();
    static ECSqlTestDataset DateTimeTests ();
    static ECSqlTestDataset FunctionTests ();
    static ECSqlTestDataset IntoTests ();
    static ECSqlTestDataset MiscTests (ECDbTestProject& testProject);
    static ECSqlTestDataset ParameterAdvancedTests ();
    static ECSqlTestDataset RelationshipEndTableMappingTests (ECDbTestProject& testProject);
    static ECSqlTestDataset RelationshipLinkTableMappingTests (ECDbTestProject& testProject);
    static ECSqlTestDataset RelationshipWithAnyClassConstraintTests (ECDbTestProject& testProject);
    static ECSqlTestDataset RelationshipWithAdditionalPropsTests (ECDbTestProject& testProject);
    static ECSqlTestDataset RelationshipWithParametersTests (ECDbTestProject& testProject);
    static ECSqlTestDataset StructTests ();
    };

END_ECDBUNITTESTS_NAMESPACE