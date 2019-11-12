/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ScalableMeshClippingOptions.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

void IScalableMeshClippingOptions::SetShouldRegenerateStaleClipFiles(bool regenerateClipFiles)
{
	return _SetShouldRegenerateStaleClipFiles(regenerateClipFiles);
}

void IScalableMeshClippingOptions::SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider)
{
	return _SetClipDefinitionsProvider(provider);
}

void IScalableMeshClippingOptions::SetHasOverlappingClips(bool overlaps)
{
	return _SetHasOverlappingClips(overlaps);
}

void ScalableMeshClippingOptions::_SetShouldRegenerateStaleClipFiles(bool regenerateClipFiles)
{
	m_shouldRegenerateClipFiles = regenerateClipFiles;
	m_fieldChangedCallback(this);
}

void ScalableMeshClippingOptions::_SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider)
{
	m_clipProvider = provider;
	m_fieldChangedCallback(this);
}

void ScalableMeshClippingOptions::_SetHasOverlappingClips(bool overlaps)
{
	m_allowOverlappingClips = overlaps;
	m_fieldChangedCallback(this);
}

ScalableMeshClippingOptions::ScalableMeshClippingOptions(std::function<void(const ScalableMeshClippingOptions*)> onDataChanged, bool shouldRegenerateClipFiles)
{
	m_allowOverlappingClips = false;
	m_shouldRegenerateClipFiles = shouldRegenerateClipFiles;
	m_fieldChangedCallback = onDataChanged;
}

bool ScalableMeshClippingOptions::ShouldRegenerateStaleClipFiles() const
{
	return m_shouldRegenerateClipFiles;
}

IClipDefinitionDataProviderPtr ScalableMeshClippingOptions::GetClipDefinitionsProvider() const
{
	return m_clipProvider;
}

bool ScalableMeshClippingOptions::HasOverlappingClips() const
{
	return m_allowOverlappingClips;
}

END_BENTLEY_SCALABLEMESH_NAMESPACE
