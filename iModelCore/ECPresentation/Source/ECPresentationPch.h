/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/ECPresentationPch.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __ECPRESENTATIONPCH_H__
#define __ECPRESENTATIONPCH_H__

// external includes
#include <Bentley/Bentley.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <rapidjson/rapidjson.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>

// ECPresentation includes
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/SelectionManager.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>

// used namespaces
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#endif // __ECPRESENTATIONPCH_H__