/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RulesetEmbedder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "RulesetEmbedder.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRulesDomain::RegisterSchema(Dgn::DgnDbR db)
    {
    if (nullptr == db.Domains().FindDomain(PRESENTATION_RULES_DOMAIN))
        {
        DgnDomainP domain = new PresentationRulesDomain();
        db.Domains().RegisterDomain(*domain);
        domain->ImportSchema(db);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnCode RulesetElement::CreateRulesetCode(Dgn::DgnModelCR model, Utf8StringCR rulesetId, Dgn::DgnDbR dgnDb)
    {
    return Dgn::DgnCode(dgnDb.CodeSpecs().GetCodeSpec(RULESET_CODESPEC)->GetCodeSpecId(), model.GetModeledElementId(), rulesetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DefinitionModelCPtr RulesetEmbedder::GetOrCreateRulesetModel()
    {
    Dgn::DefinitionModelCPtr ruleSetModel = QueryRulesetModel();
    if (ruleSetModel.IsValid())
        return ruleSetModel;

    Dgn::SubjectCPtr ruleSetSubject = InsertSubject();
    Dgn::DefinitionPartitionCPtr definitionPartition = InsertDefinitionPartition(ruleSetSubject);
    return InsertDefinitionModel(definitionPartition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DefinitionModelCPtr RulesetEmbedder::QueryRulesetModel() const
    {
    Dgn::DefinitionPartitionCPtr definitionPartition = QueryDefinitionPartition();
    if (definitionPartition.IsNull())
        return nullptr;

    return m_db.Models().Get<Dgn::DefinitionModel>(m_db.Models().QuerySubModelId(definitionPartition->GetCode()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DefinitionPartitionCPtr RulesetEmbedder::QueryDefinitionPartition() const
    {
    Dgn::SubjectCPtr subject = QuerySubject();
    if (subject.IsNull())
        return nullptr;

    Dgn::DgnElementId defitinioPartitionId = m_db.Elements().QueryElementIdByCode(Dgn::InformationPartitionElement::CreateCode(*subject, RULESET_MODEL));
    return m_db.Elements().Get<Dgn::DefinitionPartition>(defitinioPartitionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr RulesetEmbedder::QuerySubject() const
    {
    Dgn::SubjectCPtr rootSubject = m_db.Elements().GetRootSubject();
    Dgn::CodeSpecCPtr codeSpec = m_db.CodeSpecs().GetCodeSpec(BIS_CODESPEC_Subject);
    Dgn::DgnCode subjectCode = Dgn::DgnCode(codeSpec->GetCodeSpecId(), rootSubject->GetElementId(), RULESET_SUBJECT);

    Dgn::DgnElementId subjectId = m_db.Elements().QueryElementIdByCode(subjectCode);
    return m_db.Elements().Get<Dgn::Subject>(subjectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DefinitionModelCPtr RulesetEmbedder::InsertDefinitionModel(Dgn::DefinitionPartitionCPtr definitionPartition)
    {
    return Dgn::DefinitionModel::CreateAndInsert(*definitionPartition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DefinitionPartitionCPtr RulesetEmbedder::InsertDefinitionPartition(Dgn::SubjectCPtr subject)
    {
    return Dgn::DefinitionPartition::CreateAndInsert(*subject, RULESET_DEFINITIONPARTITION);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SubjectCPtr RulesetEmbedder::InsertSubject()
    {
    Dgn::SubjectCPtr rootSubject = m_db.Elements().GetRootSubject();
    return Dgn::Subject::CreateAndInsert(*rootSubject, RULESET_SUBJECT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetEmbedder::InsertCodeSpecs()
    {
    InsertCodeSpec(RULESET_CODESPEC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::CodeSpecCPtr RulesetEmbedder::InsertCodeSpec(Utf8CP name)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(m_db, name);
    if (Dgn::DgnDbStatus::Success == m_db.CodeSpecs().Insert(*codeSpec))
        return codeSpec;

    return nullptr;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetEmbedder::HandleElementOperationPrerequisites()
    {
    if (m_db.CodeSpecs().GetCodeSpec(RULESET_CODESPEC).IsValid())
        return;

    InsertCodeSpecs();
    m_db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId RulesetEmbedder::InsertRuleset(ECPresentation::PresentationRuleSetR ruleset, RulesetEmbedder::DuplicateHandlingStrategy duplicateHandlingStrategy)
    {
    HandleElementOperationPrerequisites();

    Dgn::DgnModelCPtr model = GetOrCreateRulesetModel();
    if (model.IsNull())
        {
        BeAssert(false && "Failed to create a model for a ruleset");
        return Dgn::DgnElementId();
        }

    Dgn::DgnCode rulesetCode = RulesetElement::CreateRulesetCode(*model, ruleset.GetRuleSetId(), m_db);
    Dgn::DgnElementId rulesetId = m_db.Elements().QueryElementIdByCode(rulesetCode);
    if (rulesetId.IsValid())
        return HandleDuplicateRuleset(ruleset, duplicateHandlingStrategy, rulesetId);

    return InsertNewRuleset(ruleset, model, rulesetCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId RulesetEmbedder::InsertNewRuleset(ECPresentation::PresentationRuleSetR ruleset, Dgn::DgnModelCPtr model, Dgn::DgnCode rulesetCode)
    {
    Dgn::DgnElement::CreateParams params(m_db, model->GetModelId(), m_db.Schemas().GetClassId(PRESENTATION_RULES_DOMAIN, PRESENTATION_RULESET_ELEMENT_CLASS_NAME), rulesetCode);
    Dgn::DefinitionElementPtr rulesetElement = new Dgn::DefinitionElement(params);
    rulesetElement->SetJsonProperties(rulesetElement->json_jsonProperties(), ruleset.WriteToJsonValue());
    Dgn::DgnElementCPtr inserted = rulesetElement->Insert();
    if (inserted.IsNull())
        {
        BeAssert(false);
        return Dgn::DgnElementId();
        }

    return inserted->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                12/18
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId RulesetEmbedder::HandleDuplicateRuleset(ECPresentation::PresentationRuleSetR ruleset, RulesetEmbedder::DuplicateHandlingStrategy duplicateHandlingStrategy, Dgn::DgnElementId rulesetId)
    {
    Dgn::DefinitionElementPtr rulesetElement;
    Dgn::DgnElementCPtr updated;

    if (DuplicateHandlingStrategy::SKIP == duplicateHandlingStrategy)
        return rulesetId;

    rulesetElement = m_db.Elements().GetForEdit<DefinitionElement>(rulesetId);
    if (rulesetElement.IsNull())
        {
        BeAssert(false);
        return Dgn::DgnElementId();
        }

    rulesetElement->SetJsonProperties(rulesetElement->json_jsonProperties(), ruleset.WriteToJsonValue());
    updated = rulesetElement->Update();
    if (updated.IsNull())
        {
        BeAssert(false);
        return Dgn::DgnElementId();
        }
    return rulesetId;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE