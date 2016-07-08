/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/GroupingRule.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRule.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
struct GroupingRule : public PresentationRule
    {
    private:
        Utf8String            m_schemaName;
        Utf8String            m_className;
        Utf8String            m_contextMenuCondition;
        Utf8String            m_contextMenuLabel;
        Utf8String            m_settingsId;
        GroupList             m_groups;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT GroupingRule ();

        //! Constructor.
        ECOBJECTS_EXPORT GroupingRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR contextMenuCondition, Utf8StringCR contextMenuLabel, Utf8StringCR settingsId);

        //! Desctructor.
        ECOBJECTS_EXPORT                     ~GroupingRule (void);

        //! Acceptable schema name of ECInstances on which this grouping rule will be applied.
        ECOBJECTS_EXPORT Utf8StringCR        GetSchemaName (void) const;

        //! Acceptable class name of ECInstances on which this grouping rule will be applied.
        ECOBJECTS_EXPORT Utf8StringCR        GetClassName (void) const;

        //! ECExpression condition that is used in order to define the node on which "Group By" context menu will be shown.
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECOBJECTS_EXPORT Utf8StringCR        GetContextMenuCondition (void) const;

        //! Sets the ECExpression condition that is used in order to define the node on which "Group By" context menu will be shown.
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECOBJECTS_EXPORT void                SetContextMenuCondition (Utf8String value);

        //! Label of the parent context menu for choosing one of the predefined ProeprtyGroups.
        //! If this parameters is not set, the default name will be used - "Group By". 
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECOBJECTS_EXPORT Utf8StringCR        GetContextMenuLabel (void) const;

        //! Id that is used to store current active group. This is used only if there are more than one Group available.
        ECOBJECTS_EXPORT Utf8StringCR        GetSettingsId (void) const;

        //! Returns a list of GroupSpecifications.
        ECOBJECTS_EXPORT GroupList const&     GetGroups (void) const;
        ECOBJECTS_EXPORT GroupList&           GetGroupsR (void);
    };

