/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlInsertTestDataset.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestFrameworkHelper.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

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
    static ECSqlTestDataset RelationshipLinkTableMappingTests (ECDbR);
    static ECSqlTestDataset RelationshipWithAdditionalPropsTests (ECDbR);
    static ECSqlTestDataset RelationshipWithParametersTests (ECDbR);
    static ECSqlTestDataset StructTests ();
    };

END_ECSQLTESTFRAMEWORK_NAMESPACE