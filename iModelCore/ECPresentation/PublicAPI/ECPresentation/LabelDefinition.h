/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define NAVNODE_LABEL_DEFINITION_DisplayValue              "DisplayValue"
#define NAVNODE_LABEL_DEFINITION_RawValue                  "RawValue"
#define NAVNODE_LABEL_DEFINITION_TypeName                  "TypeName"
#define NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Separator "Separator"
#define NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Values    "Values"

#define SIMPLE_RAW_VALUE_CHUNK_SIZE 128

//=======================================================================================
//! A definition of @ref NavNode display label.
// @bsiclass                                    Saulius.Skliutas                12/2019
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LabelDefinition : RefCountedBase
{
    struct SimpleRawValue;
    struct CompositeRawValue;

    //=======================================================================================
    //! Base class used for storing label definition raw value.
    // @bsiclass                                    Saulius.Skliutas                12/2019
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE RawValueBase
    {
    protected:
        virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType*) const = 0;
        virtual std::unique_ptr<RawValueBase> _Clone() const = 0;
        virtual bool _Equals(RawValueBase const& other) const = 0;
        virtual rapidjson::Document _ToInternalJson(rapidjson::Document::AllocatorType*) const = 0;
        virtual SimpleRawValue const* _AsSimpleValue() const { return nullptr; }
        virtual CompositeRawValue const* _AsCompositeValue() const { return nullptr; }

    public:
        virtual ~RawValueBase() {}
        rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const { return _AsJson(allocator); }
        bool operator==(RawValueBase const& other) const { return _Equals(other); }
        bool operator!=(RawValueBase const& other) const { return !_Equals(other); }
        std::unique_ptr<RawValueBase> Clone() const { return _Clone(); }
        SimpleRawValue const* AsSimpleValue() const { return _AsSimpleValue(); }
        CompositeRawValue const* AsCompositeValue() const { return _AsCompositeValue(); }

        rapidjson::Document ToInternalJson(rapidjson::Document::AllocatorType* allocator) const { return _ToInternalJson(allocator); }
    };

    //=======================================================================================
    //! Class used for storing simple raw value of label definition.
    // @bsiclass                                    Saulius.Skliutas                12/2019
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE SimpleRawValue : RawValueBase
    {
    private:
        rapidjson::MemoryPoolAllocator<> m_allocator;
        rapidjson::Document m_value;

    protected:
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT bool _Equals(RawValueBase const& other) const override;
        std::unique_ptr<RawValueBase> _Clone() const override { return std::make_unique<SimpleRawValue>(*this);  }
        SimpleRawValue const* _AsSimpleValue() const override { return this; }
        ECPRESENTATION_EXPORT rapidjson::Document _ToInternalJson(rapidjson::Document::AllocatorType* allocator) const override;

    public:
        SimpleRawValue(): m_allocator(SIMPLE_RAW_VALUE_CHUNK_SIZE), m_value(&m_allocator) {}
        SimpleRawValue(SimpleRawValue const& other): SimpleRawValue() { m_value.CopyFrom(other.m_value, m_value.GetAllocator()); }
        SimpleRawValue(Utf8CP value): SimpleRawValue() { m_value.SetString(value, m_value.GetAllocator()); }
        SimpleRawValue(RapidJsonValueCR value): SimpleRawValue() { m_value.CopyFrom(value, m_value.GetAllocator()); }

        RapidJsonValueCR GetValue() const { return m_value; }

        static std::unique_ptr<SimpleRawValue> FromInternalJson(RapidJsonValueCR json) { return std::make_unique<SimpleRawValue>(json); }
    };

    //=======================================================================================
    //! Class used for storing composite raw value of label definition.
    // @bsiclass                                    Saulius.Skliutas                12/2019
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE CompositeRawValue : RawValueBase
    {
    private:
        Utf8String m_separator;
        bvector<LabelDefinitionCPtr> m_values;

    protected:
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT bool _Equals(RawValueBase const& other) const override;
        std::unique_ptr<RawValueBase> _Clone() const override { return std::make_unique<CompositeRawValue>(*this); }
        CompositeRawValue const* _AsCompositeValue() const override { return this; }
        ECPRESENTATION_EXPORT rapidjson::Document _ToInternalJson(rapidjson::Document::AllocatorType* allocator) const override;

    public:
        CompositeRawValue(Utf8CP separator): m_separator(separator) {}
        CompositeRawValue(Utf8CP separator, bvector<LabelDefinitionCPtr> const& values): m_separator(separator), m_values(values) {}
        CompositeRawValue(CompositeRawValue const& other) : m_separator(other.m_separator), m_values(other.m_values) {}

        Utf8StringCR GetSeparator() const { return m_separator; }
        bvector<LabelDefinitionCPtr> const& GetValues() const { return m_values; }

        void AddValue(LabelDefinitionCR value) { m_values.push_back(&value); }
        ECPRESENTATION_EXPORT static std::unique_ptr<CompositeRawValue> FromInternalJson(RapidJsonValueCR json);
    };

private:
    Utf8String m_displayValue;
    std::unique_ptr<RawValueBase> m_rawValue;
    Utf8String m_typeName;

    LabelDefinition() : m_displayValue(""), m_typeName(""), m_rawValue(nullptr) { }
    LabelDefinition(LabelDefinitionCR);
    LabelDefinition(Utf8CP displayValue, Utf8CP typeName, std::unique_ptr<RawValueBase> rawValue) : m_displayValue(displayValue), m_typeName(typeName), m_rawValue(std::move(rawValue)) { }
    LabelDefinition(Utf8CP value, Utf8CP displayValue = nullptr) : LabelDefinition() { SetStringValue(value, displayValue); }
    LabelDefinition(ECValueCR value, Utf8CP displayValue = nullptr) : LabelDefinition() { SetECValue(value, displayValue); }
    LabelDefinition(ECPropertyCR ecProperty, DbValue const& dbValue, Utf8CP displayValue = nullptr) : LabelDefinition() { SetECPropertyValue(ecProperty, dbValue, displayValue); }
    LabelDefinition(RapidJsonValueCR value, Utf8CP typeName, Utf8CP displayValue) : LabelDefinition() { SetJsonValue(displayValue, typeName, value); }
    LabelDefinition(Utf8CP displayValue, std::unique_ptr<CompositeRawValue> value) : LabelDefinition() { SetCompositeValue(displayValue, std::move(value)); }

public:
    //! Create invalid label definition.
    static LabelDefinitionPtr Create() { return new LabelDefinition(); }
    //! Create label definition from supplied label defintion.
    static LabelDefinitionPtr Create(LabelDefinitionCR other) { return new LabelDefinition(other); }
    //! Create using string value as display label.
    static LabelDefinitionPtr Create(Utf8CP displayValue, Utf8CP typeName, std::unique_ptr<RawValueBase> rawValue) { return new LabelDefinition(displayValue, typeName, std::move(rawValue)); }
    //! Create using string value as display label.
    static LabelDefinitionPtr Create(Utf8CP value, Utf8CP displayValue = nullptr) { return new LabelDefinition(value, displayValue); }
    //! Create using ECValue as display label.
    static LabelDefinitionPtr Create(ECValueCR value, Utf8CP displayValue = nullptr) { return new LabelDefinition(value, displayValue); }
    //! Create using ECProperty value as display label.
    static LabelDefinitionPtr Create(ECPropertyCR ecProperty, DbValue const& dbValue, Utf8CP displayValue = nullptr) { return new LabelDefinition(ecProperty, dbValue, displayValue); }
    //! Create using json value as display label.
    static LabelDefinitionPtr Create(RapidJsonValueCR value, Utf8CP typeName, Utf8CP displayValue) { return new LabelDefinition(value, typeName, displayValue); }
    //! Create using composite value as display label.
    static LabelDefinitionPtr Create(Utf8CP displayValue, std::unique_ptr<CompositeRawValue> value) { return new LabelDefinition(displayValue, std::move(value)); }

    //! Is this label definition equal to the supplied one.
    ECPRESENTATION_EXPORT bool operator==(LabelDefinitionCR) const;
    //! Is this label definition not equal to the supplied one.
    ECPRESENTATION_EXPORT bool operator!=(LabelDefinitionCR) const;

    //! Check that label definition is valid.
    bool IsDefinitionValid() const { return !m_displayValue.empty() && !m_typeName.empty() && nullptr != m_rawValue; }
    //! Get display value.
    Utf8StringCR GetDisplayValue() const { return m_displayValue; }
    //! Set display value.
    void SetDisplayValue(Utf8StringCR value) { m_displayValue = value; }
    //! Get raw value.
    std::unique_ptr<RawValueBase> const& GetRawValue() const { return m_rawValue; }
    //! Get type name.
    Utf8StringCR GetTypeName() const { return m_typeName; }

    //! Set string value as display label.
    ECPRESENTATION_EXPORT LabelDefinitionCR SetStringValue(Utf8CP value, Utf8CP displayValue = nullptr);
    //! Set ECValue as display label.
    ECPRESENTATION_EXPORT LabelDefinitionCR SetECValue(ECValueCR value, Utf8CP displayValue = nullptr);
    //! Set ECProperty value as display label.
    ECPRESENTATION_EXPORT LabelDefinitionCR SetECPropertyValue(ECPropertyCR ecProperty, DbValue const& dbValue, Utf8CP displayValue = nullptr);
    //! Set json value as display label.
    ECPRESENTATION_EXPORT LabelDefinitionCR SetJsonValue(Utf8CP displayValue, Utf8CP typeName, RapidJsonValueCR value);
    //! Set composite value as display label.
    ECPRESENTATION_EXPORT LabelDefinitionCR SetCompositeValue(Utf8CP displayValue, std::unique_ptr<CompositeRawValue> value);

    //! Serialize label definition to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Serialize label definition to JSON string used internally.
    ECPRESENTATION_EXPORT Utf8String ToJsonString() const;
    //! Serialize label definition to JSON used internally.
    ECPRESENTATION_EXPORT rapidjson::Document ToInternalJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
    //! Create a label definition from the supplied JSON.
    ECPRESENTATION_EXPORT static LabelDefinitionPtr FromInternalJson(RapidJsonValueCR);
    //! Create a label definition from the supplied string.
    ECPRESENTATION_EXPORT static LabelDefinitionPtr FromString(Utf8CP);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
