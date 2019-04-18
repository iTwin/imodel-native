/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

//  These statements must occur before anything includes BeAssert.h.  Otherwise, the
//  include statement has no effect due to the "pragma once" leaving the assertions undefined.
#if defined(BEGIN_BENTLEY_NAMESPACE)
    #error This should be the first code that includes BeAssert.h
#endif

//  Prevent VersionedDgnV8Api/Bentley/BeConsole.h from defining wprintf, etc.
#define NO_IODEFS

//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyM0200".
#ifndef NO_USING_NAMESPACE_BENTLEY
#define NO_USING_NAMESPACE_BENTLEY 1
#endif // !NO_USING_NAMESPACE_BENTLEY

#ifndef VERSIONEDv8APINATIVESUPPORT
#define VERSIONEDv8APIINCLUDE(header) < VersionedDgnV8Api/header >
#else
#define VERSIONEDv8APIINCLUDE(header) < header >
#endif

#include VERSIONEDv8APIINCLUDE(Bentley/BeAssert.h)

#ifndef VERSIONEDv8APINATIVESUPPORT
#undef BeAssert
#undef BeDataAssert
#undef BeAssertOnce
#undef BeDataAssertOnce
#endif

#include <Bentley/BeAssert.h>

#define DGNV8_WSTRING_LEGACY_SUPPORT

#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnPlatform.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnFile.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnPlatformLib.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnFileIO/BentleyDgn.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/ViewGroup.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnECManager.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/ITransactionHandler.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/ExtendedElementHandler.h)
#include VERSIONEDv8APIINCLUDE(DgnGeoCoord/DgnGeoCoord.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DgnLinks.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/ProxyDisplayCore.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DetailingSymbol/DetailingSymbolCore.r.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DelegatedElementECEnabler.h)
#include VERSIONEDv8APIINCLUDE(DgnPlatform/DetailingSymbol/DetailingSymbol.h)

namespace DgnV8Api    = Bentley::DgnPlatform;
namespace ECObjectsV8 = Bentley::ECN;

typedef DgnV8Api::DgnFile&          DgnV8FileR;
typedef DgnV8Api::DgnFile const&    DgnV8FileCR;
typedef DgnV8Api::DgnFile*          DgnV8FileP;
typedef DgnV8Api::DgnFile const*    DgnV8FileCP;
typedef DgnV8Api::DgnFileLink const& DgnV8FileLinkCR;
typedef DgnV8Api::DgnLinkTreeNode const& DgnV8LinkTreeNodeCR;
typedef DgnV8Api::ViewInfo&         DgnV8ViewInfoR;
typedef DgnV8Api::ViewInfo const&   DgnV8ViewInfoCR;
typedef DgnV8Api::ViewInfo*         DgnV8ViewInfoP;
typedef DgnV8Api::ViewInfo const*   DgnV8ViewInfoCP;
typedef DgnV8Api::DgnModel&         DgnV8ModelR;
typedef DgnV8Api::DgnModel const&   DgnV8ModelCR;
typedef DgnV8Api::DgnModel*         DgnV8ModelP;
typedef DgnV8Api::DgnModel const*   DgnV8ModelCP;
typedef DgnV8Api::DgnModelRef&         DgnV8ModelRefR;
typedef DgnV8Api::DgnModelRef const&   DgnV8ModelRefCR;
typedef DgnV8Api::DgnModelRef*         DgnV8ModelRefP;
typedef DgnV8Api::DgnModelRef const*   DgnV8ModelRefCP;
typedef DgnV8Api::DgnURLLink const& DgnV8URLLinkCR;
typedef DgnV8Api::ElementHandle const& DgnV8EhCR;
typedef DgnV8Api::ElementHandle const* DgnV8EhCP;
typedef DgnV8Api::EditElementHandle& DgnV8EehR;
typedef DgnV8Api::SchemaInfo&       DgnV8SchemaInfoR;

