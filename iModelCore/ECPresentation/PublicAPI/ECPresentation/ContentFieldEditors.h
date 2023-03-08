/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/Rules/PropertyEditorSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FieldEditorJsonParams : ContentFieldEditor::Params
{
private:
    rapidjson::Document m_json;

protected:
    Utf8CP _GetName() const override {return "Json";}
    Params* _Clone() const override {return new FieldEditorJsonParams(*this);}
    ECPRESENTATION_EXPORT int _CompareTo(Params const& other) const override;
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;

public:
    FieldEditorJsonParams(PropertyEditorJsonParameters const& spec)
        {
        m_json.Parse(spec.GetJson().Stringify().c_str());
        }
    FieldEditorJsonParams(FieldEditorJsonParams const& other)
        {
        m_json.CopyFrom(other.m_json, m_json.GetAllocator());
        }
    RapidJsonDocumentCR GetJson() const {return m_json;};
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FieldEditorMultilineParams : ContentFieldEditor::Params
{
private:
    PropertyEditorMultilineParameters const& m_spec;

protected:
    Utf8CP _GetName() const override {return "Multiline";}
    Params* _Clone() const override {return new FieldEditorMultilineParams(*this);}
    ECPRESENTATION_EXPORT int _CompareTo(Params const& other) const override;
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;

public:
    FieldEditorMultilineParams(PropertyEditorMultilineParameters const& spec) : m_spec(spec) {}
    FieldEditorMultilineParams(FieldEditorMultilineParams const& other) : m_spec(other.m_spec) {}
    PropertyEditorMultilineParameters const& GetParameters() const {return m_spec;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FieldEditorRangeParams : ContentFieldEditor::Params
{
private:
    PropertyEditorRangeParameters const& m_spec;

protected:
    Utf8CP _GetName() const override {return "Range";}
    Params* _Clone() const override {return new FieldEditorRangeParams(*this);}
    ECPRESENTATION_EXPORT int _CompareTo(Params const& other) const override;
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;

public:
    FieldEditorRangeParams(PropertyEditorRangeParameters const& spec) : m_spec(spec) {}
    FieldEditorRangeParams(FieldEditorRangeParams const& other) : m_spec(other.m_spec) {}
    PropertyEditorRangeParameters const& GetParameters() const {return m_spec;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FieldEditorSliderParams : ContentFieldEditor::Params
{
private:
    PropertyEditorSliderParameters const& m_spec;

protected:
    Utf8CP _GetName() const override {return "Slider";}
    Params* _Clone() const override {return new FieldEditorSliderParams(*this);}
    ECPRESENTATION_EXPORT int _CompareTo(Params const& other) const override;
    ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;

public:
    FieldEditorSliderParams(PropertyEditorSliderParameters const& spec) : m_spec(spec) {}
    FieldEditorSliderParams(FieldEditorSliderParams const& other) : m_spec(other.m_spec) {}
    PropertyEditorSliderParameters const& GetParameters() const {return m_spec;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EditorParamsBuilder : PropertyEditorParametersSpecification::Visitor
{

private:
    ContentFieldEditor& m_editor;

protected:
    void _Visit(PropertyEditorJsonParameters const& params) override {m_editor.GetParams().push_back(new FieldEditorJsonParams(params));}
    void _Visit(PropertyEditorMultilineParameters const& params) override {m_editor.GetParams().push_back(new FieldEditorMultilineParams(params));}
    void _Visit(PropertyEditorRangeParameters const& params) override {m_editor.GetParams().push_back(new FieldEditorRangeParams(params));}
    void _Visit(PropertyEditorSliderParameters const& params) override {m_editor.GetParams().push_back(new FieldEditorSliderParams(params));}

public:
    EditorParamsBuilder(ContentFieldEditor& editor) : m_editor(editor) {}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
