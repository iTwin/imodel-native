/******************************************************************************

Pointools Vortex API Examples

clashTest.cpp

Demonstrates clash capabilities of VortexFeatures library

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "clashTest.h"
#include <VortexFeatureExtract.h>
#include <PTAPI/PointoolsVortexAPI_import.h>
#include <PTAPI/PointoolsVortexAPI_resultCodes.h>
#include <Vortex/IClashObjectManager.h>
#include <Vortex/IClashTree.h>
#include <Vortex/IClashNode.h>


//-----------------------------------------------------------------------------
ClashTool::ClashTool() : Tool(CmdGenerateClashTree, CmdGenerateClashTree)
//-----------------------------------------------------------------------------
{	
	m_displayBoxes = false;
	m_infoTextBox = NULL;
}
//-----------------------------------------------------------------------------
void ClashTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch(cmdId)
	{
	case CmdGenerateClashTree:
		{
			int numScenes = ptNumScenes();		
			if (numScenes)
			{
				PThandle *sceneHandles = new PThandle[numScenes];
				ptGetSceneHandles(sceneHandles);

				for (int s = 0; s < numScenes; s++)			
				{									
					// Create clash objects for the scenes
					vortex::IClashObject* clashObject = NULL;
					ClashObjectMap::iterator it = m_clashObjects.find(sceneHandles[s]);					
					if (it == m_clashObjects.end())
					{
						PTres res = vortex::IClashObjectManager::createClashObjectFromCloud(sceneHandles[s], clashObject);
						if (res == PTV_SUCCESS)
						{
							m_clashObjects.insert(std::pair<PThandle, vortex::IClashObject*>(sceneHandles[s], clashObject));
						}
					}
					else
					{
						clashObject = (*it).second;
					}
					
					if (clashObject)
					{					
						PTbool generated = clashObject->clashTreeFileExists();

						vortex::IClashTree* tree;
						PTres res = clashObject->getClashTree(tree);
						if (res != PTV_SUCCESS)
						{
							res = clashObject->generateClashTree(this);

							// test
							const PTstr path = clashObject->getClashTreeFilename();
							generated = clashObject->clashTreeFileExists();
							
							drawPostDisplay();
						}
					}							
				}

				delete [] sceneHandles;
			}
		}
		break;
	}
}
//-----------------------------------------------------------------------------
void ClashTool::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	using namespace vortex;

	m_clashInfoString = "";

	// draw the clash boxes if they have been generated
	ClashObjectMap::iterator it;
	for (it = m_clashObjects.begin(); it != m_clashObjects.end(); it++)
	{		
		vortex::IClashObject* clashObject = (*it).second;
		if (clashObject)
		{
			IClashTree* tree = NULL;
			if (clashObject->getClashTree(tree) == PTV_SUCCESS)
			{
				if (int numBoxes = tree->getNumLeaves())
				{
					BOOL def;
					char buff[MAX_PATH] = {0};
					int size = MAX_PATH;
					::WideCharToMultiByte(CP_ACP, 
										WC_NO_BEST_FIT_CHARS, 
										clashObject->getClashTreeFilename(), 
										-1, 
										buff, 
										size, 
										0, 
										&def);	

					m_clashInfoString += buff;
					m_clashInfoString += ": ";
					char num[16] = {0};
					_itoa(numBoxes, num, 10);
					m_clashInfoString += num;
					m_clashInfoString += "\n";					
					m_infoTextBox->set_text(m_clashInfoString.c_str());

					PTfloat* extents = new PTfloat[numBoxes * 3];
					PTdouble* center = new PTdouble[numBoxes * 3];
					PTfloat* xAxis = new PTfloat[numBoxes * 3];
					PTfloat* yAxis = new PTfloat[numBoxes * 3];
					PTfloat* zAxis = new PTfloat[numBoxes * 3];

					// N.B. tree is pre transformed, no need to do it again!
					tree->getLeafBounds(extents, center, xAxis, yAxis, zAxis);
	
					glColor3f(1.0f, 0.0f, 0.0f);
					
					for (int i = 0; i < numBoxes; i++)
					{
						Vector3d vertices[8];
						computeVertices(&extents[i*3], &center[i*3], &xAxis[i*3], &yAxis[i*3], &zAxis[i*3], vertices);
						
						glBegin(GL_LINE_STRIP);
						glVertex3dv( vertices[0] );
						glVertex3dv( vertices[1] );
						glVertex3dv( vertices[3] );
						glVertex3dv( vertices[2] );
						glVertex3dv( vertices[0] );
						glEnd();

						glBegin(GL_LINE_STRIP);
						glVertex3dv( vertices[4] );
						glVertex3dv( vertices[6] );
						glVertex3dv( vertices[7] );
						glVertex3dv( vertices[5] );
						glVertex3dv( vertices[4] );
						glEnd();

						glBegin(GL_LINES);
						glVertex3dv( vertices[3] );
						glVertex3dv( vertices[7] );

						glVertex3dv( vertices[2] );
						glVertex3dv( vertices[6] );

						glVertex3dv( vertices[0] );
						glVertex3dv( vertices[4] );

						glVertex3dv( vertices[1] );
						glVertex3dv( vertices[5] );
						glEnd(); 
					}

					glColor3f(1.0f, 1.0f, 1.0f);

					delete [] extents;
					delete [] center;
					delete [] xAxis;
					delete [] yAxis;
					delete [] zAxis;
				}

				// Quick test of the leaf node individual bounds
				IClashNode* node = tree->getRoot();
				while (node)
				{
					if (node->isLeaf())
					{
						// quick test to draw this leaf node in a different colour (yellow) from the main boxes (red)
						glColor3f(1.0f, 1.0f, 0.0f);
						glLineWidth(5.0f);

						PTfloat extents[3] = {0};
						PTdouble center[3] = {0};
						PTfloat xAxis[3] = {0};
						PTfloat yAxis[3] = {0};
						PTfloat zAxis[3] = {0};
						node->getBounds(extents, center, xAxis, yAxis, zAxis);

						Vector3d vertices[8];
						computeVertices(extents, center, xAxis, yAxis, zAxis, vertices);						

						glBegin(GL_LINE_STRIP);
						glVertex3dv( vertices[0] );
						glVertex3dv( vertices[1] );
						glVertex3dv( vertices[3] );
						glVertex3dv( vertices[2] );
						glVertex3dv( vertices[0] );
						glEnd();

						glBegin(GL_LINE_STRIP);
						glVertex3dv( vertices[4] );
						glVertex3dv( vertices[6] );
						glVertex3dv( vertices[7] );
						glVertex3dv( vertices[5] );
						glVertex3dv( vertices[4] );
						glEnd();

						glBegin(GL_LINES);
						glVertex3dv( vertices[3] );
						glVertex3dv( vertices[7] );

						glVertex3dv( vertices[2] );
						glVertex3dv( vertices[6] );

						glVertex3dv( vertices[0] );
						glVertex3dv( vertices[4] );

						glVertex3dv( vertices[1] );
						glVertex3dv( vertices[5] );
						glEnd(); 

						glLineWidth(1.0f);
					}

					node = node->getLeft();
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
void ClashTool::buildUserInterface( GLUI_Node *parent )
//-----------------------------------------------------------------------------
{
	using namespace vortex;

	GLUI_Rollout *rollout = new GLUI_Rollout(parent, "Clash", true );

	rollout->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rollout, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *pan = new GLUI_Panel( rollout, " ", GLUI_PANEL_NONE);

	new GLUI_Button( pan, "Generate Clash Tree", CmdGenerateClashTree, &Tool::dispatchCmd  );

	// Scene Information
	new GLUI_StaticText( rollout, "" );
	
	m_clashInfoString = "Num boxes";

	if (!m_infoTextBox)
	{
		m_infoTextBox = new GLUI_TextBox( rollout, m_clashInfoString, false );
		m_infoTextBox->set_w( PANEL_WIDTH );
		m_infoTextBox->set_h( 80 );
		m_infoTextBox->align();	
	}
}

PTvoid ClashTool::clashTreeGenerationProgress(PTint pcentComplete)
{
	char mess[512] = {0};
	sprintf(mess, "\nClash Tree Generation progress: %d", pcentComplete);
	OutputDebugStringA(mess);
}

void ClashTool::computeVertices(float* extents, double* center, float* xAxis, float* yAxis, float* zAxis, vortex::Vector3d* vertex)
{
	vortex::Vector3f vaxisx(xAxis[0], xAxis[1], xAxis[2]);
	vortex::Vector3f vaxisy(yAxis[0], yAxis[1], yAxis[2]);
	vortex::Vector3f vaxisz(zAxis[0], zAxis[1], zAxis[2]);
	vortex::Vector3f vcenter((float) center[0], (float) center[1], (float) center[2]);

	vortex::Vector3f ext[] = {	vaxisx * extents[0],
		vaxisy * extents[1],
		vaxisz * extents[2] };

	for (unsigned int i=0; i<8; i++)
	{
		vertex[i] = vcenter;
		vertex[i] += (i & 1) ? ext[0] : - ext[0];
		vertex[i] += (i & 2) ? ext[1] : - ext[1];
		vertex[i] += (i & 4) ? ext[2] : - ext[2];
	}
}