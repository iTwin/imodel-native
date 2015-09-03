/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecpresentationtypedefs.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/

#pragma once

#include <ECObjects/ECObjects.h>
#include <Bentley/RefCounted.h>

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

BEGIN_BENTLEY_NAMESPACE namespace Dgn {
    DEFINE_POINTER_SUFFIX_TYPEDEFS(ECQuery)
} END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
typedef RefCountedPtr<IAUIItem>                         IAUIItemPtr;
typedef RefCountedPtr<IECPresentationViewDefinition>    IECPresentationViewDefinitionPtr;
typedef RefCountedPtr<IECContentDefinition>             IECContentDefinitionPtr;
typedef RefCountedPtr<IECNativeImage>                   IECNativeImagePtr;
END_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_END__*/
