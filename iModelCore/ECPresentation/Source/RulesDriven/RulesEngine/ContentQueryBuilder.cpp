/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentQueryBuilder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryBuilder.h"
#include "ECExpressionContextsProvider.h"
#include "LoggingHelper.h"
#include "NavNodeProviders.h"

#define NO_RELATED_PROPERTIES_KEYWORD "_none_"

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                07/2017
+===============+===============+===============+===============+===============+======*/
struct PropertiesDisplayInfo
{
private:
    Utf8String m_propertyName;
    int m_priority;
    bool m_displayed;

public:
    PropertiesDisplayInfo() {}
    PropertiesDisplayInfo(Utf8String propertyName) : m_propertyName(propertyName) {}
    PropertiesDisplayInfo(Utf8String propertyName, int priority, bool displayed) : m_propertyName(propertyName), m_priority(priority), m_displayed(displayed) {}

    bool operator<(PropertiesDisplayInfo const& rhs) const
        {
        return strcmp(GetPropertyName(), rhs.GetPropertyName()) < 0;
        }

    Utf8CP GetPropertyName() const {return m_propertyName.c_str();}
    int GetPriority() const {return m_priority;}
    bool IsDisplayed() const {return m_displayed;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorCreateContext
{
private:
    IPropertyCategorySupplierR m_categorySupplier;
    IECPropertyFormatter const* m_formatter;
    ECSchemaHelper const& m_helper;
    PresentationRuleSetCR m_ruleset;
    bmap<ECClassCP, size_t> m_classCounter;
    bset<ECClassCP> m_handledClasses;
    bmap<ECSchemaCP, bmap<ECClassCP, RelatedPropertiesSpecificationList>> m_relatedPropertySpecifications;
    bmap<ECRelationshipClassCP, int> m_relationshipUseCounts;
    bmap<ECClassCP, bset<PropertiesDisplayInfo>> m_perClassPropertyDisplayInfos; // per-class property display info
    bmap<ECClassCP, bset<PropertiesDisplayInfo>> m_aggregatedPropertyDisplayInfos; // property display info, including base class properties
    bmap<ECClassCP, bmap<Utf8String, Utf8String>> m_propertyEditors;
    bmap<ECClassCP, bvector<RelatedClassPath>> m_handledNavigationPropertiesPaths;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void InitPropertiesDisplayInfo(ContentSpecificationCR specification, ContentModifierList const& contentModifiers)
        {
        for (PropertiesDisplaySpecificationCP displaySpec : specification.GetPropertiesDisplaySpecifications())
            CollectPropertiesDisplayRules(nullptr, *displaySpec);

        for (ContentModifierCP modifier : contentModifiers)
            {
            for (PropertiesDisplaySpecificationCP displaySpec : modifier->GetPropertiesDisplaySpecifications())
                {
                ECClassCP ecClass = m_helper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
                if (nullptr == ecClass)
                    {
                    BeAssert(false);
                    continue;
                    }
                CollectPropertiesDisplayRules(ecClass, *displaySpec);
                }
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void CollectPropertiesDisplayRules(ECClassCP ecClass, PropertiesDisplaySpecificationCR spec)
        {

        bset<PropertiesDisplayInfo> propertiesInfos;
        bvector<Utf8String> propertyNamesVec;
        BeStringUtilities::Split(spec.GetPropertyNames().c_str(), ",", propertyNamesVec);
        for (Utf8StringR propertyName : propertyNamesVec)
            {
            propertyName.Trim();
            propertiesInfos.insert(PropertiesDisplayInfo(propertyName, spec.GetPriority(), spec.IsDisplayed()));
            }

        auto iter = m_perClassPropertyDisplayInfos.find(ecClass);
        if (m_perClassPropertyDisplayInfos.end() == iter)
            m_perClassPropertyDisplayInfos[ecClass] = propertiesInfos;
        else
            InsertPropertiesDisplayInfo(ecClass, propertiesInfos, iter->second);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    static void InsertPropertiesDisplayInfo(ECClassCP ecClass, bset<PropertiesDisplayInfo> const& source,
        bset<PropertiesDisplayInfo>& target)
        {
        for (PropertiesDisplayInfo const& info : source)
            {
            auto iter = target.find(info);
            if (target.end() == iter)
                {
                target.insert(info);
                continue;
                }
            if (iter->GetPriority() < info.GetPriority())
                target.insert(target.erase(iter), info);
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bset<PropertiesDisplayInfo> const& GetPropertiesDisplayInfo(ECClassCR ecClass)
        {
        auto iter = m_aggregatedPropertyDisplayInfos.find(&ecClass);
        if (m_aggregatedPropertyDisplayInfos.end() == iter)
            {
            bset<PropertiesDisplayInfo> properties;

            auto anyClassIter = m_perClassPropertyDisplayInfos.find(nullptr);
            if (m_perClassPropertyDisplayInfos.end() != anyClassIter)
                InsertPropertiesDisplayInfo(&ecClass, anyClassIter->second, properties);

            auto perClassIter = m_perClassPropertyDisplayInfos.find(&ecClass);
            if (m_perClassPropertyDisplayInfos.end() != perClassIter)
                InsertPropertiesDisplayInfo(&ecClass, perClassIter->second, properties);
            
            bool hasDisplayedPropertySpec = false;
            for (PropertiesDisplayInfo const& info : properties)
                {
                if (info.IsDisplayed())
                    {
                    hasDisplayedPropertySpec = true;
                    break;
                    }
                }
            if (hasDisplayedPropertySpec)
                {
                for (ECPropertyCP prop : ecClass.GetProperties())
                    {
                    PropertiesDisplayInfo info(prop->GetName(), 0, false);
                    if (properties.end() == properties.find(info))
                        properties.insert(info);
                    }
                }

            for (ECClassCP base : ecClass.GetBaseClasses())
                InsertPropertiesDisplayInfo(base, GetPropertiesDisplayInfo(*base), properties);


            iter = m_aggregatedPropertyDisplayInfos.Insert(&ecClass, properties).first;
            }
        return iter->second;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void InitPropertyEditors(ContentSpecificationCR specification, ContentModifierList const& contentModifiers)
        {
        for (PropertyEditorsSpecificationCP displaySpec : specification.GetPropertyEditors())
            {
            bmap<Utf8String, Utf8String>& editors = m_propertyEditors[nullptr];
            editors[displaySpec->GetPropertyName()] = displaySpec->GetEditorName();
            }

        for (ContentModifierCP modifier : contentModifiers)
            {
            for (PropertyEditorsSpecificationCP editorSpec : modifier->GetPropertyEditors())
                {
                ECClassCP ecClass = m_helper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
                if (nullptr == ecClass)
                    {
                    BeAssert(false);
                    continue;
                    }
                bmap<Utf8String, Utf8String>& editors = m_propertyEditors[ecClass];
                editors[editorSpec->GetPropertyName()] = editorSpec->GetEditorName();
                }
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool IsRelatedRelationshipAllowed(ECRelationshipClassCR relationship, ECClassCR, RequiredRelationDirection direction) const
        {
        ECRelationshipConstraintCP constraint = nullptr;
        switch (direction)
            {
            case RequiredRelationDirection_Backward: constraint = &relationship.GetSource(); break;
            case RequiredRelationDirection_Forward: constraint = &relationship.GetTarget(); break;
            default: BeAssert(false); return false;
            }
        return !constraint->GetMultiplicity().IsUpperLimitUnbounded() && constraint->GetMultiplicity().GetUpperLimit() <= 1;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    RelatedPropertiesSpecificationList CreateRelatedPropertiesSpecifications(ECClassCP& parentClass, ECSchemaCR schema, IECInstanceCR attribute) const
        {
        ECValue classNameValue;
        if (ECObjectsStatus::Success != attribute.GetValue(classNameValue, "ParentClass") || !classNameValue.IsString())
            {
            BeAssert(false);
            return RelatedPropertiesSpecificationList();
            }
        Utf8String schemaName, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, classNameValue.GetUtf8CP()))
            {
            BeAssert(false);
            return RelatedPropertiesSpecificationList();
            }
        if (schemaName.empty())
            schemaName = schema.GetName();
        parentClass = m_helper.GetECClass(schemaName.c_str(), className.c_str());
        if (nullptr == parentClass)
            {
            BeAssert(false);
            return RelatedPropertiesSpecificationList();
            }

        ECValue relationshipPath;
        if (ECObjectsStatus::Success != attribute.GetValue(relationshipPath, "RelationshipPath") || !relationshipPath.IsString())
            {
            BeAssert(false);
            return RelatedPropertiesSpecificationList();
            }

        RelatedPropertiesSpecificationP rootSpec = nullptr;
        RelatedPropertiesSpecificationP currentSpec = nullptr;
        Utf8String targetRelatedClassName;

        bvector<Utf8String> path;
        BeStringUtilities::Split(relationshipPath.GetUtf8CP(), ".", path);
        for (Utf8StringCR step : path)
            {
            bvector<Utf8String> parts;
            BeStringUtilities::Split(step.c_str(), ":", parts);
            if (parts.size() < 3)
                {
                BeAssert(false);
                continue;
                }
            size_t directionLocation = (parts[1] == "0" || parts[1] == "1") ? 1 : 2;
            RequiredRelationDirection direction;
            switch (parts[directionLocation][0])
                {
                case '0': direction = RequiredRelationDirection_Forward; break;
                case '1': direction = RequiredRelationDirection_Backward; break;
                default: BeAssert(false); direction = RequiredRelationDirection_Forward;
                }

            Utf8String relationshipName;
            if (directionLocation == 1)
                relationshipName.append(schema.GetName()).append(":").append(parts[0]);
            else
                relationshipName.append(parts[0]).append(":").append(parts[1]);

            Utf8String relatedClassName;
            if (parts.size() == directionLocation + 2)
                relatedClassName.append(schema.GetName()).append(":").append(parts[directionLocation + 1]);
            else
                relatedClassName.append(parts[directionLocation + 1]).append(":").append(parts[directionLocation + 2]);

            ECClassCP relationship = m_helper.GetECClass(relationshipName.c_str());
            ECClassCP relatedClass = m_helper.GetECClass(relatedClassName.c_str());
            if (nullptr == relationship || !relationship->IsRelationshipClass() || nullptr == relatedClass || !relatedClass->IsEntityClass())
                {
                BeAssert(false);
                return RelatedPropertiesSpecificationList();
                }
            if (!IsRelatedRelationshipAllowed(*relationship->GetRelationshipClassCP(), *relatedClass, direction))
                return RelatedPropertiesSpecificationList();

            RelatedPropertiesSpecificationP spec = new RelatedPropertiesSpecification(direction, relationshipName, relatedClassName, "");
            if (nullptr == rootSpec)
                rootSpec = spec;
            if (nullptr != currentSpec)
                {
                currentSpec->SetPropertyNames(NO_RELATED_PROPERTIES_KEYWORD);
                currentSpec->GetNestedRelatedPropertiesR().push_back(spec);
                }
            currentSpec = spec;
            targetRelatedClassName = relatedClassName;
            }

        if (nullptr == rootSpec)
            return RelatedPropertiesSpecificationList();

        RelatedPropertiesSpecificationList list;
        ECValue derivedClassNames;
        if (ECObjectsStatus::Success != attribute.GetValue(derivedClassNames, "DerivedClasses") || !derivedClassNames.IsArray() || 0 == derivedClassNames.GetArrayInfo().GetCount())
            {
            list.push_back(rootSpec);
            }
        else
            {
            for (uint32_t i = 0; i < derivedClassNames.GetArrayInfo().GetCount(); ++i)
                {
                ECValue derivedClassName;
                if (ECObjectsStatus::Success != attribute.GetValue(derivedClassName, "DerivedClasses", i) || !derivedClassName.IsString())
                    {
                    BeAssert(false);
                    continue;
                    }
                Utf8String fullDerivedClassName(derivedClassName.GetUtf8CP());
                if (!fullDerivedClassName.Contains(":"))
                    fullDerivedClassName = Utf8String().append(schema.GetName()).append(":").append(derivedClassName.GetUtf8CP());
                ECClassCP derivedClass = m_helper.GetECClass(fullDerivedClassName.c_str());
                if (nullptr == derivedClass || !derivedClass->Is(m_helper.GetECClass(targetRelatedClassName.c_str())))
                    {
                    BeAssert(false);
                    continue;
                    }
                // we copy the whole spec, including the nested specs (when the path consists of more than 1 relationship)
                // and change the name of the most nested related class
                RelatedPropertiesSpecificationP derivedSpec = new RelatedPropertiesSpecification(*rootSpec);
                RelatedPropertiesSpecificationP mostNestedDerivedSpec = derivedSpec;
                while (!mostNestedDerivedSpec->GetNestedRelatedProperties().empty())
                    mostNestedDerivedSpec = mostNestedDerivedSpec->GetNestedRelatedProperties()[0];
                mostNestedDerivedSpec->SetRelatedClassNames(derivedClass->GetFullName());
                list.push_back(derivedSpec);
                }
            // rootSpec is not added to the list so it has to be deleted
            delete rootSpec;
            }
        return list;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    void InitRelatedPropertySpecifications(ECClassCR ecClass)
        {
        if (m_relatedPropertySpecifications.end() == m_relatedPropertySpecifications.find(&ecClass.GetSchema()))
            {
            IECInstancePtr specsAttribute = ecClass.GetSchema().GetCustomAttribute("Bentley_Standard_CustomAttributes", "RelatedItemsDisplaySpecifications");
            if (specsAttribute.IsValid())
                {
                ECValue specs;
                if (ECObjectsStatus::Success != specsAttribute->GetValue(specs, "Specifications") || !specs.IsArray() || !specs.GetArrayInfo().IsStructArray())
                    {
                    BeAssert(false);
                    }
                else
                    {
                    for (uint32_t i = 0; i < specs.GetArrayInfo().GetCount(); ++i)
                        {
                        ECValue specValue;
                        if (ECObjectsStatus::Success != specsAttribute->GetValue(specValue, "Specifications", i) || !specValue.IsStruct())
                            {
                            BeAssert(false);
                            continue;
                            }
                        IECInstancePtr specAttribute = specValue.GetStruct();
                        if (!specAttribute.IsValid() || !specAttribute->GetClass().GetName().Equals("RelatedItemsDisplaySpecification"))
                            {
                            BeAssert(false);
                            continue;
                            }
                        ECClassCP parentClass = nullptr;
                        RelatedPropertiesSpecificationList specs = CreateRelatedPropertiesSpecifications(parentClass, ecClass.GetSchema(), *specAttribute);
                        if (!specs.empty())
                            {
                            RelatedPropertiesSpecificationList& classSpecs = m_relatedPropertySpecifications[&ecClass.GetSchema()][parentClass];
                            classSpecs.insert(classSpecs.end(), specs.begin(), specs.end());
                            }
                        }
                    }
                }
            }

        for (ECClassCP baseClass : ecClass.GetBaseClasses())
            InitRelatedPropertySpecifications(*baseClass);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    void AddClassRelatedPropertySpecifications(RelatedPropertiesSpecificationList& list, bset<ECClassCP>& includedClasses, ECClassCR ecClass) const
        {
        auto schemaIter = m_relatedPropertySpecifications.find(&ecClass.GetSchema());
        if (m_relatedPropertySpecifications.end() != schemaIter)
            {
            auto classIter = schemaIter->second.find(&ecClass);
            if (schemaIter->second.end() != classIter && includedClasses.end() == includedClasses.find(&ecClass))
                {
                list.insert(list.end(), classIter->second.begin(), classIter->second.end());
                includedClasses.insert(&ecClass);
                }
            }
        for (ECClassCP baseClass : ecClass.GetBaseClasses())
            AddClassRelatedPropertySpecifications(list, includedClasses, *baseClass);
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    ContentDescriptorCreateContext(ECSchemaHelper const& helper, IPropertyCategorySupplierR categorySupplier, IECPropertyFormatter const* formatter, 
        PresentationRuleSetCR ruleset, ContentSpecificationCP specification)
        : m_helper(helper), m_categorySupplier(categorySupplier), m_formatter(formatter), m_ruleset(ruleset)
        {
        if (nullptr != specification)
            {
            InitPropertiesDisplayInfo(*specification, ruleset.GetContentModifierRules());
            InitPropertyEditors(*specification, ruleset.GetContentModifierRules());
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~ContentDescriptorCreateContext()
        {
        for (auto schemaIter : m_relatedPropertySpecifications)
            {
            for (auto classIter : schemaIter.second)
                {
                for (auto spec : classIter.second)
                    delete spec;
                }
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    ECSchemaHelper const& GetSchemaHelper() const {return m_helper;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    IPropertyCategorySupplierR GetCategorySupplier() const {return m_categorySupplier;}
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            03/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    PresentationRuleSetCR GetRuleset() const {return m_ruleset;}

    /*-----------------------------------------------------------------------------**//**
     * @bsimethod                                    Aidas.Vaiksnoras            03/2017
     +---------------+---------------+---------------+---------------+-----------+------*/
    IECPropertyFormatter const* GetPropertyFormatter() const {return m_formatter;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    size_t GetClassCount(ECClassCR ecClass) {return m_classCounter[&ecClass]++;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            01/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bmap<ECRelationshipClassCP, int>& GetRelationshipUseCounts() {return m_relationshipUseCounts;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool IsClassHandled(ECClassCR ecClass) const {return m_handledClasses.end() != m_handledClasses.find(&ecClass);}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    void SetClassHandled(ECClassCR ecClass) {m_handledClasses.insert(&ecClass);}

    void AddNavigationPropertiesPaths(ECClassCR ecClass, bvector<RelatedClassPath> const& navigationPropertiesPaths) {m_handledNavigationPropertiesPaths[&ecClass] = navigationPropertiesPaths;}

    bvector<RelatedClassPath> GetNavigationPropertiesPaths(ECClassCR ecClass) {return m_handledNavigationPropertiesPaths[&ecClass];}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass)
        {
        // schema custom attribute overrides everything
        IECInstancePtr hideCustomAttribute = prop.GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
        if (hideCustomAttribute.IsValid())
            return false;

        hideCustomAttribute = prop.GetCustomAttribute("EditorCustomAttributes", "HideProperty");
        if (hideCustomAttribute.IsValid())
            return false;

        bset<PropertiesDisplayInfo> const& properties = GetPropertiesDisplayInfo(ecClass);
        if (properties.empty())
            return true;

        auto iter = properties.find(PropertiesDisplayInfo(prop.GetName()));
        if (properties.end() != iter)
            return iter->IsDisplayed();
        
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    RelatedPropertiesSpecificationList GetRelatedPropertySpecifications(ECClassCR ecClass)
        {
        bset<ECClassCP> includedClasses;
        RelatedPropertiesSpecificationList list;
        InitRelatedPropertySpecifications(ecClass);
        AddClassRelatedPropertySpecifications(list, includedClasses, ecClass);
        return list;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas            07/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    Utf8CP GetPropertyEditor(ECPropertyCR ecProperty, ECClassCR ecClass, bool searchForAnyClassEditor = true)
        {
        Utf8StringCR propertyName = ecProperty.GetName();
        auto specificClassIter = m_propertyEditors.find(&ecClass);
        if (m_propertyEditors.end() != specificClassIter)
            {
            auto propertyIter = specificClassIter->second.find(propertyName);
            if (specificClassIter->second.end() != propertyIter)
                return propertyIter->second.c_str();
            }

        if (searchForAnyClassEditor)
            {
            auto anyClassIter = m_propertyEditors.find(nullptr);
            if (m_propertyEditors.end() != anyClassIter)
                {
                auto propertyIter = anyClassIter->second.find(propertyName);
                if (anyClassIter->second.end() != propertyIter)
                    return propertyIter->second.c_str();
                }
            }

        for (ECClassCP base : ecClass.GetBaseClasses())
            {
            if (nullptr != base->GetPropertyP(propertyName))
                return GetPropertyEditor(ecProperty, *base, false);
            }

        return nullptr;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNavigationClassAlias(ECClassCR ecClass, ContentDescriptorCreateContext& context)
    {
    return Utf8PrintfString("nav_%s_%s_%" PRIu64, ecClass.GetSchema().GetAlias().c_str(), ecClass.GetName().c_str(), (uint64_t)context.GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool PathStartsWith(RelatedClassPathCR path, RelatedClassPathCR sub)
    {
    if (sub.size() > path.size())
        return false;

    for (size_t i = 0; i < sub.size(); ++i)
        {
        if (sub[i] != path[i])
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static RelatedClassPath GetPathDifference(RelatedClassPathCR lhs, RelatedClassPathCR rhs)
    {
    BeAssert(PathStartsWith(lhs, rhs));
    RelatedClassPath diff;
    for (size_t i = rhs.size(); i < lhs.size(); ++i)
        diff.push_back(lhs[i]);
    return diff;
    }

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct PropertyAppender
{
private:
    ContentDescriptorR m_descriptor;
    ContentDescriptorCreateContext& m_descriptorCreateContext;
    ECClassCR m_actualClass;
    Utf8String m_actualClassAlias;
    RelatedClassPath const& m_relatedClassPath;
    bvector<RelatedClassPath>& m_navigationPropertiesPaths;
    ContentDescriptor::ECInstanceKeyField* m_keyField;
    ContentDescriptor::NestedContentField* m_nestedContentField;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateFieldDisplayLabel(ECPropertyCR ecProperty)
        {
        Utf8String displayLabel;
        if (nullptr == m_descriptorCreateContext.GetPropertyFormatter()
            || SUCCESS != m_descriptorCreateContext.GetPropertyFormatter()->GetFormattedPropertyLabel(displayLabel, ecProperty, m_actualClass, m_relatedClassPath))
            {
            if (!m_relatedClassPath.empty())
                displayLabel.append(m_actualClass.GetDisplayLabel()).append(" ").append(ecProperty.GetDisplayLabel());
            else
                displayLabel = ecProperty.GetDisplayLabel();
            }
        return displayLabel;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateNestedContentFieldName(RelatedClassPath const& relationshipPath) const
        {
        Utf8String name;
        for (RelatedClassCR rel : relationshipPath)
            {
            name.append(rel.GetTargetClass()->GetName());
            name.append("_");
            }
        name.append(m_actualClass.GetName());
        return name;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECInstanceKeyField* FindKeyField(ContentDescriptor::ECPropertiesField const& propertiesField) const
        {
        for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
            {
            if (!descriptorField->IsSystemField() || !descriptorField->AsSystemField()->IsECInstanceKeyField())
                continue;

            ContentDescriptor::ECInstanceKeyField* keyField = descriptorField->AsSystemField()->AsECInstanceKeyField();
            for (ContentDescriptor::ECPropertiesField const* keyedField : keyField->GetKeyFields())
                {
                if (keyedField == &propertiesField)
                    return keyField;
                }
            }
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECPropertiesField* GetPropertiesField(ECPropertyCR ecProperty)
        {
        ContentDescriptor::ECPropertiesField* field = nullptr;
        for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
            {
            if (!descriptorField->IsPropertiesField())
                continue;

            for (ContentDescriptor::Property const& prop : descriptorField->AsPropertiesField()->GetProperties())
                {
                // skip if already included in descriptor
                if (&ecProperty == &prop.GetProperty() 
                    && &m_actualClass == &prop.GetPropertyClass()
                    && m_actualClassAlias.Equals(prop.GetPrefix()))
                    {
                    return nullptr;
                    }

                // if properties in this field are similar, the new property should be included in this field
                bool isNewPropertyRelated = !m_relatedClassPath.empty();
                bool areSimilar = ArePropertiesSimilar(ecProperty, prop.GetProperty());
                areSimilar &= (prop.IsRelated() == isNewPropertyRelated) && (&m_actualClass == &prop.GetPropertyClass() || !isNewPropertyRelated);

                Utf8CP newPropertyEditor = m_descriptorCreateContext.GetPropertyEditor(ecProperty, m_actualClass);
                Utf8CP fieldPropertyEditor = m_descriptorCreateContext.GetPropertyEditor(prop.GetProperty(), prop.GetPropertyClass());
                areSimilar &= (nullptr == newPropertyEditor && nullptr == fieldPropertyEditor) 
                    || (nullptr != newPropertyEditor && nullptr != fieldPropertyEditor && 0 == strcmp(newPropertyEditor, fieldPropertyEditor));

                if (areSimilar)
                    {
                    field = descriptorField->AsPropertiesField();
                    m_keyField = FindKeyField(*field);
                    break;
                    }
                }

            if (nullptr != field)
                break;
            }
        
        // did not find a field with similar properties - create a new one
        if (nullptr == field)
            {
            if (nullptr == m_keyField && !m_relatedClassPath.empty())
                {
                m_keyField = new ContentDescriptor::ECInstanceKeyField();
                m_descriptor.GetAllFields().push_back(m_keyField);
                }

            ECClassCR primaryClass = m_relatedClassPath.empty() ? m_actualClass : *m_relatedClassPath.front().GetSourceClass();
            ContentDescriptor::Category fieldCategory = m_descriptorCreateContext.GetCategorySupplier().GetCategory(primaryClass, m_relatedClassPath, ecProperty);
            Utf8CP editor = m_descriptorCreateContext.GetPropertyEditor(ecProperty, m_actualClass);
            field = new ContentDescriptor::ECPropertiesField(fieldCategory, "", CreateFieldDisplayLabel(ecProperty), editor);
            m_descriptor.GetAllFields().push_back(field);

            if (!m_relatedClassPath.empty())
                m_keyField->AddKeyField(*field);

            if (ecProperty.GetIsNavigation())
                m_descriptor.GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(field));
            }

        return field;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::NestedContentField* GetNestedContentField()
        {
        if (nullptr == m_nestedContentField)
            {
            // find a content field that the new nested field should be nested in
            ContentDescriptor::NestedContentField* nestingField = nullptr;
            RelatedClassPath relationshipPath = m_relatedClassPath;
            for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
                {
                if (!descriptorField->IsNestedContentField())
                    continue;

                if (PathStartsWith(m_relatedClassPath, descriptorField->AsNestedContentField()->GetRelationshipPath()))
                    {
                    // this field should be nested in descriptorField
                    nestingField = descriptorField->AsNestedContentField();
                    relationshipPath = GetPathDifference(m_relatedClassPath, descriptorField->AsNestedContentField()->GetRelationshipPath());
                    break;
                    }
                }
        
            // create the field
            ECClassCR primaryClass = *m_relatedClassPath.front().GetTargetClass();
            ContentDescriptor::Category fieldCategory = m_descriptorCreateContext.GetCategorySupplier().GetCategory(primaryClass, 
                relationshipPath, m_actualClass);
            m_nestedContentField = new ContentDescriptor::NestedContentField(fieldCategory, CreateNestedContentFieldName(relationshipPath), 
                m_actualClass.GetDisplayLabel(), m_actualClass, relationshipPath);
            if (nullptr != nestingField)
                {
                // if the field is nested, add it to the nesting field
                nestingField->GetFields().push_back(m_nestedContentField);
                }
            else
                {
                // if the field is not nested, just append it to the descriptor
                m_descriptor.GetAllFields().push_back(m_nestedContentField);
                }
            }
        return m_nestedContentField;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool ArePropertiesSimilar(ECPropertyCR lhs, ECPropertyCR rhs)
        {
        if (!lhs.GetName().Equals(rhs.GetName()))
            return false;

        if (!lhs.GetTypeName().Equals(rhs.GetTypeName()))
            return false;

        return true;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsXToManyRelated() const
        {
        for (RelatedClassCR rel : m_relatedClassPath)
            {
            if (rel.IsForwardRelationship() && rel.GetRelationship()->GetSource().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            if (!rel.IsForwardRelationship() && rel.GetRelationship()->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            }
        return false;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendSimpleProperty(ECPropertyCR p)
        {
        // get the field to append the property to (a new field will be created if necessary)
        ContentDescriptor::ECPropertiesField* field = GetPropertiesField(p);
        if (nullptr == field)
            return false;

        // append the property definition
        field->GetProperties().push_back(ContentDescriptor::Property(m_actualClassAlias, m_actualClass, p));

        if (p.GetIsNavigation())
            {
            Utf8String propertyPrefix;
            AddNavigationPropertyPath(p, propertyPrefix);
            field->GetProperties().back().SetPrefix(propertyPrefix);
            }

        // set the "related" flag, if necessary
        if (!m_relatedClassPath.empty())
            field->GetProperties().back().SetIsRelated(m_relatedClassPath);
        
        // update field name (which depends on the properties in the field)
        field->SetName(QueryBuilderHelpers::CreateFieldName(*field));
        if (nullptr != m_keyField)
            m_keyField->RecalculateName();

        return true;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendXToManyRelatedProperty(ECPropertyCR p)
        {
        ContentDescriptor::NestedContentField* field = GetNestedContentField();
        if (nullptr == field)
            return false;

        ContentDescriptor::ECPropertiesField* propertyField = new ContentDescriptor::ECPropertiesField(m_actualClass, 
            ContentDescriptor::Property(m_actualClassAlias, m_actualClass, p));
        field->GetFields().push_back(propertyField);
        return false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddNavigationPropertyPath(ECPropertyCR ecProperty, Utf8StringR targetAlias)
        {
        RelatedClass relatedClass = m_descriptorCreateContext.GetSchemaHelper().GetForeignKeyClass(ecProperty);
        relatedClass.SetIsForwardRelationship(!relatedClass.IsForwardRelationship());
        targetAlias = GetNavigationClassAlias(*relatedClass.GetTargetClass(), m_descriptorCreateContext);
        relatedClass.SetTargetClassAlias(targetAlias);
        RelatedClassPath path = {relatedClass};
        m_navigationPropertiesPaths.push_back(path);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppender(ContentDescriptorR descriptor, ContentDescriptorCreateContext& descriptorCreateContext, 
        ECClassCR actualClass, Utf8StringCR classAlias, RelatedClassPath const& relatedClassPath, bvector<RelatedClassPath>& navigationPropertiesPaths)
        : m_descriptor(descriptor), m_descriptorCreateContext(descriptorCreateContext), m_actualClassAlias(classAlias), 
        m_relatedClassPath(relatedClassPath), m_actualClass(actualClass), m_navigationPropertiesPaths(navigationPropertiesPaths),
        m_keyField(nullptr), m_nestedContentField(nullptr)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool Append(ECPropertyCR p)
        {
        // don't append hidden properties
        if (!m_descriptorCreateContext.ShouldDisplay(p, m_actualClass))
            return false;

        // don't append binary and igeometry properties
        if (p.GetIsPrimitive() && (PRIMITIVETYPE_Binary == p.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_IGeometry == p.GetAsPrimitiveProperty()->GetType()))
            return false;

        // WIP TFS#711829 and TFS#711828
        if (p.GetIsArray() || p.GetIsStruct())
            return false;
        
        if (IsXToManyRelated())
            return AppendXToManyRelatedProperty(p);
        
        return AppendSimpleProperty(p);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetRelatedClassAlias(ECClassCR ecClass, ContentDescriptorCreateContext& context)
    {
    return Utf8PrintfString("rel_%s_%s_%" PRIu64, ecClass.GetSchema().GetAlias().c_str(), ecClass.GetName().c_str(), (uint64_t)context.GetClassCount(ecClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename SpecificationType>
static int GetRelationshipDirection(SpecificationType const& specification)
    {
    int relationshipDirection = 0;
    switch (specification.GetRequiredRelationDirection())
        {
        case RequiredRelationDirection_Forward:  relationshipDirection = (int)ECRelatedInstanceDirection::Forward; break;
        case RequiredRelationDirection_Backward: relationshipDirection = (int)ECRelatedInstanceDirection::Backward; break;
        case RequiredRelationDirection_Both:     relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward; break;
        }
    return relationshipDirection;
    }

static bvector<RelatedClassPath> AppendRelatedProperties(ContentDescriptorR, ContentDescriptorCreateContext&, RelatedClassPath const&, ECClassCR, Utf8StringCR, RelatedPropertiesSpecificationList const&, bool);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> AppendRelatedProperty(ContentDescriptorR descriptor, ContentDescriptorCreateContext& context,
    RelatedClassPath const& relatedClassPath, ECClassCR relatedClass, Utf8StringCR relatedClassAlias, 
    RelatedPropertiesSpecificationCR relatedPropertySpec)
    {
    bvector<RelatedClassPath> allPaths;

    bvector<Utf8String> propertyNames;
    BeStringUtilities::Split(relatedPropertySpec.GetPropertyNames().c_str(), ",", propertyNames);
    std::for_each(propertyNames.begin(), propertyNames.end(), [](Utf8StringR name){name.Trim();});
        
    ECSchemaHelper::RelationshipClassPathOptions options(relatedClass, GetRelationshipDirection(relatedPropertySpec), 0, context.GetRuleset().GetSupportedSchemas(), 
        relatedPropertySpec.GetRelationshipClassNames(), relatedPropertySpec.GetRelatedClassNames(), context.GetRelationshipUseCounts());
    bvector<bpair<RelatedClassPath, bool>> paths = context.GetSchemaHelper().GetRelationshipClassPaths(options);

    for (auto pair : paths)
        {
        BeAssert(pair.second); // assert this is an include path
        RelatedClassPath& path = pair.first;
        if (1 != path.size())
            {
            BeAssert(false);
            continue;
            }

        RelatedClass& relationshipInfo = path.back();

        RelatedClassPath propertyRelatedClassPath = relatedClassPath;
        Utf8String targetClassAlias = relatedClassPath.empty() ? GetRelatedClassAlias(*relationshipInfo.GetSourceClass(), context) : relatedClassAlias;
        propertyRelatedClassPath.push_back(RelatedClass(*relationshipInfo.GetTargetClass(), *relationshipInfo.GetSourceClass(), 
            *relationshipInfo.GetRelationship(), relationshipInfo.IsForwardRelationship(), 
            targetClassAlias.c_str(), relationshipInfo.GetRelationshipAlias(), relationshipInfo.IsPolymorphic()));

        // note: GetRelationshipClassPaths returns paths in opposite direction than we expect, so we have to reverse them
        relationshipInfo.SetIsForwardRelationship(!relationshipInfo.IsForwardRelationship());

        ECClassCP pathClass = relationshipInfo.GetTargetClass();
        Utf8String pathClassAlias = GetRelatedClassAlias(*pathClass, context);
        relationshipInfo.SetTargetClassAlias(pathClassAlias);

        bvector<RelatedClassPath> navigationPropetiesPaths;
        bool appendPath = false;
        if (propertyNames.empty())
            {
            PropertyAppender appender(descriptor, context, *pathClass, pathClassAlias, propertyRelatedClassPath, navigationPropetiesPaths);
            ECPropertyIterable properties = pathClass->GetProperties(true);
            for (ECPropertyCP ecProperty : properties)
                appendPath |= appender.Append(*ecProperty);
            }
        else if (1 == propertyNames.size() && propertyNames[0].EqualsI(NO_RELATED_PROPERTIES_KEYWORD))
            {
            // wip: log something
            }
        else
            {
            PropertyAppender appender(descriptor, context, *pathClass, pathClassAlias, propertyRelatedClassPath, navigationPropetiesPaths);
            for (Utf8StringCR propertyName : propertyNames)
                {
                ECPropertyCP ecProperty = pathClass->GetPropertyP(propertyName.c_str());
                if (nullptr != ecProperty)
                    appendPath |= appender.Append(*ecProperty);
                }
            }

        bvector<RelatedClassPath> relatedPaths = AppendRelatedProperties(descriptor, context, propertyRelatedClassPath, *pathClass, 
            pathClassAlias, relatedPropertySpec.GetNestedRelatedProperties(), true);
        if (!relatedPaths.empty())
            {
            for (RelatedClassPath& relatedPath : relatedPaths)
                {
                relatedPath.insert(relatedPath.begin(), relationshipInfo);
                allPaths.push_back(relatedPath);
                }
            }
        if (appendPath)
            {
            allPaths.push_back(path);
            for (RelatedClassPath& navigationPropertyPath : navigationPropetiesPaths)
                {
                navigationPropertyPath.insert(navigationPropertyPath.begin(), relationshipInfo);
                allPaths.push_back(navigationPropertyPath);
                }
            }
        }

    return allPaths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendRelatedPropertiesFromContentRule(bvector<RelatedClassPath>& paths, ContentDescriptorR descriptor, ContentDescriptorCreateContext& context,
    RelatedClassPath const& relatedClassPath, ECClassCR relatedClass, Utf8StringCR relatedClassAlias, RelatedPropertiesSpecificationList const& relatedProperties)
    {
    for (RelatedPropertiesSpecificationCP spec : relatedProperties)
        {
        bvector<RelatedClassPath> specPaths = AppendRelatedProperty(descriptor, context, relatedClassPath, relatedClass, relatedClassAlias, *spec);
        paths.insert(paths.end(), specPaths.begin(), specPaths.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> AppendRelatedProperties(ContentDescriptorR descriptor, ContentDescriptorCreateContext& context, 
    RelatedClassPath const& relatedClassPath, ECClassCR relatedClass, Utf8StringCR relatedClassAlias, 
    RelatedPropertiesSpecificationList const& relatedProperties, bool isNested)
    {
    bvector<RelatedClassPath> paths;

    // appends from content rule
    AppendRelatedPropertiesFromContentRule(paths, descriptor, context, relatedClassPath, relatedClass, relatedClassAlias, relatedProperties);

    if (!isNested)
        {
        // appends from content modifiers
        for (ContentModifierCP modifier : context.GetRuleset().GetContentModifierRules())
            {
            if (relatedClass.Is(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str()))
                AppendRelatedPropertiesFromContentRule(paths, descriptor, context, relatedClassPath, relatedClass, relatedClassAlias, modifier->GetRelatedProperties());
            }

        // appends related properties based on RelatedItemsDisplaySpecifications custom attribute
        RelatedPropertiesSpecificationList additionalRelatedItemSpecs = context.GetRelatedPropertySpecifications(relatedClass);
        AppendRelatedPropertiesFromContentRule(paths, descriptor, context, relatedClassPath, relatedClass, relatedClassAlias, additionalRelatedItemSpecs);
        }

    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ParsedSelectionInfo::ParsedSelectionInfo(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
    {
    Parse(nodeKeys, nodesLocater, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::GetNodeClasses(NavNodeKeyCR nodeKey, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
    {
    if (nullptr != nodeKey.AsDisplayLabelGroupingNodeKey() && !nodeKey.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        // Custom nodes
        // This type of nodes don't supply any content for class-based content specifications
        return;
        }

    if (nullptr != nodeKey.AsECInstanceNodeKey())
        {
        // ECInstance node
        ECInstanceNodeKey const& instanceNodeKey = *nodeKey.AsECInstanceNodeKey();
        ECClassCP ecClass = helper.GetECClass(instanceNodeKey.GetECClassId());
        if (nullptr == ecClass || !ecClass->IsEntityClass())
            {
            BeAssert(false);
            return;
            }
        if (m_classSelection.end() == m_classSelection.find(ecClass))
            m_orderedClasses.push_back(ecClass);
        m_classSelection[ecClass].push_back(instanceNodeKey.GetInstanceId());
        return;
        }
        
    // Some grouping node
    NavNodeCPtr node = nodesLocater.LocateNode(nodeKey);
    if (node.IsNull())
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(*node);
    bvector<ECInstanceKey> groupedInstanceKeys = extendedData.GetGroupedInstanceKeys();
    for (ECInstanceKeyCR key : groupedInstanceKeys)
        GetNodeClasses(*ECInstanceNodeKey::Create(key), nodesLocater, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::Parse(NavNodeKeyListCR keys, INavNodeLocater const& nodesLocater, ECSchemaHelper const& helper)
    {
    for (NavNodeKeyCPtr const& key : keys)
        GetNodeClasses(*key, nodesLocater, helper);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedSelectionInfo::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classSelection.find(&selectClass);
    if (m_classSelection.end() != iter)
        return iter->second;
        
    static bvector<ECInstanceId> s_empty;
    return s_empty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRelatedClassPaths(ECSchemaHelper const& helper, ContentDescriptorCreateContext& context, ECClassCR nodeClass, 
    ContentRelatedInstancesSpecificationCR specification, PresentationRuleSetCR ruleset)
    {
    int skipRelatedLevel = specification.IsRecursive() ? -1 : specification.GetSkipRelatedLevel();
    ECSchemaHelper::RelationshipClassPathOptions options(nodeClass, GetRelationshipDirection(specification), 
        skipRelatedLevel, ruleset.GetSupportedSchemas(), specification.GetRelationshipClassNames(), 
        specification.GetRelatedClassNames(), context.GetRelationshipUseCounts());
    bvector<bpair<RelatedClassPath, bool>> relationshipClassPaths = helper.GetRelationshipClassPaths(options);

    bvector<RelatedClassPath> paths;
    for (bpair<RelatedClassPath, bool> const& pair : relationshipClassPaths)
        {
        BeAssert(true == pair.second && "Only included paths are supported");
        paths.push_back(pair.first);
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendClass(ContentDescriptorR descriptor, ContentDescriptorCreateContext& context, 
    ECClassCR ecClass, bool isSpecificationPolymorphic, ContentSpecificationCR spec)
    {
    if (context.IsClassHandled(ecClass))
        return;
     
    descriptor.GetSelectClasses().push_back(SelectClassInfo(ecClass, isSpecificationPolymorphic));
    SelectClassInfo& info = descriptor.GetSelectClasses().back();
    
    bvector<RelatedClassPath> navigationPropertiesPaths;
    PropertyAppender appender(descriptor, context, ecClass, "this", RelatedClassPath(), navigationPropertiesPaths);
    ECPropertyIterable properties = ecClass.GetProperties(true);
    for (ECPropertyCP prop : properties)
        appender.Append(*prop);

    bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(descriptor, context, RelatedClassPath(), 
        ecClass, "this", spec.GetRelatedProperties(), false);
    relatedPropertyPaths.insert(relatedPropertyPaths.end(), navigationPropertiesPaths.begin(), navigationPropertiesPaths.end());
    info.SetRelatedPropertyPaths(relatedPropertyPaths);

    context.SetClassHandled(ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendClassPaths(ContentDescriptorR descriptor, ContentDescriptorCreateContext& context, 
    bvector<RelatedClassPath> const& paths, ECClassCR nodeClass, ContentSpecificationCR spec)
    {
    for (RelatedClassPath path : paths)
        {
        bool isSelectPolymorphic = path.back().IsPolymorphic();
        QueryBuilderHelpers::Reverse(path, "related", false);

        ECClassCR selectClass = *path.front().GetSourceClass();
        bvector<RelatedClassPath> navigationPropertiesPaths;
        if (!context.IsClassHandled(selectClass))
            {
            PropertyAppender appender(descriptor, context, selectClass, "this", RelatedClassPath(), navigationPropertiesPaths);
            ECPropertyIterable properties = selectClass.GetProperties(true);
            for (ECPropertyCP prop : properties)
                appender.Append(*prop);
            context.AddNavigationPropertiesPaths(selectClass, navigationPropertiesPaths);
            context.SetClassHandled(selectClass);
            }
        else
            navigationPropertiesPaths = context.GetNavigationPropertiesPaths(selectClass);

        SelectClassInfo appendInfo(selectClass, isSelectPolymorphic);
        appendInfo.SetPathToPrimaryClass(path);
        bvector<RelatedClassPath> relatedPropertyPaths = AppendRelatedProperties(descriptor, context, RelatedClassPath(), 
            selectClass, "this", spec.GetRelatedProperties(), true);
        relatedPropertyPaths.insert(relatedPropertyPaths.end(), navigationPropertiesPaths.begin(), navigationPropertiesPaths.end());
        appendInfo.SetRelatedPropertyPaths(relatedPropertyPaths);
        descriptor.GetSelectClasses().push_back(std::move(appendInfo));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsECClassAccepted(SelectedNodeInstancesSpecificationCR specification, ECClassCR selectedClass)
    {
    if (!specification.GetAcceptableSchemaName().empty() && !specification.GetAcceptableSchemaName().Equals(selectedClass.GetSchema().GetName()))
        return false;

    if (!specification.GetAcceptableClassNames().empty())
        {
        bool didFindAccepted = false;
        bvector<Utf8String> classNames;
        BeStringUtilities::Split(specification.GetAcceptableClassNames().c_str(), ",", classNames);
        for (Utf8String className : classNames)
            {
            className.Trim();
            if (className.Equals(selectedClass.GetName())
                || specification.GetAcceptablePolymorphically() && selectedClass.Is(selectedClass.GetSchema().GetName().c_str(), className.c_str()))
                {
                didFindAccepted = true;
                break;
                }
            }
        return didFindAccepted;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddCalculatedFieldsFromContentModifiers(ContentDescriptorR descriptor, ECClassCR ecClass, ContentQueryBuilderParameters const& params)
    {
    for (ContentModifierCP modifier : params.GetRuleset().GetContentModifierRules())
        {
        ECClassCP modifierClass = params.GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (ecClass.Is(modifierClass))
            QueryBuilderHelpers::AddCalculatedFields(descriptor, modifier->GetCalculatedProperties(), params.GetLocalizationProvider(), params.GetRuleset(), modifierClass);
        }
    }

#ifdef wip_descriptor
//=======================================================================================
//! Terminology:
//! - Calculated descriptor - created just looking at the presentation rules.
//! - Custom descriptor - created by the API user by copying the calculated descriptor and
//! modifying it. It may have different flags or other parameters, compared to the 
//! calculated descriptor.
//! - Aggregate descriptor - aggregated from multiple descriptors. Aggregating is necessary
//! because single query builder is used to create queries for multiple specifications.
//! Finally, those queries are UNIONed together, so they must use the same aggregate descriptor
//! so make sure their selected field counts and types match.
//! specifications; those queries are then UNIONed into a single query
// @bsiclass                                    Grigas.Petraitis                07/2017
//=======================================================================================
struct DescriptorHandler
{
private:
    ContentQueryBuilder& m_builder;
    ContentDescriptorP m_aggregateDescriptor;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void ApplyModifications(ContentDescriptorR target, ContentDescriptorCR source)
        {
        bvector<ContentDescriptor::Field*> targetFields = target.GetVisibleFields();
        bvector<ContentDescriptor::Field*> sourceFields = source.GetVisibleFields();
        bvector<ContentDescriptor::Field const*> erasedFields;
        for (ContentDescriptor::Field const* tf : targetFields)
            {
            if (sourceFields.end() == std::find_if(sourceFields.begin(), sourceFields.end(),
                [&](ContentDescriptor::Field const* sf){return sf->GetName().Equals(tf->GetName());}))
                {
                target.RemoveField(tf->GetName().c_str());
                }
            }

        if (nullptr != source.GetSortingField())
            target.SetSortingField(source.GetSortingField()->GetName().c_str());
        target.SetSortDirection(source.GetSortDirection());
        target.SetContentFlags(target.GetContentFlags() | source.GetContentFlags());
        target.SetFilterExpression(source.GetFilterExpression());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    DescriptorHandler(ContentQueryBuilder& builder, ContentDescriptorR calculatedDescriptor, ContentDescriptorCP customDescriptor)
        : m_builder(builder)
        {
        ContentDescriptorPtr descriptor = &calculatedDescriptor;
        if (nullptr != customDescriptor)
            ApplyModifications(*descriptor, *customDescriptor);
        m_aggregateDescriptor = m_builder.GetAggregateDescriptor(descriptor.get());
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorCR GetDescriptor() const {return *m_aggregateDescriptor;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ApplyOverrides(ContentQueryPtr& query)
        {
        ContentDescriptorCP overridesDescriptor = m_aggregateDescriptor;
        if (m_aggregateDescriptor->MergeResults())
            {
            query = m_builder.CreateMergedResultsQuery(*query, *m_aggregateDescriptor, *m_aggregateDescriptor);
            overridesDescriptor = &query->GetContract()->GetDescriptor();
            }
    
        QueryBuilderHelpers::ApplyDescriptorOverrides(query, *overridesDescriptor, m_builder.GetParameters().GetECExpressionsCache());
        }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentQueryBuilder::CreateDescriptor(SelectedNodeInstancesSpecificationCR specification, IParsedSelectionInfo const& selection)
    {
    if (selection.GetClasses().empty())
        return nullptr;

    ContentDescriptorPtr descriptor = ContentDescriptor::Create(m_params.GetPreferredDisplayType());
    ContentDescriptorCreateContext context(m_params.GetSchemaHelper(), m_params.GetCategorySupplier(), 
        m_params.GetPropertyFormatter(), m_params.GetRuleset(), &specification);
    QueryBuilderHelpers::ApplyDefaultContentFlags(*descriptor, m_params.GetPreferredDisplayType(), specification);
    for (ECClassCP ecClass : selection.GetClasses())
        {
        if (!IsECClassAccepted(specification, *ecClass))
            continue;
            
        AppendClass(*descriptor, context, *ecClass, false, specification);
        AddCalculatedFieldsFromContentModifiers(*descriptor, *ecClass, GetParameters());
        }
    if (descriptor->GetSelectClasses().empty())
        return nullptr;

    QueryBuilderHelpers::AddCalculatedFields(*descriptor, specification.GetCalculatedProperties(), 
        m_params.GetLocalizationProvider(), m_params.GetRuleset(), nullptr);

    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorPtr specificationDescriptor = CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
#ifdef wip_descriptor
    // note: at this point the descriptor is completely calculated based on rules,
    // we use DescriptorHandler to handle customizations and aggregation with other
    // descriptors (created using different rules)
    DescriptorHandler descriptorHandler(*this, *specificationDescriptor, &descriptor);
#endif

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle filtering by selection
        bvector<ECInstanceId> const& selectedInstanceIds = selection.GetInstanceIds(selectClassInfo.GetSelectClass());
        if (!selectedInstanceIds.empty())
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", {new BoundQueryIdSet(std::move(selectedInstanceIds))});

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }
    
#ifdef wip_descriptor
    if (query.IsValid())
        descriptorHandler.ApplyOverrides(query);
#else
    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());
#endif

    return query;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct RecursiveQueriesHelper
{
private:
    ContentDescriptorCR m_descriptor;
    BoundQueryRecursiveChildrenIdSet* m_fwdRecursiveChildrenIds;
    BoundQueryRecursiveChildrenIdSet* m_bwdRecursiveChildrenIds;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsRecursiveJoinForward(SelectClassInfo const& selectInfo)
        {
        if (selectInfo.GetPathToPrimaryClass().empty())
            {
            BeAssert(false);
            return true;
            }
    
        return !selectInfo.GetPathToPrimaryClass().back().IsForwardRelationship(); // invert direction
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsPathValidForRecursiveSelect(Utf8StringR errorMessage, RelatedClassPath const& path, ECClassCR selectClass)
        {
        RelatedClass const& relatedClassDef = path.front();
        if (!selectClass.Is(relatedClassDef.GetSourceClass()))
            {
            errorMessage = "Using IsRecursive requires recursive relationship";
            return false;
            }
        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    RecursiveQueriesHelper(ContentDescriptorCR descriptor) 
        : m_descriptor(descriptor), m_fwdRecursiveChildrenIds(nullptr), m_bwdRecursiveChildrenIds(nullptr)
        {}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BoundQueryRecursiveChildrenIdSet const& GetRecursiveChildrenIds(IParsedSelectionInfo const& selection, SelectClassInfo const& thisInfo)
        {
        bool forward = IsRecursiveJoinForward(thisInfo);
        BoundQueryRecursiveChildrenIdSet*& set = forward ? m_fwdRecursiveChildrenIds : m_bwdRecursiveChildrenIds;
        if (nullptr == set)
            {
            bset<ECRelationshipClassCP> relationships;
            for (SelectClassInfo const& selectClassInfo : m_descriptor.GetSelectClasses())
                {
                Utf8String validationErrorMessage;
                if (!IsPathValidForRecursiveSelect(validationErrorMessage, selectClassInfo.GetPathToPrimaryClass(), selectClassInfo.GetSelectClass()))
                    {
                    BeAssert(false);
                    LoggingHelper::LogMessage(Log::Content, validationErrorMessage.c_str(), NativeLogging::LOG_ERROR);
                    }
                else
                    {
                    for (RelatedClassCR rel : selectClassInfo.GetPathToPrimaryClass())
                        relationships.insert(rel.GetRelationship());
                    }
                }
            set = new BoundQueryRecursiveChildrenIdSet(relationships, forward, selection.GetInstanceIds(*thisInfo.GetPrimaryClass()));
            }
        return *set;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessQueryClassesPolymorphically(SupportedEntityClassInfos& infos, bvector<ECClassCP> const& entityClassList)
    {
    bset<SupportedEntityClassInfo*> infosWithBasePrimaryClasses;
    bset<SupportedEntityClassInfo> infosToAppend;
    for (SupportedEntityClassInfo& info : infos)
        {
        if (!info.IsPolymorphic())
            continue;

        for (ECClassCP entityClass: entityClassList)
            {
            // check the primary select class
            if (entityClass != &info.GetClass() && entityClass->Is(&info.GetClass()))
                {
                // the rule wants to customize a subclass of the class and leave other subclasses unsorted -
                // this means we have to expand the ecClass into its subclasses
                bset<SupportedEntityClassInfo> subclassInfos;
                for (ECClassCP derived : info.GetClass().GetDerivedClasses())
                    {
                    SupportedEntityClassInfo copy(*derived->GetEntityClassCP());
                    copy.SetFlags((int)CLASS_FLAG_Polymorphic);
                    subclassInfos.insert(std::move(copy));
                    }

                ProcessQueryClassesPolymorphically(subclassInfos, entityClassList);
                infosWithBasePrimaryClasses.insert(&info);
                infosToAppend.insert(subclassInfos.begin(), subclassInfos.end());
                }
            }
        }

    // the primary and related classes that were polymorphic and have been split into subclasses
    // should be changed to be selected non-polymorphically
    for (SupportedEntityClassInfo* selectInfo : infosWithBasePrimaryClasses)
        selectInfo->SetFlags(selectInfo->GetFlags() & ~CLASS_FLAG_Polymorphic);

    // don't want to append selects that are already in the input vector
    for (SupportedEntityClassInfo const& selectInfo : infos)
        {
        auto iter = infosToAppend.find(selectInfo);
        if (infosToAppend.end() != iter)
            infosToAppend.erase(iter);
        }

    // append the additional selects
    for (SupportedEntityClassInfo const& info : infosToAppend)
        infos.insert(info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessRelationshipPathsPolymorphically(bvector<RelatedClassPath>& relatedPaths, bvector<ECClassCP> const& modifierClassList)
    {
    bvector<RelatedClassPath> childPaths;
    for (ECClassCP modifierClass : modifierClassList)
        {
        for (RelatedClassPath& related : relatedPaths)
            {
            if (!related.back().IsPolymorphic() || !modifierClass->Is(related.back().GetTargetClass()))
                continue;

            for (ECClassCP derived : related.back().GetTargetClass()->GetDerivedClasses())
                {
                RelatedClassPath copy(related);
                copy.back().SetTargetClass(*derived->GetEntityClassCP());
                childPaths.push_back(copy);
                }
            related.back().SetIsPolymorphic(false);
            ProcessRelationshipPathsPolymorphically(childPaths, modifierClassList);
            }
        }
    for (RelatedClassPath const& relatedPath : childPaths)
        relatedPaths.push_back(relatedPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECClassCP> CollectContentModifiers(ContentQueryBuilderParameters const& params)
    {
    bvector<ECClassCP> modifierClassList;
    for (ContentModifierP modifier : params.GetRuleset().GetContentModifierRules())
        {
        ECClassCP ecClass = params.GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
        if (nullptr == ecClass)
            BeAssert(false);
        else
            modifierClassList.push_back(ecClass); 
        }
    return modifierClassList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentQueryBuilder::CreateDescriptor(ContentRelatedInstancesSpecificationCR specification, IParsedSelectionInfo const& selection)
    {
    if (selection.GetClasses().empty())
        return nullptr;
    
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(m_params.GetPreferredDisplayType());
    ContentDescriptorCreateContext context(m_params.GetSchemaHelper(), m_params.GetCategorySupplier(), 
        m_params.GetPropertyFormatter(), m_params.GetRuleset(), &specification);
    QueryBuilderHelpers::ApplyDefaultContentFlags(*descriptor, m_params.GetPreferredDisplayType(), specification);

    bvector<ECClassCP> modifierClasses = CollectContentModifiers(m_params);

    for (ECClassCP ecClass : selection.GetClasses())
        {
        bvector<RelatedClassPath> paths = GetRelatedClassPaths(m_params.GetSchemaHelper(), context, *ecClass, specification, GetParameters().GetRuleset());
        ProcessRelationshipPathsPolymorphically(paths, modifierClasses);
        for (RelatedClassPathCR relatedPath : paths)
            AddCalculatedFieldsFromContentModifiers(*descriptor, *relatedPath.back().GetTargetClass(), GetParameters());
        AppendClassPaths(*descriptor, context, paths, *ecClass, specification/*, specification.IsRecursive()*/);
        }
    if (descriptor->GetSelectClasses().empty())
        return nullptr;

    QueryBuilderHelpers::AddCalculatedFields(*descriptor, specification.GetCalculatedProperties(), 
        GetParameters().GetLocalizationProvider(), GetParameters().GetRuleset(), nullptr);
    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorPtr specificationDescriptor = CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
#ifdef wip_descriptor
    // note: at this point the descriptor is completely calculated based on rules,
    // we use DescriptorHandler to handle customizations and aggregation with other
    // descriptors (created using different rules)
    DescriptorHandler descriptorHandler(*this, *specificationDescriptor, &descriptor);
#endif
    RecursiveQueriesHelper recursiveQueries(*specificationDescriptor);

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        if (specification.IsRecursive())
            {
            // in case of recursive query just bind the children ids
            classQuery->Where("InVirtualSet(?, [this].[ECInstanceId])", 
                {new BoundQueryRecursiveChildrenIdSet(recursiveQueries.GetRecursiveChildrenIds(selection, selectClassInfo))});
            }
        else
            {
            BeAssert(!selectClassInfo.GetPathToPrimaryClass().empty());
            classQuery->Join(selectClassInfo.GetPathToPrimaryClass(), false);
            classQuery->Where("InVirtualSet(?, [related].[ECInstanceId])", {new BoundQueryIdSet(selection.GetInstanceIds(*selectClassInfo.GetPrimaryClass()))});
            }

            // handle related properties
            for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
                classQuery->Join(path, true);
            
            // handle instance filtering
            if (!specification.GetInstanceFilter().empty())
                classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }
    
#ifdef wip_descriptor
    if (query.IsValid())
        descriptorHandler.ApplyOverrides(query);
#else
    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());
#endif

    return query;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentQueryBuilder::CreateDescriptor(ContentInstancesOfSpecificClassesSpecificationCR specification)
    {
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(m_params.GetPreferredDisplayType());
    SupportedEntityClassInfos classInfos = m_params.GetSchemaHelper().GetECClassesFromClassList(specification.GetClassNames(), false);
    ContentDescriptorCreateContext context(m_params.GetSchemaHelper(), m_params.GetCategorySupplier(), 
        m_params.GetPropertyFormatter(), m_params.GetRuleset(), &specification);
    QueryBuilderHelpers::ApplyDefaultContentFlags(*descriptor, m_params.GetPreferredDisplayType(), specification);

    if (specification.GetArePolymorphic())
        ProcessQueryClassesPolymorphically(classInfos, CollectContentModifiers(m_params));

    for (SupportedEntityClassInfo const& classInfo : classInfos)
        {
        bool appendPolymorphically = specification.GetArePolymorphic() & classInfo.IsPolymorphic();
        AppendClass(*descriptor, context, classInfo.GetClass(), appendPolymorphically, specification);
        AddCalculatedFieldsFromContentModifiers(*descriptor, classInfo.GetClass(), GetParameters());
        }
    
    QueryBuilderHelpers::AddCalculatedFields(*descriptor, specification.GetCalculatedProperties(), 
        GetParameters().GetLocalizationProvider(), GetParameters().GetRuleset(), nullptr);
    return descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentInstancesOfSpecificClassesSpecificationCR specification, ContentDescriptorCR descriptor)
    {
    ContentDescriptorPtr specificationDescriptor = CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
#ifdef wip_descriptor
    // note: at this point the descriptor is completely calculated based on rules,
    // we use DescriptorHandler to handle customizations and aggregation with other
    // descriptors (created using different rules)
    DescriptorHandler descriptorHandler(*this, *specificationDescriptor, &descriptor);
#endif
        
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        classQuery->SelectContract(*ContentQueryContract::Create(descriptor, &selectClassInfo.GetSelectClass(), *classQuery), "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);

        // handle instance filtering
        if (!specification.GetInstanceFilter().empty())
            classQuery->Where(ECExpressionsHelper(m_params.GetECExpressionsCache()).ConvertToECSql(specification.GetInstanceFilter()).c_str(), BoundQueryValuesList());

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }
    
#ifdef wip_descriptor
    if (query.IsValid())
        descriptorHandler.ApplyOverrides(query);
#else
    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());
#endif

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentDescriptor::NestedContentField const& contentField)
    {
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(m_params.GetPreferredDisplayType());
    ContentDescriptorCreateContext context(m_params.GetSchemaHelper(), m_params.GetCategorySupplier(), 
        m_params.GetPropertyFormatter(), m_params.GetRuleset(), nullptr);

    for (ContentDescriptor::Field const* field : contentField.GetFields())
        {
        if (field->IsPropertiesField())
            {
            ContentDescriptor::ECPropertiesField const* propertiesField = field->AsPropertiesField();
            BeAssert(1 == propertiesField->GetProperties().size());
            if (propertiesField->GetProperties().front().GetProperty().GetIsNavigation())
                continue;

            bvector<RelatedClassPath> navigationPropertiesPaths;
            PropertyAppender appender(*descriptor, context, contentField.GetContentClass(), "this", RelatedClassPath(), navigationPropertiesPaths);
            appender.Append(propertiesField->GetProperties().front().GetProperty());
            }
        else if (field->IsNestedContentField())
            {
            ContentDescriptor::NestedContentField const* nestedField = field->AsNestedContentField();
            descriptor->GetAllFields().push_back(nestedField->Clone());
            }
        else
            {
            BeAssert(false && "Unexpected type of field");
            }
        }
        
    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    query->SelectContract(*ContentQueryContract::Create(*descriptor, &contentField.GetContentClass(), *query), "this");
    query->From(contentField.GetContentClass(), true, "this");

    RelatedClassPath relationshipPath;
    for (auto iter = contentField.GetRelationshipPath().rbegin(); iter != contentField.GetRelationshipPath().rend(); ++iter)
        relationshipPath.push_back(*iter);
    query->Join(relationshipPath, false);
    
    return query;
    }
