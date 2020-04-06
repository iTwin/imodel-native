/*--------------------------------------------------------------------------------------+
|
|     $Source: Presentation/UpdateRecordsHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UpdateRecordsHandler.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUpdateRecordsHandler::WipReport
{
private:
    bvector<HierarchyUpdateRecord> m_partialHierarchyUpdates;
    bvector<FullUpdateRecord> m_fullUpdates;
public:
    void Accept(HierarchyUpdateRecord const& record) {m_partialHierarchyUpdates.push_back(record);}
    void Accept(FullUpdateRecord const& record) {m_fullUpdates.push_back(record);}
    bvector<HierarchyUpdateRecord> const& GetPartialHierarchyUpdates() const {return m_partialHierarchyUpdates;}
    bvector<FullUpdateRecord> const& GetFullUpdates() const {return m_fullUpdates;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUpdateRecordsHandler::FinalReport
{
    struct PartialHierarchyUpdates : bvector<HierarchyUpdateRecord>
    {
    private:
        uint64_t m_lastUsedTimestamp;
    public:
        void push_back(HierarchyUpdateRecord record)
            {
            m_lastUsedTimestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            bvector<HierarchyUpdateRecord>::push_back(record);
            }
        uint64_t GetLastUsedTimestamp() const {return m_lastUsedTimestamp;}
    };

private:
    static uint64_t const s_partialHierarchyUpdatesLimit = 1000000; // TODO: use 1M for now, need to measure size of update records and tweak this number
    BeMutex m_mutex;
    uint64_t m_partialHierarchyUpdatesCount;
    bmap<Utf8String, PartialHierarchyUpdates> m_partialHierarchyUpdates;
    bset<Utf8String> m_fullHierarchyUpdates;
    bset<Utf8String> m_fullContentUpdates;

private:
    template<typename TRecord> static Utf8CP GetRulesetId(TRecord const& record)
        {
        if (!record.GetRulesetId().empty())
            return record.GetRulesetId().c_str();
        return "*";
        }

    static RapidJsonValueR GetMemberJson(RapidJsonValueR obj, rapidjson::Document::AllocatorType& alloc, Utf8CP name, rapidjson::Type memberType)
        {
        rapidjson::Document::MemberIterator iter = obj.FindMember(name);
        if (iter != obj.MemberEnd())
            return iter->value;

        obj.AddMember(rapidjson::StringRef(name), rapidjson::Value(memberType), alloc);
        return obj[name];
        }

    void ClearLeastRecentlyUsedPartialHierarchyUpdates()
        {
        bpair<uint64_t, Utf8String> lru(0, "");
        for (auto entry : m_partialHierarchyUpdates)
            {
            if (entry.second.GetLastUsedTimestamp() < lru.first || 0 == lru.first)
                lru = bpair<uint64_t, Utf8String>(entry.second.GetLastUsedTimestamp(), entry.first);
            }
        auto iter = m_partialHierarchyUpdates.find(lru.second);
        if (m_partialHierarchyUpdates.end() != iter)
            {
            m_partialHierarchyUpdatesCount -= iter->second.size();
            m_fullHierarchyUpdates.insert(iter->first);
            m_partialHierarchyUpdates.erase(iter);
            }
        }

public:
    FinalReport() : m_partialHierarchyUpdatesCount(0) {}
    void Accept(WipReport const& wip)
        {
        BeMutexHolder lock(m_mutex);
        for (FullUpdateRecord const& record : wip.GetFullUpdates())
            {
            Utf8CP rulesetId = GetRulesetId(record);
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Hierarchy == record.GetUpdateTarget())
                {
                if (0 == strcmp("*", rulesetId))
                    m_partialHierarchyUpdates.clear();
                else
                    m_partialHierarchyUpdates.erase(rulesetId);
                m_fullHierarchyUpdates.insert(rulesetId);
                }
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Content == record.GetUpdateTarget())
                {
                m_fullContentUpdates.insert(rulesetId);
                }
            }
        for (HierarchyUpdateRecord const& record : wip.GetPartialHierarchyUpdates())
            {
            while (m_partialHierarchyUpdatesCount > s_partialHierarchyUpdatesLimit && !m_partialHierarchyUpdates.empty())
                ClearLeastRecentlyUsedPartialHierarchyUpdates();

            Utf8CP rulesetId = GetRulesetId(record);
            bool hasFullUpdateRecord = m_fullHierarchyUpdates.end() != m_fullHierarchyUpdates.find(rulesetId)
                || 0 != strcmp("*", rulesetId) && m_fullHierarchyUpdates.end() != m_fullHierarchyUpdates.find("*");
            if (hasFullUpdateRecord)
                continue;

            auto iter = m_partialHierarchyUpdates.find(rulesetId);
            if (m_partialHierarchyUpdates.end() == iter)
                iter = m_partialHierarchyUpdates.Insert(rulesetId, PartialHierarchyUpdates()).first;
            iter->second.push_back(record);
            ++m_partialHierarchyUpdatesCount;
            }
        }

    rapidjson::Document BuildJsonAndReset()
        {
        BeMutexHolder lock(m_mutex);
        if (m_partialHierarchyUpdates.empty() && m_fullHierarchyUpdates.empty() && m_fullContentUpdates.empty())
            return rapidjson::Document();

        rapidjson::Document json(rapidjson::kObjectType);
        for (auto entry : m_partialHierarchyUpdates)
            {
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(json, json.GetAllocator(), entry.first.c_str(), rapidjson::kObjectType);
            RapidJsonValueR hierarchyUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "hierarchy", rapidjson::kArrayType);
            for (HierarchyUpdateRecord const& record : entry.second)
                hierarchyUpdatesJson.PushBack(record.AsJson(&json.GetAllocator()), json.GetAllocator());
            }
        m_partialHierarchyUpdates.clear();
        for (Utf8StringCR rulesetId : m_fullHierarchyUpdates)
            {
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(json, json.GetAllocator(), rulesetId.c_str(), rapidjson::kObjectType);
            RapidJsonValueR hierarchyUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "hierarchy", rapidjson::kStringType);
            hierarchyUpdatesJson.SetString("FULL");
            }
        m_fullHierarchyUpdates.clear();
        for (Utf8StringCR rulesetId : m_fullContentUpdates)
            {
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(json, json.GetAllocator(), rulesetId.c_str(), rapidjson::kObjectType);
            RapidJsonValueR contentUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "content", rapidjson::kStringType);
            contentUpdatesJson.SetString("FULL");
            }
        m_fullContentUpdates.clear();
        return json;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
IModelJsECPresentationUpdateRecordsHandler::IModelJsECPresentationUpdateRecordsHandler()
    : m_finalReport(std::make_unique<FinalReport>())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
IModelJsECPresentationUpdateRecordsHandler::~IModelJsECPresentationUpdateRecordsHandler()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Start()
    {
    m_wipReportMutex.lock();
    m_wipReport = std::make_unique<WipReport>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Accept(HierarchyUpdateRecord const& record)
    {
    m_wipReport->Accept(record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Accept(FullUpdateRecord const& record)
    {
    m_wipReport->Accept(record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Finish()
    {
    m_finalReport->Accept(*m_wipReport);
    m_wipReport.reset();
    m_wipReportMutex.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationUpdateRecordsHandler::GetReport()
    {
    return m_finalReport->BuildJsonAndReset();
    }
