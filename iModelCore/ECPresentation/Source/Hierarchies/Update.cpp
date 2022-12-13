/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Update.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeChangeCounter : HierarchyChangeRecordDiffHandler
{
private:
    uint32_t m_numChanges;

protected:
    void _HandleKey(NavNodeKeyCR newValue) override {++m_numChanges;}
    void _HandleHasChildren(bool newValue) override {++m_numChanges;}
    void _HandleIsChecked(bool newValue) override {++m_numChanges;}
    void _HandleIsCheckboxVisible(bool newValue) override {++m_numChanges;}
    void _HandleIsCheckboxEnabled(bool newValue) override {++m_numChanges;}
    void _HandleShouldAutoExpand(bool newValue) override {++m_numChanges;}
    void _HandleDescription(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleImageId(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleForeColor(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleBackColor(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleFontStyle(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleType(Utf8StringCR newValue) override {++m_numChanges;}
    void _HandleLabelDefinition(LabelDefinitionCR newValue) override {++m_numChanges;}

public:
    NodeChangeCounter(): m_numChanges(0) {}

    uint32_t GetNumChanges() const {return m_numChanges;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t NodeChanges::GetNumChangedFields() const
    {
    if (m_numChangedFields.IsValid())
        return m_numChangedFields.Value();

    NodeChangeCounter changeCounter;
    FindChanges(changeCounter);

    m_numChangedFields = changeCounter.GetNumChanges();
    return m_numChangedFields.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodeChanges::FindChanges(HierarchyChangeRecordDiffHandler& handler) const
    {
    if (*m_previousNode->GetKey() != *m_updatedNode->GetKey())
        handler.HandleKey(*m_updatedNode->GetKey());

    if (m_previousNode->HasChildren() != m_updatedNode->HasChildren())
        handler.HandleHasChildren(m_updatedNode->HasChildren());

    if (m_previousNode->IsChecked() != m_updatedNode->IsChecked())
        handler.HandleIsChecked(m_updatedNode->IsChecked());

    if (m_previousNode->IsCheckboxVisible() != m_updatedNode->IsCheckboxVisible())
        handler.HandleIsCheckboxVisible(m_updatedNode->IsCheckboxVisible());

    if (m_previousNode->IsCheckboxEnabled() != m_updatedNode->IsCheckboxEnabled())
        handler.HandleIsCheckboxEnabled(m_updatedNode->IsCheckboxEnabled());

    if (m_previousNode->ShouldAutoExpand() != m_updatedNode->ShouldAutoExpand())
        handler.HandleShouldAutoExpand(m_updatedNode->ShouldAutoExpand());

    if (m_previousNode->GetDescription() != m_updatedNode->GetDescription())
        handler.HandleDescription(m_updatedNode->GetDescription());

    if (m_previousNode->GetImageId() != m_updatedNode->GetImageId())
        handler.HandleImageId(m_updatedNode->GetImageId());

    if (m_previousNode->GetForeColor() != m_updatedNode->GetForeColor())
        handler.HandleForeColor(m_updatedNode->GetForeColor());

    if (m_previousNode->GetBackColor() != m_updatedNode->GetBackColor())
        handler.HandleBackColor(m_updatedNode->GetBackColor());

    if (m_previousNode->GetFontStyle() != m_updatedNode->GetFontStyle())
        handler.HandleFontStyle(m_updatedNode->GetFontStyle());

    if (m_previousNode->GetType() != m_updatedNode->GetType())
        handler.HandleType(m_updatedNode->GetType());

    if (m_previousNode->GetLabelDefinition() != m_updatedNode->GetLabelDefinition())
        handler.HandleLabelDefinition(m_updatedNode->GetLabelDefinition());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyChangeRecord::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyChangeRecord::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyUpdateRecord::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyUpdateRecord::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyUpdateRecord::ExpandedNode::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyUpdateRecord::ExpandedNode::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }
