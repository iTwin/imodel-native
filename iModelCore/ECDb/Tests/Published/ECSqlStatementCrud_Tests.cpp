/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementCrud_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlCrudTestFixture.h"
#include "ECSqlCommonTestDataset.h"
#include "ECSqlSelectStatementCrudTestDataset.h"
#include "ECSqlInsertStatementCrudTestDataset.h"
#include "ECSqlUpdateStatementCrudTestDataset.h"
#include "ECSqlDeleteStatementCrudTestDataset.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//********* Select ***************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, ArrayTests)
    {
    auto dataset = ECSqlSelectTestDataset::ArrayTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, BetweenOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::BetweenOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, CastTests)
    {
    auto dataset = ECSqlSelectTestDataset::CastTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, CommonGeometryTests)
    {
    auto dataset = ECSqlSelectTestDataset::CommonGeometryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, DateTimeTests)
    {
    auto dataset = ECSqlSelectTestDataset::DateTimeTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                      10/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, AliasTests)
    {
    auto dataset = ECSqlSelectTestDataset::AliasTests (PerClassRowCount);
    RunTest (dataset);
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, ECInstanceIdTests)
    {
    auto dataset = ECSqlSelectTestDataset::ECInstanceIdTests (GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, FromTests)
    {
    auto dataset = ECSqlSelectTestDataset::FromTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, FunctionTests)
    {
    auto dataset = ECSqlSelectTestDataset::FunctionTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, GroupByTests)
    {
    auto dataset = ECSqlSelectTestDataset::GroupByTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, InOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::InOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, JoinTests)
    {
    auto dataset = ECSqlSelectTestDataset::JoinTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, LikeOperatorTests)
    {
    auto dataset = ECSqlSelectTestDataset::LikeOperatorTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, LimitTests)
    {
    auto dataset = ECSqlSelectTestDataset::LimitTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, MiscTests)
    {
    auto dataset = ECSqlSelectTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, NullLiteralTests)
    {
    auto dataset = ECSqlSelectTestDataset::NullLiteralTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, OrderByTests)
    {
    auto dataset = ECSqlSelectTestDataset::OrderByTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, ParameterAdvancedTests)
    {
    auto dataset = ECSqlSelectTestDataset::ParameterAdvancedTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, PointTests)
    {
    auto dataset = ECSqlSelectTestDataset::PointTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, PolymorphicTests)
    {
    auto dataset = ECSqlSelectTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, PrimitiveTests)
    {
    auto dataset = ECSqlSelectTestDataset::PrimitiveTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFixture, SelectClauseTests)
    {
    auto dataset = ECSqlSelectTestDataset::SelectClauseTests(PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, SourceTargetConstraintTests)
    {
    auto dataset = ECSqlSelectTestDataset::SourceTargetConstraintTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, StructTests)
    {
    auto dataset = ECSqlSelectTestDataset::StructTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, SubqueryTests)
    {
    auto dataset = ECSqlSelectTestDataset::SubqueryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFixture, UnionTests)
    {
    auto dataset = ECSqlSelectTestDataset::UnionTests(PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFixture, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlSelectTestFixture, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlSelectTestFixture, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Select, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//********************* Insert **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlInsertTestFixture : public ECSqlNonSelectTestFixture
    {
public:
    ECSqlInsertTestFixture () : ECSqlNonSelectTestFixture () {}
    virtual ~ECSqlInsertTestFixture () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, ArrayTests)
    {
    auto dataset = ECSqlInsertTestDataset::ArrayTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, CommonGeometryTests)
    {
    auto dataset = ECSqlInsertTestDataset::CommonGeometryTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, DateTimeTests)
    {
    auto dataset = ECSqlInsertTestDataset::DateTimeTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, FunctionTests)
    {
    auto dataset = ECSqlInsertTestDataset::FunctionTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, IntoTests)
    {
    auto dataset = ECSqlInsertTestDataset::IntoTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, MiscTests)
    {
    auto dataset = ECSqlInsertTestDataset::MiscTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, ParameterAdvancedTests)
    {
    auto dataset = ECSqlInsertTestDataset::ParameterAdvancedTests ();
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, RelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipEndTableMappingTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, RelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipLinkTableMappingTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, RelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithAnyClassConstraintTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, RelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithAdditionalPropsTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, RelationshipWithParametersTests)
    {
    auto dataset = ECSqlInsertTestDataset::RelationshipWithParametersTests (GetECDb());
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  12/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlInsertTestFixture, StructTests)
    {
    auto dataset = ECSqlInsertTestDataset::StructTests ();
    RunTest (dataset);
    }


//********************* Update **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlUpdateTestFixture : public ECSqlNonSelectTestFixture
    {
public:
    ECSqlUpdateTestFixture () : ECSqlNonSelectTestFixture () {}
    virtual ~ECSqlUpdateTestFixture () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, ArrayTests)
    {
    auto dataset = ECSqlUpdateTestDataset::ArrayTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, CommonGeometryTests)
    {
    auto dataset = ECSqlUpdateTestDataset::CommonGeometryTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, DateTimeTests)
    {
    auto dataset = ECSqlUpdateTestDataset::DateTimeTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, FunctionTests)
    {
    auto dataset = ECSqlUpdateTestDataset::FunctionTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, MiscTests)
    {
    auto dataset = ECSqlUpdateTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, ParameterAdvancedTests)
    {
    auto dataset = ECSqlUpdateTestDataset::ParameterAdvancedTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, PolymorphicTests)
    {
    auto dataset = ECSqlUpdateTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, RelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipEndTableMappingTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, RelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipLinkTableMappingTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, RelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipWithAnyClassConstraintTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, RelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlUpdateTestDataset::RelationshipWithAdditionalPropsTests (GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, StructTests)
    {
    auto dataset = ECSqlUpdateTestDataset::StructTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, TargetClassTests)
    {
    auto dataset = ECSqlUpdateTestDataset::TargetClassTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdateTestFixture, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlUpdateTestFixture, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlUpdateTestFixture, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Update, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//********************* Delete **********************
//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/13
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlDeleteTestFixture : public ECSqlNonSelectTestFixture
    {
public:
    ECSqlDeleteTestFixture () : ECSqlNonSelectTestFixture () {}
    virtual ~ECSqlDeleteTestFixture () {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, FromTests)
    {
    auto dataset = ECSqlDeleteTestDataset::FromTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, MiscTests)
    {
    auto dataset = ECSqlDeleteTestDataset::MiscTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, PolymorphicTests)
    {
    auto dataset = ECSqlDeleteTestDataset::PolymorphicTests (PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereAbstractClassTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAbstractClassTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeleteTestFixture, WhereAndOrPrecedenceTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereAndOrPrecedenceTests(ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereBasicsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereBasicsTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereCommonGeometryTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereCommonGeometryTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereFunctionTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereFunctionTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlDeleteTestFixture, WhereMatchTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereMatchTests(ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest(dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereRelationshipEndTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipEndTableMappingTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereRelationshipLinkTableMappingTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipLinkTableMappingTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereRelationshipWithAdditionalPropsTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAdditionalPropsTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereRelationshipWithAnyClassConstraintTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereRelationshipWithAnyClassConstraintTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECSqlDeleteTestFixture, WhereStructTests)
    {
    auto dataset = ECSqlCommonTestDataset::WhereStructTests (ECSqlType::Delete, GetECDb(), PerClassRowCount);
    RunTest (dataset);
    }


END_ECDBUNITTESTS_NAMESPACE