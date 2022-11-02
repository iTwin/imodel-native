/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UpdateRecordsHandler.h"
#include "ECPresentationUtils.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUpdateRecordsHandler::FinalReport
{
    struct UpdateRecordKey
    {
    private:
        Utf8String m_rulesetId;
        Utf8String m_ecdbFileName;
    public:
        UpdateRecordKey() {}
        UpdateRecordKey(Utf8String rulesetId, Utf8String ecdbFileName) : m_rulesetId(rulesetId), m_ecdbFileName(ecdbFileName) {}
        bool operator==(UpdateRecordKey const& other) const
            {
            return m_rulesetId == other.m_rulesetId && m_ecdbFileName == other.m_ecdbFileName;
            }
        bool operator<(UpdateRecordKey const& other) const
            {
            int rulesetIdCmp = m_rulesetId.CompareTo(other.m_rulesetId);
            if (rulesetIdCmp < 0)
                return true;
            if (rulesetIdCmp > 0)
                return false;
            return m_ecdbFileName < other.m_ecdbFileName;
            }

        bool IsAnyRuleset() const {return 0 == strcmp("*", m_rulesetId.c_str());}
        bool IsAnyECDb() const {return 0 == strcmp("*", m_ecdbFileName.c_str());}
        Utf8StringCR GetRulesetId() const {return m_rulesetId;}
        Utf8StringCR GetECDbFileName() const {return m_ecdbFileName;}
    };

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
    bmap<UpdateRecordKey, PartialHierarchyUpdates> m_partialHierarchyUpdates;
    bset<UpdateRecordKey> m_fullHierarchyUpdates;
    bset<UpdateRecordKey> m_fullContentUpdates;

private:
    template<typename TRecord> static Utf8CP GetRulesetId(TRecord const& record)
        {
        if (!record.GetRulesetId().empty())
            return record.GetRulesetId().c_str();
        return "*";
        }

    template<typename TRecord> static Utf8CP GetECDbFileName(TRecord const& record)
        {
        if (!record.GetECDbFileName().empty())
            return record.GetECDbFileName().c_str();
        return "*";
        }

    static RapidJsonValueR GetMemberJson(RapidJsonValueR obj, rapidjson::Document::AllocatorType& alloc, Utf8CP name, rapidjson::Type memberType)
        {
        rapidjson::Document::MemberIterator iter = obj.FindMember(name);
        if (iter != obj.MemberEnd())
            return iter->value;

        obj.AddMember(rapidjson::Value(name, alloc), rapidjson::Value(memberType), alloc);
        return obj[name];
        }

    void ClearLeastRecentlyUsedPartialHierarchyUpdates()
        {
        bpair<uint64_t, UpdateRecordKey> lru(0, UpdateRecordKey());
        for (auto entry : m_partialHierarchyUpdates)
            {
            if (entry.second.GetLastUsedTimestamp() < lru.first || 0 == lru.first)
                lru = bpair<uint64_t, UpdateRecordKey>(entry.second.GetLastUsedTimestamp(), entry.first);
            }
        auto iter = m_partialHierarchyUpdates.find(lru.second);
        if (m_partialHierarchyUpdates.end() != iter)
            {
            m_partialHierarchyUpdatesCount -= iter->second.size();
            m_fullHierarchyUpdates.insert(iter->first);
            m_partialHierarchyUpdates.erase(iter);
            }
        }

    void RemovePartialUpdateRecords(UpdateRecordKey const& key)
        {
        bool anyRuleset = key.IsAnyRuleset();
        bool anyECDb = key.IsAnyECDb();
        if (!anyRuleset && !anyECDb)
            {
            m_partialHierarchyUpdates.erase(key);
            return;
            }

        if (anyECDb && anyRuleset)
            {
            m_partialHierarchyUpdates.clear();
            return;
            }

        auto iter = m_partialHierarchyUpdates.begin();
        while (iter != m_partialHierarchyUpdates.end())
            {
            if ((anyECDb && iter->first.GetRulesetId() == key.GetRulesetId()) || (anyRuleset && iter->first.GetECDbFileName() == key.GetECDbFileName()))
                iter = m_partialHierarchyUpdates.erase(iter);
            else
                iter++;
            }
        }

public:
    FinalReport() : m_partialHierarchyUpdatesCount(0) {}
    void Accept(WipReport const& wip)
        {
        BeMutexHolder lock(m_mutex);
        for (FullUpdateRecord const& record : wip.GetFullUpdates())
            {
            UpdateRecordKey key(GetRulesetId(record), GetECDbFileName(record));
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Hierarchy == record.GetUpdateTarget())
                {
                RemovePartialUpdateRecords(key);
                m_fullHierarchyUpdates.insert(key);
                }
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Content == record.GetUpdateTarget())
                {
                m_fullContentUpdates.insert(key);
                }
            }
        for (HierarchyUpdateRecord const& record : wip.GetPartialHierarchyUpdates())
            {
            while (m_partialHierarchyUpdatesCount > s_partialHierarchyUpdatesLimit && !m_partialHierarchyUpdates.empty())
                ClearLeastRecentlyUsedPartialHierarchyUpdates();

            UpdateRecordKey key(GetRulesetId(record), GetECDbFileName(record));
            bool hasFullUpdateRecord = m_fullHierarchyUpdates.end() != std::find_if(m_fullHierarchyUpdates.begin(), m_fullHierarchyUpdates.end(),
                [&](UpdateRecordKey const& entry)
                {
                return entry == key || (key.IsAnyRuleset() && key.GetECDbFileName() == entry.GetECDbFileName()) || (key.IsAnyECDb() && key.GetRulesetId() == entry.GetRulesetId());
                });
            if (hasFullUpdateRecord)
                continue;

            auto iter = m_partialHierarchyUpdates.find(key);
            if (m_partialHierarchyUpdates.end() == iter)
                iter = m_partialHierarchyUpdates.Insert(key, PartialHierarchyUpdates()).first;
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
            RapidJsonValueR ecdbUpdatesJson = GetMemberJson(json, json.GetAllocator(), entry.first.GetECDbFileName().c_str(), rapidjson::kObjectType);
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(ecdbUpdatesJson, json.GetAllocator(), entry.first.GetRulesetId().c_str(), rapidjson::kObjectType);
            RapidJsonValueR hierarchyUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "hierarchy", rapidjson::kArrayType);
            for (HierarchyUpdateRecord const& record : entry.second)
                hierarchyUpdatesJson.PushBack(record.AsJson(&json.GetAllocator()), json.GetAllocator());
            }
        m_partialHierarchyUpdates.clear();
        for (UpdateRecordKey const& key : m_fullHierarchyUpdates)
            {
            RapidJsonValueR ecdbUpdatesJson = GetMemberJson(json, json.GetAllocator(), key.GetECDbFileName().c_str(), rapidjson::kObjectType);
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(ecdbUpdatesJson, json.GetAllocator(), key.GetRulesetId().c_str(), rapidjson::kObjectType);
            RapidJsonValueR hierarchyUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "hierarchy", rapidjson::kStringType);
            hierarchyUpdatesJson.SetString("FULL");
            }
        m_fullHierarchyUpdates.clear();
        for (UpdateRecordKey const& key : m_fullContentUpdates)
            {
            RapidJsonValueR ecdbUpdatesJson = GetMemberJson(json, json.GetAllocator(), key.GetECDbFileName().c_str(), rapidjson::kObjectType);
            RapidJsonValueR rulesetUpdatesJson = GetMemberJson(ecdbUpdatesJson, json.GetAllocator(), key.GetRulesetId().c_str(), rapidjson::kObjectType);
            RapidJsonValueR contentUpdatesJson = GetMemberJson(rulesetUpdatesJson, json.GetAllocator(), "content", rapidjson::kStringType);
            contentUpdatesJson.SetString("FULL");
            }
        m_fullContentUpdates.clear();
        return json;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IModelJsECPresentationUpdateRecordsHandler::IModelJsECPresentationUpdateRecordsHandler()
    : m_finalReport(std::make_unique<FinalReport>())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IModelJsECPresentationUpdateRecordsHandler::~IModelJsECPresentationUpdateRecordsHandler()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Start()
    {
    m_wipReportMutex.lock();
    m_wipReport = std::make_unique<WipReport>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Accept(HierarchyUpdateRecord const& record)
    {
    m_wipReport->Accept(record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Accept(FullUpdateRecord const& record)
    {
    m_wipReport->Accept(record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationUpdateRecordsHandler::_Finish()
    {
    m_finalReport->Accept(*m_wipReport);
    m_wipReport.reset();
    m_wipReportMutex.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationUpdateRecordsHandler::GetReport()
    {
    return m_finalReport->BuildJsonAndReset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationHierarchiesCompareRecordsHandler::GetReport(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document report(rapidjson::kArrayType, allocator);
    for (auto const& record : m_records)
        report.PushBack(record.AsJson(&report.GetAllocator()), report.GetAllocator());
    return report;
    }
