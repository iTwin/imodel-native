/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
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
//!   - GetRelatedInstance("RelationshipName:0|1:RelatedClassName") - Returns related ECInstance
//!     ECExpression context.
//!   - HasRelatedInstance("RelationshipSchemaName:RelationshipName", "Forward|Backward", 
//!     "RelatedClassSchemaName:RelatedClassName") - Returns whether the ECInstance in the current 
//!     expression context has any related instances based on the supplied parameters.
//!   - GetRelatedValue("RelationshipSchemaName:RelationshipName", "Forward|Backward", 
//!     "RelatedClassSchemaName:RelatedClassName", "PropertyName") - Returns the specified 
//!     property value of the specified related instance. Returns NULL if there're no related instances.
//!
//! Warning: This class registers the symbols provider into a static list and thus is not thread
//! safe. If thread safety is needed, use `ECDbExpressionSymbolProvider`.
// @bsiclass                                      Grigas.Petraitis              02/2016
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
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProvider final : ECN::IECSymbolProvider
{
    struct ECDbExpressionEvaluationContext;

private:
    ECDbExpressionEvaluationContext* m_context;

    static ECN::ExpressionStatus GetRelatedInstanceQueryFormatOld(Utf8StringR, ECN::ECEntityClassCP&, ECDbCR, ECN::ECInstanceListCR, ECN::EvaluationResult const&);
    static ECN::ExpressionStatus GetRelatedInstanceQueryFormatNew(Utf8StringR, ECDbCR, ECN::ECInstanceListCR, ECN::EvaluationResultVector& args);
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
