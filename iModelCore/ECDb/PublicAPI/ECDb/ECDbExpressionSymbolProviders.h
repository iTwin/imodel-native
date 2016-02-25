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
struct ECDbExpressionContext : ECN::SymbolExpressionContext
{
private:
    ECDbCR m_db;
private:
    Utf8CP GetPath() const {return m_db.GetDbFileName();}
    ECN::ECValue GetName() const {return ECN::ECValue(BeFileName(m_db.GetDbFileName()).GetFileNameWithoutExtension().c_str());}
protected:
    ECDB_EXPORT ECDbExpressionContext(ECDbCR db);
    ECDbCR GetECDb() const {return m_db;}
public:
    static RefCountedPtr<ECDbExpressionContext> Create(ECDbCR db) {return new ECDbExpressionContext(db);}
};

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProvider : ECN::IECSymbolProvider
{
private:
    ECDbCR m_db;
protected:    
    virtual Utf8CP _GetName() const override {return "ECDbECExpressionSymbolProvider";}
    ECDB_EXPORT virtual void _PublishSymbols(ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;
    ECDbCR GetECDb() const {return m_db;}
public:
    ECDbExpressionSymbolProvider(ECDbCR db) : m_db(db) {}
};

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbInstancesExpressionSymbolProvider : ECDbExpressionSymbolProvider
{
private:
    static ECN::ExpressionStatus GetRelatedInstance(ECN::EvaluationResult& evalResult, void* context, ECN::ECInstanceListCR instanceData, ECN::EvaluationResultVector& args);
    static BentleyStatus FindRelationshipAndClassInfo(ECDbCR, ECN::ECRelationshipClassCP&, Utf8CP relationshipName, ECN::ECEntityClassCP&, Utf8CP className);

protected:    
    virtual Utf8CP _GetName() const override {return "ECDbInstancesExpressionSymbolProvider";}
    ECDB_EXPORT virtual void _PublishSymbols(ECN::SymbolExpressionContextR context, bvector<Utf8String> const& requestedSymbolSets) const override;
public:
    ECDbInstancesExpressionSymbolProvider(ECDbCR db) : ECDbExpressionSymbolProvider(db) {}
};

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbInstancesExpressionSymbolsContext
{
private:
    ECDbInstancesExpressionSymbolProvider* m_provider;
public:
    ECDB_EXPORT ECDbInstancesExpressionSymbolsContext(ECDbCR ecdb);
    ECDB_EXPORT ~ECDbInstancesExpressionSymbolsContext();
    ECDB_EXPORT void LeaveContext();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
