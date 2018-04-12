/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/SelectionSyncHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECPresentation/SelectionManager.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
//! Selection synchronization handler for rules-driven controls. Basically this is just a
//! helper base class which creates content request options from the selection event
//! and creates rules-driven presentation manager specific selection event extended data.
//! @ingroup GROUP_RulesDrivenPresentation
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct RulesDrivenSelectionSyncHandler : SelectionSyncHandler
{
    //===================================================================================
    //! Helper class to read/write selection change event extended data.
    // @bsiclass                                    Grigas.Petraitis            08/2016
    //===================================================================================
    struct SelectionExtendedData : RapidJsonAccessor
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RulesetId;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_UseSelectionScope;

        //! Constructor. Creates a read-write accessor.
        SelectionExtendedData() : RapidJsonAccessor() {}
        //! Constructor. Creates a read-only accessor.
        SelectionExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        SelectionExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : RapidJsonAccessor(data, allocator) {}
        //! Copy constructor.
        SelectionExtendedData(SelectionExtendedData const& other) : RapidJsonAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset which is used by the control that is firing
        //! the selection change event.
        SelectionExtendedData(Utf8CP rulesetId) : RapidJsonAccessor() {SetRulesetId(rulesetId);}
        //! Constructor. Creates a read-only accessor for the specified selection change event.
        SelectionExtendedData(SelectionChangedEventCR evt) : RapidJsonAccessor(evt) {}

        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().IsObject() && GetJson().HasMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().HasMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].GetString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {AddMember(OPTION_NAME_RulesetId, rapidjson::Value(rulesetId, GetAllocator()));}

        bool GetUseSelectionScope() const { return GetJson().HasMember(OPTION_NAME_UseSelectionScope) ? GetJson()[OPTION_NAME_UseSelectionScope].GetBool() : false; }
        void SetUseSelectionScope(bool useSelectionScope) { AddMember(OPTION_NAME_UseSelectionScope, rapidjson::Value(useSelectionScope)); }
        };

private:
    Utf8String m_rulesetId;

protected:
    //! Creates rules-driven presentation manager specific content request options
    //! using the supplied selection change event.
    ECPRESENTATION_EXPORT Json::Value _CreateContentOptionsForSelection(SelectionChangedEventCR) const override;

    //! Creates rules-driven presentation manager specific selection event extended
    //! data.
    ECPRESENTATION_EXPORT rapidjson::Document _CreateSelectionEventExtendedData() const override;

protected:
    //! Constructor.
    //! @param[in] rulesetId The ID of the default ruleset to use when requesting for
    //! content in case the selection event doesn't specify a ruleset.
    RulesDrivenSelectionSyncHandler(Utf8CP rulesetId) : m_rulesetId(rulesetId) {}

    //! Sets the default ruleset ID.
    void SetRulesetId(Utf8CP rulesetId) {m_rulesetId = rulesetId;}

    //! Get default ruleset ID.
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
