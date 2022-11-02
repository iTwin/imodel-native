/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef __ECPRESENTATIONPCH_H__
#define __ECPRESENTATIONPCH_H__

// external includes
#include <memory>
#include <Bentley/Bentley.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Logging.h>
#include <BeRapidJson/BeRapidJson.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>

// ECPresentation includes
#include <ECPresentation/Iterators.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/ECPresentationManagerRequestParams.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/PresentationRules.h>

// used namespaces
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#endif // __ECPRESENTATIONPCH_H__
