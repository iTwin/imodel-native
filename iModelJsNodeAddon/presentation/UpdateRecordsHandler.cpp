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
    bvector<FullUpdateRecord> m_fullUpdates;
public:
    void Accept(FullUpdateRecord const& record) {m_fullUpdates.push_back(record);}
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

private:
    BeMutex m_mutex;
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

public:
    void Accept(WipReport const& wip)
        {
        BeMutexHolder lock(m_mutex);
        for (FullUpdateRecord const& record : wip.GetFullUpdates())
            {
            UpdateRecordKey key(GetRulesetId(record), GetECDbFileName(record));
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Hierarchy == record.GetUpdateTarget())
                {
                m_fullHierarchyUpdates.insert(key);
                }
            if (FullUpdateRecord::UpdateTarget::Both == record.GetUpdateTarget() || FullUpdateRecord::UpdateTarget::Content == record.GetUpdateTarget())
                {
                m_fullContentUpdates.insert(key);
                }
            }
        }

    rapidjson::Document BuildJsonAndReset()
        {
        BeMutexHolder lock(m_mutex);
        if (m_fullHierarchyUpdates.empty() && m_fullContentUpdates.empty())
            return rapidjson::Document();

        rapidjson::Document json(rapidjson::kObjectType);
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
