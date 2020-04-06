/*--------------------------------------------------------------------------------------+
|
|     $Source: Presentation/UpdateRecordsHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2020
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
