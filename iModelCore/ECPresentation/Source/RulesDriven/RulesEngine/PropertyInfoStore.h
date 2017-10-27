/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PropertyInfoStore.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct PropertyInfoStore
{
private:
    ECSchemaHelper const& m_schemaHelper;
    bmap<ECClassCP, bset<PropertiesDisplayInfo>> m_perClassPropertyDisplayInfos; // per-class property display info
    mutable bmap<ECClassCP, bset<PropertiesDisplayInfo>> m_aggregatedPropertyDisplayInfos; // property display info, including base class properties
    bmap<ECClassCP, bmap<Utf8String, ContentFieldEditor const*>> m_propertyEditors;

private:
    static void InsertPropertiesDisplayInfo(ECClassCP ecClass, bset<PropertiesDisplayInfo> const& source, bset<PropertiesDisplayInfo>& target);
    void CollectPropertiesDisplayRules(ECClassCP ecClass, PropertiesDisplaySpecificationCR spec);
    void InitPropertiesDisplayInfo(ContentSpecificationCP specification, ContentModifierList const& contentModifiers);
    bset<PropertiesDisplayInfo> const& GetPropertiesDisplayInfo(ECClassCR ecClass) const;
    static ContentFieldEditor const* CreateEditor(PropertyEditorsSpecificationCR spec);
    void InitPropertyEditors(ContentSpecificationCP specification, ContentModifierList const& contentModifiers);

public:
    PropertyInfoStore(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, ContentSpecificationCP spec);
    bool ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass) const;
    ContentFieldEditor const* GetPropertyEditor(ECPropertyCR ecProperty, ECClassCR ecClass, bool searchForAnyClassEditor = true) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
