/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesDrivenProviderContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <Logging/bentleylogging.h>
#include "RulesEngineTypes.h"
#include "CustomFunctions.h"
#include "LocalizationHelper.h"
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct UsedUserSettingsListener;

struct UsedUserSettingsListener;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenProviderContext : RefCountedBase
{
private:
    // common
    PresentationRuleSetCR m_ruleset;
    bool m_holdsRuleset;
    IJsonLocalState const* m_localState;
    IUserSettings const& m_userSettings;
    mutable UsedUserSettingsListener* m_usedSettingsListener;
    RelatedPathsCache& m_relatedPathsCache;
    ECExpressionsCache& m_ecexpressionsCache;
    JsonNavNodesFactory const& m_nodesFactory;
    
    // localization context
    bool m_isLocalizationContext;
    ILocalizationProvider const* m_localizationProvider;
    
    // ECDb context
    bool m_isQueryContext;
    BeSQLite::EC::ECDb const* m_db;
    BeSQLite::EC::ECSqlStatementCache const* m_statementCache;
    ECSchemaHelper* m_schemaHelper;

private:
    void Init();

protected:
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(PresentationRuleSetCR, bool holdRuleset, IUserSettings const&, ECExpressionsCache&, RelatedPathsCache&, JsonNavNodesFactory const&, IJsonLocalState const*);
    ECPRESENTATION_EXPORT RulesDrivenProviderContext(RulesDrivenProviderContextCR other);
    
    ECPRESENTATION_EXPORT void SetQueryContext(BeSQLite::EC::ECDbCR, BeSQLite::EC::ECSqlStatementCache const&, CustomFunctionsInjector&);
    ECPRESENTATION_EXPORT void SetQueryContext(RulesDrivenProviderContextCR other);

public:
    ECPRESENTATION_EXPORT ~RulesDrivenProviderContext();

    // common
    PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
    JsonNavNodesFactory const& GetNodesFactory() const {return m_nodesFactory;}
    IUserSettings const& GetUserSettings() const {return m_userSettings;}
    IUsedUserSettingsListener& GetUsedSettingsListener() const;
    bvector<Utf8String> GetRelatedSettingIds() const;
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IJsonLocalState const* GetLocalState() const {return m_localState;}
    
    // localization context
    ECPRESENTATION_EXPORT void SetLocalizationContext(ILocalizationProvider const& provider);
    ECPRESENTATION_EXPORT void SetLocalizationContext(RulesDrivenProviderContextCR other);
    bool IsLocalizationContext() const {return m_isLocalizationContext;}
    ILocalizationProvider const& GetLocalizationProvider() const {BeAssert(IsLocalizationContext()); return *m_localizationProvider;}

    // ECDb context
    bool IsQueryContext() const {return m_isQueryContext;}
    BeSQLite::EC::ECDbR GetDb() const {BeAssert(IsQueryContext()); return const_cast<BeSQLite::EC::ECDbR>(*m_db);}
    BeSQLite::EC::ECSqlStatementCache const& GetStatementCache() const {BeAssert(IsQueryContext()); return *m_statementCache;}
    ECSchemaHelper const& GetSchemaHelper() const {BeAssert(IsQueryContext()); return *m_schemaHelper;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
