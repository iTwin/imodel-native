#include <gl/glew.h>

#include "CachedRenderer.h"
#include "3SMGL.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <GL/GLU.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH

bool ScalableMesh::s_dontkeepIntermediateDisplayData = false;
bool ScalableMesh::s_useVBO = false;
//-------------------------------------------------------------------------------------------------
CachedRenderer::CachedRenderer()
//-------------------------------------------------------------------------------------------------
{
	m_bUseLight = false;
	m_bDrawBox = false;
	m_bDrawFaces = true; // draw mesh
	m_bTextured = true;
	m_bWireframe = false;
	m_bUseLight = false;
	m_nQueryId = -1;
	m_avLevel = 0;

	glewInit();
}
//-------------------------------------------------------------------------------------------------
void CachedRenderer::startQuery(IScalableMeshPtr sm, Camera *camera)
//-------------------------------------------------------------------------------------------------
{
	if (!camera) return;

	if (!(m_lastCamera == *camera)) // check if camera has changed, else no need to restart a new query
	{
		//view->camera().resetDirtyFlag();

		// stop first the previous query
		StatusInt status;
		if (progressiveQueryEngine != nullptr && m_nQueryId != -1)
		{
			status = progressiveQueryEngine->StopQuery(m_nQueryId);
		}

		m_nQueryId = 1;
   
		IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());
	
		// Setting screen Resolution preferrences
		// Use rather MaxPixelError for Reality meshes
		viewDependentQueryParams->SetMaxPixelError(1.0); // this will increase/decrease the returned resolution
														 //viewDependentQueryParams->SetMinScreenPixelsPerPoint(s_minScreenPixelsPerPoint); // for non-reality meshes (terrain)

		bvector<bool> clips; // not used in query - TODO remove from interface

							  
		DMatrix4d World2View;
		camera->getTransformationMatrix(World2View); // construct a Matrix that converts any point in World pos into a Screen point 
		viewDependentQueryParams->SetRootToViewMatrix(World2View.coff);

		// Create the scene clippings planes ------------------------------------------
		ClipVectorPtr clipVector(ClipVector::Create());
		bvector<ClipPlane> clipPlanes;

		camera->createCameraFrustumClips(clipPlanes);
		
		clipVector->Create();
		ClipPlaneSet planeSet(clipPlanes.data(), clipPlanes.size());
		clipVector->AppendPlanes(clipVector, planeSet);
		viewDependentQueryParams->SetViewClipVector(clipVector);

		// Start the Progressive query ----------------------------------------------
		status = progressiveQueryEngine->StartQuery(m_nQueryId,    //Picked by caller
			viewDependentQueryParams,
			meshNodes,          //Not used
			m_bTextured,		//Whether to load textures or not
			clips,              //Not used
			sm
		);
		m_lastCamera = *camera;
	}
}
//-------------------------------------------------------------------------------------------------
bool CachedRenderer::drawScalableMesh(IScalableMeshPtr sm, Camera *camera, int texUnitUniform)
//-------------------------------------------------------------------------------------------------
{
	if (sm == nullptr) return true;

	glLineWidth(1.0);

	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glPointSize(2.0f);

	// convert 3SM Camera into OGL camera : need to translate to the node center
	//	DPoint3d camEye, camTarget, camUp;
	//	view->camera().getEyeTargetUp(camEye, camTarget, camUp);

	if (m_bUseLight || !m_bTextured)
	{
		glEnable(GL_LIGHTING);
		DPoint3d lightPoint, lx, ly;
		camera->getBasisVectorsFromGL( lx, ly, lightPoint );
		float lightPos[4] = { 2.f, 1.f, 5.f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	}
	else
		glDisable(GL_LIGHTING);

	if (!displayNodesCache.m_DisplayMgrPtr.IsValid())
	{
		displayNodesCache.Init();
	}

	displayNodesCache.m_DisplayMgrPtr->m_Center = CoordinateSystem::instance().worldCenter();

	if (progressiveQueryEngine == nullptr)
	{
		IScalableMeshDisplayCacheManagerPtr DCMPtr = displayNodesCache.m_DisplayMgrPtr;
		progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(sm, DCMPtr);
	}

	// start the query according to camera and scene
	startQuery(sm, camera);

	bool iscomplete = false;
	bool m_bDisplayMesh = true;
	if (m_bDisplayMesh)
	{
		if (m_bTextured)
		{
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else
		{
			glColor3f(0.35f, 0.35f, 0.35f);
		}
		if (progressiveQueryEngine->IsQueryComplete(m_nQueryId))  // query is complete, we can draw all visible nodes
		{
			meshNodes.clear();
			StatusInt status = progressiveQueryEngine->GetRequiredNodes(meshNodes, m_nQueryId);
			
			drawMeshNodes(meshNodes, m_bDrawFaces ? 1 : 0, false);

			if (m_bWireframe)
			{
				glColor3f(1.0f, 0.3f, 1.0f);
				drawMeshNodes(meshNodes, 1, true, texUnitUniform);
			}
			iscomplete = true;
		}
		else
		{
			StatusInt status;
			if (m_bTextured)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else
			{
				glColor3f(0.35f, 0.35f, 0.35f);
			}

			overviewNodes.clear(); //These nodes are placeholders while the nodes are loaded
			status = progressiveQueryEngine->GetOverviewNodes(overviewNodes, m_nQueryId);
			drawMeshNodes(overviewNodes, m_bDrawFaces ? 1 : 0, false);

			if (m_bWireframe)
			{
				glColor3f(1.0f, 0.3f, 1.0f);
				drawMeshNodes(overviewNodes, 1, true);
			}

			meshNodes.clear(); //These nodes are at the right LOD for the view
			status = progressiveQueryEngine->GetRequiredNodes(meshNodes, m_nQueryId); // we get only the few available
			
			if (m_bTextured)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else
			{
				glColor3f(0.35f, 0.35f, 0.35f);
			}
			drawMeshNodes(meshNodes, m_bDrawFaces ? 1 : 0, false);

			if (m_bWireframe)
			{
				glColor3f(1.0f, 0.3f, 1.0f);
				drawMeshNodes(overviewNodes, 1, true, texUnitUniform);			
			}
		}
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	
	glDisable(GL_LIGHTING);

	return iscomplete;
}
//-------------------------------------------------------------------------------------------------
void CachedRenderer::drawBackground()
//-------------------------------------------------------------------------------------------------
{
	glClearColor(0, 0, 0.2f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
//-------------------------------------------------------------------------------------------------
void CachedRenderer::reset()
//-------------------------------------------------------------------------------------------------
{
	meshNodes.clear();
	overviewNodes.clear();
	if (progressiveQueryEngine != nullptr && m_nQueryId != -1)
	{
		progressiveQueryEngine->StopQuery(m_nQueryId);
	}
	progressiveQueryEngine = nullptr;
}
//-------------------------------------------------------------------------------------------------
void CachedRenderer::createTexture(SmCachedDisplayTexture * cacheTex)
//-------------------------------------------------------------------------------------------------
{
	if (cacheTex == nullptr)
		return;
	if ((cacheTex->m_textureId != -1) && glIsTexture(cacheTex->m_textureId))
	{
		glBindTexture(GL_TEXTURE_2D, cacheTex->m_textureId);
		return; // texture already exists -> only bind
	}

	// The texture we're going to render needs to be created
	GLuint renderTextureId;
	glGenTextures(1, &renderTextureId);

	cacheTex->m_textureId = renderTextureId; // store the id for future bindings

											 // "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderTextureId);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cacheTex->width, cacheTex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, cacheTex->texels);

	// Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (ScalableMesh::s_dontkeepIntermediateDisplayData)
	{
		delete[] cacheTex->texels;
		cacheTex->texels = nullptr;
	}
}
//-------------------------------------------------------------------------------------------------
void CachedRenderer::drawMeshNodes(Bentley::bvector<IScalableMeshCachedDisplayNodePtr>& _meshNodes, int displayElement, bool bWire, int texUnitUniform)
//-------------------------------------------------------------------------------------------------
{
	if (bWire)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_MATERIAL);
		glColor3f(0.3f, 0.0f, 0.5f);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);//GL_LINE);// 
		glDisable(GL_COLOR_MATERIAL);
		
		if (m_bTextured)
			glEnable(GL_TEXTURE_2D);
		
		GLfloat ambient[] = { 1.0f, 1.0f, 0.2f, 1.0f };
		GLfloat diffuse[] = { 0.8f, 0.5f, 0.3f, 1.0f };
		GLfloat specular[] = { 0.0f, 0.0f, 1.0, 1.0f };
		GLfloat materialEmission[] = { 1.0f,1.0f, 0.0f, 1.0f };
		GLfloat shine = 20.0f;
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMaterialf(GL_FRONT, GL_SHININESS, shine);
	}

	int nb = 0;
	m_avLevel = 0;
	for (auto meshNode : _meshNodes)
	{
		bvector<SmCachedDisplayMesh*> cachedMesh;
		bvector<bpair<bool, uint64_t>> texIds;
		meshNode->GetCachedMeshes(cachedMesh, texIds);
		m_avLevel += meshNode->GetLevel();

		for (auto mesh : cachedMesh)
		{
			if (displayElement == 2)
			{
				glBegin(GL_POINTS);
				for (size_t k = 0; k < mesh->nbPoints; k++)
				{
					glVertex3d(mesh->points[k].x, mesh->points[k].y, mesh->points[k].z);
				}
				glEnd();
			}

			if (displayElement == 1)
			{
				// get or create the texture
				if (m_bTextured)
				{
					SmCachedDisplayTexture *pTex = mesh->cachedTexture;
					createTexture(pTex);

					if (texUnitUniform>=0)
						glUniform1i(texUnitUniform, pTex->m_textureId);
				}

				//DrawMeshPart(mesh);
				if (!ScalableMesh::s_useVBO)
					drawMeshArray(mesh); // draw one mesh node
				else
					drawOrTransferMeshData(mesh);
			}
		}
	}
	m_avLevel /= (float)_meshNodes.size();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);//GL_LINE);// 
}

//-------------------------------------------------------------------------------------------------
void CachedRenderer::drawOrTransferMeshData(SmCachedDisplayMesh * aNodeMesh)
//-------------------------------------------------------------------------------------------------
{
	if (aNodeMesh->displayIndexIBO == -1)
	{
		glGenBuffers(1, (unsigned int*)&aNodeMesh->displayIndex);
		glBindBuffer(GL_ARRAY_BUFFER, aNodeMesh->displayIndex);
		float * interleaved = new float[5 * aNodeMesh->nbPoints];
		for (size_t i = 0, j=0; i < aNodeMesh->nbPoints; ++i,j+=5)
		{
			interleaved[j] = aNodeMesh->positions[i*3];
			interleaved[j+1] = aNodeMesh->positions[i * 3+1];
			interleaved[j + 2] = aNodeMesh->positions[i * 3 + 2];
			interleaved[j + 3] = aNodeMesh->uvs[i * 2];
			interleaved[j + 4] = aNodeMesh->uvs[i * 2+1];
		}

		glBufferData(GL_ARRAY_BUFFER, aNodeMesh->nbPoints*5*sizeof(GLfloat), interleaved, GL_STATIC_DRAW);
		delete[] interleaved;

		delete[] aNodeMesh->positions;
		aNodeMesh->positions = nullptr;
		delete[] aNodeMesh->uvs;

		aNodeMesh->uvs = nullptr;

		glGenBuffers(1, (unsigned int*)&aNodeMesh->displayIndexIBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aNodeMesh->displayIndexIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, aNodeMesh->nbTriangles * 3 * sizeof(GLuint), aNodeMesh->indices, GL_STATIC_DRAW);

		delete[] aNodeMesh->indices;
		aNodeMesh->indices = nullptr;
	}
	glPushMatrix();
	glTranslated(aNodeMesh->localCenter.x, aNodeMesh->localCenter.y, aNodeMesh->localCenter.z); // vertices are stored relative to a node center
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, aNodeMesh->displayIndex); 
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aNodeMesh->displayIndexIBO);
	glDrawElements(GL_TRIANGLES, 3*aNodeMesh->nbTriangles, GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glPopMatrix();
}

