/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/GroupingRule.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct GroupSpecification;
struct ClassGroup;
struct PropertyGroup;
struct PropertyRangeGroupSpecification;

/*__PUBLISH_SECTION_START__*/

typedef bvector<GroupSpecificationP>              GroupList;
typedef bvector<PropertyRangeGroupSpecificationP> PropertyRangeGroupList;

/*---------------------------------------------------------------------------------**//**
Presentation rule for child nodes advanced grouping in the hierarchy.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupingRule : public PresentationRule
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString               m_schemaName;
        WString               m_className;
        WString               m_contextMenuCondition;
        WString               m_contextMenuLabel;
        WString               m_settingsId;
        GroupList             m_groups;

    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT GroupingRule ()
            : PresentationRule (), m_schemaName (L""), m_className (L""), m_contextMenuCondition (L""), m_contextMenuLabel (L""), m_settingsId (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT GroupingRule (WStringCR condition, int priority, bool onlyIfNotHandled, WStringCR schemaName, WStringCR className, WStringCR contextMenuCondition, WStringCR contextMenuLabel, WStringCR settingsId)
            : PresentationRule (condition, priority, onlyIfNotHandled), 
              m_schemaName (schemaName), m_className (className), m_contextMenuCondition (contextMenuCondition), m_contextMenuLabel (contextMenuLabel), m_settingsId (settingsId)
            {
            }

        //! Desctructor.
        ECOBJECTS_EXPORT                     ~GroupingRule (void);

        //! Acceptable schema name of ECInstances on which this grouping rule will be applied.
        ECOBJECTS_EXPORT WStringCR           GetSchemaName (void) const              { return m_schemaName; }

        //! Acceptable class name of ECInstances on which this grouping rule will be applied.
        ECOBJECTS_EXPORT WStringCR           GetClassName (void) const               { return m_className; }

        //! ECExpression condition that is used in order to define the node on which "Group By" context menu will be shown.
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECOBJECTS_EXPORT WStringCR           GetContextMenuCondition (void) const    { return m_contextMenuCondition; }

        //! Label of the parent context menu for choosing one of the predefined ProeprtyGroups.
        //! If this parameters is not set, the default name will be used - "Group By". 
        //! Menu will be shown only if there are more than 2 PropertyGroups defined in the rule.
        ECOBJECTS_EXPORT WStringCR           GetContextMenuLabel (void) const        { return m_contextMenuLabel; }

        //! Id that is used to store current active group. This is used only if there are more than one Group available.
        ECOBJECTS_EXPORT WStringCR           GetSettingsId (void) const              { return m_settingsId; }

        //! Returns a list of GroupSpecifications.
        ECOBJECTS_EXPORT GroupList&          GetGroups (void)                        { return m_groups; }

    };

/*---------------------------------------------------------------------------------**//**
GroupSpecification that identifies parameters on how to group ECInstances
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupSpecification
    {
//__PUBLISH_SECTION_END__
    private:
        WString  m_contextMenuLabel;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECOBJECTS_EXPORT GroupSpecification ()
        : m_contextMenuLabel (L"")
        {
        }

    //! Constructor.
    ECOBJECTS_EXPORT GroupSpecification (WStringCR contextMenuLabel)
        : m_contextMenuLabel (contextMenuLabel)
        {
        }

    //! Returns XmlElement name that is used to read/save this rule information.
    ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName () = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode) = 0;

    //! Writes rule information to given XmlNode.
    ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode) = 0;

public:
    //! Reads group specification from xml node.
    ECOBJECTS_EXPORT bool                     ReadXml (BeXmlNodeP xmlNode);

    //! Writes group specification to xml node.
    ECOBJECTS_EXPORT void                     WriteXml (BeXmlNodeP parentXmlNode);

    //! ContextMenu label of this particular grouping option. If not set ECClass or ECProperty DisplayLabel will be used.
    ECOBJECTS_EXPORT WStringCR                GetContextMenuLabel (void) const            { return m_contextMenuLabel; }
    };

/*---------------------------------------------------------------------------------**//**
This grouping option allows to create a Instance NavNode that represents mutiple instances
of the same label.
* @bsiclass                                     Eligijus.Mauragas               11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SameLabelInstanceGroup : public GroupSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SameLabelInstanceGroup ()
            : GroupSpecification ()
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT SameLabelInstanceGroup (WStringCR contextMenuLabel)
            : GroupSpecification (contextMenuLabel)
            {
            }

    };

/*---------------------------------------------------------------------------------**//**
ClassGroup that identifies parameters on how to group ECInstances.
* @bsiclass                                     Andrius.Zonys                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassGroup : public GroupSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool      m_createGroupForSingleItem;
        WString   m_schemaName;
        WString   m_baseClassName;

    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ClassGroup ()
            : GroupSpecification (), m_createGroupForSingleItem (false), m_schemaName (L""), m_baseClassName (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT ClassGroup (WStringCR contextMenuLabel, bool createGroupForSingleItem, WStringCR schemaName, WStringCR baseClassName)
            : GroupSpecification (contextMenuLabel), m_createGroupForSingleItem (createGroupForSingleItem), m_schemaName (schemaName), m_baseClassName (baseClassName)
            {
            }

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

        //! ECSchema name of base class.
        ECOBJECTS_EXPORT WStringCR                GetSchemaName (void) const                  { return m_schemaName; }

        //! Base ECClass name to group ECInstances by.
        ECOBJECTS_EXPORT WStringCR                GetBaseClassName (void) const               { return m_baseClassName; }

    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances 
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyGroup : public GroupSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString                 m_imageId;
        bool                    m_createGroupForSingleItem;
        WString                 m_propertyName;
        PropertyRangeGroupList  m_ranges;

    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP           _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool             _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void             _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT PropertyGroup ()
            : GroupSpecification (), m_imageId (L""), m_createGroupForSingleItem (false), m_propertyName (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT PropertyGroup (WStringCR contextMenuLabel, WStringCR imageId, bool createGroupForSingleItem, WStringCR propertyName)
            : GroupSpecification (contextMenuLabel), m_imageId (imageId), m_createGroupForSingleItem (createGroupForSingleItem), m_propertyName (propertyName)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                          ~PropertyGroup (void);

        //! ImageId of the grouping node. Can be ECExpression. If not set ECClass or ECProperty ImageId will be used.
        ECOBJECTS_EXPORT WStringCR                GetImageId (void) const                     { return m_imageId; }

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

        //! ECProperty name to group ECInstances by.
        ECOBJECTS_EXPORT WStringCR                GetPropertyName (void) const                { return m_propertyName; }

        //! List of grouping ranges. If grouping ranges are not specified ECInstances will be grouped by common value.
        ECOBJECTS_EXPORT PropertyRangeGroupList&  GetRanges (void)                            { return m_ranges;    }

    };


/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances 
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyRangeGroupSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString  m_label;
        WString  m_imageId;
        WString  m_fromValue;
        WString  m_toValue;

    public:
    /*__PUBLISH_SECTION_START__*/
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT PropertyRangeGroupSpecification ()
            : m_label (L""), m_imageId (L""), m_fromValue (L""), m_toValue (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT PropertyRangeGroupSpecification (WStringCR label, WStringCR imageId, WStringCR fromValue, WStringCR toValue)
            : m_label (label), m_imageId (imageId), m_fromValue (fromValue), m_toValue (toValue)
            {
            }

        //! Reads specification from xml.
        ECOBJECTS_EXPORT bool                     ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECOBJECTS_EXPORT void                     WriteXml (BeXmlNodeP parentXmlNode);

        //! ImageId of the grouping range node. If not set ECProperty ImageId will be used.
        ECOBJECTS_EXPORT WStringCR                GetLabel (void) const                       { return m_label; }

        //! ImageId of the grouping node. Can be ECExpression. If not set ECProperty ImageId will be used.
        ECOBJECTS_EXPORT WStringCR                GetImageId (void) const                     { return m_imageId; }

        //! Property that defines the range starting point. It is string for being able to define Units.
        ECOBJECTS_EXPORT WStringCR                GetFromValue (void) const                   { return m_fromValue; }

        //! Property that defines the range end point. It is string for being able to define Units.
        ECOBJECTS_EXPORT WStringCR                GetToValue (void) const                     { return m_toValue; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE