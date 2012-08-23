/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecpresentationtypedefs.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjects.h>

EC_TYPEDEFS (IAUIItem);
EC_TYPEDEFS (IAUIItemInfo);
EC_TYPEDEFS (IAUIDataContext);
EC_TYPEDEFS (IUICommand);
EC_TYPEDEFS (UICommand);
EC_TYPEDEFS (IJournalItem);
EC_TYPEDEFS (IECPresentationProvider);
EC_TYPEDEFS (ECPresentationCommandProvider);
EC_TYPEDEFS (IJournalProvider);
EC_TYPEDEFS (ECPresentationManager);
EC_TYPEDEFS (IECPresentationViewDefinition);
EC_TYPEDEFS (IECPresentationViewProvider);
EC_TYPEDEFS (IECContentDefinition);
EC_TYPEDEFS (IAUIContentServiceProvider);
EC_TYPEDEFS (ECImageKey);
EC_TYPEDEFS (IECNativeImage);
EC_TYPEDEFS (ECNativeImage);
EC_TYPEDEFS (ECPresentationImageProvider);
EC_TYPEDEFS (ECPresentationLocalizationProvider);
EC_TYPEDEFS (IECPresentationViewTransform);
EC_TYPEDEFS (IECPresentationUIItem);
EC_TYPEDEFS (IECPresentationUIItemInfo);
EC_TYPEDEFS (ECPresentationMenuItem);
EC_TYPEDEFS (ECPresentationMenuItemInfo);
EC_TYPEDEFS (ECEvent);
EC_TYPEDEFS (ECSelectionEvent);
EC_TYPEDEFS (ECSelectionListener);
EC_TYPEDEFS (ECInstanceIterableDataContext);

namespace Bentley { namespace DgnPlatform {
    struct ECQuery;
    typedef ECQuery const*      ECQueryCP;
    }}

BEGIN_BENTLEY_EC_NAMESPACE
typedef RefCountedPtr<IAUIItem>             IAUIItemPtr;
typedef RefCountedPtr<IECPresentationViewDefinition>    IECPresentationViewDefinitionPtr;
typedef RefCountedPtr<IECContentDefinition> IECContentDefinitionPtr;
typedef RefCountedPtr<IECNativeImage>       IECNativeImagePtr;
END_BENTLEY_EC_NAMESPACE