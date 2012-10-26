/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/GroupingRule.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct PropertyGroup;
struct PropertyRangeGroupSpecification;

/*__PUBLISH_SECTION_START__*/

typedef bvector<PropertyGroupP>                   PropertyGroupList;
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
        PropertyGroupList     m_groups;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP      _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool        _ReadXml (BeXmlNodeP xmlNode) override;
        ECOBJECTS_EXPORT virtual void        _WriteXml (BeXmlNodeP xmlNode) override;

    public:
        ECOBJECTS_EXPORT GroupingRule ()
            : PresentationRule (), m_schemaName (L""), m_className (L""), m_contextMenuCondition (L""), m_contextMenuLabel (L"")
            {
            }

        ECOBJECTS_EXPORT GroupingRule (WStringCR condition, int priority, bool onlyIfNotHandled, WStringCR schemaName, WStringCR className, WStringCR contextMenuCondition, WStringCR contextMenuLabel)
            : PresentationRule (condition, priority, onlyIfNotHandled), 
              m_schemaName (schemaName), m_className (className), m_contextMenuCondition (contextMenuCondition), m_contextMenuLabel (contextMenuLabel)
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

        //! Returns a list of PropertyGroups.
        ECOBJECTS_EXPORT PropertyGroupList&  GetGroups (void)                        { return m_groups;    }

    };

/*---------------------------------------------------------------------------------**//**
PropertyGroup that identifies parameters on how to group specific class ECInstances 
by a specific property.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyGroup
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString                 m_propertyName;
        WString                 m_imageId;
        WString                 m_contextMenuLabel;
        bool                    m_createGroupForSingleItem;
        PropertyRangeGroupList  m_ranges;

    public:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT PropertyGroup ()
            : m_propertyName (L""), m_imageId (L""), m_contextMenuLabel (L""), m_createGroupForSingleItem (false)
            {
            }

        ECOBJECTS_EXPORT PropertyGroup (WStringCR propertyName, WStringCR imageId, WStringCR contextMenuLabel, bool createGroupForSingleItem)
            : m_propertyName (propertyName), m_imageId (imageId), m_contextMenuLabel (contextMenuLabel), m_createGroupForSingleItem (createGroupForSingleItem)
            {
            }

        //! Destructor.
        ECOBJECTS_EXPORT                          ~PropertyGroup (void);

        //! Reads PresentationRule from xml node.
        ECOBJECTS_EXPORT bool                     ReadXml (BeXmlNodeP xmlNode);

        //! Writes group to xml node.
        ECOBJECTS_EXPORT void                     WriteXml (BeXmlNodeP parentXmlNode);

        //! ECProperty name to group ECInstances by.
        ECOBJECTS_EXPORT WStringCR                GetPropertyName (void) const                { return m_propertyName; }

        //! ImageId of the grouping node. Can be ECExpression. If not set ECProperty ImageId will be used.
        ECOBJECTS_EXPORT WStringCR                GetImageId (void) const                     { return m_imageId; }

        //! Idendifies whether a group should be created even if there is only single of particular group.
        ECOBJECTS_EXPORT WStringCR                GetContextMenuLabel (void) const            { return m_contextMenuLabel; }

        //! ContextMenu label of this particular grouping option. If not set ECProperty DisplayLabel will be used.
        ECOBJECTS_EXPORT bool                     GetCreateGroupForSingleItem (void) const    { return m_createGroupForSingleItem; }

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
        ECOBJECTS_EXPORT PropertyRangeGroupSpecification ()
            : m_label (L""), m_imageId (L""), m_fromValue (L""), m_toValue (L"")
            {
            }

        ECOBJECTS_EXPORT PropertyRangeGroupSpecification (WStringCR label, WStringCR imageId, WStringCR fromValue, WStringCR toValue)
            : m_label (label), m_imageId (imageId), m_fromValue (fromValue), m_toValue (toValue)
            {
            }

        //! Grouping range node label. If not set label will be formated using From and To values.
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