/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMeshClippingOptions.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshClippingOptions : public RefCounted<IScalableMeshClippingOptions>
{
private:

	IClipDefinitionDataProviderPtr m_clipProvider;
	bool          m_shouldRegenerateClipFiles;
	bool m_allowOverlappingClips;

	std::function<void(const ScalableMeshClippingOptions*)> m_fieldChangedCallback;

protected:
	virtual void _SetShouldRegenerateStaleClipFiles(bool regenerateClipFiles) override;

	virtual void _SetClipDefinitionsProvider(const IClipDefinitionDataProviderPtr& provider) override;

	virtual void _SetHasOverlappingClips(bool overlaps) override;

public:

	void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted(size); }
	void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
	void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
	void    operator delete [](void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

	ScalableMeshClippingOptions() : m_allowOverlappingClips(false), m_shouldRegenerateClipFiles(false){};
	ScalableMeshClippingOptions(std::function<void(const ScalableMeshClippingOptions*)> onDataChanged, bool shouldRegenerateClipFiles=false);

	bool ShouldRegenerateStaleClipFiles() const;
	
	IClipDefinitionDataProviderPtr GetClipDefinitionsProvider() const;

	bool HasOverlappingClips() const;

};

END_BENTLEY_SCALABLEMESH_NAMESPACE
