/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! @bsiclass                                   Grigas.Petraitis                12/2017
//=======================================================================================
struct ECPresentationUtils
{
    static RulesDrivenECPresentationManager* CreatePresentationManager(IConnectionManagerR, Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin&);
    static void SetupRulesetDirectories(RulesDrivenECPresentationManager&, bvector<Utf8String> const&);
    static void GetRootNodesCount(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetRootNodes(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetChildrenCount(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetChildren(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetNodePaths(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetFilteredNodesPaths(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetContentDescriptor(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetContent(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetContentSetSize(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void GetDistinctValues(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
    static void SaveValueChange(IECPresentationManagerR, ECDbR, JsonValueCR params, rapidjson::Document& response);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
