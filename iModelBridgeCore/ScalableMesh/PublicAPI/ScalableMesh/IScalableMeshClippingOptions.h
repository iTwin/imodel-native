/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IClipDefinitionDataProvider.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshClippingOptions;

typedef RefCountedPtr<IScalableMeshClippingOptions> IScalableMeshClippingOptionsPtr;

/*=================================================================================**//**
 * @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshClippingOptions abstract : virtual public RefCountedBase
{
private:

protected:

	virtual void _SetShouldRegenerateStaleClipFiles(bool regenerateClipFiles) = 0;

	virtual void _SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider) = 0;

public:

	BENTLEY_SM_EXPORT void SetShouldRegenerateStaleClipFiles(bool regenerateClipFiles);

	//Set an application-implemented clip provider to obtain the clip polygons asynchronously when needed. 
	BENTLEY_SM_EXPORT void SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider);
};

END_BENTLEY_SCALABLEMESH_NAMESPACE
