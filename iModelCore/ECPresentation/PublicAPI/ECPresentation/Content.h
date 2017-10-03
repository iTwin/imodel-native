/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Content.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/RulesDriven/Rules/RelatedPropertiesSpecification.h>
#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define CONTENTRECORD_MERGED_VALUE_FORMAT   "*** %s ***"

//=======================================================================================
//! A struct that describes current selection.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct SelectionInfo
{
private:
    Utf8String m_selectionProviderName;
    INavNodeKeysContainerCPtr m_keys;
    bool m_isSubSelection;
    bool m_isValid;

public:
    //! Constructor. Creates an invalid selection info object.
    SelectionInfo() : m_isValid(false), m_isSubSelection(false) {}

    //! Move constructor.
    SelectionInfo(SelectionInfo&& other)
        : m_isValid(other.m_isValid), m_selectionProviderName(std::move(other.m_selectionProviderName)), m_isSubSelection(other.m_isSubSelection), m_keys(other.m_keys)
        {}

    //! Copy constructor.
    SelectionInfo(SelectionInfo const& other)
        : m_isValid(other.m_isValid), m_selectionProviderName(other.m_selectionProviderName), m_isSubSelection(other.m_isSubSelection), m_keys(other.m_keys)
        {}

    //! Constructor.
    //! @param[in] providerName Name of the selection provider which last changed the selection.
    //! @param[in] isSubSelection Did the last selection change happen in sub-selection.
    //! @param[in] selectedNodeKeys The selection.
    SelectionInfo(Utf8String providerName, bool isSubSelection, INavNodeKeysContainerCR selectedNodeKeys)
        : m_isValid(true), m_selectionProviderName(providerName), m_isSubSelection(isSubSelection), m_keys(&selectedNodeKeys)
        {}
    
    //! Constructor. Initializes the instance from the provided selection event.
    //! @param[in] selectionProvider The provider used to get the current selection.
    //! @param[in] evt The last selection event.
    ECPRESENTATION_EXPORT SelectionInfo(ISelectionProvider const& selectionProvider, SelectionChangedEventCR evt);

    //! Constructor. Initializes the instance from the provided list of ECClasses.
    //! @param[in] classes List of ECClasses to create the selection info for.
    ECPRESENTATION_EXPORT SelectionInfo(bvector<ECN::ECClassCP> const& classes);

    //! Compare this selection event info object with the supplied one.
    ECPRESENTATION_EXPORT bool operator==(SelectionInfo const& other) const;
    
    //! Compare this selection event info object with the supplied one.
    ECPRESENTATION_EXPORT bool operator<(SelectionInfo const& other) const;
    
    //! Assignment operator.
    ECPRESENTATION_EXPORT SelectionInfo& operator=(SelectionInfo const& other);

    //! Move assignment operator.
    ECPRESENTATION_EXPORT SelectionInfo& operator=(SelectionInfo&& other);

    //! Is this struct valid.
    bool IsValid() const {return m_isValid;}

    //! Get the name of the selection source which caused the last selection change.
    Utf8StringCR GetSelectionProviderName() const {return m_selectionProviderName;}

    //! Did the last selection change happen in sub-selection.
    bool IsSubSelection() const {return m_isSubSelection;}

    //! Get the selection.
    INavNodeKeysContainerCR GetSelectedNodeKeys() const {return *m_keys;}
};

//=======================================================================================
//! Flags that control content format.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                07/2016
//=======================================================================================
enum class ContentFlags
    {
    KeysOnly =       1,      //!< Each content record has only ECInstanceKey and no data
    ShowImages =     1 << 1, //!< Each content record additionally has an image id
    ShowLabels =     1 << 2, //!< Each content record additionally has a label
    MergeResults =   1 << 3, //!< All content records are merged into a single record
    DistinctValues = 1 << 4, //!< Content has only distinct values
    };

//=======================================================================================
//! Displayed content types. Affects how the content is formatted, e.g. 
//! the ContentFlags
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                05/2016
//=======================================================================================
struct ContentDisplayType
    {
    //! Unknown content type.
    ECPRESENTATION_EXPORT static const Utf8CP Undefined;

    //! Grid or table view content type. By default adds ContentFlags::ShowLabels flag.
    ECPRESENTATION_EXPORT static const Utf8CP Grid;

    //! Property pane content type. By default adds ContentFlags::MergeResults flag.
    ECPRESENTATION_EXPORT static const Utf8CP PropertyPane;

    //! Content type for graphic content controls, e.g. the viewport. 
    //! By default adds ContentFlags::KeysOnly flag.
    ECPRESENTATION_EXPORT static const Utf8CP Graphics;
    };

//=======================================================================================
//! Data structure that describes an ECClass in ContentDescriptor. In addition to the class
//! itself the structure holds its relationship path to the primary ECClass and paths
//! to related property classes. Dependencies are related as follows:
//! 
//!                                  /---- Related Properties Path 1
//! Primary Class ----- Select Class
//!                                  \---- Related Properties Path 2
//!
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                05/2016
//=======================================================================================
struct SelectClassInfo
{
private:
    ECN::ECClassCP m_selectClass;
    bool m_isPolymorphic;
    bvector<RelatedClassPath> m_relatedPropertyPaths;
    RelatedClassPath m_pathToPrimaryClass;

public:
    //! Constructor. Creates an invalid object.
    SelectClassInfo() : m_selectClass(nullptr) {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClassInfo(ECN::ECClassCR selectClass, bool isPolymorphic) : m_selectClass(&selectClass), m_isPolymorphic(isPolymorphic) {}

    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClassInfo const& other) const
        {
        return m_selectClass == other.m_selectClass
            && m_isPolymorphic == other.m_isPolymorphic
            && m_pathToPrimaryClass == other.m_pathToPrimaryClass
            && m_relatedPropertyPaths == other.m_relatedPropertyPaths;
        }
    //! Equals operator override.
    bool operator==(SelectClassInfo const& other) const {return Equals(other);}
    //! NotEquals operator override.
    bool operator!=(SelectClassInfo const& other) const {return !Equals(other);}

    //! Get the select ECClass.
    ECN::ECClassCR GetSelectClass() const {return *m_selectClass;}

    //! Is the select polymorphic.
    bool IsSelectPolymorphic() const {return m_isPolymorphic;}

    //! Get the primary ECClass. 
    ECN::ECClassCP GetPrimaryClass() const {return m_pathToPrimaryClass.empty() ? nullptr : m_pathToPrimaryClass.back().GetTargetClass();}

    //! Get paths to related property ECClasses.
    bvector<RelatedClassPath> const& GetRelatedPropertyPaths() const {return m_relatedPropertyPaths;}
    //! Set paths to related property ECClasses.
    void SetRelatedPropertyPaths(bvector<RelatedClassPath> propertyPaths) {m_relatedPropertyPaths = propertyPaths;}
    
    //! Get path to the primary ECClass. 
    RelatedClassPath const& GetPathToPrimaryClass() const {return m_pathToPrimaryClass;}
    //! Set path to the primary ECClass.
    void SetPathToPrimaryClass(RelatedClassPath path) {m_pathToPrimaryClass = path;}
};

//=======================================================================================
//! Describes content field editor.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct ContentFieldEditor
{
    struct Params
    {
    protected:
        virtual Utf8CP _GetName() const = 0;
        virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType*) const = 0;
        virtual Params* _Clone() const = 0;
        virtual bool _Equals(Params const& other) const {return 0 == strcmp(GetName(), other.GetName());}
    public:
        virtual ~Params() {}
        Utf8CP GetName() const {return _GetName();}
        rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}
        Params* Clone() const {return _Clone();}
        bool Equals(Params const& other) const {return _Equals(other);}
        bool operator==(Params const& other) const {return Equals(other);}
    };

private:
    Utf8String m_name;
    bvector<Params const*> m_params;

public:
    ContentFieldEditor(Utf8String name) : m_name(name) {}
    ContentFieldEditor(ContentFieldEditor const& other) 
        : m_name(other.m_name)
        {
        for (Params const* otherParams : other.m_params)
            m_params.push_back(otherParams->Clone());
        }
    ContentFieldEditor(ContentFieldEditor&& other) 
        : m_name(std::move(other.m_name)), m_params(other.m_params)
        {
        other.m_params.clear();
        }
    ECPRESENTATION_EXPORT ~ContentFieldEditor();
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT bool Equals(ContentFieldEditor const&) const;
    bvector<Params const*> const& GetParams() const {return m_params;}
    bvector<Params const*>& GetParams() {return m_params;}
    Utf8StringCR GetName() const {return m_name;}
};

//=======================================================================================
//! Describes the content: fields, sorting, filtering, format. Users may change 
//! @ref ContentDescriptor to control what content they get and how they get it.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ContentDescriptor : RefCountedBase
{
    //===================================================================================
    //! A struct that describes a @ref Field category.
    // @bsiclass                                    Grigas.Petraitis            09/2016
    //===================================================================================
    struct Category
    {
    private:
        Utf8String m_name;
        Utf8String m_label;
        Utf8String m_description;
        bool m_shouldExpand;
        int m_priority;

    public:
        //! Constructor. Creates an invalid category.
        Category() : m_shouldExpand(false), m_priority(0) {}
        
        //! Copy constructor. 
        Category(Category const& other) : m_name(other.m_name), m_label(other.m_label), m_priority(other.m_priority), m_description(other.m_description), m_shouldExpand(other.m_shouldExpand) {}

        //! Move constructor. 
        Category(Category&& other) : m_name(std::move(other.m_name)), m_label(std::move(other.m_label)), m_priority(other.m_priority), m_description(other.m_description), m_shouldExpand(other.m_shouldExpand) {}

        //! Constructor. 
        //! @param[in] name Name of the category.
        //! @param[in] label Label of the category.
        //! @param[in] description Description of the category.
        //! @param[in] priority Priority of the category.
        //! @param[in] shouldExpand Should this category be auto-expanded.
        Category(Utf8String name, Utf8String label, Utf8String description, int priority, bool shouldExpand = false) : m_name(name), m_label(label), m_priority(priority), m_description(description), m_shouldExpand(shouldExpand) {}

        //! Copy-assignment operator.
        Category& operator=(Category const& other) {m_name = other.m_name; m_label = other.m_label; m_priority = other.m_priority; m_description = other.m_description;  m_shouldExpand = other.m_shouldExpand; return *this;}

        //! Is this category equal to the supplied one.
        bool operator==(Category const& other) const {return m_name == other.m_name && m_label == other.m_label && m_priority == other.m_priority && m_description == other.m_description && m_shouldExpand == other.m_shouldExpand;}

        //! Serialize this category to JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;

        //! Get the name of the category.
        Utf8StringCR GetName() const {return m_name;}
        //! Set the name of the category.
        void SetName(Utf8String value) {m_name = value;}

        //! Get the label of the category.
        Utf8StringCR GetLabel() const {return m_label;}
        //! Set the label of the category.
        void SetLabel(Utf8String value) {m_label = value;}

        //! Get the priority of the category.
        int GetPriority() const {return m_priority;}
        //! Set the priority of the category.
        void SetPriority(int value) {m_priority = value;}

        //! Get the description of the category.
        Utf8StringCR GetDescription() const {return m_description;}
        //! Set the description of the category.
        void SetDescription(Utf8String value) {m_description = value;}

        //! Should this category be automatically expanded.
        bool ShouldExpand() const {return m_shouldExpand;}
        //! Set whether this category should be automatically expanded.
        void SetShouldExpand(bool value) {m_shouldExpand = value;}

    public:
        ECPRESENTATION_EXPORT static Category GetDefaultCategory();
        ECPRESENTATION_EXPORT static Category GetFavoriteCategory();
    };
    
    //===================================================================================
    //! Describes a single ECProperty that's included in a @ref Field.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct Property
    {
        static int const DEFAULT_PRIORITY = 0;
    private:
        Utf8String m_prefix;
        ECN::ECClassCP m_propertyClass;
        ECN::ECPropertyCP m_property;
        RelatedClassPath m_relatedClassPath;

    private:
        ECPRESENTATION_EXPORT static ECN::PrimitiveECPropertyCP GetPrimitiveProperty(ECN::StructECPropertyCR, Utf8StringCR accessString);

    public:
        //! Constructor. Creates a property for a primitive ECProperty.
        //! @param[in] prefix Class alias that's used to query this property.
        //! @param[in] propertyClass The exact class of this property.
        //! @param[in] ecProperty The ECProperty that's wrapped by this struct.
        Property(Utf8String prefix, ECN::ECClassCR propertyClass, ECN::ECPropertyCR ecProperty) 
            : m_prefix(prefix), m_propertyClass(&propertyClass), m_property(&ecProperty)
            {}

        //! Is this struct equal to the supplied one.
        ECPRESENTATION_EXPORT bool operator==(Property const& other) const;

        //! Serialize this struct to JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;

        //! Get the class alias used to query this property.
        Utf8CP GetPrefix() const {return m_prefix.c_str();}
        //! Set the class alias used to query this property.
        void SetPrefix(Utf8String prefix) {m_prefix = prefix;}

        // Get the exact class that the root wrapped property belongs to.
        ECN::ECClassCR GetPropertyClass() const {return *m_propertyClass;}

        //! Get the wrapped property.
        ECN::ECPropertyCR GetProperty() const {return *m_property;}

        //! Is this a related property.
        bool IsRelated() const {return !m_relatedClassPath.empty();}
        //! Make this a related property.
        //! @param[in] path The relationship path that describes the relationship between this property and main properties.
        void SetIsRelated(RelatedClassPath path) {m_relatedClassPath = path;}
        //! Make this a related property.
        //! @param[in] relatedClass The related class object that describes the relationship between this property and main properties.
        void SetIsRelated(RelatedClass relatedClass) {m_relatedClassPath.clear(); m_relatedClassPath.push_back(relatedClass);}
        //! Get the relationship path that describes the relationship between this property and main properties.
        RelatedClassPathCR GetRelatedClassPath() const {return m_relatedClassPath;}
        //! Get the priority of this property.
        int GetPriority() const {return m_property->GetPriority();}
    };
    
    struct DisplayLabelField;
    struct ECPropertiesField;
    struct CalculatedPropertyField;
    struct NestedContentField;
    struct SystemField;
    //===================================================================================
    //! Describes a single content field. A field is usually represented as a grid column
    //! or a property pane row.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct Field
    {
        struct EXPORT_VTABLE_ATTRIBUTE TypeDescription : RefCountedBase
        {
        private:
            Utf8String m_typeName;
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType*) const;
        public:
            TypeDescription(Utf8String typeName) : m_typeName(typeName) {}
            virtual ~TypeDescription() {}
            Utf8StringCR GetTypeName() const {return m_typeName;}
            rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}
        };
        typedef RefCountedPtr<TypeDescription> TypeDescriptionPtr;

    private:
        Category m_category;
        Utf8String m_name;
        Utf8String m_label;
        ContentFieldEditor const* m_editor;
        mutable TypeDescriptionPtr m_type;
        
    protected:
        Field() : m_editor(nullptr) {}
        virtual DisplayLabelField* _AsDisplayLabelField() {return nullptr;}
        virtual DisplayLabelField const* _AsDisplayLabelField() const {return nullptr;}
        virtual CalculatedPropertyField* _AsCalculatedPropertyField() {return nullptr;}
        virtual CalculatedPropertyField const* _AsCalculatedPropertyField() const {return nullptr;}
        virtual ECPropertiesField* _AsPropertiesField() {return nullptr;}
        virtual ECPropertiesField const* _AsPropertiesField() const {return nullptr;}
        virtual NestedContentField* _AsNestedContentField() {return nullptr;}
        virtual NestedContentField const* _AsNestedContentField() const {return nullptr;}
        virtual SystemField* _AsSystemField() {return nullptr;}
        virtual SystemField const* _AsSystemField() const {return nullptr;}
        virtual Field* _Clone() const = 0;
        virtual TypeDescriptionPtr _CreateTypeDescription() const = 0;
        virtual bool _IsReadOnly() const = 0;
        virtual bool _IsVisible() const {return true;}
        virtual bool _Equals(Field const& other) const {return m_category == other.m_category && m_name == other.m_name && m_label == other.m_label;}
        ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const;
        virtual int _GetPriority() const = 0;
        virtual void _OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) {}
        virtual bool _OnFieldRemoved(Field const&) {return false;}

    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] name The per-descriptor unique name of this field.
        //! @param[in] label The label of this field.
        //! @param[in] editor The custom editor for this field.
        Field(Category category, Utf8String name, Utf8String label, ContentFieldEditor const* editor = nullptr) 
            : m_category(category), m_name(name), m_label(label), m_editor(editor) 
            {}

        //! Copy constructor.
        Field(Field const& other) 
            : m_category(other.m_category), m_name(other.m_name), m_label(other.m_label), m_editor(other.m_editor)
            {
            if (nullptr != other.m_editor)
                m_editor = new ContentFieldEditor(*other.m_editor);
            }

        //! Move constructor.
        Field(Field&& other) 
            : m_category(std::move(other.m_category)), m_name(std::move(other.m_name)), m_label(std::move(other.m_label)), 
            m_editor(std::move(other.m_editor))
            {}

        //! Virtual destructor.
        virtual ~Field() {}

        //! Clone this field.
        Field* Clone() const {return _Clone();}

        //! Serialize this field to JSON.
        rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const {return _AsJson(allocator);}

        //! Is this field equal to the supplied one.
        bool operator==(Field const& other) const {return _Equals(other);}
        //! Is this field not equal to the supplied one.
        bool operator!=(Field const& other) const {return !_Equals(other);}

        //! Is this a display label field.
        bool IsDisplayLabelField() const {return nullptr != _AsDisplayLabelField();}
        //! Get this field as @ref DisplayLabelField
        DisplayLabelField* AsDisplayLabelField() {return _AsDisplayLabelField();}
        //! Get this field as @ref DisplayLabelField
        DisplayLabelField const* AsDisplayLabelField() const {return _AsDisplayLabelField();}

        //! Is this an ECProperty field.
        bool IsPropertiesField() const {return nullptr != _AsPropertiesField();}
        //! Get this field as an @ref ECPropertiesField
        ECPropertiesField* AsPropertiesField() {return _AsPropertiesField();}
        //! Get this field as an @ref ECPropertiesField
        ECPropertiesField const* AsPropertiesField() const {return _AsPropertiesField();}

        //! Is this a calculated field.
        bool IsCalculatedPropertyField() const {return nullptr != _AsCalculatedPropertyField();}
        //! Get this field as a @ref CalculatedPropertyField
        CalculatedPropertyField* AsCalculatedPropertyField() {return _AsCalculatedPropertyField();}
        //! Get this field as a @ref CalculatedPropertyField
        CalculatedPropertyField const* AsCalculatedPropertyField() const {return _AsCalculatedPropertyField();}
        
        //! Is this a nested content field.
        bool IsNestedContentField() const {return nullptr != _AsNestedContentField();}
        //! Get this field as a @ref NestedContentField
        NestedContentField* AsNestedContentField() {return _AsNestedContentField();}
        //! Get this field as a @ref NestedContentField
        NestedContentField const* AsNestedContentField() const {return _AsNestedContentField();}

        //! Is this a system field.
        bool IsSystemField() const {return nullptr != _AsSystemField();}
        //! Get this field as a @ref SystemField
        SystemField* AsSystemField() {return _AsSystemField();}
        //! Get this field as a @ref SystemField
        SystemField const* AsSystemField() const {return _AsSystemField();}

        //! Get the category of this field.
        Category const& GetCategory() const {return m_category;}
        //! Set the category for this field.
        void SetCategory(Category category) {m_category = category;}

        //! Get the name of this field.
        Utf8StringCR GetName() const {return m_name;}
        //! Set the name for this field.
        void SetName(Utf8String name) {m_name = name;}

        //! Get the label of this field.
        Utf8StringCR GetLabel() const {return m_label;}
        //! Set the label for this field.
        void SetLabel(Utf8String label) {m_label = label;}

        //! Get the editor of this field.
        ContentFieldEditor const* GetEditor() const {return m_editor;}
        //! Set the editor for this field.
        //! @note The field takes ownership of the editor object.
        void SetEditor(ContentFieldEditor const* editor) {m_editor = editor;}

        //! Get field's priority.
        int GetPriority() const {return _GetPriority();}

        //! Get field's type information.
        ECPRESENTATION_EXPORT TypeDescription const& GetTypeDescription() const;
        
        //! Call this after related sibling fields were remapped (e.g. after copying)
        void NotifyFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) {_OnFieldsCloned(fieldsRemapInfo);}

        //! Call this after a related sibling field is removed from descriptor.
        //! @return True if this field should also be removed.
        bool NotifyFieldRemoved(Field const& field) {return _OnFieldRemoved(field);}
    };
    
    //===================================================================================
    //! Describes a single content display label field. This field is usually included in
    //! content for the @ref ContentDisplayType::Grid display type.
    // @bsiclass                                    Grigas.Petraitis            10/2016
    //===================================================================================
    struct DisplayLabelField : Field
    {
    private:
        int m_priority;

    protected:
        DisplayLabelField* _AsDisplayLabelField() override {return this;}
        DisplayLabelField const* _AsDisplayLabelField() const override {return this;}
        Field* _Clone() const override {return new DisplayLabelField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        int _GetPriority() const override {return m_priority;}
        bool _IsReadOnly() const override {return true;}

    public:
        //! Constructor.
        //! @param[in] label The label of this field.
        //! @param[in] priority Field priority.
        DisplayLabelField(Utf8String label, int priority = Property::DEFAULT_PRIORITY) 
            : Field(Category::GetDefaultCategory(), "DisplayLabel", label), m_priority(priority)
            {}

        //! Set the priority for this field.
        void SetPriority(int priority) {m_priority = priority;}
    };

    //===================================================================================
    //! Describes a single content calculated property field. Calculated fields use
    //! ECExpressions to determine the value.
    // @bsiclass                                     Tautvydas.Zinys            10/2016
    //===================================================================================
    struct CalculatedPropertyField : Field
    {
    private:
        int m_priority;
        Utf8String m_valueExpression;
        ECN::ECClassCP m_class;

    protected:
        CalculatedPropertyField* _AsCalculatedPropertyField() override {return this;}
        CalculatedPropertyField const* _AsCalculatedPropertyField() const override {return this;}
        Field* _Clone() const override {return new CalculatedPropertyField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        int _GetPriority() const override {return m_priority;}
        bool _IsReadOnly() const override {return true;}

    public:
        //! Constructor.
        //! @param[in] label Field label. Supports localization.
        //! @param[in] name Unique field name. 
        //! @param[in] valueExpression Value ECExpression. Supports localization.
        //! @param[in] ecClass Entity class this field is intended for.
        //! @param[in] priority Field priority.
        CalculatedPropertyField(Utf8String label, Utf8String name, Utf8String valueExpression, ECN::ECClassCP ecClass, int priority = Property::DEFAULT_PRIORITY)
            : Field(Category::GetDefaultCategory(), name, label), m_valueExpression(valueExpression), m_class(ecClass), m_priority(priority)
            {}

        //! Get the ECExpression used to calculate field's value.
        Utf8String const& GetValueExpression() const {return m_valueExpression;}

        //! Get the class this field is intended for.
        ECN::ECClassCP GetClass() const {return m_class;}
        
        //! Set the priority for this field.
        void SetPriority(int priority) {m_priority = priority;}
    };
    
    //===================================================================================
    //! Describes a single content field which is based on ECProperties. The field should
    //! be based on one or more properties of the similar type.
    // @bsiclass                                    Grigas.Petraitis            10/2016
    //===================================================================================
    struct ECPropertiesField : Field
    {
    private:
        bvector<Property> m_properties;

    private:
        ECPRESENTATION_EXPORT void InitFromProperty(ECN::ECClassCR, Property const&, IPropertyCategorySupplierP);

    protected:
        ECPropertiesField* _AsPropertiesField() override {return this;}
        ECPropertiesField const* _AsPropertiesField() const override {return this;}
        Field* _Clone() const override {return new ECPropertiesField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT bool _IsReadOnly() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT int _GetPriority() const override;
    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] name The per-descriptor unique name of this field.
        //! @param[in] label The label of this field.
        //! @param[in] editor The custom editor for this field.
        ECPropertiesField(Category category, Utf8String name, Utf8String label, ContentFieldEditor const* editor = nullptr) 
            : Field(category, name, label, editor) 
            {}

        //! Constructor. Creates a field with a single @ref Property.
        //! @param[in] primaryClass ECClass of primary (root) instance. Matches @e prop.GetPropertyClass() if the property is not of related instance.
        //! @param[in] prop Property that this field is based on.
        //! @param[in] categorySupplier The category supplier used to determine the category of this field.
        ECPropertiesField(ECN::ECClassCR primaryClass, Property const& prop, IPropertyCategorySupplierR categorySupplier)
            {
            InitFromProperty(primaryClass, prop, &categorySupplier);
            }
        
        //! Constructor. Creates a field with a single @ref Property and invalid Category.
        //! @param[in] primaryClass ECClass of primary (root) instance. Matches @e prop.GetPropertyClass() if the property is not of related instance.
        //! @param[in] prop Property that this field is based on.
        ECPropertiesField(ECN::ECClassCR primaryClass, Property const& prop) {InitFromProperty(primaryClass, prop, nullptr);}

        //! Copy constructor.
        ECPropertiesField(ECPropertiesField const& other) : Field(other), m_properties(other.m_properties) {}

        //! Move constructor.
        ECPropertiesField(ECPropertiesField&& other) : Field(std::move(other)), m_properties(std::move(other.m_properties)) {}

        //! Is this field equal to the supplied one.
        bool operator==(ECPropertiesField const& other) const {return Field::operator==(other) && m_properties == other.m_properties;}

        //! Does this field contain composite properties (structs or arrays)
        ECPRESENTATION_EXPORT bool IsCompositePropertiesField() const;

        //! Get the properties that this field is based on.
        bvector<Property>& GetProperties() {return m_properties;}
        //! Get the properties that this field is based on.
        bvector<Property> const& GetProperties() const {return m_properties;}
        //! Find properties that match the supplied class. If nullptr is supplied, 
        //! all properties are returned.
        bvector<Property const*> FindMatchingProperties(ECN::ECClassCP) const;
    };
    
    //===================================================================================
    //! Describes a single content field which contains related instances' content.
    // @bsiclass                                    Grigas.Petraitis            07/2017
    //===================================================================================
    struct NestedContentField : Field
    {
    private:
        ECN::ECClassCR m_contentClass;
        Utf8String m_contentClassAlias;
        RelatedClassPath m_relationshipPath;
        bvector<Field*> m_fields;
        int m_priority;

    protected:
        NestedContentField* _AsNestedContentField() override {return this;}
        NestedContentField const* _AsNestedContentField() const override {return this;}
        Field* _Clone() const override {return new NestedContentField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        bool _IsReadOnly() const override {return true;}
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        int _GetPriority() const override {return m_priority;}
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::MemoryPoolAllocator<>*) const override;
    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] name The per-descriptor unique name of this field.
        //! @param[in] label The label of this field.
        //! @param[in] contentClass ECClass whose content is returned by this field
        //! @param[in] contentClassAlias Alias of the content class.
        //! @param[in] relationshipPath Path from the @e contentClass to the primary instance class.
        //! @param[in] fields A list of fields which this field consists from.
        //! @param[in] priority Priority of the field
        NestedContentField(Category category, Utf8String name, Utf8String label, 
            ECN::ECClassCR contentClass, Utf8String contentClassAlias, RelatedClassPath relationshipPath,
            bvector<Field*> fields = bvector<Field*>(), int priority = Property::DEFAULT_PRIORITY) 
            : Field(category, name, label), m_contentClass(contentClass), m_contentClassAlias(contentClassAlias), 
            m_relationshipPath(relationshipPath), m_fields(fields), m_priority(priority)
            {}
        
        //! Copy constructor.
        NestedContentField(NestedContentField const& other) 
            : Field(other), m_contentClass(other.m_contentClass), m_contentClassAlias(other.m_contentClassAlias), 
            m_relationshipPath(other.m_relationshipPath), m_priority(other.m_priority)
            {
            for (Field const* field : other.m_fields)
                m_fields.push_back(field->Clone());
            }

        //! Move constructor.
        NestedContentField(NestedContentField&& other) 
            : Field(std::move(other)), m_contentClass(other.m_contentClass), m_contentClassAlias(std::move(other.m_contentClassAlias)), 
            m_relationshipPath(std::move(other.m_relationshipPath)), m_fields(std::move(other.m_fields)), m_priority(other.m_priority)
            {}

        //! Destructor
        ~NestedContentField() {for (Field const* field : m_fields) {DELETE_AND_CLEAR(field);}}

        //! Get the content class whose content is returned by this field.
        ECN::ECClassCR GetContentClass() const {return m_contentClass;}

        //! Get alias of the content class
        Utf8StringCR GetContentClassAlias() const {return m_contentClassAlias;}
        
        //! Path from the @e "content class" to the primary instance class.
        RelatedClassPath const& GetRelationshipPath() const {return m_relationshipPath;}

        //! A list of fields which this field consists from.
        bvector<Field*> const& GetFields() const {return m_fields;}
        bvector<Field*>& GetFields() {return m_fields;}
        };

    struct ECInstanceKeyField;
    struct ECNavigationInstanceIdField;
    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct SystemField : Field
    {
    protected:
        SystemField* _AsSystemField() override {return this;}
        SystemField const* _AsSystemField() const override {return this;}
        int _GetPriority() const override {return Property::DEFAULT_PRIORITY;}
        bool _IsReadOnly() const override {return true;}
        bool _IsVisible() const override {return false;}
    protected:
        SystemField(Utf8StringCR fieldName) : Field(Category(), fieldName, "") {}
        virtual ECInstanceKeyField* _AsECInstanceKeyField() {return nullptr;}
        virtual ECInstanceKeyField const* _AsECInstanceKeyField() const {return nullptr;}
        virtual ECNavigationInstanceIdField* _AsECNavigationInstanceIdField() {return nullptr;}
        virtual ECNavigationInstanceIdField const* _AsECNavigationInstanceIdField() const {return nullptr;}
    public:
        bool IsECInstanceKeyField() const {return nullptr != _AsECInstanceKeyField();}
        ECInstanceKeyField* AsECInstanceKeyField() {return _AsECInstanceKeyField();}
        ECInstanceKeyField const* AsECInstanceKeyField() const {return _AsECInstanceKeyField();}
        bool IsECNavigationInstanceIdField() const {return nullptr != _AsECNavigationInstanceIdField();}
        ECNavigationInstanceIdField* AsECNavigationInstanceIdField() {return _AsECNavigationInstanceIdField();}
        ECNavigationInstanceIdField const* AsECNavigationInstanceIdField() const {return _AsECNavigationInstanceIdField();}
    };
    
    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct ECInstanceKeyField : SystemField
    {
    private:
        bvector<ContentDescriptor::ECPropertiesField const*> m_keyFields;
    protected:
        ECInstanceKeyField* _AsECInstanceKeyField() override {return this;}
        ECInstanceKeyField const* _AsECInstanceKeyField() const override {return this;}
        Field* _Clone() const override {return new ECInstanceKeyField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT void _OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) override;
        ECPRESENTATION_EXPORT bool _OnFieldRemoved(Field const&) override;
    public:
        ECInstanceKeyField() : SystemField("Invalid") {}
        ECPRESENTATION_EXPORT void RecalculateName();
        bvector<ContentDescriptor::ECPropertiesField const*> const& GetKeyFields() const {return m_keyFields;}
        void AddKeyField(ContentDescriptor::ECPropertiesField const& field) {m_keyFields.push_back(&field); RecalculateName();}
    };

    //===================================================================================
    // @bsiclass                                    Saulius.Skliutas            08/2017
    //===================================================================================
    struct ECNavigationInstanceIdField : SystemField
    {
    private:
        ECPropertiesField const* m_propertyField;
    protected:
        ECNavigationInstanceIdField* _AsECNavigationInstanceIdField() override {return this;}
        ECNavigationInstanceIdField const* _AsECNavigationInstanceIdField() const override {return this;}
        Field* _Clone() const override {return new ECNavigationInstanceIdField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT void _OnFieldsCloned(bmap<Field const*, Field const*> const&) override;
        ECPRESENTATION_EXPORT bool _OnFieldRemoved(Field const&) override;
    public:
        ECNavigationInstanceIdField(ECPropertiesField const& propertiesField)
            : SystemField(""), m_propertyField(&propertiesField)
            {}
        ECNavigationInstanceIdField(ECNavigationInstanceIdField const& other)
            : SystemField(other), m_propertyField(other.m_propertyField)
            {}
        ECPropertiesField const& GetPropertiesField() const {return *m_propertyField;}       
    };

private:
    Utf8String m_preferredDisplayType;
    bvector<SelectClassInfo> m_classes;
    bvector<Field*> m_fields;
    int m_sortingFieldIndex;
    SortDirection m_sortDirection;
    int m_contentFlags;
    Utf8String m_filterExpression;

private:
    ContentDescriptor(Utf8String preferredDisplayType) 
        : m_preferredDisplayType(preferredDisplayType), m_contentFlags(0), m_sortingFieldIndex(-1), m_sortDirection(SortDirection::Ascending)
        {}
    ECPRESENTATION_EXPORT ContentDescriptor(ContentDescriptorCR other);
    ECPRESENTATION_EXPORT int GetFieldIndex(Utf8CP name) const;
    void OnFlagAdded(ContentFlags flag);
    void OnFlagRemoved(ContentFlags flag);

protected:
    ECPRESENTATION_EXPORT ~ContentDescriptor();

public:
    //! Creates a content descriptor.
    //! @param[in] preferredDisplayType The display type to create the descriptor for.
    static ContentDescriptorPtr Create(Utf8CP preferredDisplayType = ContentDisplayType::Undefined) {return new ContentDescriptor(preferredDisplayType);}

    //! Copies the supplied content descriptor.
    //! @param[in] other The descriptor to copy.
    static ContentDescriptorPtr Create(ContentDescriptorCR other) {return new ContentDescriptor(other);}

    //! Serializes this descriptor to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;

    //! Is this desciptor equal to the supplied one.
    ECPRESENTATION_EXPORT bool Equals(ContentDescriptorCR other) const;
    //! Is this desciptor equal to the supplied one.
    bool operator==(ContentDescriptorCR other) const {return Equals(other);}
    //! Is this desciptor not equal to the supplied one.
    bool operator!=(ContentDescriptorCR other) const {return !Equals(other);}

    //! Merge this descriptor with the supplied one.
    ECPRESENTATION_EXPORT void MergeWith(ContentDescriptorCR other);

    //! Get the preferred display type which this descriptor is created for.
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    
    //! Get information about ECClasses which the descriptor consists from.
    bvector<SelectClassInfo> const& GetSelectClasses() const {return m_classes;}
    //! Get information about ECClasses which the descriptor consists from.
    bvector<SelectClassInfo>& GetSelectClasses() {return m_classes;}

    //! Get the fields in this descriptor (excluding system ones).
    ECPRESENTATION_EXPORT bvector<Field*> GetVisibleFields() const;
    //! Get the fields in this descriptor (including system ones).
    bvector<Field*> const& GetAllFields() const {return m_fields;}
    //! Get the fields in this descriptor.
    bvector<Field*>& GetAllFields() {return m_fields;}

    //! Remove a field from this descriptor.
    ECPRESENTATION_EXPORT void RemoveField(Field const& field);
    //! Remove a field from this descriptor by index.
    void RemoveField(size_t index) {RemoveField(*m_fields[index]);}
    //! Remove a field from this descriptor by name.
    ECPRESENTATION_EXPORT void RemoveField(Utf8CP name);

    //! Get the sorting field used to sort content.
    Field const* GetSortingField() const {return (m_sortingFieldIndex < 0 || m_sortingFieldIndex >= (int)m_fields.size()) ? nullptr : m_fields[m_sortingFieldIndex];}
    //! Set the sorting field by index.
    void SetSortingField(int index) {m_sortingFieldIndex = index;}
    //! Set the sorting field by name.
    void SetSortingField(Utf8CP name) {m_sortingFieldIndex = GetFieldIndex(name);}

    //! Set sorting direction.
    void SetSortDirection(SortDirection direction) {m_sortDirection = direction;}
    //! Get sorting direction.
    SortDirection GetSortDirection() const {return m_sortDirection;}

    //! Set filtering ECExpression.
    void SetFilterExpression(Utf8String expression) {m_filterExpression = expression;}
    //! Get filtering ECExpression.
    Utf8StringCR GetFilterExpression() const {return m_filterExpression;}
    
    //! Get the content flags.
    //! @see ContentFlags
    int GetContentFlags() const {return m_contentFlags;}
    //! Set the content flags.
    //! @see ContentFlags
    ECPRESENTATION_EXPORT void SetContentFlags(int flags);
    //! Add a content flag.
    ECPRESENTATION_EXPORT void AddContentFlag(ContentFlags flag);
    //! Remove a content flag.
    ECPRESENTATION_EXPORT void RemoveContentFlag(ContentFlags flag);
    //! Does this descriptor have the supplied content flag.
    ECPRESENTATION_EXPORT bool HasContentFlag(ContentFlags flag) const;
    //! Should content include images.
    bool ShowImages() const {return HasContentFlag(ContentFlags::ShowImages);}
    //! Should content include display labels.
    bool ShowLabels() const {return HasContentFlag(ContentFlags::ShowLabels);}
    //! Should the content be merged into a single record.
    bool MergeResults() const {return HasContentFlag(ContentFlags::MergeResults);}
    //! Should only distinct values be returned
    bool OnlyDistinctValues() const {return HasContentFlag(ContentFlags::DistinctValues);}
};

//=======================================================================================
//! A struct that represents a single content record.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct ContentSetItem : RefCountedBase, RapidJsonExtendedDataHolder<>
{
    //===================================================================================
    //! Describes ContentSetItem parts which may be serialized.
    // @bsiclass                                    Grigas.Petraitis            07/2017
    //===================================================================================
    enum SerializationFlags
        {
        SERIALIZE_DisplayLabel  = 1 << 0,
        SERIALIZE_ImageId       = 1 << 1,
        SERIALIZE_Values        = 1 << 2,
        SERIALIZE_DisplayValues = 1 << 3,
        SERIALIZE_ClassInfo     = 1 << 4,
        SERIALIZE_PrimaryKeys   = 1 << 5,
        SERIALIZE_MergedFieldNames = 1 << 6,
        SERIALIZE_FieldPropertyInstanceKeys = 1 << 7,
        SERIALIZE_All           = 0xFF // 8 bits
        };
    
    //===================================================================================
    //! Data structure containing a pair of ECPropertiesField and an index of a property
    //! in that field.
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct FieldProperty
    {
    private:
        ContentDescriptor::ECPropertiesField const* m_field;
        size_t m_propertyIndex;
    public:
        FieldProperty() : m_field(nullptr), m_propertyIndex(0) {}
        FieldProperty(ContentDescriptor::ECPropertiesField const& field, size_t propertyIndex) 
            : m_field(&field), m_propertyIndex(propertyIndex)
            {}
        bool operator<(FieldProperty const& other) const
            {
            int fieldNameCmp = m_field->GetName().CompareTo(other.m_field->GetName());
            return fieldNameCmp < 0 || fieldNameCmp == 0 && m_propertyIndex < other.m_propertyIndex;
            }
        ContentDescriptor::ECPropertiesField const& GetField() const {return *m_field;}
        ContentDescriptor::Property const& GetProperty() const {return m_field->GetProperties()[m_propertyIndex];}
        size_t GetPropertyIndex() const {return m_propertyIndex;}
    };
    typedef bmap<FieldProperty, bvector<BeSQLite::EC::ECInstanceKey>> FieldPropertyInstanceKeyMap;

private:
    ECN::ECClassCP m_class;
    bvector<BeSQLite::EC::ECInstanceKey> m_keys;
    Utf8String m_displayLabel;
    Utf8String m_imageId;
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    FieldPropertyInstanceKeyMap m_fieldPropertyInstanceKeys;
    bvector<Utf8String> m_mergedFieldNames;

private:
    ContentSetItem(bvector<BeSQLite::EC::ECInstanceKey> keys, Utf8String displayLabel, Utf8String imageId, rapidjson::Document&& values, 
        rapidjson::Document&& displayValues, bvector<Utf8String> mergedFieldNames, FieldPropertyInstanceKeyMap&& fieldPropertyInstanceKeys)
        : m_class(nullptr), m_keys(keys), m_displayLabel(displayLabel), m_imageId(imageId), 
        m_values(std::move(values)), m_displayValues(std::move(displayValues)), 
        m_mergedFieldNames(mergedFieldNames), m_fieldPropertyInstanceKeys(std::move(fieldPropertyInstanceKeys))
        {}
    
//__PUBLISH_SECTION_END__
public:
    rapidjson::Document const& GetValues() const {return m_values;}
    rapidjson::Document& GetValues() {return m_values;}
    rapidjson::Document const& GetDisplayValues() const {return m_displayValues;}
    rapidjson::Document& GetDisplayValues() {return m_displayValues;}
    bvector<Utf8String>& GetMergedFieldNames() {return m_mergedFieldNames;}
    FieldPropertyInstanceKeyMap& GetFieldInstanceKeys() {return m_fieldPropertyInstanceKeys;}
//__PUBLISH_SECTION_START__

public:
    //! Creates a @ref ContentSetItem.
    //! @param[in] keys The keys which describe whose values this item contains.
    //! @param[in] displayLabel The label of this content item.
    //! @param[in] imageId The image ID for this item.
    //! @param[in] values The values map.
    //! @param[in] displayValues The display values map.
    //! @param[in] mergedFieldNames Names of merged fields in this record.
    //! @param[in] fieldPropertyInstanceKeys ECInstanceKeys of related instances for each field in this record.
    static ContentSetItemPtr Create(bvector<BeSQLite::EC::ECInstanceKey> keys, Utf8String displayLabel, Utf8String imageId, 
        rapidjson::Document&& values, rapidjson::Document&& displayValues, bvector<Utf8String> mergedFieldNames,
        FieldPropertyInstanceKeyMap&& fieldPropertyInstanceKeys)
        {
        return new ContentSetItem(keys, displayLabel, imageId, std::move(values), std::move(displayValues), 
            mergedFieldNames, std::move(fieldPropertyInstanceKeys));
        }

    //! Serialize this item to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(int flags = SERIALIZE_All, rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;

    //! Get keys of ECInstances whose values this item contains.
    bvector<BeSQLite::EC::ECInstanceKey> const& GetKeys() const {return m_keys;}
    
    //! Get names of merged fields in this record.
    bvector<Utf8String> const& GetMergedFieldNames() const {return m_mergedFieldNames;}

    //! Are the values of field with the specified name merged in this record.
    ECPRESENTATION_EXPORT bool IsMerged(Utf8StringCR fieldName) const;

    //! Get the ECInstance keys whose values are contained in the field with the specified name.
    ECPRESENTATION_EXPORT bvector<BeSQLite::EC::ECInstanceKey> const& GetPropertyValueKeys(FieldProperty const&) const;

    //! Get the ECClass whose values are contained in this record.
    //! @note May be null when the record contains multiple merged values of different classes.
    ECN::ECClassCP GetClass() const {return m_class;}
    //! Set the ECClass whose values are contained in this record.
    void SetClass(ECN::ECClassCP ecClass) {m_class = ecClass;}
    
    //! Get the display label of this item.
    Utf8StringCR GetDisplayLabel() const {return m_displayLabel;}

    //! Get the image ID of this item.
    Utf8StringCR GetImageId() const {return m_imageId;}
    //! Set the image ID for this item.
    void SetImageId(Utf8String imageId) {m_imageId = imageId;}
};

//=======================================================================================
//! A struct that contains the @ref ContentDescriptor and a @ref IDataSource of @ref ContentSetItem
//! objects which are based on that descriptor.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct Content : RefCountedBase
{
private:
    ContentDescriptorCPtr m_descriptor;
    IDataSourcePtr<ContentSetItemCPtr> m_contentSource;

private:
    Content(ContentDescriptorCR descriptor, IDataSource<ContentSetItemCPtr>& contentSource) : m_descriptor(&descriptor), m_contentSource(&contentSource) {}

public:
    //! Create the @ref Content object.
    //! @param[in] descriptor The descriptor used to create the content.
    //! @param[in] contentSource The source of @ref ContentSetItem objects.
    static ContentPtr Create(ContentDescriptorCR descriptor, IDataSource<ContentSetItemCPtr>& contentSource) {return new Content(descriptor, contentSource);}

    //! Get the descriptor that was used to create the @ref ContentSetItem objects.
    ContentDescriptorCR GetDescriptor() const {return *m_descriptor;}

    //! Get the content set.
    DataContainer<ContentSetItemCPtr> GetContentSet() const {return DataContainer<ContentSetItemCPtr>(*m_contentSource);}

    //! Serialize this object to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;
};

//=======================================================================================
//! Holds information about a single changed instance.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct ChangedECInstanceInfo
{
private:
    ECN::ECClassCP m_primaryInstanceClass;
    BeSQLite::EC::ECInstanceId m_primaryInstanceId;
    ECN::ECClassCP m_changedInstanceClass;
    BeSQLite::EC::ECInstanceId m_changedInstanceId;
    RelatedClassPath m_pathToPrimary;

public:
    ChangedECInstanceInfo(ECN::ECClassCR primaryInstanceClass, BeSQLite::EC::ECInstanceId primaryInstanceId)
        : m_primaryInstanceClass(&primaryInstanceClass), m_primaryInstanceId(primaryInstanceId), m_changedInstanceClass(nullptr)
        {}
    ChangedECInstanceInfo(ECN::ECClassCR primaryInstanceClass, BeSQLite::EC::ECInstanceId primaryInstanceId, 
        ECN::ECClassCR changedInstanceClass, BeSQLite::EC::ECInstanceId changedInstanceId, RelatedClassPath pathToPrimaryInstance)
        : m_primaryInstanceClass(&primaryInstanceClass), m_primaryInstanceId(primaryInstanceId), 
        m_changedInstanceClass(&changedInstanceClass), m_changedInstanceId(changedInstanceId), m_pathToPrimary(pathToPrimaryInstance)
        {}

    ECN::ECClassCR GetPrimaryInstanceClass() const {return *m_primaryInstanceClass;}
    BeSQLite::EC::ECInstanceId GetPrimaryInstanceId() const {return m_primaryInstanceId;}

    ECN::ECClassCR GetChangedInstanceClass() const {return (nullptr != m_changedInstanceClass) ? *m_changedInstanceClass : *m_primaryInstanceClass;}
    BeSQLite::EC::ECInstanceId GetChangedInstanceId() const {return (nullptr != m_changedInstanceClass) ? m_changedInstanceId : m_primaryInstanceId;}
    RelatedClassPath const& GetPathToPrimaryInstance() const {return m_pathToPrimary;}
};

//=======================================================================================
//! Holds the result on ECInstance change.
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct ECInstanceChangeResult
{
private:
    BentleyStatus m_status;
    ECN::ECValue m_changedValue;
    Utf8String m_errorMessage;

private:
    ECInstanceChangeResult(BentleyStatus status) : m_status(status) {}

public:
    ECPRESENTATION_EXPORT static ECInstanceChangeResult Success(ECN::ECValue changedValue);
    ECPRESENTATION_EXPORT static ECInstanceChangeResult Error(Utf8String message);

    ECInstanceChangeResult(ECInstanceChangeResult const& other)
        : m_status(other.m_status), m_changedValue(other.m_changedValue), m_errorMessage(other.m_errorMessage)
        {}
    ECInstanceChangeResult& operator=(ECInstanceChangeResult const& other)
        {
        m_status = other.m_status;
        m_changedValue = other.m_changedValue;
        m_errorMessage = other.m_errorMessage;
        return *this;
        }

    ECInstanceChangeResult(ECInstanceChangeResult&& other)
        : m_status(other.m_status), m_changedValue(std::move(other.m_changedValue)), m_errorMessage(std::move(other.m_errorMessage))
        {}
    ECInstanceChangeResult& operator=(ECInstanceChangeResult&& other)
        {
        m_status = other.m_status;
        m_changedValue = std::move(other.m_changedValue);
        m_errorMessage = std::move(other.m_errorMessage);
        return *this;
        }

    BentleyStatus GetStatus() const {return m_status;}
    ECN::ECValueCR GetChangedValue() const {return m_changedValue;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::MemoryPoolAllocator<>* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct IECPropertyFormatter
{
protected:
    //! Virtual destructor.
    virtual ~IECPropertyFormatter() {}

    //! @see GetFormattedPropertyValue
    virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECN::ECPropertyCR, ECN::ECValueCR) const = 0;
    virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECN::ECPropertyCR, ECN::ECClassCR, RelatedClassPath const&, RelationshipMeaning) const = 0;

public:
    //! Formats the supplied ECValue.
    //! @param[out] formattedValue The formatted value.
    //! @param[in] ecProperty The property whose value is being formatted.
    //! @param[in] ecValue The value to format.
    //! @return SUCCESS if the value was successfully formatted.
    BentleyStatus GetFormattedPropertyValue(Utf8StringR formattedValue, ECN::ECPropertyCR ecProperty, ECN::ECValueCR ecValue) const 
        {
        return _GetFormattedPropertyValue(formattedValue, ecProperty, ecValue);
        }

    //! Returns formatted label for the supplied ECProperty.
    //! @param[out] formattedLabel The formatted property label.
    //! @param[in] ecProperty The property whose label is being formatted.
    //! @param[in] propertyClass The class of ECProperty whose label is being formatted.
    //! @param[in] relatedClassPath Relationship path from the displayed ECInstance class to the supplied ECProperty class.
    //! @param[in] relationshipMeaning Meaning of relationship between displayed ECInstace class and ECProperty class.
    //! @return SUCCESS if the label was successfully formatted.
    BentleyStatus GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECN::ECPropertyCR ecProperty, ECN::ECClassCR propertyClass, 
        RelatedClassPath const& relatedClassPath, RelationshipMeaning relationshipMeaning) const
        {
        return _GetFormattedPropertyLabel(formattedLabel, ecProperty, propertyClass, relatedClassPath, relationshipMeaning);
        }
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct DefaultPropertyFormatter : IECPropertyFormatter
{
protected:
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECN::ECPropertyCR, ECN::ECValueCR) const override;
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECN::ECPropertyCR, ECN::ECClassCR, RelatedClassPath const&, RelationshipMeaning) const override;
};

