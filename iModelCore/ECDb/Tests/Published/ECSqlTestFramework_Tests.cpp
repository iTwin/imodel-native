/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlTestFramework_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestFrameworkFixture.h"
#include "ECSqlCommonTestDataset.h"
#include "ECSqlSelectTestDataset.h"
#include "ECSqlInsertTestDataset.h"
#include "ECSqlUpdateTestDataset.h"
#include "ECSqlDeleteTestDataset.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

//********* Select ***************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, ArrayTests)
    {
    auto dataset = ECSqlSelectTestDataset::ArrayTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, BetweenOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::BetweenOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, CastTests)
    {
    auto dataset = ECSqlSelectTestDataset::CastTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, CommonGeometryTests)
    {
    auto dataset = ECSqlSelectTestDataset::CommonGeometryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, DateTimeTests)
    {
    auto dataset = ECSqlSelectTestDataset::DateTimeTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                      10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, AliasTests)
    {
    auto dataset = ECSqlSelectTestDataset::AliasTests (PerClassRowCount);
    RunTest (dataset);
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, ECInstanceIdTests)
    {
    auto dataset = ECSqlSelectTestDataset::ECInstanceIdTests (GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, FromTests)
    {
    auto dataset = ECSqlSelectTestDataset::FromTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, FunctionTests)
    {
    auto dataset = ECSqlSelectTestDataset::FunctionTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, GroupByTests)
    {
    auto dataset = ECSqlSelectTestDataset::GroupByTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, InOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::InOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, JoinTests)
    {
    auto dataset = ECSqlSelectTestDataset::JoinTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, LikeOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::LikeOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, LimitTests)
    {
    auto dataset = ECSqlSelectTestDataset::LimitTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, MiscTests)
    {
    auto dataset = ECSqlSelectTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, NullLiteralTests)
    {
    auto dataset = ECSqlSelectTestDataset::NullLiteralTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFramework, OptionsTests)
    {
    auto dataset = ECSqlCommonTestDataset::OptionsTests(ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, OrderByTests)
    {
    auto dataset = ECSqlSelectTestDataset::OrderByTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, ParameterAdvancedTests)
    {
    auto dataset = ECSqlSelectTestDataset::ParameterAdvancedTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, PointTests)
    {
    auto dataset = ECSqlSelectTestDataset::PointTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, PolymorphicTests)
    {
    auto dataset = ECSqlSelectTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, PrimitiveTests)
    {
    auto dataset = ECSqlSelectTestDataset::PrimitiveTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFramework, SelectClauseTests)
    {
    auto dataset = ECSqlSelectTestDataset::SelectClauseTests(PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, SourceTargetConstraintTests)
    {
    auto dataset = ECSqlSelectTestDataset::SourceTargetConstraintTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, StructTests)
    {
    auto dataset = ECSqlSelectTestDataset::StructTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, SubqueryTests)
    {
    auto dataset = ECSqlSelectTestDataset::SubqueryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFramework, UnionTests)
    {
    auto dataset = ECSqlSelectTestDataset::UnionTests(PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFramework, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFramework, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFramework, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//********************* Insert **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlInsertTestFramework : public ECSqlNonSelectTestFrameworkFixture
    {
public:
    ECSqlInsertTestFramework () : ECSqlNonSelectTestFrameworkFixture () {}
    virtual ~ECSqlInsertTestFramework () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, ArrayTests)
    {
    auto dataset = ECSqlInsertTestDataset::ArrayTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, CommonGeometryTests)
    {
    auto dataset = ECSqlInsertTestDataset::CommonGeometryTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, DateTimeTests)
    {
    auto dataset = ECSqlInsertTestDataset::DateTimeTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, FunctionTests)
    {
    auto dataset = ECSqlInsertTestDataset::FunctionTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, IntoTests)
    {
    auto dataset = ECSqlInsertTestDataset::IntoTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, MiscTests)
    {
    auto dataset = ECSqlInsertTestDataset::MiscTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlInsertTestFramework, OptionsTests)
    {
    auto dataset = ECSqlCommonTestDataset::OptionsTests(ECSqlType::Insert, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, ParameterAdvancedTests)
    {
    auto dataset = ECSqlInsertTestDataset::ParameterAdvancedTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, RelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipEndTableMappingTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, RelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipLinkTableMappingTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, RelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithAnyClassConstraintTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, RelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithAdditionalPropsTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, RelationshipWithParametersTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithParametersTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFramework, StructTests)
    {
    auto dataset = ECSqlInsertTestDataset::StructTests ();
    RunTest (dataset);
    }


//********************* Update **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlUpdateTestFramework : public ECSqlNonSelectTestFrameworkFixture
    {
public:
    ECSqlUpdateTestFramework () : ECSqlNonSelectTestFrameworkFixture () {}
    virtual ~ECSqlUpdateTestFramework () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, ArrayTests)
    {
    auto dataset = ECSqlUpdateTestDataset::ArrayTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, CommonGeometryTests)
    {
    auto dataset = ECSqlUpdateTestDataset::CommonGeometryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, DateTimeTests)
    {
    auto dataset = ECSqlUpdateTestDataset::DateTimeTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, FunctionTests)
    {
    auto dataset = ECSqlUpdateTestDataset::FunctionTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, MiscTests)
    {
    auto dataset = ECSqlUpdateTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdateTestFramework, OptionsTests)
    {
    auto dataset = ECSqlCommonTestDataset::OptionsTests(ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, ParameterAdvancedTests)
    {
    auto dataset = ECSqlUpdateTestDataset::ParameterAdvancedTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, PolymorphicTests)
    {
    auto dataset = ECSqlUpdateTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, RelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipEndTableMappingTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, RelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipLinkTableMappingTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, RelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipWithAnyClassConstraintTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, RelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipWithAdditionalPropsTests (GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, StructTests)
    {
    auto dataset = ECSqlUpdateTestDataset::StructTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, TargetClassTests)
    {
    auto dataset = ECSqlUpdateTestDataset::TargetClassTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdateTestFramework, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdateTestFramework, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFramework, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//********************* Delete **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlDeleteTestFramework : public ECSqlNonSelectTestFrameworkFixture
    {
public:
    ECSqlDeleteTestFramework () : ECSqlNonSelectTestFrameworkFixture () {}
    virtual ~ECSqlDeleteTestFramework () {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, FromTests)
    {
    auto dataset = ECSqlDeleteTestDataset::FromTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, MiscTests)
    {
    auto dataset = ECSqlDeleteTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeleteTestFramework, OptionsTests)
    {
    auto dataset = ECSqlCommonTestDataset::OptionsTests(ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, PolymorphicTests)
    {
    auto dataset = ECSqlDeleteTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeleteTestFramework, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeleteTestFramework, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFramework, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

END_ECSQLTESTFRAMEWORK_NAMESPACE