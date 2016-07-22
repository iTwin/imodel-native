/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbExpressionSymbolProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include <ECObjects/ECExpressions.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProvider : ECN::IECSymbolProvider
{
private:
    ECDbCR m_db;

    static ECN::ExpressionStatus GetRelatedInstanceQueryFormatOld(Utf8StringR, ECN::ECEntityClassCP&, ECDbCR, ECN::ECInstanceListCR, ECN::EvaluationResult const&);
    static ECN::ExpressionStatus GetRelatedInstanceQueryFormatNew(Utf8StringR, ECDbCR, ECN::ECInstanceListCR, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus HasRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetRelatedValue(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static ECN::ExpressionStatus GetECClassId(ECN::EvaluationResult& evalResult, void* context, ECN::EvaluationResultVector& args);
    static BentleyStatus FindRelationshipAndClassInfo(ECDbCR, ECN::ECRelationshipClassCP&, Utf8CP relationshipName, ECN::ECEntityClassCP&, Utf8CP className);

    virtual Utf8CP _GetName() const override {return "ECDbExpressionSymbolProvider";}
    virtual void _PublishSymbols(ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;

public:
    explicit ECDbExpressionSymbolProvider(ECDbCR db) : ECN::IECSymbolProvider(), m_db(db) {}
};

END_BENTLEY_SQLITE_EC_NAMESPACE
