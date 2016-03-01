/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbExpressionSymbolProviders.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDb.h>
#include <ECObjects/ECExpressions.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct EXPORT_VTABLE_ATTRIBUTE ECDbExpressionSymbolProvider : ECN::IECSymbolProvider
{
private:
    ECDbCR m_db;

    static ECN::ExpressionStatus GetRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static BentleyStatus FindRelationshipAndClassInfo(ECDbCR, ECN::ECRelationshipClassCP&, Utf8CP relationshipName, ECN::ECEntityClassCP&, Utf8CP className);

    virtual Utf8CP _GetName() const override {return "ECDbExpressionSymbolProvider";}
    ECDB_EXPORT virtual void _PublishSymbols(ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;

public:
    explicit ECDbExpressionSymbolProvider(ECDbCR db) : ECN::IECSymbolProvider(), m_db(db) {}
};

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolContext
{
private:
    ECDbExpressionSymbolProvider* m_provider;
public:
    ECDB_EXPORT explicit ECDbExpressionSymbolContext(ECDbCR ecdb);
    ECDB_EXPORT ~ECDbExpressionSymbolContext();
    ECDB_EXPORT void LeaveContext();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
