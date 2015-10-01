/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlInsertStatementCrudTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    static ECSqlTestDataset MiscTests (ECDbR);
    static ECSqlTestDataset ParameterAdvancedTests ();
    static ECSqlTestDataset RelationshipEndTableMappingTests (ECDbR);
    static ECSqlTestDataset RelationshipLinkTableMappingTests (ECDbCR);
    static ECSqlTestDataset RelationshipWithAnyClassConstraintTests (ECDbR);
    static ECSqlTestDataset RelationshipWithAdditionalPropsTests (ECDbCR);
    static ECSqlTestDataset RelationshipWithParametersTests (ECDbR);
    static ECSqlTestDataset StructTests ();
    };

END_ECDBUNITTESTS_NAMESPACE