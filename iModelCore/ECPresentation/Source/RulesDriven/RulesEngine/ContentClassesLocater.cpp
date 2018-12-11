/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentClassesLocater.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "ContentClassesLocater.h"
#include "PropertyInfoStore.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> CollectRuleConditionClasses(ECSchemaHelper const& schemaHelper, PresentationRuleSetCR ruleset, ECExpressionsCache& expressionsCache)
    {
    ECExpressionsHelper expressionsHelper(expressionsCache);
    bset<ECClassCP> classes;
    for (ContentRuleCP rule : ruleset.GetContentRules())
        {
        Utf8StringCR condition = rule->GetCondition();
        bvector<Utf8String> classNames =  expressionsHelper.GetUsedClasses(condition);
        for (Utf8StringCR name : classNames)
            {
            Utf8String schemaName, className;
            if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, name))
                continue;

            if (!schemaName.empty())
                {
                classes.insert(schemaHelper.GetECClass(schemaName.c_str(), className.c_str()));
                }
            else
                {
                bvector<ECClassCP> classesWithName = schemaHelper.GetECClassesByName(className.c_str());
                classes.insert(classesWithName.begin(), classesWithName.end());
                }
            }
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECClassCP> CollectDerivedClassesWithNavigationProperties(ECClassCR base, SchemaManagerCR schemas)
    {
    bvector<ECClassCP> classes;
    for (ECClassCP derived : schemas.GetDerivedClasses(base))
        {
        for (ECPropertyCP prop : derived->GetProperties(false))
            {
            if (prop->GetIsNavigation())
                {
                classes.push_back(derived);
                break;
                }
            }
        bvector<ECClassCP> derivedWithNavigationProperties = CollectDerivedClassesWithNavigationProperties(*derived, schemas);
        std::move(derivedWithNavigationProperties.begin(), derivedWithNavigationProperties.end(), std::back_inserter(classes));
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TList>
static void SplitPolymorphicClassesList(bvector<ECClassCP>& result, TList const& searchList, bset<ECClassCP> const& splitClasses, SchemaManagerCR schemas)
    {
    for (ECClassCP ecClass : searchList)
        {
        bool checkDerivedClasses = false;
        for (ECClassCP splitClass : splitClasses)
            {
            if (ecClass == splitClass)
                result.push_back(ecClass);
            if (splitClass != ecClass && splitClass->Is(ecClass))
                {
                checkDerivedClasses = true;
                break;
                }
            }
        if (checkDerivedClasses)
            SplitPolymorphicClassesList(result, schemas.GetDerivedClasses(*ecClass), splitClasses, schemas);
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ClassInput : IParsedInput
{
private:
    bvector<ECClassCP> m_classes;
protected:
    bvector<ECClassCP> const& _GetClasses() const override
        {
        return m_classes;
        }
    bvector<ECInstanceId> const& _GetInstanceIds(ECClassCR) const override
        {
        static bvector<ECInstanceId> s_empty;
        return s_empty;
        }
public:
    ClassInput(NavNodeKeyListCR keys, SchemaManagerCR schemas)
        {
        bset<ECClassId> classIds;
        for (NavNodeKeyCPtr const& key : keys)
            {
            if (nullptr == key->AsECInstanceNodeKey())
                {
                BeAssert(false);
                continue;
                }
            ECClassId classId = key->AsECInstanceNodeKey()->GetECClassId();
            if (classIds.end() != classIds.find(classId))
                continue;

            ECClassCP ecClass = schemas.GetClass(classId);
            m_classes.push_back(ecClass);
            classIds.insert(classId);
            }
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct DisplayedPropertyAppender : ContentSpecificationsHandler::PropertyAppender
{
private:
    PropertyInfoStore m_propertyInfos;
    ECClassCR m_propertyClass;
protected:
    bool _Supports(ECPropertyCR prop) override {return m_propertyInfos.ShouldDisplay(prop, m_propertyClass);}
    bool _Append(ECPropertyCR, Utf8CP) override {return true;}
public:
    DisplayedPropertyAppender(ContentSpecificationsHandler::Context const& context, ContentSpecificationCR spec, ECClassCR propertyClass)
        : m_propertyInfos(context.GetSchemaHelper(), context.GetRuleset(), &spec), m_propertyClass(propertyClass)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentClassesLocaterImpl : ContentSpecificationsHandler, PresentationRuleSpecificationVisitor
{
private:
    bvector<SelectClassInfo> m_classes;
    IParsedInput const* m_inputInfo;
    uint32_t m_handledSpecifications;
    ContentSpecificationCP m_currentSpecification;

private:
    bvector<NavNodeKeyCPtr> GetClassKeys(bvector<ECClassCP> const&) const;
    Context const& GetContext() const {return static_cast<Context const&>(ContentSpecificationsHandler::GetContext());}

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(SelectedNodeInstancesSpecificationCR specification) override
        {
        if (m_handledSpecifications > 0 && specification.GetOnlyIfNotHandled())
            return;

        m_currentSpecification = &specification;
        HandleSpecification(specification, *m_inputInfo);
        m_currentSpecification = nullptr;
        m_handledSpecifications++;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+---------------+--*/
    void _Visit(ContentInstancesOfSpecificClassesSpecificationCR specification) override
        {
        m_currentSpecification = &specification;
        HandleSpecification(specification);
        m_currentSpecification = nullptr;
        m_handledSpecifications++;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppenderPtr _CreatePropertyAppender(ECClassCR propertyClass, RelatedClassPath const&, RelationshipMeaning) override
        {
        return new DisplayedPropertyAppender(GetContext(), *m_currentSpecification, propertyClass);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AppendClass(SelectClassInfo const& classInfo) override
        {
        // located classes are always considered polymorphic
        SelectClassInfo copy(classInfo);
        copy.SetIsSelectPolymorphic(true);
        m_classes.push_back(copy);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _OnBeforeAppendClassInfos(bvector<SupportedEntityClassInfo>& infos) override
        {
        bvector<ECClassCP> navigationPropertyClasses;
        for (SupportedEntityClassInfo& info : infos)
            {
            info.SetFlags(info.GetFlags() | CLASS_FLAG_Polymorphic);

            bvector<ECClassCP> classes = CollectDerivedClassesWithNavigationProperties(info.GetClass(), GetContext().GetSchemaHelper().GetConnection().GetECDb().Schemas());
            std::move(classes.begin(), classes.end(), std::back_inserter(navigationPropertyClasses));
            }
        for (ECClassCP ecClass : navigationPropertyClasses)
            {
            if (ecClass->IsEntityClass())
                infos.push_back(SupportedEntityClassInfo(*ecClass->GetEntityClassCP(), CLASS_FLAG_Polymorphic | CLASS_FLAG_Include));
            }

        ContentSpecificationsHandler::_OnBeforeAppendClassInfos(infos);
        }

public:
    ContentClassesLocaterImpl(ContentClassesLocater::Context& context) : ContentSpecificationsHandler(context), m_currentSpecification(nullptr) {}
    void SetCurrentInput(IParsedInput const* input) {m_inputInfo = input;}
    bvector<SelectClassInfo> const& GetClasses() const {return m_classes;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodeKeyCPtr> ContentClassesLocater::GetClassKeys(bvector<ECClassCP> const& input) const
    {
    bvector<ECClassCP> lookup = input;
    bset<ECClassCP> ruleConditionClasses = CollectRuleConditionClasses(m_context.GetSchemaHelper(),
        m_context.GetRuleset(), m_context.GetECExpressionsCache());
    SplitPolymorphicClassesList(lookup, input, ruleConditionClasses, m_context.GetSchemaHelper().GetConnection().GetECDb().Schemas());

    bvector<NavNodeKeyCPtr> keys;
    for (ECClassCP ecClass : lookup)
        keys.push_back(ECInstanceNodeKey::Create(ECClassInstanceKey(ecClass, ECInstanceId()), bvector<Utf8String>()));
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo> ContentClassesLocater::Locate(bvector<ECClassCP> const& classes) const
    {
    RulesPreprocessor preprocessor(m_context.GetConnections(), m_context.GetConnection(), m_context.GetRuleset(), m_context.GetLocale(),
        m_context.GetUserSettings(), nullptr, m_context.GetECExpressionsCache());
    RulesPreprocessor::ContentRuleParameters params(*NavNodeKeyListContainer::Create(GetClassKeys(classes)),
        m_context.GetPreferredDisplayType(), nullptr, m_context.GetNodesLocater());
    ContentRuleInputKeysList ruleSpecs = preprocessor.GetContentSpecifications(params);

    ContentClassesLocaterImpl locater(m_context);
    for (ContentRuleInputKeys const& rule : ruleSpecs)
        {
        ClassInput input(rule.GetMatchingNodeKeys(), m_context.GetSchemaHelper().GetConnection().GetECDb().Schemas());
        locater.SetCurrentInput(&input);
        for (ContentSpecificationCP spec : rule.GetRule().GetSpecifications())
            spec->Accept(locater);
        locater.SetCurrentInput(nullptr);
        }

    return locater.GetClasses();
    }