//-------------------------------------------------------------------------------------------------
void CachedRenderer::drawMeshArray(SmCachedDisplayMesh * aNodeMesh)
//-------------------------------------------------------------------------------------------------
{
	if (aNodeMesh->displayIndex > 0)
	{
		// draw the display list
		glCallList(aNodeMesh->displayIndex);
		return;
	}

	if (s_dontkeepIntermediateDisplayData)
	{
		aNodeMesh->displayIndex = (int)aNodeMesh->nodeId;
		glNewList(aNodeMesh->displayIndex, GL_COMPILE);
	}
	glPushMatrix();
	glTranslated(aNodeMesh->localCenter.x, aNodeMesh->localCenter.y, aNodeMesh->localCenter.z); // vertices are stored relative to a node center

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, aNodeMesh->positions);
	glTexCoordPointer(2, GL_FLOAT, 0, aNodeMesh->uvs);

	glDrawElements(GL_TRIANGLES, 3 * aNodeMesh->nbTriangles, GL_UNSIGNED_INT, aNodeMesh->indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPopMatrix();

	glEndList();


	glCallList(aNodeMesh->displayIndex); // draw

	if (s_dontkeepIntermediateDisplayData)
	{
		delete[] aNodeMesh->positions;
		aNodeMesh->positions = nullptr;
		delete[] aNodeMesh->indices;
		aNodeMesh->indices = nullptr;

		if (aNodeMesh->uvs)
			delete[] aNodeMesh->uvs;
		aNodeMesh->uvs = nullptr;
	}

}

