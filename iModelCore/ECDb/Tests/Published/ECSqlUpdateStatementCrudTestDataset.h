/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlUpdateStatementCrudTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestFrameworkHelper.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

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
    static ECSqlTestDataset RelationshipEndTableMappingTests (int rowCountPerClass);
    static ECSqlTestDataset RelationshipLinkTableMappingTests (int rowCountPerClass);
    static ECSqlTestDataset RelationshipWithAnyClassConstraintTests (int rowCountPerClass);
    static ECSqlTestDataset RelationshipWithAdditionalPropsTests (ECDbR, int rowCountPerClass);
    static ECSqlTestDataset StructTests (int rowCountPerClass);
    static ECSqlTestDataset TargetClassTests (int rowCountPerClass);
    };


END_ECSQLTESTFRAMEWORK_NAMESPACE