/*---------------------------------------------------------------------------------**//**
GroupSpecification that identifies parameters on how to group ECInstances
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupSpecification
    {
private:
    Utf8String  m_contextMenuLabel;
    Utf8String  m_defaultLabel;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECOBJECTS_EXPORT GroupSpecification ();

    //! Constructor.
    ECOBJECTS_EXPORT GroupSpecification (Utf8StringCR contextMenuLabel, Utf8CP defaultLabel = NULL);

    //! Returns XmlElement name that is used to read/save this rule information.
    virtual CharCP _GetXmlElementName () const = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    virtual bool _ReadXml (BeXmlNodeP xmlNode) = 0;

    //! Writes rule information to given XmlNode.
    virtual void _WriteXml (BeXmlNodeP xmlNode) const = 0;
    
    //! Allows the visitor to visit this group specification.
    virtual void _Accept(GroupingRuleSpecificationVisitor& visitor) const = 0;

public:
    //! Virtual destructor.
    virtual ~GroupSpecification(){}

public:
    //! Allows the visitor to visit this group specification.
    ECOBJECTS_EXPORT void Accept(GroupingRuleSpecificationVisitor& visitor) const;
    
    //! Reads group specification from xml node.
    ECOBJECTS_EXPORT bool                     ReadXml (BeXmlNodeP xmlNode);

    //! Writes group specification to xml node.
    ECOBJECTS_EXPORT void                     WriteXml (BeXmlNodeP parentXmlNode) const;

    //! ContextMenu label of this particular grouping option. If not set ECClass or ECProperty DisplayLabel will be used.
    ECOBJECTS_EXPORT Utf8StringCR             GetContextMenuLabel (void) const;

    //! Default group label to use when the grouping property is null or empty. Optional - overrides the default.
    ECOBJECTS_EXPORT Utf8StringCR             GetDefaultLabel (void) const;
    };

/*---------------------------------------------------------------------------------**//**
This grouping option allows to create a Instance NavNode that represents mutiple instances
of the same label.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SameLabelInstanceGroup : public GroupSpecification
    {
    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Allows the visitor to visit this group specification.
        ECOBJECTS_EXPORT virtual void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SameLabelInstanceGroup ();

        //! Constructor.
        ECOBJECTS_EXPORT SameLabelInstanceGroup (Utf8StringCR contextMenuLabel);

    };

/*---------------------------------------------------------------------------------**//**
ClassGroup that identifies parameters on how to group ECInstances.
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassGroup : public GroupSpecification
    {
    private:
        bool         m_createGroupForSingleItem;
        Utf8String   m_schemaName;
        Utf8String   m_baseClassName;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Allows the visitor to visit this group specification.
        ECOBJECTS_EXPORT virtual void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ClassGroup ();

        //! Constructor.
        ECOBJECTS_EXPORT ClassGroup (Utf8StringCR contextMenuLabel, bool createGroupForSingleItem, Utf8StringCR schemaName, Utf8StringCR baseClassName);

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForSingleItem (void) const;

        //! ECSchema name of base class.
        ECOBJECTS_EXPORT Utf8StringCR             GetSchemaName (void) const;

        //! Base ECClass name to group ECInstances by.
        ECOBJECTS_EXPORT Utf8StringCR             GetBaseClassName (void) const;
    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances 
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyGroup : public GroupSpecification
    {
    private:
        Utf8String              m_imageId;
        bool                    m_createGroupForSingleItem;
        bool                    m_createGroupForUnspecifiedValues;
        Utf8String              m_propertyName;
        PropertyRangeGroupList  m_ranges;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Allows the visitor to visit this group specification.
        ECOBJECTS_EXPORT virtual void _Accept(GroupingRuleSpecificationVisitor& visitor) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT PropertyGroup ();

        //! Constructor.
        ECOBJECTS_EXPORT PropertyGroup (Utf8StringCR contextMenuLabel, Utf8StringCR imageId, bool createGroupForSingleItem, Utf8StringCR propertyName, Utf8CP defaultLabel = NULL);

        //! Destructor.
        ECOBJECTS_EXPORT                          ~PropertyGroup (void);

        //! ImageId of the grouping node. Can be ECExpression. If not set ECClass or ECProperty ImageId will be used.
        ECOBJECTS_EXPORT Utf8StringCR             GetImageId (void) const;

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForSingleItem (void) const;
        
        //! Idendifies whether a group should be created for unspecified (NULL) values.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForUnspecifiedValues() const;
        
        //! Should a group should be created for unspecified (NULL) values.
        ECOBJECTS_EXPORT void                     SetCreateGroupForUnspecifiedValues(bool value);

        //! ECProperty name to group ECInstances by.
        ECOBJECTS_EXPORT Utf8StringCR             GetPropertyName (void) const;

        //! List of grouping ranges. If grouping ranges are not specified ECInstances will be grouped by common value.
        ECOBJECTS_EXPORT PropertyRangeGroupList const&  GetRanges (void) const;
        ECOBJECTS_EXPORT PropertyRangeGroupList&  GetRangesR (void);
    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances 
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyRangeGroupSpecification
    {
    private:
        Utf8String  m_label;
        Utf8String  m_imageId;
        Utf8String  m_fromValue;
        Utf8String  m_toValue;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT PropertyRangeGroupSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT PropertyRangeGroupSpecification (Utf8StringCR label, Utf8StringCR imageId, Utf8StringCR fromValue, Utf8StringCR toValue);

        //! Reads specification from xml.
        ECOBJECTS_EXPORT bool                     ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECOBJECTS_EXPORT void                     WriteXml (BeXmlNodeP parentXmlNode) const;

        //! ImageId of the grouping range node. If not set ECProperty ImageId will be used.
        ECOBJECTS_EXPORT Utf8StringCR             GetLabel (void) const;

        //! ImageId of the grouping node. Can be ECExpression. If not set ECProperty ImageId will be used.
        ECOBJECTS_EXPORT Utf8StringCR             GetImageId (void) const;

        //! Property that defines the range starting point. It is string for being able to define Units.
        ECOBJECTS_EXPORT Utf8StringCR             GetFromValue (void) const;

        //! Property that defines the range end point. It is string for being able to define Units.
        ECOBJECTS_EXPORT Utf8StringCR             GetToValue (void) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
