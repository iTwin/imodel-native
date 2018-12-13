/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RulesetEmbedder.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnDb.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

#define PRESENTATION_RULES_ECSCHEMA_PATH    L"ECSchemas/Domain/PresentationRules.ecschema.xml"
#define PRESENTATION_RULES_DOMAIN           "PresentationRules"
#define RULESET_MODEL                       "PresentationRules"
#define RULESET_SUBJECT                     "PresentationRules"
#define RULESET_DEFINITIONPARTITION         "PresentationRules"
#define RULESET_CODESPEC                    "PresentationRules:Ruleset"

//=======================================================================================
//! A Doman for Embedding and retrieving rulesets in iModelDb
// @bsiclass                                    Haroldas.Vitunskas             12/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PresentationRulesDomain : Dgn::DgnDomain
    {
    private: 
        PresentationRulesDomain() : DgnDomain(PRESENTATION_RULES_DOMAIN, "", 1) {}

    protected:
        WCharCP _GetSchemaRelativePath() const override { return PRESENTATION_RULES_ECSCHEMA_PATH; }

    public:
        static void RegisterSchema(Dgn::DgnDbR db);
    };

//=======================================================================================
//! An element for embedding a ruleset in iModelDb
// @bsiclass                                    Haroldas.Vitunskas             12/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RulesetElement : Dgn::DefinitionElement
    {
    public:
        static Dgn::DgnCode CreateRulesetCode(Dgn::DgnModelCR model, Utf8StringCR rulesetId, Dgn::DgnDbR dgnDb);
    };

//=======================================================================================
//! A class for embedding ruleset in iModelDb
// @bsiclass                                    Haroldas.Vitunskas             12/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RulesetEmbedder
    {
    public:
        enum DuplicateHandlingStrategy
            {
            SKIP,
            REPLACE
            };

    private:
        Dgn::DgnDbR m_db;

    private:
        Dgn::DefinitionModelCPtr GetOrCreateRulesetModel();
        
        Dgn::DefinitionModelCPtr QueryRulesetModel() const;
        Dgn::DefinitionPartitionCPtr QueryDefinitionPartition() const;
        Dgn::SubjectCPtr QuerySubject() const;

        Dgn::DefinitionModelCPtr InsertDefinitionModel(Dgn::DefinitionPartitionCPtr definitionPartition);
        Dgn::DefinitionPartitionCPtr InsertDefinitionPartition(Dgn::SubjectCPtr subject);
        Dgn::SubjectCPtr InsertSubject();
        
        void InsertCodeSpecs();
        Dgn::CodeSpecCPtr InsertCodeSpec(Utf8CP name);

        void HandleElementOperationPrerequisites();
        Dgn::DgnElementId HandleDuplicateRuleset(ECPresentation::PresentationRuleSetR ruleset, DuplicateHandlingStrategy duplicateHandlingStrategy, Dgn::DgnElementId rulesetId);
        Dgn::DgnElementId InsertNewRuleset(ECPresentation::PresentationRuleSetR ruleset, Dgn::DgnModelCPtr model, Dgn::DgnCode rulesetCode);

    public:
        //! Create a new embedder.
        //! @param[in] db   DgnDb reference to embed rulesets to
        DGNDBSYNC_EXPORT RulesetEmbedder(Dgn::DgnDbR dgnDb) : m_db(dgnDb) { PresentationRulesDomain::RegisterSchema(dgnDb); }
        
        //! Embeds a ruleset to Db
        //! @param[in] ruleset                      a presentation ruleset to embed
        //! @param[in] duplicateHandlingStrategy    strategy for handling duplicate ruleset:
        //!                                         SKIP    Skip inserting this ruleset and leave ruleset in db unchanged
        //!                                         REPLACE Replace ruleset in db with this ruleset
        //! @returns in case of successful insert returns DgnElementId of inserted element.
        //!          in case of failure inserts invalid ID (0)
        DGNDBSYNC_EXPORT Dgn::DgnElementId InsertRuleset(ECPresentation::PresentationRuleSetR ruleset, DuplicateHandlingStrategy duplicateHandlingStrategy = SKIP);
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
