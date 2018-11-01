/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PropertyInfoStore.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                07/2017
+===============+===============+===============+===============+===============+======*/
struct PropertyDisplayInfo
{
private:
    Utf8String m_propertyName;
    int m_priority;
    bool m_displayed;

public:
    PropertyDisplayInfo() {}
    PropertyDisplayInfo(Utf8String propertyName) : m_propertyName(propertyName) {}
    PropertyDisplayInfo(Utf8String propertyName, int priority, bool displayed) : m_propertyName(propertyName), m_priority(priority), m_displayed(displayed) {}

    bool operator<(PropertyDisplayInfo const& rhs) const
        {
        return strcmp(GetPropertyName(), rhs.GetPropertyName()) < 0;
        }

    Utf8CP GetPropertyName() const {return m_propertyName.c_str();}
    int GetPriority() const {return m_priority;}
    bool IsDisplayed() const {return m_displayed;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct DefaultDisplayInfo
{
private:
    bool m_display;
    int m_priority;
public:
    DefaultDisplayInfo() : m_display(false), m_priority(0) {}
    DefaultDisplayInfo(bool display, int priority) : m_display(display), m_priority(priority) {}
    bool ShouldDisplay() const {return m_display;}
    int GetPriority() const {return m_priority;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct DisplayInfo
{
private:
    bmap<ECClassCP, DefaultDisplayInfo> m_defaultDisplayInfos;
    bset<PropertyDisplayInfo> m_propertyDisplayInfos;

public:
    DefaultDisplayInfo const* GetDefaultDisplayInfo(ECClassCP ecClass) const
        {
        auto iter = m_defaultDisplayInfos.find(ecClass);
        return (m_defaultDisplayInfos.end() != iter) ? &iter->second : nullptr;
        }
    void AddDefaultDisplayInfo(ECClassCP ecClass, DefaultDisplayInfo info) {m_defaultDisplayInfos[ecClass] = info;}

    bset<PropertyDisplayInfo> const& GetPropertyDisplayInfos() const {return m_propertyDisplayInfos;}
    bset<PropertyDisplayInfo>& GetPropertyDisplayInfos() {return m_propertyDisplayInfos;}

    bool ShouldDisplay(ECClassCR, ECPropertyCR) const;
    void Merge(DisplayInfo const& other);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct PropertyInfoStore
{
private:
    ECSchemaHelper const& m_schemaHelper;
    bmap<ECClassCP, DisplayInfo> m_perClassPropertyDisplayInfos; // per-class property display info
    mutable bmap<ECClassCP, DisplayInfo> m_aggregatedPropertyDisplayInfos; // property display info, including base class properties
    bmap<ECClassCP, bmap<Utf8String, ContentFieldEditor const*>> m_propertyEditors;

private:
    void CollectPropertiesDisplayRules(ECClassCP ecClass, PropertiesDisplaySpecificationCR spec);
    void InitPropertyDisplayInfos(ContentSpecificationCP specification, ContentModifierList const& contentModifiers);
    DisplayInfo const& GetDisplayInfo(ECClassCR ecClass) const;
    static ContentFieldEditor const* CreateEditor(PropertyEditorsSpecificationCR spec);
    void InitPropertyEditors(ContentSpecificationCP specification, ContentModifierList const& contentModifiers);

public:
    PropertyInfoStore(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, ContentSpecificationCP spec);
    bool ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass) const;
    ContentFieldEditor const* GetPropertyEditor(ECPropertyCR ecProperty, ECClassCR ecClass, bool searchForAnyClassEditor = true) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