//=======================================================================================
//! Interface for a category supplier which is used to determine categories for 
//! @ref ContentDescriptor::Field.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2016
//=======================================================================================
struct IPropertyCategorySupplier
{
protected:
    //! Virtual destructor.
    virtual ~IPropertyCategorySupplier() {}

    //! @see GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECPropertyCR)
    virtual ContentDescriptor::Category _GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECPropertyCR) = 0;
    
    //! @see GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECClassCR)
    virtual ContentDescriptor::Category _GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECClassCR) = 0;

public:
    //! Get category for the specified ECProperty.
    //! @param[in] primaryClass ECClass of the primary (root) instance.
    //! @param[in] path Relationship path from @e primaryClass to @e ecProperty class. Not empty only if @e ecProperty belongs
    //!                 to a related instance.
    //! @param[in] ecProperty ECProperty whose category is requested.
    ContentDescriptor::Category GetCategory(ECN::ECClassCR primaryClass, RelatedClassPathCR path, ECN::ECPropertyCR ecProperty)
        {
        return _GetCategory(primaryClass, path, ecProperty);
        }

    //! Get category for the specified nested content ECClass.
    //! @param[in] primaryClass ECClass of the primary (root) instance.
    //! @param[in] path Relationship path from @e primaryClass to @e nestedContentClass.
    //! @param[in] nestedContentClass ECClass whose category is requested.
    ContentDescriptor::Category GetCategory(ECN::ECClassCR primaryClass, RelatedClassPathCR path, ECN::ECClassCR nestedContentClass)
        {
        return _GetCategory(primaryClass, path, nestedContentClass);
        }
};

//=======================================================================================
//! Default property category supplier that uses the "Category" custom attribute to determine
//! the ECProperty category. If no such custom attribute exists, the "Miscellaneous" category
//! is returned.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefaultCategorySupplier : IPropertyCategorySupplier
{
public:
    static const int NESTED_CONTENT_CATEGORY_PRIORITY = 400000; // matches Standard::General
protected:
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECPropertyCR) override;
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _GetCategory(ECN::ECClassCR, RelatedClassPathCR, ECN::ECClassCR) override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
