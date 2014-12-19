/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlUpdateStatementCrudTestDataset.h $
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
struct ECSqlUpdateTestDataset
    {
private:
    ECSqlUpdateTestDataset ();
    ~ECSqlUpdateTestDataset ();
public:
    static ECSqlTestDataset ArrayTests (int rowCountPerClass);
    static ECSqlTestDataset CommonGeometryTests (int rowCountPerClass);
    static ECSqlTestDataset DateTimeTests (int rowCountPerClass);
    static ECSqlTestDataset FunctionTests (int rowCountPerClass);
    static ECSqlTestDataset MiscTests (int rowCountPerClass);
    static ECSqlTestDataset ParameterAdvancedTests (int rowCountPerClass);
    static ECSqlTestDataset PolymorphicTests (int rowCountPerClass);
    static ECSqlTestDataset RelationshipEndTableMappingTests (ECDbTestProject& testProject, int rowCountPerClass);
    static ECSqlTestDataset RelationshipLinkTableMappingTests (ECDbTestProject& testProject, int rowCountPerClass);
    static ECSqlTestDataset RelationshipWithAnyClassConstraintTests (ECDbTestProject& testProject, int rowCountPerClass);
    static ECSqlTestDataset RelationshipWithAdditionalPropsTests (ECDbTestProject& testProject, int rowCountPerClass);
    static ECSqlTestDataset StructTests (int rowCountPerClass);
    static ECSqlTestDataset TargetClassTests (int rowCountPerClass);
    };


END_ECDBUNITTESTS_NAMESPACE