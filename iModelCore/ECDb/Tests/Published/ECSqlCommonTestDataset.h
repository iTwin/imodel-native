/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCommonTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
// @bsiclass                                           Krischan.Eberle            01/14
//=======================================================================================    
enum class ECSqlType
    {
    Select,
    Insert,
    Update,
    Delete
    };

//=======================================================================================    
// @bsiclass                                           Krischan.Eberle            01/14
//=======================================================================================    
struct ECSqlCommonTestDataset
    {
private:
    ECSqlCommonTestDataset ();
    ~ECSqlCommonTestDataset ();

    static ECSqlTestItem& AddTestItem (ECSqlTestDataset& dataset, ECSqlType ecsqlType, Utf8CP ecsql, int expectedResultRows);
    static bool ToECSql (Utf8StringR ecsql, ECSqlType type, ECN::ECClassCR targetClass, bool polymorphic);
    static bool FindPrimitivePropertyAccessStringInClass (Utf8StringR propertyAccessString, ECN::ECClassCR ecClass, bool includeBaseProperties);

public:
    //Generates test datasets common to all ECSQL types
    static ECSqlTestDataset WhereAbstractClassTests (ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereAndOrPrecedenceTests(ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereBasicsTests(ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereCommonGeometryTests (ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereFunctionTests (ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereMatchTests(ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipEndTableMappingTests(ECSqlType ecsqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipLinkTableMappingTests (ECSqlType ecsqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipWithAnyClassConstraintTests (ECSqlType ecsqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipWithAdditionalPropsTests (ECSqlType ecsqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereStructTests (ECSqlType ecsqlType, ECDbCR, int rowCountPerClass);
    };

END_ECDBUNITTESTS_NAMESPACE