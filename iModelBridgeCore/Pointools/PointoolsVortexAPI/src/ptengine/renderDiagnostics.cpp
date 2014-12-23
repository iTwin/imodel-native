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
	vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	pt::BoundingBoxD vbb;

	const PointCloud *pc = vox->pointCloud();

	if (!pc->displayInfo().visible()) return;

	glMatrixMode(GL_MODELVIEW);
	pc->pushUserTransformation();
	pc->registration().parent()->pushGL();
	pc->registration().pushGL(); 

	ptgl::Viewstore vs(true);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	//

	ptgl::PixelView pv;

	int index = 0;

	int numSelected = 0;
	int numPartSelected = 0;
	int numHidden = 0;
	int numPartHidden = 0;
	int numPainted = 0;
	int p=0;
	char buff[64];

	vector3 cam( &vs.model_mat[12] );	//translation of model view matrix is cam position

	vbb = vox->extents();
	vbb.translateBy(-basepoint);

	/* fade by distance */ 
	double pos[3];
	vs.project3v(vbb.center(), pos);

	double a = vbb.center().dist(pt::vector3d(cam.x, cam.y, cam.z));
	a /= 65.0f;
	if (a > 1.0f) a = 1.0f;
	if (a < 0) a = 0.1;

	glColor4f(1.0f,1.0f,1.0f, a);
	if ( vox->flag( pcloud::WholeHidden ) )
	{
		glColor4f(1.0f,0.0f,0.0f, a);	
		buff[p++] = 'H';
	}
	if ( vox->flag( pcloud::PartHidden ) )
	{
		glColor4f(1.0f,0.4f,0.4f, a);					
		buff[p++] = 'h';
	}
	if ( vox->flag( pcloud::WholeSelected ) 
		|| vox->parent()->flag( pcloud::WholeSelected )
		|| vox->parent()->parent()->flag( pcloud::WholeSelected ))
	{
		glColor4f(0.0f,1.0f,1.0f, a);					
		buff[p++] = 'S';
		buff[p++] = '*';
	}
	if ( vox->flag( pcloud::PartSelected ) )
	{
		glColor4f(0,1.0f,0.6f, a);					
		buff[p++] = 's';
	}
	// density
	sprintf( buff, "%0.3f", vox->densityValue() );

	//buff[p++] = 0;
	ptgl::Text::textOut(pos[0], pos[1], buff);
	a = 1.0 - pos[2];

	p = 0; int i;
	ubyte lyr = vox->layers(0);
	glColor4f(0.5f,0.8f,1.0f, a);
	for ( i=0; i<8; i++)
	{
		if (lyr & (1<<i)) buff[p++]='1';
		else buff[p++] = '0';
	}
	buff[p++] = 0;
	ptgl::Text::textOut(pos[0], pos[1]-10, buff);

	p = 0;
	glColor4f(0.8f,1.0f,0.5f, a);
	lyr = vox->layers(1);
	for (i=0; i<8; i++)
	{
		if (lyr & (1<<i)) buff[p++]='1';
		else buff[p++] = '0';
	}
	buff[p++] = 0;
	ptgl::Text::textOut(pos[0], pos[1]-20, buff);

	glColor4f(1.0,0,0, a);
	float rs = 0;
	float fp = 0;

	if (vox->lodPointCount())
	{
		rs = (float) 100 * vox->lodPointCount() / vox->fullPointCount();
		fp = (float) 100 * vox->numPointsEdited() / vox->fullPointCount();
	}
	sprintf(buff, "%i:L=%i E=%i %s%s", (int)vox->indexInCloud(), (int)rs,(int)fp, vox->channel(pcloud::PCloud_Filter) ? "f" : " "
		, vox->flag(pcloud::Painted) ? "p" : " ");
	ptgl::Text::textOut(pos[0], pos[1]-30, buff);

	glColor3f(1.0f, 0.2f, 0.2f);
	const double *sc = vox->channel(pcloud::PCloud_Geometry)->scaler();
	if (sc)
	{
		sprintf(buff, "sc: %f, %f, %f", sc[0], sc[1], sc[2]);
		//ptgl::Text::textOut(pos[0], pos[1]-40, buff);
	}

	const double *off = vox->channel(pcloud::PCloud_Geometry)->offset();
	if (off)
	{
		sprintf(buff, "off: %f, %f, %f", off[0], off[1], off[2]);
		//ptgl::Text::textOut(pos[0], pos[1]-50, buff);
	}

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
		glLineWidth( 2.0f );

		pt::BoundingBox vbb;
		const PointCloud *pc = v->pointCloud();

		if (!pc->displayInfo().visible()) return;

		glMatrixMode(GL_MODELVIEW);
		pc->pushUserTransformation();
		pc->registration().parent()->pushGL();
		pc->registration().pushGL(); 

		v->getBounds(vbb);
		
		glColor3f(1.0f,1.0f,1.0f);

		if (v->flag(DebugShowRed)) 		glColor3f(1.0f, 0, 0);
		if (v->flag(DebugShowBlue)) 	glColor3f(0, 0.3f, 1.0f);
		if (v->flag(DebugShowGreen)) 	glColor3f(0, 1.0f, 0.3f);
		if (v->flag(DebugShowPurple)) 	glColor3f(1.0f, 0.2f, 1.0f);

		renderBox(&vbb);

		glPopMatrix(); //scene
		glPopMatrix(); //offset
		glPopMatrix(); //registration


		vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		glColor3f(v->priority(), 0.2f, 0.2f);

		glLineWidth( 1.0f );

		pt::BoundingBoxD vbbd = v->extents();
		vbbd.translateBy(-basepoint);
		renderBox(&vbbd);
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
	ptgl::Text::setFont("gothic10");
	ptgl::Text::beginText();

	glDisable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	ptgl::Text::beginText();
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

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}