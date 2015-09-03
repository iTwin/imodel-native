#include <ptengine/renderDiagnostics.h>
#include <ptcloud2/pointcloud.h>
#include <pt/project.h>

#include <ptgl/glfont.h>
#include <ptgl/gltext.h>
#include <ptgl/glviewstore.h>
#include <ptgl/glViewSetup.h>

using namespace pointsengine;
using namespace pcloud;
using namespace pt;

/*****************************************************************************/
/**
* @brief
* @param bb
* @return void
*/
/*****************************************************************************/
template<typename T> void RenderVoxelDiagnosticInfo::renderBox( const pt::BBox<T> *bb )
{
	glBegin(GL_LINE_STRIP);
		glVertex3f(bb->lower(0), bb->lower(1), bb->lower(2));
		glVertex3f(bb->upper(0), bb->lower(1), bb->lower(2));
		glVertex3f(bb->upper(0), bb->upper(1), bb->lower(2));
		glVertex3f(bb->lower(0), bb->upper(1), bb->lower(2));
		glVertex3f(bb->lower(0), bb->lower(1), bb->lower(2));

		glVertex3f(bb->lower(0), bb->lower(1), bb->upper(2));
		glVertex3f(bb->upper(0), bb->lower(1), bb->upper(2));
		glVertex3f(bb->upper(0), bb->upper(1), bb->upper(2));
		glVertex3f(bb->lower(0), bb->upper(1), bb->upper(2));
		glVertex3f(bb->lower(0), bb->lower(1), bb->upper(2));
	glEnd();

	glBegin(GL_LINES);

		glVertex3f(bb->lower(0), bb->upper(1), bb->lower(2));
		glVertex3f(bb->lower(0), bb->upper(1), bb->upper(2));

		glVertex3f(bb->upper(0), bb->upper(1), bb->lower(2));
		glVertex3f(bb->upper(0), bb->upper(1), bb->upper(2));

		glVertex3f(bb->upper(0), bb->lower(1), bb->lower(2));
		glVertex3f(bb->upper(0), bb->lower(1), bb->upper(2));

	glEnd();
}
/*****************************************************************************/
/**
* @brief
* @param vx
* @return void
*/
/*****************************************************************************/
void RenderVoxelDiagnosticInfo::renderVoxelEditState( const pcloud::Voxel *vox )
{
	 // disable

	vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	pt::BoundingBoxD vbb;

	const PointCloud *pc = vox->pointCloud();

	if (!pc->displayInfo().visible()) return;

	ptgl::Viewstore vs(true);

	//
 
	ptgl::PixelView pv;

	int index = 0;

	int numSelected = 0;
	int numPartSelected = 0;
	int numHidden = 0;
	int numPartHidden = 0;
	int numPainted = 0;
	int p=0;

	vector3 cam( &vs.model_mat[12] );	//translation of model view matrix is cam position

	vbb = vox->extents();
	vbb.translateBy(-basepoint);

	/* fade by distance */ 
	double pos[3];
	double qc[3] = {0,0,0};
	vs.project3v(vbb.center(), pos);

	// alt method: 
	//vs.project3v( qc, pos );

	double a = vbb.center().dist(pt::vector3d(cam.x, cam.y, cam.z));
	a /= 45.0f;
	if (a > 1.0f) a = 1.0f;
	if (a < 0) a = 0.2;

	glColor4f(0.0f,1.0f,0.0f, a);

	// whole hidden = RED dot
	// part hidden = ORANGE dot
	// green dot
	if ( vox->flag( pcloud::WholeHidden ) )
	{
		glColor4f(1.0f,0.0f,0.0f, a);	
	}
	if ( vox->flag( pcloud::PartHidden ) )
	{
		glColor4f(1.0f,0.6f,0.4f, a);					
	}
	
	glPointSize(5.0f);
	glBegin(GL_POINTS);
		glVertex2f( pos[0], pos[1]+5 );
	glEnd();

	// draw a tick mark if it has a channel
	if (vox->channel( PCloud_Filter ))	
	{
		glColor3f(1.0f, 0, 1.0f);
		glBegin( GL_LINE_STRIP );
			glVertex2f( pos[0]-10, pos[1]+3);
			glVertex2f( pos[0]-7, pos[1]);
			glVertex2f( pos[0]-4, pos[1]+9);
		glEnd();
	}

	if (vox->lodPointCount())
	{
		float w = 80;
		float s = w * (float)vox->lodPointCount() / vox->fullPointCount();

		// background
		glColor3f( 0.3f,0.3f,0.3f );
		glBegin(GL_QUADS);
			glVertex2f( pos[0]+5, pos[1] );
			glVertex2f( pos[0]+5+w, pos[1] );
			glVertex2f( pos[0]+5+w, pos[1]+15 );
			glVertex2f( pos[0]+5, pos[1]+15 );
		glEnd();

		// lod loaded
		glColor3f( 0.0f,0.5f,1.0f );

		glBegin(GL_QUADS);
			glVertex2f( pos[0]+5, pos[1] );
			glVertex2f( pos[0]+5+s, pos[1] );
			glVertex2f( pos[0]+5+s, pos[1]+4 );
			glVertex2f( pos[0]+5, pos[1]+4 );
		glEnd();

		// lod display
		glColor3f( 0.8f,0.8f,0.2f );

		s = w * vox->getRequestLOD();

		glBegin(GL_QUADS);
			glVertex2f( pos[0]+5, pos[1]+5 );
			glVertex2f( pos[0]+5+s, pos[1]+5 );
			glVertex2f( pos[0]+5+s, pos[1]+10 );
			glVertex2f( pos[0]+5, pos[1]+10 );
		glEnd();

		// num pts edited
		glColor3f( 1.0f,0.8f,1.0f );

		if (vox->channel( PCloud_Filter ))
		{
			s = w * (float)vox->numPointsEdited() / vox->fullPointCount();

			glBegin(GL_LINES);
				glVertex2f( pos[0]+5+s, pos[1]-2 );
				glVertex2f( pos[0]+5+s, pos[1]+12 );
			glEnd();
		}

		// layers
		glBegin(GL_POINTS);
		ubyte lyr = 1;
		for (int i=0; i<6; i++)
		{
			glColor3f( 0.5f,0.5f,0.5f );

			if (vox->layers(1) & lyr)
			{
				glColor3f( 1.0f,0.8f,0.1f );
			}
			else if (vox->layers(0) & lyr)
			{
				glColor3f( 1.0f,1.0f,1.0f ); 

			}
			glVertex2f( pos[0]+5+i*6, pos[1]-4 );

			lyr <<= 1;
		}
		glEnd();
	}
	glPointSize(1.0f);
}

