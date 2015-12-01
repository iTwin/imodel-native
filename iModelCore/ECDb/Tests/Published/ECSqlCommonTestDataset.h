/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlCommonTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
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

    static ECSqlTestItem& AddTestItem (ECSqlTestDataset&, ECSqlType, Utf8CP ecsql, int expectedResultRows);
    static bool ToECSql (Utf8StringR ecsql, ECSqlType, ECN::ECClassCR targetClass, bool polymorphic);
    static bool FindPrimitivePropertyAccessStringInClass (Utf8StringR propertyAccessString, ECN::ECClassCR, bool includeBaseProperties);

public:
    //Generates test datasets common to all ECSQL types
    static ECSqlTestDataset WhereAbstractClassTests (ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereAndOrPrecedenceTests(ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereBasicsTests(ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereCommonGeometryTests (ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereFunctionTests (ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereMatchTests(ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipEndTableMappingTests(ECSqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipLinkTableMappingTests (ECSqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereRelationshipWithAdditionalPropsTests (ECSqlType, ECDbR, int rowCountPerClass);
    static ECSqlTestDataset WhereStructTests (ECSqlType, ECDbCR, int rowCountPerClass);
    static ECSqlTestDataset OptionsTests(ECSqlType, ECDbCR, int rowCountPerClass);
    };

END_ECSQLTESTFRAMEWORK_NAMESPACE