//-------------------------------------------------------------------------------------------------
void CachedRenderer::createCameraFrustumClips(bvector<ClipPlane> &clipPlanes, Camera *camera)
//-------------------------------------------------------------------------------------------------
{
	glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix
	
	DPoint3d camEye, camTarget, camUp;
	camera->getEyeTargetUpFromGL(camEye, camTarget, camUp);

	DPoint3d worldEye = camEye + DVec3d::From(CoordinateSystem::instance().worldCenter());

	DVec3d camDir = camTarget - camEye;
	camDir.Normalize();
	DVec3d camX, camY;
	camX.crossProduct(&camUp, &camDir);
	camY.crossProduct(&camX, &camDir);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	double size = camera->m_farPlane;
	double tangent = tan(0.5 * camera->m_fov * PI / 180.0);
	double sizeX = size * tangent * 2.0;
	double sizeY = sizeX * (double)camera->m_viewportH / camera->m_viewportW;

	DPoint3d P0 = camEye; // recentered
	DPoint3d P1 = P0 + size * camDir - sizeX * camX - sizeY * camY;
	DPoint3d P2 = P0 + size * camDir + sizeX * camX - sizeY * camY;
	DPoint3d P3 = P0 + size * camDir + sizeX * camX + sizeY * camY;
	DPoint3d P4 = P0 + size * camDir - sizeX * camX + sizeY * camY;

	DVec3d V12; V12.NormalizedCrossProduct(P2 - P0, P1 - P0);
	DVec3d V23; V23.NormalizedCrossProduct(P3 - P0, P2 - P0);
	DVec3d V34; V34.NormalizedCrossProduct(P4 - P0, P3 - P0);
	DVec3d V41; V41.NormalizedCrossProduct(P1 - P0, P4 - P0);

	P0 = camEye; // eye in 3SM World
	clipPlanes.push_back(ClipPlane(V12, P0));  // normal needs to be inward
	clipPlanes.push_back(ClipPlane(V23, P0));
	clipPlanes.push_back(ClipPlane(V34, P0));
	clipPlanes.push_back(ClipPlane(V41, P0));
	clipPlanes.push_back(ClipPlane(camDir, P0)); // keep in front of camera

												 // one can add the far clip plane if wanted here
												 // clipPlanes.push_back( ClipPlane(-camDir, P0+size*camDir) ); 
}
//-------------------------------------------------------------------------------------------------
float CachedRenderer::averageLevel() const
//-------------------------------------------------------------------------------------------------
{
	return m_avLevel;
}
