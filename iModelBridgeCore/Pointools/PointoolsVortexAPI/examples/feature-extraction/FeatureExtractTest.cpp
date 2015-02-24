/******************************************************************************

Pointools Vortex API Examples

FeatureExtractTool.cpp

Demonstrates plane and cylinder extraction capabilities of VortexFeatures library

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "FeatureExtractTest.h"
#include "VortexFeatureExtract.h"
//-----------------------------------------------------------------------------
FeatureExtractTool::FeatureExtractTool() : Tool(CmdFitCylinder, CmdFitPlane)
//-----------------------------------------------------------------------------
{
	extern HMODULE hPTAPI;
	vortex::InitializeFeatureExtractAPI( 0 );

	hasPlane = false;
	hasCylinder = false;

	cylinder.radius=-1.0f;
}
//-----------------------------------------------------------------------------
void FeatureExtractTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch(cmdId)
	{
	case CmdFitCylinder:
	{
		PTubyte white [] = {0};
		//PThandle ch = ptCreatePointChannel(L"diagnostic", 1, 1, white, 0 );

		PThandle h = ptCreateSelPointsQuery();

		char output[128];
		cylinder.axis.zero();
		cylinder.radius=0;
		float rsm = vortex::FitCylinderToPointsf(h, cylinder, false, false );
		hasCylinder = true;

		sprintf(output, "Cylinder, radius = %0.2f, height = %0.2f - RMS Error = %0.4f"
			, cylinder.radius, cylinder.height, rsm);
		
		Tool::addStatisticMessage(output);
		
		ptDeleteQuery( h );

		ptUnselectAll();

		Tool::viewRedraw();

		break;
	}
	case CmdFitPlane:
	{
		PThandle h = ptCreateSelPointsQuery();
		
		char output[128];
		float rsm = vortex::FitPlanarRectangleToPointsf(h, planeCorners, false );

		sprintf(output, "Plane RMS Error = %0.4f", rsm);
		hasPlane = true;

		Tool::addStatisticMessage(output);

		ptDeleteQuery( h );

		ptUnselectAll();

		Tool::viewRedraw();

		break;
	}
	}
}
//-----------------------------------------------------------------------------
void FeatureExtractTool::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	using namespace vortex;

	// draw cylinder
	//simple without glut
	if (hasCylinder && cylinder.radius>0)
	{
		glColor3f(0,1.0f,0);
		glLineWidth(2.0f);

		Point3f cylinderEnd(cylinder.base);
		cylinderEnd += cylinder.axis*cylinder.height;

		// cross product to get perp
		Vector3f basis_x( cylinder.axis.cross( Vector3f(0,0,1) ));
		Vector3f basis_y( cylinder.axis.cross( basis_x ));

		glBegin( GL_LINES );
			glVertex3fv( &cylinder.base.x );
			glVertex3fv( &cylinderEnd.x );
		glEnd();

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef( cylinder.base.x, cylinder.base.y, cylinder.base.z);

		glBegin( GL_QUADS );
			for (float th=0; th<6.3; th+=0.2)
			{
				if ((int)(th/0.4) % 2==0)
					glColor3f(0.62f,0.6f,1.0f);
				else
					glColor3f(1.0f,1.0f,1.0f);

				Vector3f p1( basis_x*cylinder.radius*sin(th) + basis_y*cylinder.radius*cos(th) );
				Vector3f p2( basis_x*cylinder.radius*sin(th+0.2f) + basis_y*cylinder.radius*cos(th+0.2f) );
				
				Vector3f p3( p2 + cylinder.axis * cylinder.height );
				Vector3f p4( p1 + cylinder.axis * cylinder.height );

				glVertex3fv(&p1.x);
				glVertex3fv(&p2.x);
				glVertex3fv(&p3.x);
				glVertex3fv(&p4.x);
			}
		glEnd();

		glPopMatrix();
	}
	if (hasPlane)
	{
		// simple rectangle
		glBegin( GL_QUADS );
			glColor3f(0.62f,0.6f,1.0f);
			
			for (int i=0; i<4; i++)
				glVertex3fv(&planeCorners[i].x);
		glEnd();
	}
}
//-----------------------------------------------------------------------------
void FeatureExtractTool::buildUserInterface( GLUI_Node *parent )
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rollout = new GLUI_Rollout(parent, "Feature Extraction", true );

	rollout->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rollout, "" );
	spacer->set_w( PANEL_WIDTH );

	GLUI_Panel *pan = new GLUI_Panel( rollout, " ", GLUI_PANEL_NONE);

	new GLUI_Button( pan, "Fit Cylinder", CmdFitCylinder, &Tool::dispatchCmd );

	GLUI_Column *col = new GLUI_Column( pan, false );

	new GLUI_Button( pan, "Fit Plane", CmdFitPlane, &Tool::dispatchCmd  );
}
