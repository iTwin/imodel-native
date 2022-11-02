/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECObjects/ECExpressions.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Sets up ECDb ECExpression symbol provider that's available for the lifetime of this 
//! class object. The registered symbol provider provides such ECExpression symbols:
//! - ECDb.Path - Returns the path to ECDb.
//! - ECDb.Name - Returns the name of the ECDb.
//! - GetECClassId("ClassName", "SchemaName") - Returns the ID of the specified ECClass.
//! - ECInstance ECExpression context methods:
//!   - GetRelatedInstance("RelationshipName:0|1:RelatedClassName") - returns related ECInstance ECExpression context.
//!   - HasRelatedInstance("RelationshipSchemaName:RelationshipName", "Forward|Backward", "RelatedClassSchemaName:RelatedClassName") - returns whether any ECInstance in the current expression context has any related instance based on relationship, direction and related class name.
//!   - HasRelatedInstance("RelatedClassSchemaName:RelatedClassName", {lambda as an instance filter}) - returns whether any ECInstance in the current expression context has any related instance based on related class name and lambda, which is responsible for joining related class to current instance.
//!   - GetRelatedInstancesCount("RelationshipSchemaName:RelationshipName", "Forward|Backward", "RelatedClassSchemaName:RelatedClassName") - returns number of instances related to ECInstances in the current expression context based on relationship, direction and related class name.
//!   - GetRelatedInstancesCount("RelatedClassSchemaName:RelatedClassName", {lambda as an instance filter}) - returns number of instances related to ECInstances in the current expression context based on related class name and lambda, which is responsible for joining related class to current instance.
//!   - GetRelatedValue("RelationshipSchemaName:RelationshipName", "Forward|Backward", "RelatedClassSchemaName:RelatedClassName", "PropertyName") - returns the specified property value of the first instance related to any ECInstance in the current expression context. Returns NULL if there're no related instances. If there're more ECInstances in current context or they have more than one related instance, only the first value is returned (order is undefined).
//!   - GetRelatedValue("RelatedClassSchemaName:RelatedClassName", {lambda as an instance filter}, "PropertyName") - returns the specified property value of the first instance related to any ECInstance in the current expression context. Returns NULL if there're no related instances. If there're more ECInstances in current context or they have more than one related instance, only the first value is returned (order is undefined).
//!
//! Warning: This class registers the symbols provider into a static list and thus is not thread
//! safe. If thread safety is needed, use `ECDbExpressionSymbolProvider`.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolContext final
{
private:
    ECN::IECSymbolProvider* m_provider;
    ECSqlStatementCache const* m_statementCache;
    bool m_ownsStatementCache;
public:
    //! Constructor. Registers the symbol provider for the specified ECDb.
    ECDB_EXPORT explicit ECDbExpressionSymbolContext(ECDbCR ecdb, ECSqlStatementCache const* statementCache = nullptr);

    //! Destructor. Unregisters the registered symbol provider.
    ECDB_EXPORT ~ECDbExpressionSymbolContext();

    //! Unregisters the registered symbol provider.
    ECDB_EXPORT void LeaveContext();
};

//=======================================================================================
// Note: The provider must stay valid for the lifetime of the symbol expression contexts
// it published symbols to.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProvider final : ECN::IECSymbolProvider
{
    struct ECDbExpressionEvaluationContext;

private:
    ECDbExpressionEvaluationContext* m_context;

    static ECN::ExpressionStatus GetRelatedInstanceQueryFormatOld(Utf8StringR, ECN::ECEntityClassCP&, ECDbCR, ECN::ECInstanceListCR, ECN::EvaluationResult const&);
    static ECN::ExpressionStatus HasRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetRelatedInstancesCount(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetRelatedValue(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetClassId(ECN::EvaluationResult& evalResult, void* context, ECN::EvaluationResultVector& args);
    static BentleyStatus FindRelationshipAndClassInfo(ECDbCR, ECN::ECRelationshipClassCP&, Utf8CP relationshipName, ECN::ECEntityClassCP&, Utf8CP className);

    Utf8CP _GetName() const override {return "ECDbExpressionSymbolProvider";}
    void _PublishSymbols(ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;

public:
    ECDB_EXPORT ECDbExpressionSymbolProvider(ECDbCR db, ECSqlStatementCache const& statementCache);
    ECDB_EXPORT ~ECDbExpressionSymbolProvider();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
