/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auipresentationapi.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjectsAPI.h>

EC_TYPEDEFS (IAUIItem);
EC_TYPEDEFS (IAUIItemInfo);
EC_TYPEDEFS (IAUIDataContext);
EC_TYPEDEFS (IUICommand);
EC_TYPEDEFS (UICommand);
EC_TYPEDEFS (IJournalItem);
EC_TYPEDEFS (ECPresentationProvider);
EC_TYPEDEFS (ECPresentationCommandProvider);
EC_TYPEDEFS (IJournalProvider);
EC_TYPEDEFS (ECPresentationManager);
EC_TYPEDEFS (IECViewDefinition);
EC_TYPEDEFS (IECViewDefinitionProvider);
EC_TYPEDEFS (IECContentDefinition);
EC_TYPEDEFS (IAUIContentServiceProvider);
EC_TYPEDEFS (ECImageKey);
EC_TYPEDEFS (IECNativeImage);
EC_TYPEDEFS (ECNativeImage);
EC_TYPEDEFS (ECPresentationImageProvider);

namespace Bentley { namespace DgnPlatform {
    struct ECQuery;
    typedef ECQuery const*      ECQueryCP;
    }}

BEGIN_BENTLEY_EC_NAMESPACE
typedef RefCountedPtr<IAUIItem>             IAUIItemPtr;
typedef RefCountedPtr<IECViewDefinition>    IECViewDefinitionPtr;
typedef RefCountedPtr<IECContentDefinition> IECContentDefinitionPtr;
typedef RefCountedPtr<IECNativeImage>       IECNativeImagePtr;
END_BENTLEY_EC_NAMESPACE

#include <EcPresentation/auicommand.h>
#include <EcPresentation/auiitem.h>
#include <EcPresentation/auiprovider.h>
#include <EcPresentation/auijournal.h>
#include <EcPresentation/auipresentationmgr.h>
#include <EcPresentation/ecviewdefinition.h>
#include <EcPresentation/eccontentdefinition.h>
#include <EcPresentation/ecimage.h>
