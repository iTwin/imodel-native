#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#ifdef VANCOUVER_API
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME
#else
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME::Dgn
#endif
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IClipDefinitionDataProvider;

typedef RefCountedPtr<IClipDefinitionDataProvider> IClipDefinitionDataProviderPtr;

struct IClipDefinitionDataProvider: virtual public RefCountedBase
{

public:
	virtual ~IClipDefinitionDataProvider(){};

	virtual void GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id) = 0;
	virtual void GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id, SMNonDestructiveClipType& type) = 0;
    virtual void GetClipVector(CLIP_VECTOR_NAMESPACE::ClipVectorPtr& poly, uint64_t id, SMNonDestructiveClipType& type) = 0;
	virtual void SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id, SMNonDestructiveClipType type) = 0;
    virtual void SetClipVector(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& poly, uint64_t id, SMNonDestructiveClipType type) = 0;
	virtual void SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id) = 0;
	virtual void RemoveClipPolygon(uint64_t id) = 0;

	virtual void RemoveTerrainRegion(uint64_t id) = 0;
	virtual void GetTerrainRegion(bvector<DPoint3d>& poly, uint64_t id) = 0;
	virtual void SetTerrainRegion(const bvector<DPoint3d>& poly, uint64_t id) = 0;

	virtual void SetTerrainRegionName(const Utf8String& name, uint64_t id) = 0;
	virtual void GetTerrainRegionName(Utf8String& name, uint64_t id) = 0;
	virtual void RemoveTerrainRegionName(uint64_t id) = 0;

	virtual void ListClipIDs(bvector<uint64_t>& ids) = 0;
	virtual void ListTerrainRegionIDs(bvector<uint64_t>& ids) = 0;
};


END_BENTLEY_SCALABLEMESH_NAMESPACE