/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlSelectStatementCrudTestDataset.h $
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
struct ECSqlSelectTestDataset
    {
private:
    ECSqlSelectTestDataset ();
    ~ECSqlSelectTestDataset ();
public:
    static ECSqlTestDataset AliasTests (int rowCountPerClass);
    static ECSqlTestDataset ArrayTests (int rowCountPerClass);
    static ECSqlTestDataset BetweenOperatorTests (int rowCountPerClass);
    static ECSqlTestDataset CastTests (int rowCountPerClass);
    static ECSqlTestDataset CommonGeometryTests (int rowCountPerClass);
    static ECSqlTestDataset DateTimeTests (int rowCountPerClass);
    static ECSqlTestDataset ECInstanceIdTests (int rowCountPerClass);
    static ECSqlTestDataset FromTests (int rowCountPerClass);
    static ECSqlTestDataset FunctionTests (int rowCountPerClass);
    static ECSqlTestDataset GroupByTests (int rowCountPerClass);
    static ECSqlTestDataset InOperatorTests (int rowCountPerClass);
    static ECSqlTestDataset JoinTests (int rowCountPerClass);
    static ECSqlTestDataset LikeOperatorTests (int rowCountPerClass);
    static ECSqlTestDataset LimitTests (int rowCountPerClass);
    static ECSqlTestDataset MiscTests (int rowCountPerClass);
    static ECSqlTestDataset NullLiteralTests (int rowCountPerClass);
    static ECSqlTestDataset OrderByTests (int rowCountPerClass);
    static ECSqlTestDataset ParameterAdvancedTests (int rowCountPerClass);
    static ECSqlTestDataset PointTests (int rowCountPerClass);
    static ECSqlTestDataset PolymorphicTests (int rowCountPerClass);
    static ECSqlTestDataset PrimitiveTests (int rowCountPerClass);
    static ECSqlTestDataset SourceTargetConstraintTests (int rowCountPerClass);
    static ECSqlTestDataset StructTests (int rowCountPerClass);
    static ECSqlTestDataset SubqueryTests (int rowCountPerClass);
    };

END_ECDBUNITTESTS_NAMESPACE