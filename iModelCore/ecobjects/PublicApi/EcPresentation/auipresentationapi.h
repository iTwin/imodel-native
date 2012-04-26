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
EC_TYPEDEFS (IAUIProvider);
EC_TYPEDEFS (IUICommandProvider);
EC_TYPEDEFS (IJournalProvider);
EC_TYPEDEFS (UIPresentationManager);
EC_TYPEDEFS (IECViewDefinition);
EC_TYPEDEFS (IECViewDefinitionProvider);
EC_TYPEDEFS (IECContentDefinition);
EC_TYPEDEFS (IAUIContentServiceProvider);

namespace Bentley { namespace DgnPlatform {
    struct ECQuery;
    typedef ECQuery const*      ECQueryCP;
    }}

BEGIN_BENTLEY_EC_NAMESPACE
typedef RefCountedPtr<IAUIItem>                 IAUIItemPtr;
typedef RefCountedPtr<IECViewDefinition>        IECViewDefinitionPtr;
typedef RefCountedPtr<IECContentDefinition> IECContentDefinitionPtr;
END_BENTLEY_EC_NAMESPACE

#include <EcPresentation/auicommand.h>
#include <EcPresentation/auiitem.h>
#include <EcPresentation/auiprovider.h>
#include <EcPresentation/auijournal.h>
#include <EcPresentation/auipresentationmgr.h>
#include <EcPresentation/ecviewdefinition.h>
#include <EcPresentation/eccontentdefinition.h>
