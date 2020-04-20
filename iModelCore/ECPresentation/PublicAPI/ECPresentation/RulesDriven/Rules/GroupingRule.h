/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct GroupSpecification;
struct ClassGroup;
struct PropertyGroup;
struct PropertyRangeGroupSpecification;

typedef bvector<GroupSpecificationP>              GroupList;
typedef bvector<PropertyRangeGroupSpecificationP> PropertyRangeGroupList;

/*---------------------------------------------------------------------------------**//**
* Interface for grouping rule specification visitor.
* @bsiclass                                     Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupingRuleSpecificationVisitor
{
    friend struct SameLabelInstanceGroup;
    friend struct ClassGroup;
    friend struct PropertyGroup;

protected:
    virtual ~GroupingRuleSpecificationVisitor() {}
    virtual void _Visit(SameLabelInstanceGroup const& specification) {}
    virtual void _Visit(ClassGroup const& specification) {}
    virtual void _Visit(PropertyGroup const& specification) {}
};

/*---------------------------------------------------------------------------------**//**
Presentation rule for child nodes advanced grouping in the hierarchy.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupingRule : public ConditionalCustomizationRule
    {
    private:
        Utf8String m_schemaName;
        Utf8String m_className;
        Utf8String m_contextMenuCondition;
        Utf8String m_contextMenuLabel;
        Utf8String m_settingsId;
        GroupList m_groups;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //!Accepts customization rule visitor
        ECPRESENTATION_EXPORT void _Accept(CustomizationRuleVisitor& visitor) const override;

        //! Computes rule hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

        //! Clones rule.
        CustomizationRule* _Clone() const override {return new GroupingRule(*this);}

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT GroupingRule();

        //! Constructor.
        ECPRESENTATION_EXPORT GroupingRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR contextMenuCondition, Utf8StringCR contextMenuLabel, Utf8StringCR settingsId);

        //! Constructor.
        ECPRESENTATION_EXPORT GroupingRule(GroupingRuleCR);

        //! Desctructor.
        ECPRESENTATION_EXPORT                     ~GroupingRule (void);

        //! Acceptable schema name of ECInstances on which this grouping rule will be applied.
        ECPRESENTATION_EXPORT Utf8StringCR        GetSchemaName (void) const;

        //! Acceptable class name of ECInstances on which this grouping rule will be applied.
        ECPRESENTATION_EXPORT Utf8StringCR        GetClassName (void) const;

        //! ECExpression condition that is used in order to define the node on which "Group By" context menu will be shown.
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECPRESENTATION_EXPORT Utf8StringCR        GetContextMenuCondition (void) const;

        //! Sets the ECExpression condition that is used in order to define the node on which "Group By" context menu will be shown.
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECPRESENTATION_EXPORT void                SetContextMenuCondition (Utf8String value);

        //! Label of the parent context menu for choosing one of the predefined ProeprtyGroups.
        //! If this parameters is not set, the default name will be used - "Group By".
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECPRESENTATION_EXPORT Utf8StringCR        GetContextMenuLabel (void) const;

        //! Id that is used to store current active group. This is used only if there are more than one Group available.
        ECPRESENTATION_EXPORT Utf8StringCR        GetSettingsId (void) const;

        //! Returns a list of GroupSpecifications.
        ECPRESENTATION_EXPORT GroupList const&     GetGroups (void) const;

        //! Add GroupSpecification.
        ECPRESENTATION_EXPORT void AddGroup(GroupSpecificationR group);
    };

/*---------------------------------------------------------------------------------**//**
GroupSpecification that identifies parameters on how to group ECInstances
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupSpecification : HashableBase
    {
private:
    Utf8String  m_contextMenuLabel;
    Utf8String  m_defaultLabel;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT GroupSpecification ();

    //! Constructor.
    ECPRESENTATION_EXPORT GroupSpecification (Utf8StringCR contextMenuLabel, Utf8CP defaultLabel = NULL);

    ECPRESENTATION_EXPORT virtual Utf8CP _GetXmlElementName() const = 0;
    ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) {return true;}
    ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const {}

    ECPRESENTATION_EXPORT virtual Utf8CP _GetJsonElementType() const = 0;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) {return true;}
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const {}

    //! Allows the visitor to visit this group specification.
    virtual void _Accept(GroupingRuleSpecificationVisitor& visitor) const = 0;

    //! Clones this specification.
    virtual GroupSpecification* _Clone() const = 0;

    //! Computes specification hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

public:
    //! Virtual destructor.
    virtual ~GroupSpecification(){}

public:
    static GroupSpecification* Create(JsonValueCR);

    //! Clones this specification.
    GroupSpecification* Clone() const {return _Clone();}

    //! Allows the visitor to visit this group specification.
    ECPRESENTATION_EXPORT void Accept(GroupingRuleSpecificationVisitor& visitor) const;

    //! Reads group specification from xml node.
    ECPRESENTATION_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

    //! Writes group specification to xml node.
    ECPRESENTATION_EXPORT void WriteXml (BeXmlNodeP parentXmlNode) const;

    //! Reads group specification from json.
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR);

    //! Reads group specification from json.
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;

    //! ContextMenu label of this particular grouping option. If not set ECClass or ECProperty DisplayLabel will be used.
    ECPRESENTATION_EXPORT Utf8StringCR             GetContextMenuLabel (void) const;

    //! Default group label to use when the grouping property is null or empty. Optional - overrides the default.
    ECPRESENTATION_EXPORT Utf8StringCR             GetDefaultLabel (void) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
enum class SameLabelInstanceGroupApplicationStage
    {
    Query,
    PostProcess,
    };

/*---------------------------------------------------------------------------------**//**
This grouping option allows to create a Instance NavNode that represents mutiple instances
of the same label.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE SameLabelInstanceGroup : public GroupSpecification
{
private:
    SameLabelInstanceGroupApplicationStage m_applicationStage;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Allows the visitor to visit this group specification.
    ECPRESENTATION_EXPORT void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

    //! Clones this specification.
    GroupSpecification* _Clone() const override {return new SameLabelInstanceGroup(*this);}

    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    SameLabelInstanceGroup(SameLabelInstanceGroupApplicationStage applicationStage = SameLabelInstanceGroupApplicationStage::Query)
        : GroupSpecification(), m_applicationStage(applicationStage)
        {}
    SameLabelInstanceGroup(Utf8StringCR contextMenuLabel)
        : GroupSpecification(contextMenuLabel), m_applicationStage(SameLabelInstanceGroupApplicationStage::Query)
        {}
    SameLabelInstanceGroupApplicationStage GetApplicationStage() const {return m_applicationStage;}
    void SetApplicationStage(SameLabelInstanceGroupApplicationStage value) {m_applicationStage = value;}
};

/*---------------------------------------------------------------------------------**//**
ClassGroup that identifies parameters on how to group ECInstances.
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ClassGroup : public GroupSpecification
    {
    private:
        bool         m_createGroupForSingleItem;
        Utf8String   m_schemaName;
        Utf8String   m_baseClassName;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Allows the visitor to visit this group specification.
        ECPRESENTATION_EXPORT void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

        //! Clones this specification.
        GroupSpecification* _Clone() const override {return new ClassGroup(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ClassGroup ();

        //! Constructor.
        ECPRESENTATION_EXPORT ClassGroup (Utf8StringCR contextMenuLabel, bool createGroupForSingleItem, Utf8StringCR schemaName, Utf8StringCR baseClassName);

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECPRESENTATION_EXPORT bool                     GetCreateGroupForSingleItem (void) const;

        //! ECSchema name of base class.
        ECPRESENTATION_EXPORT Utf8StringCR             GetSchemaName (void) const;

        //! Base ECClass name to group ECInstances by.
        ECPRESENTATION_EXPORT Utf8StringCR             GetBaseClassName (void) const;
    };

/*---------------------------------------------------------------------------------**//**
* Lists possible grouping values.
* @bsiclass                                     Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PropertyGroupingValue
    {
    PropertyValue,  //!< Groups by property value
    DisplayLabel,   //!< Groups by display label
    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE PropertyGroup : public GroupSpecification
    {
    private:
        Utf8String              m_imageId;
        bool                    m_createGroupForSingleItem;
        bool                    m_createGroupForUnspecifiedValues;
        PropertyGroupingValue   m_groupingValue;
        PropertyGroupingValue   m_sortingValue;
        Utf8String              m_propertyName;
        PropertyRangeGroupList  m_ranges;

    protected:
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

        //! Allows the visitor to visit this group specification.
        ECPRESENTATION_EXPORT void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

        //! Clones this specification.
        GroupSpecification* _Clone() const override {return new PropertyGroup(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT PropertyGroup ();

        //! Constructor.
        ECPRESENTATION_EXPORT PropertyGroup (Utf8StringCR contextMenuLabel, Utf8StringCR imageId, bool createGroupForSingleItem, Utf8StringCR propertyName, Utf8CP defaultLabel = NULL);

        //! Constructor.
        ECPRESENTATION_EXPORT PropertyGroup(PropertyGroupCR);

        //! Destructor.
        ECPRESENTATION_EXPORT                          ~PropertyGroup (void);

        //! ImageId of the grouping node. Can be ECExpression. If not set ECClass or ECProperty ImageId will be used.
        ECPRESENTATION_EXPORT Utf8StringCR             GetImageId (void) const;

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECPRESENTATION_EXPORT bool                     GetCreateGroupForSingleItem (void) const;

        //! Idendifies whether a group should be created for unspecified (NULL) values.
        ECPRESENTATION_EXPORT bool                     GetCreateGroupForUnspecifiedValues() const;

        //! Should a group should be created for unspecified (NULL) values.
        ECPRESENTATION_EXPORT void                     SetCreateGroupForUnspecifiedValues(bool value);

        //! ECProperty name to group ECInstances by.
        ECPRESENTATION_EXPORT Utf8StringCR             GetPropertyName (void) const;

        //! Get the property grouping value type.
        PropertyGroupingValue GetPropertyGroupingValue() const {return m_groupingValue;}

        //! Set the property grouping value type.
        void SetPropertyGroupingValue(PropertyGroupingValue value) {m_groupingValue = value;}

        //! Get the sorting value type.
        PropertyGroupingValue GetSortingValue() const {return m_sortingValue;}

        //! Set the sorting value type.
        void SetSortingValue(PropertyGroupingValue value) {m_sortingValue = value;}

        //! List of grouping ranges. If grouping ranges are not specified ECInstances will be grouped by common value.
        ECPRESENTATION_EXPORT PropertyRangeGroupList const&  GetRanges (void) const;

        ECPRESENTATION_EXPORT void AddRange(PropertyRangeGroupSpecificationR range);
    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyRangeGroupSpecification : HashableBase
    {
    private:
        Utf8String  m_label;
        Utf8String  m_imageId;
        Utf8String  m_fromValue;
        Utf8String  m_toValue;

    protected:
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT PropertyRangeGroupSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT PropertyRangeGroupSpecification (Utf8StringCR label, Utf8StringCR imageId, Utf8StringCR fromValue, Utf8StringCR toValue);

        //! Reads specification from xml.
        ECPRESENTATION_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECPRESENTATION_EXPORT void WriteXml (BeXmlNodeP parentXmlNode) const;

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

        //! Reads rule information from Json, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT Json::Value WriteJson() const;

        //! ImageId of the grouping range node. If not set ECProperty ImageId will be used.
        ECPRESENTATION_EXPORT Utf8StringCR GetLabel (void) const;

        //! ImageId of the grouping node. Can be ECExpression. If not set ECProperty ImageId will be used.
        ECPRESENTATION_EXPORT Utf8StringCR GetImageId (void) const;

        //! Property that defines the range starting point. It is string for being able to define Units.
        ECPRESENTATION_EXPORT Utf8StringCR GetFromValue (void) const;

        //! Property that defines the range end point. It is string for being able to define Units.
        ECPRESENTATION_EXPORT Utf8StringCR GetToValue (void) const;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