/*****************************************************************************/
/**
* @brief
* @param xv
* @return void
*/
/*****************************************************************************/
void RenderVoxelDiagnosticInfo::renderVoxelOutline( const pcloud::Voxel *v )
{
	glDisable(GL_TEXTURE_2D);

	if (1 || v->flag(DebugShowRed) || v->flag(DebugShowBlue) || 
		v->flag(DebugShowGreen) || v->flag(DebugShowPurple))
	{
		glLineWidth( 1.0f );

		vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		pt::BoundingBoxD vbb;
		const PointCloud *pc = v->pointCloud();

		if (!pc->displayInfo().visible()) return;

		vbb = v->extents();
		vbb.translateBy(-basepoint);

		glColor3f(1.0f,1.0f,1.0f);

		if (v->flag(DebugShowRed)) 		glColor3f(1.0f, 0, 0);
		if (v->flag(DebugShowBlue)) 	glColor3f(0, 0.3f, 1.0f);
		if (v->flag(DebugShowGreen)) 	glColor3f(0, 1.0f, 0.3f);
		if (v->flag(DebugShowPurple)) 	glColor3f(1.0f, 0.2f, 1.0f);

		renderBox(&vbb);

	}
	glEnable(GL_TEXTURE_2D);
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void RenderVoxelDiagnosticInfo::beginVoxelEditStateRender()
{
	glDisable( GL_DEPTH_TEST );

	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void RenderVoxelDiagnosticInfo::endVoxelEditStateRender()
{
	glDisable(GL_BLEND);
	glEnable( GL_DEPTH_TEST );

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
}