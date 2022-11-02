/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationUpdateRecordsHandler : IUpdateRecordsHandler
{
    struct WipReport;
    struct FinalReport;
private:
    BeMutex m_wipReportMutex;
    std::unique_ptr<WipReport> m_wipReport;
    std::unique_ptr<FinalReport> m_finalReport;
protected:
    void _Start() override;
    void _Accept(HierarchyUpdateRecord const&) override;
    void _Accept(FullUpdateRecord const&) override;
    void _Finish() override;
public:
    IModelJsECPresentationUpdateRecordsHandler();
    ~IModelJsECPresentationUpdateRecordsHandler();
    rapidjson::Document GetReport();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationHierarchiesCompareRecordsHandler : IHierarchyChangeRecordsHandler
{
private:
    bvector<HierarchyChangeRecord> m_records;
protected:
    void _Start() override {m_records.clear();}
    void _Accept(HierarchyChangeRecord const& record) override {m_records.push_back(record);}
    void _Finish() override {}
public:
    rapidjson::Document GetReport(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};
END_BENTLEY_ECPRESENTATION_NAMESPACE
