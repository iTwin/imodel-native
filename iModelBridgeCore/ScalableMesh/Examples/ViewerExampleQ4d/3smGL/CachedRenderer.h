#pragma once

#include "SMDisplayMgr.h"
#include "Camera.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

struct GL_Camera;

class CachedRenderer
{
public:
	CachedRenderer();

	void startQuery(IScalableMeshPtr sm, Camera *camera);
	bool drawScalableMesh(IScalableMeshPtr sm, Camera *camera, int texUnitUniform=-1);	// returns true if complete
	void drawBackground();

	void reset();

	void createTexture(SmCachedDisplayTexture* cacheTex);
	void drawMeshNodes(Bentley::bvector<IScalableMeshCachedDisplayNodePtr>& _meshNodes, int displayElement, bool bWire, int texUnitUniform=-1); // Draw list of nodes
	void drawMeshArray(SmCachedDisplayMesh* aNodeMesh);
	void createCameraFrustumClips(bvector<ClipPlane>& clipPlanes, Camera *camera);

	float averageLevel() const;

	bool m_bDrawFaces;
	bool m_bDrawBox;
	bool m_bTextured;
	bool m_bWireframe;

private:
	IScalableMeshProgressiveQueryEnginePtr progressiveQueryEngine;
	bvector<IScalableMeshCachedDisplayNodePtr> meshNodes;       // the returned query nodes
	bvector<IScalableMeshCachedDisplayNodePtr> overviewNodes;   // the nodes to display when a query is not complete
	DisplayCacheHandler displayNodesCache;                      // the cache container for displaying mesh nodes
	
	int m_nQueryId;
	bool m_bUseLight;
	bool m_bSyncCam;
	bool m_bMove3SMCam;
	float m_avLevel;
	Camera m_lastCamera;
};