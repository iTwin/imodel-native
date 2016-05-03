/*system stuff						*/ 
#include <tchar.h>
#include <pt/trace.h>

/*gl Headers						*/ 
#include <gl/glew.h>

//#include <ptgl/glExtensions.h> <-- now replaced by glew
#include <pt/timestamp.h>

/*ptgl headers						*/ 
#include <ptgl/glViewSetup.h>
#include <ptgl/glEffects.h>
#include <ptgl/glText.h>
#include <ptgl/glClipBox.h>
#include <ptgl/glVertexProgram.h>
#include <ptedit/edit.h>

/*other stuff						*/ 
#include <ptappdll/ptapp.h>
#include <pt/project.h>
#include <bmg/bmgdll.h>

/*points engine headers				*/
#include <ptcloud2/gradient.h>
#include <ptengine/renderEngine.h>
#include <ptengine/pointsScene.h>
#include <ptengine/pointspager.h>
#include <ptengine/PointsFilter.h>
#include <ptengine/userchannels.h>
#include <ptengine/engine.h>

/* std lib							*/ 
#include <iostream>

//#define MANUAL_DRAW_TEST

using namespace pointsengine;
using namespace pcloud;
using namespace pt;
using namespace ptgl;


static RenderEngine *_renderengine = 0;
#define DEG2RAD(a) a * 0.017453292519943295769236907684886

#ifndef __INTEL_COMPILER
#define fmax(a,b) (((a)>(b)) ? (a) : (b))
#endif

#define PT_MIN_DENSITY 0.001f

/*----------------------------------------------------------*/ 
/* GL extensions											*/ 
/*----------------------------------------------------------*/ 
static bool _glhasVP = false;
static bool _glhasPointParam = false;
static bool _glhasOcclusion = false;
static bool _glhasMultitexture = false;
static int _maxTexelUnits = 0 ;
static bool _glhasTextureShader = false;
static pcloud::Voxel *_currentVoxel=0;

//static VoxelPointsRenderer g_voxelRenderer;

#define REQUIRES_GEOM 0x01
#define REQUIRES_LIGHTING 0x02
#define REQUIRES_CLIP 0x04
#define REQUIRES_FOG 0x08
#define REQUIRES_EDIT 0x10
#define REQUIRES_EDGESHADER 0x20 
#define REQUIRES_OFFSET 0x40

#define SEL_COL_R 255
#define SEL_COL_G 128
#define SEL_COL_B 36
#define SEL_COL_RGB SEL_COL_R, SEL_COL_G, SEL_COL_B

inline static void setupShaderEdgeMode(const RenderEngine::ShaderEdgeMode &m)
{
	switch (m)
	{
	case RenderEngine::RepeatEdge:
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		break;
	case RenderEngine::ClampEdge:
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		break;
	case RenderEngine::BlackEdge:
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		break;
	}
}
/*----------------------------------------------------------*/ 
/* Geometry Shader (Height Shader)							*/ 
/*----------------------------------------------------------*/ 
struct _GeomShader
{
	static void begin(VertexProgram *vp)
	{
		if ( _glhasMultitexture && _glhasVP )
		{
			glActiveTexture(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_1D);
			int ramp = _renderengine->geomShaderRamp();
			glBindTexture(GL_TEXTURE_1D, _renderengine->texID(ramp));
			
			setupShaderEdgeMode(_renderengine->geomShaderEdgeMode());

			glActiveTexture(GL_TEXTURE0_ARB);
			vector4d v;
			v.x = _renderengine->geomShaderParams()[0];
			v.y = _renderengine->geomShaderParams()[1];
			v.z = _renderengine->geomShaderParams()[2];
			v.w = _renderengine->geomShaderParams()[3];//1.0;
			
			vector3d v3(_renderengine->geomShaderParams());
			double scaler = v3.length();

			vp->setProgramVector(false, 2, v);
		}
	}
	static void end()
	{
		if ( _glhasMultitexture && _glhasVP )
		{
			glActiveTexture(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_1D);
			glActiveTexture(GL_TEXTURE0_ARB);
		}
	}
};
/* GeomShader Vertex Program --------------------------------*/ 
class GeomShader : public VertexProgram
{
public:
	GeomShader(const char*file) : VertexProgram("GeomShader", file) {};
	bool begin()	
	{	
		VertexProgram::begin();
		_GeomShader::begin(this);
		return true;
	}
	void end()
	{	
		_GeomShader::end();
		ptgl::VertexProgram::end(); 
	}
};
/*----------------------------------------------------------*/ 
/* ClipBox VertexProgram									*/ 
/*----------------------------------------------------------*/ 
class ClipBoxProgram : public VertexProgram
{
public:
	ClipBoxProgram(const char*file) : VertexProgram("ClipBox", file) {};
	bool begin()
	{
		if (!ptgl::VertexProgram::begin()) return false;
		if (!bounds() || !angles() ) return false;
	
		updateClipMatrix();
		if (_renderengine->isGeomShaderEnabled())
		{
			gs = true;
			_GeomShader::begin(this);
		}
		else gs = false;
		
		return true;
	}
	static void updateClipMatrix()
	{
		if (!overrideMat())
		{
			if (!bounds() || !angles() )
			{
				mat() = mmatrix4d::identity();		
				return;
			}

			BoundingBox B(*bounds());
			vector3 c(B.center());

			vector4d  sc(B.dx()/2, B.dy()/2, B.dz()/2, 1.0);
			vector4d  tr(c.x, c.y, c.z, 0);

			mat() = mmatrix4d::scale(sc) >> mmatrix4d::zRotation(DEG2RAD(angles()->z)) >> mmatrix4d::translation(tr);
			mat().invert();
		}
	}
	static void overrideMatrix(const mmatrix4d &omat)
	{
		mat() = omat;
		overrideMat() = true;
	}
	static void cancelOverrideMatrix()
	{
		overrideMat() = false;
	}	
	static bool &overrideMat() { static bool overrideMat = false; return overrideMat; }

	void end()
	{
		ptgl::VertexProgram::end();
		if (gs) _GeomShader::end();
	}
	void setTransformMatrix(const mmatrix4d &m) const
	{	
		if (!_active) return;
		
		mmatrix4d M(m);

		if (gs)
		{
			M.transpose();

			/* this is for the geom shader */ 
			glMatrixMode(GL_MATRIX3_ARB);
			glLoadIdentity();
			//glPushMatrix();
			//glLoadIdentity();
			glMultMatrixd((double*)(&M));
			glMatrixMode(GL_MODELVIEW);

			M.transpose();
		}
		M >>= mat();
		VertexProgram::setTransformMatrix(M);
	}
	bool gs;

	inline static const vector3* &angles()		{ static const vector3 *a=0; return a; }
	inline static const BoundingBox* &bounds()	{ static const BoundingBox *b=0; return b; }
	inline static mmatrix4d &mat() { static mmatrix4d m=mmatrix4d::identity(); return m; }
};
void RenderEngine::clipBox(const pt::BoundingBox *bb, const pt::vector3 *angles)	{ _clipbox = bb; _clipboxangles = angles; }
/*----------------------------------------------------------*/ 
/* Point Lighting VertexProgram								*/ 
/*----------------------------------------------------------*/ 
class PointLighting : public ptgl::VertexProgram
{
public:
	PointLighting(const char *shader_file = "pnt_lgt.vsh") 
		: ptgl::VertexProgram("PointLighting", shader_file ) {}
	bool begin()	
	{	
		gs = false;
		VertexProgram::begin();
		if (_renderengine->isEdgeShaderEnabled())
		{
			float ep = _renderengine->getEdgeShaderTolerance();
			GLdouble e[] = {ep,ep,ep,ep};
			setProgramVector(false, 2, e);
			return true;
		}
		if (_renderengine->isGeomShaderEnabled())
		{
			gs = true;
			_GeomShader::begin(this);
		}
		return true;
	}
	void end()
	{	
		if (gs) _GeomShader::end();
		ptgl::VertexProgram::end(); 
	}
private:
	bool gs;
};

//---------------------------------------------------------
// Channel Setups
//---------------------------------------------------------
namespace ptrender
{
	PointsScene::VOXELSLIST _voxlist;
	int voxlistState	=-1;

	/* lighting shaders */ 
	PointLighting		_vplight;	
	PointLighting		_vplightfog("pnt_lgt_f.vsh"); 
	PointLighting		_vpedge("pnt_edge.vsh");

	/* Clip box shader */ 
	ClipBoxProgram		_vpclip("pnt_clp.vsh");
	ClipBoxProgram		_vpclipfog("pnt_clp_f.vsh");
	ClipBoxProgram		_vpcliplight("pnt_lgt_clp.vsh");
	ClipBoxProgram		_vpcliplightfog("pnt_lgt_clp_f.vsh");
	ClipBoxProgram		_vpclipedge("pnt_clp_e.vsh");
	
	/* Plane Shader */ 
	GeomShader			_vpgeom("pnt_geom.vsh");
	GeomShader			_vpgeomfog("pnt_geom_f.vsh");
	ClipBoxProgram		_vpclipgeom("pnt_geom_clp.vsh");
	ClipBoxProgram		_vpclipgeomfog("pnt_geom_clp_f.vsh");

	/* Edit programs */ 
	PointLighting		_vpEditLight("pnt_edit_lgt.vsh"); 
	PointLighting		_vpEditLightFog("pnt_edit_lgt_f.vsh"); 
	GeomShader			_vpEditGeom("pnt_edit_geom.vsh"); 
	VertexProgram		_vpEdit("Edit", "pnt_edit.vsh");
	VertexProgram		_vpEditFog("Edit", "pnt_edit_f.vsh");
	ClipBoxProgram		_vpclipedit("pnt_edit_clp.vsh");

	/* User Channel Offset */ 
	VertexProgram		_vpEditOffset("EditWOffset", "pnt_ucoff.vsh");
	VertexProgram		_vpOffset("Offset", "pnt_edit_ucoff.vsh");

	ptgl::VertexProgram *_vp = 0;
	ptgl::VertexProgram *_vpne = 0;
	ptgl::VertexProgram *_lvp = 0;

	ubyte *_editBufferForEmpties = 0;
	int _editBufferSize = 0;
	
	bool _blockQuantizeInMatrix	= false;
//----------------------------------------------------------------------
// Setup
//----------------------------------------------------------------------
struct SetupGeometry
{
	static const void* setup(const DataChannel *dc) { 
		if (dc->data())
		{
			if (dc->storeType() == pcloud::Float32)		
				glVertexPointer(3, GL_FLOAT, 12, dc->data()); 
			
			else if (dc->storeType() == pcloud::Short16)
				glVertexPointer(3, GL_SHORT, 6, dc->data()); 

			return dc->data();
		}
		return 0;
	}
	static void startFrame() 
	{
	};
	static void endFrame() {};
	static void pushMatrix(const DataChannel *dc) 
	{
		if (!_blockQuantizeInMatrix)
		{
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			/*quantized data transform to local space (not scene space) */ 
			glTranslated(dc->offset()[0], dc->offset()[1], dc->offset()[2]);
			glScaled(dc->scaler()[0], dc->scaler()[1], dc->scaler()[2]);
		}
		if (_vp) _vp->setQuantizeMatrix(dc->scaler(), dc->offset());
	}
	static void popMatrix()
	{
		if (!_blockQuantizeInMatrix)
		{
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}
	}
};	
//----------------------------------------------------------------------
// Setup
//----------------------------------------------------------------------
struct SetupColour
{
	static const void* setup(const DataChannel *dc)
	{
		if (dc->data() && dc->size())
		{
			glColorPointer(3, GL_UNSIGNED_BYTE, 3, dc->data());	
			return dc->data();
		}
		return 0;
	}
	static void startFrame() {};
	static void endFrame() {};
	static void pushMatrix(const DataChannel *dc) {}
	static void popMatrix() {}
};
static void contrastBrightnessValue(float &contrast, float &brightness)
{
	contrast *= 10.0f;
	brightness = (brightness -0.5f) * 2.0f;
	brightness += 0.5f;
}
//----------------------------------------------------------------------
// Setup Scaled Colour
//----------------------------------------------------------------------
struct SetupScaledColour
{
	static void startFrame()
	{
		/*texture unit 0 is used for intensity shader*/ 
		float white = _renderengine->dataWhitePoint();
		float black = _renderengine->dataBlackPoint();
		int ramp = _renderengine->dataRamp();

		if (_glhasMultitexture) glActiveTexture(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_1D, _renderengine->texID(ramp));
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				
		/*texture matrix*/ 
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		float contrast = black;
		float brightness = white;
		contrastBrightnessValue(contrast, brightness);

		/* brightness / contrast */ 
		glTranslated(brightness, 0, 0);
		glScaled(contrast, 0, 0);
		glScaled(1.0/65536, 0, 0);
	}
	static void endFrame()
	{
		if (_glhasMultitexture) glActiveTexture(GL_TEXTURE0_ARB);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();		
	}
	static const void* setup(const DataChannel *dc) 
	{
		if (dc->data() && dc->size())
		{
			glTexCoordPointer(1, GL_SHORT, 0, dc->data());
			return dc->data();
		}
		return 0;
	}
	static void pushMatrix(const DataChannel *dc) { }
	static void popMatrix()	{ }
};
//
static bool s_filterSetup = false;
//----------------------------------------------------------------------
// Setup Selection Filter
//----------------------------------------------------------------------
static bool bSelHasDC = false;
static bool s_dovp;
struct SetupSelectionFilter
{
	static void startFrame()
	{
	}
	static void endFrame()
	{
	}
	static const void* setup(const DataChannel *dc) 
	{
		if ( (dc && dc->data() && dc->size()) && 
				( _currentVoxel && (_currentVoxel->flag( pcloud::PartHidden ) || _currentVoxel->flag( pcloud::PartSelected ) )
				) 
				)
		{
			s_dovp = true;

			glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)dc->data());
			return dc->data();
		}
		else
		{
			return 0;
		}
	}
	static void pushMatrix(const DataChannel *dc)
	{
		if (dc && dc->size() && theRenderEngine().useVertexArrays())
		{
			glEnableVertexAttribArrayARB(1); 
			bSelHasDC = true;
		}
	}
	static void popMatrix()	
	{
		if (bSelHasDC && theRenderEngine().useVertexArrays())
		{
			glDisableVertexAttribArrayARB(1);
			bSelHasDC = false;
		}
	}
	static void allocateBuffer(int size)
	{
		return;
		/* don't use this, its too slow, faster to swith VP on/off */ 
		if (size > _editBufferSize)
		{
			_editBufferSize = size;
			if (_editBufferForEmpties) delete [] _editBufferForEmpties;
			_editBufferForEmpties = new ubyte[size];

			ZeroMemory(_editBufferForEmpties, size);
		}
	}
};
//----------------------------------------------------------------------
// Offset Channel (User channel)
//----------------------------------------------------------------------
//
//struct SetupOffsetChannel
//{
//	static void startFrame() {}
//	static void endFrame() {}
//
//	static const void* setup(const UserChannel *uc) 
//	{
//		if ( (uc && uc->getData()) 
//			&& ( _currentVoxel && (_currentVoxel->flag( pcloud::PartHidden ) || _currentVoxel->flag( pcloud::PartSelected ) )
//				) 
//				)
//		{
//			s_dovp = true;
//			glVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)uc->getData());
//			return uc->getData();
//		}
//		else
//		{
//			return 0;
//		}
//	}
//	static void pushMatrix(const DataChannel *dc)
//	{
//		if (uc && uc->getData() && theRenderEngine().useVertexArrays())
//		{
//			glEnableVertexAttribArrayARB(2); 
//			bSelHasDC = true;
//		}
//	}
//	static void popMatrix()	
//	{
//		if (bSelHasDC && theRenderEngine().useVertexArrays())
//		{
//			glDisableVertexAttribArrayARB(2);
//			bSelHasDC = false;
//		}
//	}
//};

static void setupOffsetChannel( pcloud::Voxel *v, VoxelChannelData *uc, float blend )
{
	/* put it into texture 1 */ 
	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(3, GL_FLOAT, 0, (GLvoid*)uc->getData());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);

	static double b[4];
	b[0] = blend;
	b[1] = blend;
	b[2] = blend;
	b[3] = 0;
	if (_vp) _vp->setProgramVector(true, 0, b);

	//glVertexAttribPointerARB(9, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)uc->getData());
//	glDisableVertexAttribArrayARB(9);
}
static void cleanupOffsetChannel()
{
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);

//	glDisableVertexAttribArrayARB(9);
}
//----------------------------------------------------------------------
// Setup
//----------------------------------------------------------------------
struct SetupNormals
{
	static const void* setup(const DataChannel *dc)
	{
		if (dc->data() && dc->size())
		{
			glNormalPointer(GL_SHORT, 0, dc->data());
			return dc->data();
		}
		return 0;
	}
	static void startFrame() 
	{
		if (_glhasMultitexture && _renderengine->isEdgeShaderEnabled())
		{
			glActiveTexture(GL_TEXTURE1_ARB);
			int ramp = _renderengine->dataRamp();
			glBindTexture(GL_TEXTURE_1D, _renderengine->texID(ramp));
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		}
	};
	static void endFrame()   
	{
	
	};
	static void pushMatrix(const DataChannel *dc) {}
	static void popMatrix() {}
};
struct SetupNull
{
	static bool setup(const DataChannel *dc) { return true; }
	static void startFrame() {};
	static void endFrame()   {};
	static void pushMatrix(const DataChannel *dc) {}
	static void popMatrix() {}
};
//----------------------------------------------------------------------
// Setup
//----------------------------------------------------------------------
const PointCloud* _cpc=0;

class SetupPointCloud
{
public:
	SetupPointCloud(const PointCloud* pc, bool forceEvaluate=false, uint filterOut=0)
	{
		if (pc != _cpc || forceEvaluate)
		{
			/* point size */ 
			if (pc->displayUseGlobalPointSize())
				glPointSize(_renderengine->pointSize());
			else
				glPointSize(pc->displayPointSize());

			glMatrixMode(GL_MODELVIEW);
			if (_cpc) 
			{
				glPopMatrix(); //scene
				glPopMatrix(); //offset
				glPopMatrix(); //registration
			}
			if (pc) 
			{			
				pc->pushUserTransformation();
				pc->registration().parent()->pushGL();
				pc->registration().pushGL(); 
			}

			bool enabled = _vp ? true : false;
			_vp = 0;

			/*//determine vertex program to use*/ 
			/* filter overrides others */ 
			uint mode;

			mode = 0;
			mode |= _renderengine->isClipboxEnabled() ? REQUIRES_CLIP : 0;
			mode |= glIsEnabled(GL_FOG) ? REQUIRES_FOG : 0;

			if (_renderengine->isInEditMode())
			{
				if (pc->root()->flag( pcloud::PartSelected ) || 
					pc->root()->flag( pcloud::PartHidden ))
					mode |= REQUIRES_EDIT;
			}
			/* Offset Channel */ 
			UserChannel *offsetChannel = UserChannelManager::instance()->renderOffsetChannel();

			if (_renderengine->isUserChannelRenderEnabled() && offsetChannel)
				mode |= REQUIRES_OFFSET;

			if (_renderengine->isEdgeShaderEnabled() && 
				pc->voxels()[0]->channel(PCloud_Normal))
			{
				mode |= REQUIRES_EDGESHADER;
			}
			else
			{
				mode |= _renderengine->isGeomShaderEnabled() ? REQUIRES_GEOM : 0;
				mode |= _renderengine->renderFlags() & RenderEngine::Render_Lighting
					&& pc->voxels()[0]->channel(PCloud_Normal) ? REQUIRES_LIGHTING : 0;
			}
			mode &= ~filterOut;

			if (_glhasVP)
			{
				_vp = _renderengine->vertexProgram(mode);
				mode &= ~REQUIRES_EDIT;
				_vpne = _renderengine->vertexProgram(mode);
			}
			if (_vp)
			{
				ptgl::State::enable(GL_VERTEX_PROGRAM_ARB);
				if (_vp != _lvp)
				{
					if (_lvp) _lvp->end();
					_vp->begin();
				}
				/* compile the project matrix */ 
				const_cast<Transform*>(&pc->registration())->compileMatrix();
				_vp->setEyeMatrix(); 

				mmatrix4d mat = pc->registration().cmpMatrix();
				mat >>= pc->userTransformationMatrix();

				_vp->setTransformMatrix(mat);
			}
			else ptgl::State::disable(GL_VERTEX_PROGRAM_ARB);

			_lvp = _vp;
		}
		if (pc) glColor3ubv(pc->displayBaseColor());

		_cpc = pc;
	}
	~SetupPointCloud()			{}
	static void startFrame()	{ _cpc = 0; _vp = 0; }
	static void endFrame()
	{
		if (_cpc)
		{
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix(); // scene
			glPopMatrix(); // user transformation
			glPopMatrix(); // registration
		}
		if (_vp) _vp->end();
		_vp = 0;
		_lvp = 0;

		ptgl::State::disable(GL_VERTEX_PROGRAM_ARB);
	}
};
}
using namespace ptrender;

//-----------------------------------------------------
// _Channel Renderer renderFlags
//-----------------------------------------------------
uint RenderEngine::_ChannelRenderer::renderFlags() const { return _renderengine->renderFlags(); }
uint RenderEngine::_ChannelRenderer::channelFilter() const { return _renderengine->channelFilter(); }
bool RenderEngine::_ChannelRenderer::useVertexArray() const { return _renderengine->useVertexArrays(); }

//-----------------------------------------------------
// Editmode enabling / disabling 
//-----------------------------------------------------
void RenderEngine::enableEditMode()		{ /*if (thePointsSelect().isEnabled())*/ _inEditMode = true; }
void RenderEngine::disableEditMode()	{ _inEditMode = false; }

//-----------------------------------------------------
// Construction and initialization
//-----------------------------------------------------
RenderEngine::RenderEngine()
{
	PTTRACE_FUNC

	static bool constructed = false;
	if (constructed) 
	{
		std::cout << "Second instance of RenderEngine constructed..." << std::endl;
		exit(0);
	}
	_renderflags = Render_Geometry | Render_RGB | Render_Intensity;
	_blend = Render_RGB | Render_Intensity;
	_renderengine = this;

	/*build channel renderers*/ 
	for (int c=0;c < MAX_CHANNELS; c++)
		_chrenderers[c] = 0;

	_chrenderers[PCloud_Geometry] = new ChannelRenderer
		<PCloud_Geometry, SetupGeometry, Render_Geometry, GL_VERTEX_ARRAY>();

	_chrenderers[PCloud_RGB] = new ChannelRenderer
		<PCloud_RGB, SetupColour, Render_RGB, GL_COLOR_ARRAY>();
	
	_chrenderers[PCloud_Intensity] = new ChannelRenderer
	<PCloud_Intensity, SetupScaledColour, Render_Intensity, GL_TEXTURE_COORD_ARRAY, GL_TEXTURE_1D>();

	_chrenderers[PCloud_Normal] = new ChannelRenderer
		<PCloud_Normal, SetupNormals, Render_Lighting, GL_NORMAL_ARRAY, GL_LIGHTING>();

	_chrenderers[PCloud_Filter] = new ChannelRenderer
		<PCloud_Filter, SetupSelectionFilter, Render_Geometry, /*GL_WEIGHT_ARRAY_ARB*/ 0, /*GL_VERTEX_BLEND_ARB*/ 0>();

	_multiplier = 0.5f;
	_pointSize = 2.0f;
	_rampBlack = 0.1388f;
	_rampWhite = 0.5f;
	_rampid = -1;
	_rampEdge = RepeatEdge;
	
	_pointDensity = PointDensityViewOptimal;


	_ambient[0] = 0.15f;
	_ambient[1] = 0.15f;
	_ambient[2] = 0.15f;
	_ambient[3] = 0.15f;

	_diffuse[0] = 0.7f;
	_diffuse[1] = 0.7f;
	_diffuse[2] = 0.7f;
	_diffuse[3] = 1.0f;

	_specular[0] = 0.8f;
	_specular[1] = 0.8f;
	_specular[2] = 0.8f;
	_specular[3] = 1.0f;

	_glossiness = 0.8f;//60.0f;

	_lastcomplete = false;


	constructed = true;
	_channelFilter = 0;
	_autoPointSize = true;
	_allowInterrupt = false;

	_geomshader = false;
	_geomshaderRamp = 5;
	_geomshaderParams[0] = 0; 
	_geomshaderParams[1] = 0; 
	_geomshaderParams[2] = 20.0; 
	_geomshaderParams[3] = 0;
	_geomedge = RepeatEdge;
	_geomshaderShowScale = false;

	_renderScanners = false;
	_renderMethod = RenderPoints;

	_scanPosStyle = ScanPosPoint;

	_cancelRender = false;
	_edgeShader = false;
	_edgeShaderTolerance = 2.0f;
	_minDynamicOutput = 0.005f;

	_hasFocalPoint = false;

	_clipbox = 0;
	_clipboxangles = 0; 

	//SetupSelectionFilter::allocateBuffer(1e6);
	_useVertexArray = true;
	_diagnosticDisplay = 0;//DiagnosticLayers;
	TimeStamp::initialize();

	_rampsManager.createDefaultGradients();
	initVertexProgramsMap();
}
//-----------------------------------------------------
// Deconstruction
//-----------------------------------------------------
RenderEngine::~RenderEngine()
{
	delete _chrenderers[PCloud_Geometry];
	delete _chrenderers[PCloud_RGB];
	delete _chrenderers[PCloud_Intensity];
	delete _chrenderers[PCloud_Normal];
	delete _chrenderers[PCloud_Filter];

	for (ContextMap::iterator i=_contexts.begin();
		i != _contexts.end();  i++)
	{
		if (i->first) delete i->second;
	}
}
//
// initialize combinations of vertex program options
//
void RenderEngine::initVertexProgramsMap()
{
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM, &_vpgeom));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_FOG, &_vpgeomfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_LIGHTING, &_vplight));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_CLIP, &_vpclipgeom));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_LIGHTING | REQUIRES_CLIP, &_vpcliplight));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_LIGHTING | REQUIRES_CLIP | REQUIRES_FOG, &_vpcliplightfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_LIGHTING | REQUIRES_FOG, &_vplightfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM | REQUIRES_CLIP | REQUIRES_FOG, &_vpclipfog));

	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_LIGHTING, &_vplight));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_CLIP, &_vpclip));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_LIGHTING | REQUIRES_CLIP, &_vpcliplight));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_LIGHTING | REQUIRES_CLIP | REQUIRES_FOG, &_vpcliplightfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_LIGHTING | REQUIRES_FOG, &_vplightfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_CLIP | REQUIRES_FOG, &_vpclipfog));

	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_GEOM, &_vpEditGeom));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_GEOM | REQUIRES_FOG, &_vpgeomfog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_LIGHTING, &_vpEditLight));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_LIGHTING | REQUIRES_FOG, &_vpEditLightFog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_FOG, &_vpEditFog));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT, &_vpEdit));

	/* user channel offset */ 
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDIT | REQUIRES_OFFSET, &_vpEditOffset));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_OFFSET, &_vpOffset));

	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_GEOM, &_vpgeom));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDGESHADER, &_vpedge));
	_vertexPrograms.insert(VPMAP::value_type(REQUIRES_EDGESHADER | REQUIRES_CLIP, &_vpclipedge));
}
//-----------------------------------------------------------------------------
// set allow interrupt flag
//-----------------------------------------------------------------------------
void RenderEngine::allowInterrupt(bool f) { _allowInterrupt = f; }
//-----------------------------------------------------------------------------
// renderVoxelNoVP
//-----------------------------------------------------------------------------
void RenderEngine::renderVoxelNoVP( pcloud::Voxel * vox, uint size ) 
{
	//if (vox)
	//	g_voxelRenderer.renderPoints(vox, size);
	//else g_voxelRenderer.flushRendering(false, true);
}
//-----------------------------------------------------------------------------
// filterUnusedChannels
//-----------------------------------------------------------------------------
void RenderEngine::filterUnusedChannels(bool f)
{
	if (f == pointsFilteringState().filterPagingByDisplay()) return;

	pointsFilteringState().filterPagingByDisplay(f);

	if (f)
	{
		/*update the channel filter*/ 
		if (!(_renderflags & Render_RGB)) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_RGB);
		if (!(_renderflags & Render_Lighting)
			&& !_edgeShader) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_Normal);
		if (!(_renderflags & Render_Intensity)) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_Intensity);
	}
}
//
// add render flag
//
void RenderEngine::addRenderFlag(RenderFlag flag)
{ 
	_renderflags |= flag; 

	if (_filterUnusedChannels)
	{
		/*update the channel filter*/ 
		if (_renderflags & Render_RGB) pointsFilteringState().remDisplayChannelFilter(pcloud::PCloud_RGB);
		if (_renderflags & Render_Lighting || _edgeShader) pointsFilteringState().remDisplayChannelFilter(pcloud::PCloud_Normal);
		if (_renderflags & Render_Intensity) pointsFilteringState().remDisplayChannelFilter(pcloud::PCloud_Intensity);
	}
}
//
// remove render flag
//
void RenderEngine::remRenderFlag(RenderFlag flag)
{ 
	_renderflags &= ~flag; 

	/*update the channel filter*/ 
	if (_filterUnusedChannels)
	{
		if (!(_renderflags & Render_RGB)) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_RGB);
		if (!(_renderflags & Render_Lighting) && !_edgeShader) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_Normal);
		if (!(_renderflags & Render_Intensity)) pointsFilteringState().addDisplayChannelFilter(pcloud::PCloud_Intensity);
	}
}
//-----------------------------------------------------
// Initialize in valid context
//-----------------------------------------------------
bool RenderEngine::initialize()
{
	TimeStamp::initialize();
	initializeContext();
	return true;
}
//-----------------------------------------------------
// dataWhitePoint
//-----------------------------------------------------
void RenderEngine::dataWhitePoint(float white)
{ 
	_rampWhite = white;
	//if (fabs(_rampWhite - _rampBlack) < 0.00001f) _rampWhite+=0.003f; 
}
//-----------------------------------------------------
// dataBlackPoint
//-----------------------------------------------------
void RenderEngine::dataBlackPoint(float black)
{ 
	_rampBlack = black; 
	//if (fabs(_rampWhite - _rampBlack) < 0.00001f) _rampBlack-=0.003f; 
}
//-----------------------------------------------------
// Initialize in valid context
//-----------------------------------------------------
bool RenderEngine::initializeGL()
{
	PTTRACE_FUNC

	glewInit();
	_glhasVP =  GLEW_ARB_vertex_program ? true : false;
	_glhasMultitexture = glewIsSupported("GL_ARB_multitexture") ? true : false;
	_glhasTextureShader = glewIsSupported("GL_NV_texture_shader") ? true : false;

	return loadRamps();
}
void RenderEngine::getClipBoxMatrix(mmatrix4d &mat)
{
	ClipBoxProgram::updateClipMatrix();
	mat = ClipBoxProgram::mat();
}
void RenderEngine::setClipBoxMatrixOverride(const mmatrix4d &mat)
{
	ClipBoxProgram::overrideMatrix(mat);
}
void RenderEngine::cancelClipBoxOverride()
{
	ClipBoxProgram::cancelOverrideMatrix();
}
//-----------------------------------------------------
// RenderNoVertexArrays
// This method has been developed for AutoCAD
// which crashes when orbiting with a vertex array draw
//-----------------------------------------------------
void RenderEngine::renderNoVertexArraysGL()
{

}
//-----------------------------------------------------
// RenderSafeGL- uses only basic OpenGL features
// Engine MUST be paused to use this method - no mutexs are locked
//-----------------------------------------------------
void RenderEngine::renderSafeGL()
{
	PTTRACE_FUNC

	int voxelcount = thePointsScene().voxels().size();
	if (!voxelcount) return;

	bool store_paused = _paused;
	_paused = true;

	_lastcomplete = true;

	ptgl::State::clear();

	_channelFilter = pointsFilteringState().displayChannelFilter();
	
	
	if (_renderflags & Render_Lighting)
	{
		setupLighting();
		glEnable(GL_COLOR_MATERIAL);
	}

	PointsScene::VoxIterator t,i,e;
	pcloud::Voxel *vox = 0;

	//ptgl::State::enable(GL_POINT_SMOOTH);

	int vi = 0;
	int c;
	int liststate = -1;

	PointsScene::VOXELSLIST vlist;
	PointsScene::UseSceneVoxels voxelslock(vlist, liststate);
	i = vlist.begin();
	e = vlist.end();

	bool useclipbox = pointsFilteringState().clipboxFilter();

	ClipBoxProgram::updateClipMatrix();
	mmatrix4d B = ClipBoxProgram::mat();
	mmatrix4d F, T;

	const PointCloud *lpc = 0;
	const PointCloud *pc = 0;

	float geomzscale = vector3(_geomshaderParams).length();

	if (fabs(geomzscale) < 1) geomzscale = 1.0;
	geomzscale = 1.0 / geomzscale;

	float contrast = _rampBlack;
	float brightness = _rampWhite;
	contrastBrightnessValue(contrast, brightness);

	const Ramp *ramp = _actContext->ramp(_rampid);
	const Ramp *gramp = _actContext->ramp(_geomshaderRamp);

	while (i != e)
	{
		if (_cancelRender) 
		{
			break;
		}

		vox = (*i);
		++i;

		pc = vox->pointCloud();
	
		if (pc != lpc)
		{
			T = pc->scene()->registration().matrix();
			T >>= pc->registration().matrix();
			F = T >> B;
			lpc = pc;

			if (pc->displayUseGlobalPointSize())
			{
				glPointSize(_pointSize);
			}
			else
			{
				glPointSize(pc->displayPointSize());
			}
		}
		glColor3f(1.0f,1.0f,1.0f);

		/*dont load clipped voxels*/ 
		if (vox->flag(pcloud::WholeClipped)
			|| vox->flag(pcloud::WholeHidden)
			|| !vox->flag(pcloud::Visible)) continue;

		/* load points in this voxel, destructor will unload to original amount*/ 
		VoxelLoader voxloader(vox, vox->lodRequest());
		uint size = calcVoxelRenderDensity(vox);
		if (!size) continue;

		const DataChannel *dc = vox->channel(PCloud_Geometry);
		
		const ubyte			*rgb = 0;
		const pt::vector3s	*normal = 0;
		const pt::vector3s	*geom16 = 0;
		const pt::vector3	*geom = 0;
		const short			*intensity = 0;
		const ubyte			*filter = 0;

		if (vox->channel(PCloud_Geometry)->storeType() == pcloud::Short16)
			geom16 = reinterpret_cast<const pt::vector3s*>(vox->channel(PCloud_Geometry)->begin());
		else geom = reinterpret_cast<const pt::vector3*>(vox->channel(PCloud_Geometry)->begin());

		if (vox->channel(PCloud_RGB))		rgb = reinterpret_cast<const ubyte*>(vox->channel(PCloud_RGB)->begin());
		if (vox->channel(PCloud_Intensity)) intensity = reinterpret_cast<const short*>(vox->channel(PCloud_Intensity)->begin());
		if (vox->channel(PCloud_Normal))	normal = reinterpret_cast<const pt::vector3s*>(vox->channel(PCloud_Normal)->begin());
		if (vox->channel(PCloud_Filter))	filter = reinterpret_cast<const ubyte*>(vox->channel(PCloud_Filter)->begin());

		/*filter for settings*/ 
		pushBlending(vox);

		if (!(_renderflags & Render_RGB))		rgb = 0;
		if (!(_renderflags & Render_Intensity))	intensity = 0;
		if (!(_renderflags & Render_Lighting))	normal = 0;
		
		/*no data check*/ 
		if (	(rgb && !vox->channel(PCloud_RGB)->size())
			||	(intensity && !vox->channel(PCloud_Intensity)->size())
			||	(normal && !vox->channel(PCloud_Normal)->size())
			) continue;


		/* base color */ 
		uint32 pcol[] = { 255, 255, 255 };
		
		if (vox->flag(WholeSelected))
		{
				pcol[0] = SEL_COL_R;
				pcol[1] = SEL_COL_G;
				pcol[2] = SEL_COL_B;
		}
	
		glMatrixMode(GL_MODELVIEW);
		vox->pointCloud()->pushUserTransformation();
		vox->pointCloud()->scene()->registration().pushGL();
		vox->pointCloud()->registration().pushGL();

		if (normal) ptgl::State::enable(GL_LIGHTING);
		else ptgl::State::disable(GL_LIGHTING);

		ptgl::State::flush();
		
		/*calculate constants for intensity*/ 
		double rw = _rampWhite*65536;
		double sc = (1.0 / (_rampWhite - _rampBlack))/65538;

		
		vector3 geom_scaler, geom_offset;
		geom_scaler.set( vox->channel(PCloud_Geometry)->scaler() );
		geom_offset.set( vox->channel(PCloud_Geometry)->offset() );

		glBegin(GL_POINTS);

		pt::vector3 g, g1;
			for (int p=0; p<size; p++)
			{
				/* filter */ 
				if (filter)
				{
				//	if (*filter & 0) continue;
				}
				/* vertex */ 
				if (geom16)
				{
					g.set(geom16->x, geom16->y, geom16->z);
					++geom16;
				}
				else if (geom)
				{
					g = *geom;
					++geom;
				}
				g *= geom_scaler;
				g += geom_offset;

				/* normal */ 
				if (normal)				glNormal3sv(*normal);	
				
				/* calculate colour contributions */ 

				if (rgb)
				{
					pcol[0] *= rgb[0];
					pcol[1] *= rgb[1];
					pcol[2] *= rgb[2];
					pcol[0] /= 255;
					pcol[1] /= 255;
					pcol[2] /= 255;
				}
				if (intensity)
				{		
					double _in = (*intensity);
					_in /= 65536;
					_in *= contrast;
					_in += brightness;
					/* clamp */ 
					
					if (_in > 1.0f) _in = 1.0f;
					else if (_in < 0) _in = 0;

					//while (_in<0) _in += 1.0f;

					int in = (_in * ramp->width);
					
					pcol[0] *= ramp->data[in*4];
					pcol[1] *= ramp->data[in*4+1];
					pcol[2] *= ramp->data[in*4+2];
					pcol[0] /= 255;
					pcol[1] /= 255;
					pcol[2] /= 255;
				}
				if (_geomshader)
				{
					vector3 gz, ps(_geomshaderParams);
					T.vec3_multiply_mat4f(g, gz);
					
					float z = gz.dot(ps) + _geomshaderParams[3];
					bool isblack = false;

					/*edge behaviour*/ 
					if (z > 1.0 || z < 0)
					{
						switch (geomShaderEdgeMode())
						{
						case RepeatEdge:
							z = fmod(z, 1.0f);					
							while (z<0) z += 1.0f;
							break;
						case ClampEdge:
							if (z > 1.0f) z = 1.0f;
							else if (z < 0) z = 0;
							break;
						case BlackEdge:
							pcol[0] = pcol[1] = pcol[2] = 0;
							isblack = true;
							break;
						}
					}
					if (!isblack)
					{
						int zp = (z * gramp->width);
						
						pcol[0] *= gramp->data[zp*4];
						pcol[1] *= gramp->data[zp*4+1];
						pcol[2] *= gramp->data[zp*4+2];
						pcol[0] /= 255;
						pcol[1] /= 255;
						pcol[2] /= 255;
					}
				}

				glColor3ub(pcol[0],pcol[1],pcol[2]);
				
				/* clip box */ 
				if (useclipbox)
				{
					vector3 f(g), f1;
					F.vec3_multiply_mat4f(f, f1); 

					if (f1.x >= -1 && f1.y >= -1 && f1.z >= -1 && 
						f1.x <= 1 && f1.y <= 1 && f1.z <= 1) 
						glVertex3fv(g);
				}
				else glVertex3fv(g);
			
				if (rgb) rgb += 3;
				if (intensity) ++intensity;
				if (normal) ++normal;
				if (filter) ++filter;
			}
		glEnd();

		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		popBlending();
	}
	//SetupPointCloud::endFrame();
	_cancelRender = false;

	ptgl::State::disable(GL_POINT_SMOOTH);
	ptgl::State::disable(GL_LIGHTING);
	ptgl::State::flush();
	glFinish();
	_paused = store_paused;
}
//-----------------------------------------------------
// compute max points to render for voxel
//-----------------------------------------------------
int RenderEngine::calcVoxelRenderDensity(const pcloud::Voxel* vox)
{
	if (vox->pointCloud()->displayInfo().visible() && vox->flag(Visible))
	{
		if ( vox->flag( pcloud::WholeHidden ) ) return 0;

		int reqSize = vox->lodRequest() * vox->fullPointCount();
		int editedSize = 0;//vox->editedLodPointCount();	// this is unused anyway
		
		int renderSize = min(reqSize, editedSize);
		float renderProp = (float)renderSize / vox->fullPointCount();

		if ( renderSize > 0 && renderProp < _minDynamicOutput)
			renderSize = _minDynamicOutput * vox->fullPointCount();

		if (!renderSize) return 0;

		/*no data check - must do sanity check on channels to ensure data is available */ 
		/* this is done post OOC load, so is still valid for OOC nodes */ 
		bool nodata = false;
		const DataChannel *dc;
		uint channelbit = 1;

		for (int c=1;c<MAX_CHANNELS; c++) 
		{
			dc = vox->channel(c);
			if (_chrenderers[c] && dc && !(channelbit & _channelFilter))
			{
				if (dc->size() < renderSize) renderSize = dc->size();
			}
			channelbit <<= 1;
		}
		if (!renderSize) return 0;

		return renderSize;
	}
	return 0;
}
//-----------------------------------------------------
// setupLighting
//-----------------------------------------------------
void RenderEngine::setupLighting()
{
	{
		float gloss = _glossiness * 128;

		glShadeModel(GL_FLAT);

		glMaterialfv(GL_FRONT, GL_AMBIENT, _ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, _diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, _specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, &gloss);
	}
}
//-----------------------------------------------------
// Render Scanners
//-----------------------------------------------------
void RenderEngine::renderScanners()
{
	int size = thePointsScene().size();

	glPointSize(10.0f);

	for (int s=0; s<size; s++)
	{
		const pcloud::Scene *scene = thePointsScene()[s];

		int num_scanners = scene->numScanPositions();
		int sc = 0;

		for (sc=0; sc<num_scanners; sc++)
		{
			const ScanPosition *sp = scene->scanPos(sc);
			if (!sp->displayInfo().visible()) return;

			sp->registration().pushGL();
			switch(_scanPosStyle)
			{
			case ScanPosPoint:
				glPointSize(12.0f);
				glBegin(GL_POINTS);
					glColor3f(1.0f,1.0f,1.0f);
				glEnd();
				glPointSize(10.0f);
				glBegin(GL_POINTS);
					glColor3f(0.4f,1.0f,1.0f);
					glVertex3f(0,0,0);
				glEnd();
				break;
			case ScanPosAxis:
				glLineWidth(2.0f);

				glBegin(GL_LINES);
					glColor3f(1.0f,0,0);
					glVertex3f(0,0,0);
					glVertex3f(1.0f,0,0);
					
					glColor3f(0,1.0f,0);
					glVertex3f(0,0,0);
					glVertex3f(0,1.0f,0);

					glColor3f(0,0,1.0f);
					glVertex3f(0,0,0);
					glVertex3f(0,0,1.0f);
				glEnd();

			break;
			}
			glPopMatrix();			
		}
	}
	return;

	/* draw the scan images */ 
	//ScanImage::pushGLstate();
	unsigned int s;
	////glEnable(GL_TEXTURE_2D);
	//for (s=0; s<size; s++)
	//{
	//	const pcloud::Scene *scene = thePointsScene()[s];

	//	int num_scanners = scene->numScanPositions();
	//	int sc = 0;

	//	for (sc=0; sc<num_scanners; sc++)
	//	{			
	//		const ScanPosition *sp = scene->scanPos(sc);
	//		
	//		for (int img=0; img<sp->numImages(); img++)
	//		{
	//			ScanImage *si = const_cast<ScanImage*>(&sp->image(img));
	//			
	//			glMatrixMode(GL_MODELVIEW);
	//			sp->registration().pushGL();

	//			//si.drawGL();
	//			si->startTextureProjection();

	//			glMatrixMode(GL_MODELVIEW);
	//			glPopMatrix();
	//		}
	//	}
	//}
	////ScanImage::popGLstate();
}
//
static void renderBox(const BoundingBox *vbb)
{
	glBegin(GL_LINE_STRIP);
		glVertex3f(vbb->lower(0), vbb->lower(1), vbb->lower(2));
		glVertex3f(vbb->upper(0), vbb->lower(1), vbb->lower(2));
		glVertex3f(vbb->upper(0), vbb->upper(1), vbb->lower(2));
		glVertex3f(vbb->lower(0), vbb->upper(1), vbb->lower(2));
		glVertex3f(vbb->lower(0), vbb->lower(1), vbb->lower(2));

		glVertex3f(vbb->lower(0), vbb->lower(1), vbb->upper(2));
		glVertex3f(vbb->upper(0), vbb->lower(1), vbb->upper(2));
		glVertex3f(vbb->upper(0), vbb->upper(1), vbb->upper(2));
		glVertex3f(vbb->lower(0), vbb->upper(1), vbb->upper(2));
		glVertex3f(vbb->lower(0), vbb->lower(1), vbb->upper(2));
	glEnd();

	glBegin(GL_LINES);

		glVertex3f(vbb->lower(0), vbb->upper(1), vbb->lower(2));
		glVertex3f(vbb->lower(0), vbb->upper(1), vbb->upper(2));

		glVertex3f(vbb->upper(0), vbb->upper(1), vbb->lower(2));
		glVertex3f(vbb->upper(0), vbb->upper(1), vbb->upper(2));

		glVertex3f(vbb->upper(0), vbb->lower(1), vbb->lower(2));
		glVertex3f(vbb->upper(0), vbb->lower(1), vbb->upper(2));

	glEnd();	
}

//
// Show boundary and extents of Voxel for debugging purposes
//
static void renderVoxel(Voxel *v)
{
	BoundingBox vbb;
	const PointCloud *pc = v->pointCloud();
	if (!pc->displayInfo().visible()) return;
	if (!v->flag(pcloud::Visible)) return;

	glMatrixMode(GL_MODELVIEW);
	pc->pushUserTransformation();
	pc->registration().parent()->pushGL();	
	pc->registration().pushGL();

	v->getBounds(vbb);

	glColor3f(0.1f, 0.5f, 0.5f);
	renderBox(&vbb);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	vector3 basepoint(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	glColor3f(1-v->usage(), 0, v->usage());
	if (v->flag( pcloud::OutOfCore ))
		glColor3f(0,1.0f,1);

	vbb = v->extents();
	vbb.translateBy(-basepoint);
	//renderBox(&vbb);

	glPointSize(8.0f);

	glBegin(GL_POINTS);
	glVertex3fv(vbb.center());
	glEnd();

	glPointSize(1.0f);
	/* markings */ 

	int pos[3];
	//ptgl::Viewstore vs(true);
	//vs.project3v(vbb.center(), pos);

	//ptgl::PixelView pv;

	char buff[32];
	//	sprintf(buff, "%0.2f", v->usage());
	//	ptgl::Text::textOut(pos[0], pos[1], buff);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}
//
// Renders layer info and LOD info on voxels. NO box rendered
//
void RenderEngine::renderDebugInfo()
{
	int clipped = 0;	
	int c = 0;
	pcloud::Voxel *vox=0;
	BoundingBoxD vbb;

	PointsScene::UseDepthSortedVoxels voxelslock(_voxlist, voxlistState);
	int voxelcount = _voxlist.size();
	if (!voxelcount) return;

	vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	/* some vars */ 
	PointsScene::VoxIterator t, i, e;
	
	ptgl::Viewstore vs(true);

	
	SetupPointCloud::startFrame();

	i = _voxlist.begin();
	e = _voxlist.end();

	glMatrixMode(GL_MODELVIEW);

	ptgl::Text::setFont("gothic10");
	ptgl::Text::beginText();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	ptgl::Text::beginText();

	ptgl::PixelView pv;

	int index = 0;

	while (i != e)
	{
		vox = (*i);
		++i;

		/* diagnostics */ 
		if (_diagnosticDisplay & DiagnosticStructure)
				renderVoxel(*i);

		if (_diagnosticDisplay & DiagnosticLayers)
		{
			boost::mutex::scoped_lock lock(vox->mutex(), boost::try_to_lock);
			if (!lock.owns_lock()) continue;

			if ( (_clip && vox->flag(pcloud::WholeClipped))
				|| vox->flag(pcloud::WholeHidden)
				|| !vox->flag(pcloud::Visible)) 
				continue; 

			vbb = vox->extents();
			vbb.translateBy(-basepoint);
					
			//glColor3f(vox->priority() + (0.5f-vox->priority()*0.5f), 0.5f,0.5f);

			int p=0;
			char buff[64];

			/* fade by distance */ 
			vector3 cam;
			_camera->getLocation(cam);

			double a = vbb.center().dist(vector3d(cam));
			a /= 65.0;
			if (a > 1.0) a = 1.0;
			if (a < 0) a = 0.1;
			a = 1.0 - a;

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
			if ( vox->flag( pcloud::WholeSelected ) )
			{
				glColor4f(0.0f,1.0f,1.0f, a);					
				buff[p++] = 'S';
			}
			if ( vox->flag( pcloud::PartSelected ) )
			{
				glColor4f(0,1.0f,0.6f, a);					
				buff[p++] = 's';
			}
			buff[p++] = 0;
			int pos[2];
			vs.project3v(vbb.center(), pos);
			ptgl::Text::textOut(pos[0], pos[1], buff);


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
				rs = (float) calcVoxelRenderDensity(vox) / vox->lodPointCount();
				fp = (float) vox->numPointsEdited() / vox->lodPointCount();
			}
			sprintf(buff, "(%i/%i) %s", (int)(rs*100),(int)(fp*100), vox->channel(pcloud::PCloud_Filter) ? "f" : " ");
			ptgl::Text::textOut(pos[0], pos[1]-30, buff);

			//glColor3f(1.0f, 0.2f, 0.2f);
			//const double *sc = vox->channel(pcloud::PCloud_Geometry)->scaler();
			//if (sc)
			//{
			//	sprintf(buff, "sc: %f, %f, %f", sc[0], sc[1], sc[2]);
			//	ptgl::Text::textOut(pos[0], pos[1]-40, buff);
			//}

			//const double *off = vox->channel(pcloud::PCloud_Geometry)->offset();
			//if (off)
			//{
			//	sprintf(buff, "off: %f, %f, %f", off[0], off[1], off[2]);
			//	ptgl::Text::textOut(pos[0], pos[1]-50, buff);
			//}

		//	sprintf(buff, "%.2f", vox->priority());
		//	sprintf(buff, "%d (%.2f)", index++, vox->priority());
		}
	}
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	SetupPointCloud::endFrame();
}
//
// auto point size for dynamic redering
//
static float calcPointSize(float red, float pointsize)
{
	float ps = pointsize;
	if (red < 0.0156f) ps = 8.0f;
	else if (red < 0.0204f) ps = 7.0f;
	else if (red < 0.028f) ps = 6.0f;
	else if (red <= 0.04f) ps = 5.0f;
	else if (red < 0.07f) ps = 4.0f;
	else if (red < 0.15f) ps = 3.0f;
	else if (red < 0.5f) ps = 2.0f;
	else ps = 1.0f;

	return (ps > pointsize) ? ps : pointsize;
}
struct __rgb__ { ubyte r; ubyte g; ubyte b; };
//
//
//
void RenderEngine::renderVoxelProxy(const pcloud::Voxel *vox)
{
	//COLTEST
	return;

	vector3s pos;
	if (!vox->pointCloud()->displayInfo().visible()
		|| !vox->pointCloud()->parent()->displayInfo().visible())
		return;

	if (vox->channel(PCloud_Geometry)->getSample(0, pos))
	{
		SetupPointCloud pointcloud(vox->pointCloud());
		int i=0;

		glPointSize(8.0f);

		_chrenderers[PCloud_Geometry]->predraw(vox);

		if (vox->channel(PCloud_RGB)) 
			_chrenderers[PCloud_RGB]->predraw(vox);

		if (vox->channel(PCloud_Intensity)) 
			_chrenderers[PCloud_Intensity]->predraw(vox);

		/* render the sample points */ 
		
		__rgb__ rgb;
		short intensity;

		glBegin(GL_POINTS);

		while (vox->channel(PCloud_Geometry)->getSample(i, pos))
		{
			if (vox->channel(PCloud_RGB)) 
			{
				vox->channel(PCloud_RGB)->getSample(i, rgb);
				glColor3ub(rgb.r, rgb.g, rgb.b);
			}
			if (vox->channel(PCloud_Intensity))
			{
				vox->channel(PCloud_Intensity)->getSample(i, intensity);
				glTexCoord1s(intensity);
			}
			glVertex3sv(pos);
			++i;
		}
		glEnd();

		_chrenderers[PCloud_Geometry]->postdraw();
		
		if (vox->channel(PCloud_RGB)) 
			_chrenderers[PCloud_RGB]->postdraw();

		if (vox->channel(PCloud_Intensity)) 
			_chrenderers[PCloud_Intensity]->postdraw();

		if (vox->pointCloud()->displayUseGlobalPointSize())
			glPointSize(_pointSize);
		else
			glPointSize(vox->pointCloud()->displayPointSize());
	}
}
//-----------------------------------------------------
// RenderGL
//-----------------------------------------------------
void RenderEngine::renderProxies(int millisecs)
{
	if (_paused) return;
	int clipped = 0;	
	int c = 0;
	pcloud::Voxel *vox=0;
	vector3s pos;
	
	PointsScene::UseSceneVoxels voxelslock(_voxlist, voxlistState);
	int voxelcount = _voxlist.size();
	if (!voxelcount) return;

	_channelFilter = pointsFilteringState().displayChannelFilter();
	uint channelbit = 1;

	/* some vars */ 
	PointsScene::VoxIterator t, i, e;

	SetupPointCloud::startFrame();

	i = _voxlist.begin();
	e = _voxlist.end();

	glPointSize(7.0f);

	glMatrixMode(GL_MODELVIEW);

	while (i != e)
	{
		vox = (*i);
		vox->flag(pcloud::Rendered, true);
		++i;
		
		/* some quick get outs */ 
		if ((_vp && _inEditMode) && vox->flag(pcloud::WholeHidden)) continue;
		if (_clip && vox->flag(pcloud::WholeClipped)) continue; 

		DataChannel *gdc = vox->channel(PCloud_Geometry);

		if (gdc->getSample(0, pos))
		{
			SetupPointCloud pointcloud(vox->pointCloud());
			c = 0;

			glPushMatrix();
			/*quantized data transform to local space (not scene space) */ 
			glTranslated(gdc->offset()[0], gdc->offset()[1], gdc->offset()[2]);
			glScaled(gdc->scaler()[0], gdc->scaler()[1], gdc->scaler()[2]);

			/* render the sample points */ 
			__rgb__ rgb;

			glBegin(GL_POINTS);
				while (vox->channel(PCloud_Geometry)->getSample(c, pos))
				{
					vox->channel(PCloud_RGB)->getSample(c, rgb);
				
					glColor3ub(rgb.r, rgb.g, rgb.b);
					glVertex3sv(pos);
					++c;
				}
			glEnd();
			glPopMatrix();
		}
	}
	SetupPointCloud::endFrame();
}
//-----------------------------------------------------
// Render Analysis Info
//-----------------------------------------------------
void RenderEngine::renderAnalysisInfo()
{
	if ( !_hasAnalysisPoint ) return;
	
	for (int i=0; i< thePointsScene().size(); i++)
	{
		int num_pc = thePointsScene()[i]->numObjects();

		for (int j=0; j<num_pc; j++)
		{
			pcloud::Node *n = const_cast<pcloud::Node*>(thePointsScene()[i]->cloud(j)->root())->findContainingLeaf( _analysisPoint );

			if (n)
			{
				renderVoxel(static_cast<pcloud::Voxel *>(n));
			}
		}
	}
}
//-----------------------------------------------------
// Set Context
//-----------------------------------------------------
bool RenderEngine::initializeContext()
{
	HGLRC context = wglGetCurrentContext();
	
	static RenderEngine::Context g_defaultContext;

	if (!context)
	{
		_actContext = &g_defaultContext;
		if (!g_defaultContext.numRamps())
			loadRamps();
		return true;
	}
	ContextMap::iterator i = _contexts.find(context);
	if ( i == _contexts.end())
	{
		_actContext = new Context;

		if (!initializeGL()) 
		{ 
			_initialisationFailure = true;
			_actContext = &g_defaultContext;
			_lastError = "OpenGL context setup failure";
			return false;
		} 

		loadRamps(); 
		_initialisationFailure = false;
		_contexts.insert( ContextMap::value_type(context, _actContext) );
	}
	else
	{
		_actContext = i->second;
	}
	return true;
}
//-----------------------------------------------------
// RenderGL
//-----------------------------------------------------
void RenderEngine::renderGL(int millisecs, bool front, const pcloud::Scene *scene)
{	
	if (this->_clip)
	{
		_inEditMode = false;
	}
	else _inEditMode = true; 

	PointsScene::UseSceneVoxels voxelslock( _voxlist, voxlistState );
	int voxelcount = _voxlist.size();
	
	if (!voxelcount || !initializeContext() || _paused)  
		return;

	int clipped = 0;

	vector3d wfocalPoint;
	if (_hasFocalPoint)
		pt::Project3D::project().project2WorldSpace(_focalPoint, wfocalPoint);

	ptgl::State::clear();
	ptgl::State::enable(GL_COLOR_SUM_ARB);

	TimeStamp t0;
	t0.tick();

	// setup clipbox
	_clip = pointsFilteringState().clipboxFilter();

	if (_clip)
	{
		ClipBoxProgram::angles() = _clipboxangles;
		ClipBoxProgram::bounds() = _clipbox;
	}
	_channelFilter = pointsFilteringState().displayChannelFilter();
	uint channelbit = 1;

	_lastcomplete = millisecs ? false : true;

	if (_renderScanners)	renderScanners();

	/* lighting is a hack to get normals through for edge shader */ 
	if (isEdgeShaderEnabled())
		addRenderFlag(RenderEngine::Render_Lighting);

	/* some vars */ 
	PointsScene::VoxIterator	t, i, e;
	pcloud::Voxel				*vox = 0;
	int							vi = 0, c, count =0, voxpoints = 0, voxcount =0;
	uint						size=0;
	static bool					first = true;
	pt::Bounds<1, float>		priorityBounds = theVisibilityEngine().priorityBounds();

	i = _voxlist.begin();
	e = _voxlist.end();

	/* gl state */ 
	ptgl::State::enable(GL_COLOR_MATERIAL);
	ptgl::State::disable(GL_LIGHTING);

	if (_renderflags & Render_Lighting) setupLighting();
	glPointSize(_pointSize);

	/* initialize channel renderers */ 
	for (c=1;c<MAX_CHANNELS; c++)
	{
		if (_chrenderers[c] && !(channelbit & _channelFilter))
			_chrenderers[c]->startFrame();
		channelbit <<= 1;
	}
	SetupPointCloud::startFrame();
	
	/*Indicates that the last voxel wasnt plotted*/ 
	bool lastvoxelskip = false;

	//this is a hack, should be working in the ptgl::State anyway
	glEnable(GL_COLOR_MATERIAL);

	/* focal point */ 
	Bounds<1, double> distBounds;
	distBounds.makeEmpty();

	/* User CHANNELS */ 
	UserChannel *userOffset = UserChannelManager::instance()->renderOffsetChannel();
	float offsetBlend = UserChannelManager::instance()->renderOffsetBlendValue();

	bool inEditMode = isInEditMode();

	if (_hasFocalPoint)
	{	
		while (i != e)
		{
			vox = (*i);
			++i;

			bool vis = vox->pointCloud()->displayInfo().visible();
			if (vox->flag(WholeClipped)) continue;
			if (!vis || !vox->flag(Visible)) continue;

			vector3d vcen(vox->extents().center());
			double d = wfocalPoint.dist2(vcen);
			distBounds.expand(&d);
		}
		i = _voxlist.begin();
	}
	bool useEdit = false;
	bool lastUseEdit = false;

	float randcolR = 0.5f;
	float randcolG = 0.25f;
	float randcolB = 0.125f;

//	g_voxelRenderer.startFrame();

	PointsScene::VOXELSLIST retry;
	/* voxel iteration */ 
	while (i != e)
	{
		randcolR += 0.15f;
		randcolG += 0.225f;
		randcolB += 0.065;

		vox = (*i);
		t = i;
		++i;
		_currentVoxel = vox;

		if (voxcount ++ % 4 == 0 && !millisecs && _allowInterrupt)
		{
			unsigned short ks = ::GetAsyncKeyState(VK_MBUTTON) | ::GetAsyncKeyState(VK_LBUTTON);
			if (ks & 0x8000) break;
		}

		/* this voxel has been processed */ 
		if (front && vox->flag(pcloud::Rendered))
			continue;
 		
		/* scene filtering */ 
		if (scene)
		{
			if (vox->pointCloud()->scene() != scene) continue;
		}

		vox->flag(pcloud::Rendered, true);
		
		/* some quick get outs */ 
		if (inEditMode && vox->flag(pcloud::WholeHidden)) 
			continue;
		if (_clip && vox->flag(pcloud::WholeClipped)) 
			continue; 		

		bool proxyOnly = false;
		
		float loadAmount = -1.0;
		if (_pointDensity == PointDensityFull) loadAmount = vox->lodRequest();
		else if (vox->flag( pcloud::OutOfCore ) && !millisecs) loadAmount = vox->lodRequest();

		voxpoints = vox->lodPointCount(); if (!voxpoints) proxyOnly = true;
		size = calcVoxelRenderDensity(vox); 

		if (loadAmount < 0 && !size) 
		{
			vox->flag(pcloud::Rendered, false);		
			continue;
		}

		/* try mutex */ 
		boost::mutex::scoped_lock vlock(vox->mutex(), boost::try_to_lock);
		if (!vlock.owns_lock()) 
		{
			vox->flag(pcloud::Rendered, false);
			continue;
		}

		size = calcVoxelRenderDensity(vox); /* because may have changed since lock */ 
		count += voxpoints;

		/* load the data if density is full or out of core rendering is needed*/ 
		VoxelLoader vloader(loadAmount > 0 ? vox : 0, loadAmount, false, false);
		if (loadAmount > 0) size = calcVoxelRenderDensity(vox);

		VoxelChannelData *vcd = 0;

		uint programFilterOut = 0;

		/*predraw*/ 
		{
			if (1)
			{
				ptgl::State::flush();
				ptgl::ClientState::flush();

				if (millisecs && !vox->flag( pcloud::OutOfCore ))
				{
					double adj = fmax(_minDynamicOutput, _multiplier);// : _minoutput ;
					
					double d_size = adj < 1 ? size * adj : size;
					size = d_size < size ? (d_size > 0 ? d_size : 1) : size;
				}
				renderVoxelNoVP(vox, size);	
			}
			else
			{

				
				/* if partial filter channel then use the vertex program */ 
				if ( vox->channel(PCloud_Filter) && 
						(vox->flag(pcloud::PartSelected) || vox->flag( pcloud::PartHidden ) ))
							useEdit = true;
				else useEdit = false;

				/* user Channels */ 
				bool userChannelSave = _userChannelRender;
				_blockQuantizeInMatrix = false;

				if (userOffset)
				{
					vcd = userOffset->voxelChannel(vox);
					if (vcd) userOffset->unlock( vcd, vox->lodRequest() * vox->fullPointCount() );

					if (!vcd || !vcd->getData()) 
					{
						_userChannelRender = false;
						programFilterOut = REQUIRES_OFFSET;
					}
					else
					{
						_userChannelRender = true;
						_blockQuantizeInMatrix = true;
						setupOffsetChannel(vox, vcd, offsetBlend);
					}
				}
				else _userChannelRender = false;
				
				programFilterOut |= (!useEdit ? REQUIRES_EDIT : 0);

				/* setup shading / transform for this pointcloud */ 
				bool forcePCEval = (lastUseEdit != useEdit ? true : false) || _userChannelRender;

				SetupPointCloud pointcloud(vox->pointCloud(), forcePCEval, programFilterOut );
			
				if (_vp == &_vpOffset && (!vcd || !vcd->getData()))
				{
					ptgl::State::disable(GL_VERTEX_PROGRAM_ARB);
					if (useEdit) _vp = &_vpEdit;
					else _vp = 0;
				}	

				if (_diagnosticDisplay)
				{
					glColor3f(fmod(randcolR, 0.6f) + 0.4f, fmod(randcolG, 0.6f) + 0.4f, fmod(randcolB, 0.6f) + 0.4f);
				}

				/* colours for edit mode selection */ 
				if (_inEditMode || !_useVertexArray || !_glhasVP) 
					ptgl::State::disable(GL_VERTEX_PROGRAM_ARB);

				/*this is a hack - not very nice*/ 
				pushBlending(vox); 

				channelbit = 1;

				for (c=1;c<MAX_CHANNELS; c++) 
				{
					if ((useEdit && c == pcloud::PCloud_Filter) ||
						(vox->channel(c) && _chrenderers[c] && !(_channelFilter & channelbit)))
						_chrenderers[c]->predraw(vox);
					channelbit <<= 1;
				}
				_userChannelRender = userChannelSave;
				
				/* edit mode whole voxel colouring */ 
				if (_inEditMode && _useVertexArray)
				{
					if (vox->flag(pcloud::WholeSelected))
					{
						glColor3ub(SEL_COL_RGB);
						ptgl::ClientState::disable(GL_COLOR_ARRAY);
					}
					if (useEdit || _vp)
					{
						ptgl::State::enable(GL_VERTEX_PROGRAM_ARB);
					}
				}

				/* dynamic reduction handling */ 
				if (millisecs && !vox->flag( pcloud::OutOfCore ))
				{ 
					//double d_size = size;
					double adj = fmax(_minDynamicOutput, _multiplier);// : _minoutput ;
					
					/* front bias calcs */ 
					if (_hasFocalPoint)
					{
						double nfocal = 1.0;
						double nfocal2 = 1.0;

						vector3d vcen(vox->extents().center());
						double vdist = vcen.dist2(wfocalPoint);

						distBounds.normalizedValue(&vdist, &nfocal);

						vdist *= vdist;
						distBounds.normalizedValue(&vdist, &nfocal2);
						if (nfocal2 > 1.0) nfocal2 = 1.0;

						nfocal = 1.0 - nfocal;
						nfocal2 = 1.0 - nfocal2;

						adj *= 0.5 * (nfocal + 3.0 * nfocal2);

						if (vox->extents().inBounds(vector3(wfocalPoint)))
							adj = 3.0;
					}
					else if (_renderflags & Render_FrontBias)
					{
						/*dont do this for 'flat' scenes*/ 
						if (priorityBounds.size(0) > 0.2f)
						{
							float p = vox->priority();
							/*scale p 0 -> 1*/ 
							p -= priorityBounds.lower(0);
							p /= priorityBounds.size(0);

							if (p > 0.001f)
							{
								p *= p;
								adj *= p*3.0f;/*multiply by 3 for invariance (area under x^2[0..1] = 1/3)*/ 
							}
						}
					}
					if (_autoPointSize) glPointSize(calcPointSize(adj, _pointSize));
					
					double d_size = adj < 1 ? size * adj : size;
					size = d_size < size ? (d_size > 0 ? d_size : 1) : size;
				}			
				/* hack */ 
				if (_glhasMultitexture)
				{
					glActiveTexture(GL_TEXTURE0_ARB);
					glClientActiveTexture(GL_TEXTURE0_ARB);
				}		

				/* flush gl state */ 
				ptgl::State::flush();
				ptgl::ClientState::flush();

				/* final size check */ 
				if (size > 0)
				{
					if (!_useVertexArray)
					{				
						renderVoxelPerPoint(vox, size);						// rendering without vertex arrays
					}
					else
					{
						if (_vp && !_glhasVP)								// alternative, non shader rendering
							renderVoxelNoVP(vox, size);						
						
						else
							glDrawArrays(GL_POINTS , 0, size * _density);	// standard rendering
					}
				}
				i == e ? 0 : (*i);

				/* offset channel */ 
				cleanupOffsetChannel();
				if (userOffset && vcd )userOffset->lock( vcd );

				channelbit = 1;
				for (c=1;c<MAX_CHANNELS; c++) 
				{
					if ((useEdit && c == pcloud::PCloud_Filter) ||
						(vox->channel(c) && _chrenderers[c] && !(_channelFilter & channelbit)))
						_chrenderers[c]->postdraw();
					channelbit <<= 1;
				}
				popBlending();

				/* this voxel is not fully rendered */ 
				if (size < vox->lodRequest() * vox->fullPointCount())
					vox->flag(pcloud::Rendered, false);
				else if (front)
					vox->flag(pcloud::Rendered, true);

				lastUseEdit = useEdit;
			}
		}
		/* quit if render is taking too long */ 
		TimeStamp t1; t1.tick();
		if (t1.delta_ms(t0,t1) > (front ? 1000 : 3000) && _pointDensity != PointDensityFull)
			break;
	}
	renderVoxelNoVP(0, 0);		//flush buffers

	//std::cout << g_voxelRenderer.getNumBatchesInFrame() << " batches" << std::endl;

	/* set flag on anything not rendered */ 
	while (i != e)
	{
		(*i)->flag(pcloud::Rendered, false);
		++i;
	}

	SetupPointCloud::endFrame();
	_currentVoxel = 0;

	glDisable(GL_VERTEX_PROGRAM_ARB);

	channelbit = 1;
	for (c=1;c<MAX_CHANNELS; c++)
	{
		if (_chrenderers[c] && !(_channelFilter & channelbit))
			_chrenderers[c]->endFrame();
		channelbit <<= 1;
	}
	ptgl::State::disable(GL_COLOR_SUM_ARB);

	ptgl::ClientState::flush();
	ptgl::State::flush();

	glFlush();

	TimeStamp t1; t1.tick();

	/* lighting is a hack to get normals through for edge shader */ 
	if (isEdgeShaderEnabled())
		remRenderFlag(RenderEngine::Render_Lighting);

	if (millisecs)
	{
		double ms = TimeStamp::delta_ms(t0,t1);
		if (ms > 0.01)	_multiplier *= ((float)millisecs) / ms;
		if (_multiplier > 1000.0) _multiplier = 1000.0;
		if (_multiplier < 0.005f) _multiplier = 0.005f;
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	//if (isGeomShaderEnabled()) drawGeomShaderScale(false);

	if (_diagnosticDisplay) renderDebugInfo();
}
//-----------------------------------------------------
// Set Global Density Value
//-----------------------------------------------------
void RenderEngine::globalDensity(float density)	
{ 
	_density = density; 
	if (_density > 1.0f) _density = 1.0f;
	if (_density < PT_MIN_DENSITY) _density = PT_MIN_DENSITY;
}
//-----------------------------------------------------
// push blending state
//-----------------------------------------------------
void RenderEngine::pushBlending(const pcloud::Voxel *vox)
{
	_renderflagsStore = _renderflags;

	if (!(_renderflags & (Render_RGB | Render_Intensity))) return;

	if (vox->channel(pcloud::PCloud_Intensity) && vox->channel(pcloud::PCloud_RGB))
	{
		/*check blending option*/ 
		if (!(_blend & Render_Intensity))	_renderflags &= ~Render_Intensity;
		if (!(_blend & Render_RGB))			_renderflags &= ~Render_RGB;
	}
	//else if (vox->channel(pcloud::PCloud_Intensity)) _renderflags &= Render_Intensity;
	//else _renderflags &= Render_RGB;
}
void RenderEngine::popBlending()
{
	_renderflags = _renderflagsStore;
}
//-----------------------------------------------------
// RenderGL to Front buffer
//-----------------------------------------------------
void RenderEngine::renderFrontGL()
{
	if (_paused || _pointDensity == PointDensityFull) 
		return;

	renderGL(0, true);
	return;
}
//-----------------------------------------------------
// renderVoxelOutline
//-----------------------------------------------------
void RenderEngine::renderVoxelOutline(const Node*v)
{
	glBegin(GL_LINE_STRIP);
		glVertex3f(v->lx(), v->ly(), v->lz());
		glVertex3f(v->ux(), v->ly(), v->lz());
		glVertex3f(v->ux(), v->uy(), v->lz());
		glVertex3f(v->lx(), v->uy(), v->lz());
		glVertex3f(v->lx(), v->ly(), v->lz());
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3f(v->lx(), v->ly(), v->uz());
		glVertex3f(v->ux(), v->ly(), v->uz());
		glVertex3f(v->ux(), v->uy(), v->uz());
		glVertex3f(v->lx(), v->uy(), v->uz());
		glVertex3f(v->lx(), v->ly(), v->uz());
	glEnd();
	
	glBegin(GL_LINES);
		glVertex3f(v->lx(), v->ly(), v->lz());
		glVertex3f(v->lx(), v->ly(), v->uz());

		glVertex3f(v->ux(), v->ly(), v->lz());
		glVertex3f(v->ux(), v->ly(), v->uz());

		glVertex3f(v->ux(), v->uy(), v->lz());
		glVertex3f(v->ux(), v->uy(), v->uz());

		glVertex3f(v->lx(), v->uy(), v->lz());
		glVertex3f(v->lx(), v->uy(), v->uz());
	glEnd();
}
//-----------------------------------------------------
// renderProgress
//-----------------------------------------------------
void RenderEngine::renderProgress()
{
	return;

	//ptgl::PixelView pview;

	//double loaded = thePointsPager().KBytesLoaded();
	//double requested = thePointsPager().KBytesRequested();

	//if (fabs(requested) < 1) return;

	//glColor3f(0.7f,0.7f,0.7f);

	//loaded /= requested;
	//loaded *= 100;
	//if (loaded > 100) loaded = 100;

	//int r = loaded + pview._r - 120;

	//glColor3f(1.0f,1.0f,1.0f);

	//glBegin(GL_LINE_STRIP);
	//	glVertex2i(r, pview._b + 40);
	//	glVertex2i(r, pview._b + 50);
	//	glVertex2i(pview._r - 120, pview._b + 50);
	//	glVertex2i(pview._r - 120, pview._b + 40);
	//	glVertex2i(r, pview._b + 40);
	//glEnd(); 
}
//-----------------------------------------------------
// renderVoxelOutline
//-----------------------------------------------------
void RenderEngine::renderPriorities()
{
	PointsScene::VoxIterator i = _voxlist.begin();
	PointsScene::VoxIterator e = _voxlist.end();
	
	glColor3f(1.0f,0,0);
	glPointSize(10.0f);

	glBegin(GL_POINTS);

	while (i != e)
	{
		Voxel *vox = *i;
		SetupPointCloud c(vox->pointCloud());

		glColor3f(1.0f-vox->usage(),0,vox->usage());

		vector3 p = vox->extents().center();

		glVertex3fv(p);
		++i;
	}
	glEnd();
}	  
#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif
//-----------------------------------------------------
// Chop off index letter to sort
//-----------------------------------------------------
static bool SortRamps(const std::tstring &a, const std::tstring &b)
{
	std::tstring a1(&a.c_str()[1]);
	std::tstring b1(&b.c_str()[1]);

	return a1 < b1 ? true : false;
}
//-----------------------------------------------------
// load the ramps - searches ramps folder
//-----------------------------------------------------
bool RenderEngine::loadRamps()
{
	thePointsPager().pause();

	if (_glhasMultitexture)
		glActiveTexture(GL_TEXTURE0_ARB);

	/* This sets the alignment requirements for the start of each pixel row in memory.*/ 
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);


	for (int i=0; i<_rampsManager.numColourGradients(); i++)
	{
		ColourGradient *ramp = _rampsManager.getGradientByIndex(i);

		Ramp *r = new Ramp;
		r->data = ramp->m_gradient.img();
		r->height  = 1;
		r->width = ramp->m_gradient.imgWidth();
		r->tex_1d = 0;
		r->tex_2d = 0;
		r->_step = 16338 / r->width;

		_actContext->addRamp(r);

		/* Generate a texture with the associative texture ID stored in the array*/ 
		glGenTextures(1, &r->tex_1d);

		/* Bind the texture to the index and init the texture*/ 
		glBindTexture(GL_TEXTURE_1D, r->tex_1d);

		/* Build Mipmaps for LOD*/ 
		gluBuild1DMipmaps(GL_TEXTURE_1D, 3, r->width, /*height,*/ GL_BGRA, GL_UNSIGNED_BYTE, r->data);

		/*texture quality		*/ 
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}

	thePointsPager().unpause();

//#if defined(_WIN32)
//	WIN32_FIND_DATA ff;
//	TCHAR filepath[260];    
//	std::vector<std::tstring> paths;
//
//	_stprintf(filepath, _T("%s\\ramps\\*.tga"), ptapp::apppath());
//
//	HANDLE hFind = ::FindFirstFile(filepath, &ff);
//	
//	int i=0;
//
//	if (hFind != INVALID_HANDLE_VALUE)
//	{			
//		paths.push_back(ff.cFileName);
//
//		while (::FindNextFile(hFind, &ff))
//			paths.push_back(ff.cFileName);
//	}
//	FindClose(hFind);
//#endif
//	//std::sort(paths.begin(), paths.end(), &SortRamps);
//
//	if (_rampid == -1)
//	{
//		_rampid = 0;
//		_geomshaderRamp = 0;
//	}
//	for (int i=0; i<paths.size(); i++) 
//	{
//		loadRamp(paths[i].c_str());
//		
//		if (_rampid == -1 && 
//			paths[i].rfind(L"Hue.tga") != std::tstring::npos)
//		{
//			_rampid = i;
//			_geomshaderRamp = i;
//		}
//	}
//	thePointsPager().unpause();
//	if (paths.size() < 5) 
//	{
//		if (paths.size()) _lastError = "Some ramps not loaded";
//		else _lastError = "Intensity ramps not loaded";
//
//		/* put some empty ramps in to avoid a crash */ 
//		for (int i=0;i<5;i++)
//		{
//			ubyte *data = new ubyte[256];
//			memset(data, 0, 256);
//
//			Ramp *r = new Ramp;
//			r->data = data;
//			r->height  =1;
//			r->width = 256;
//			r->tex_1d = 0;
//			r->tex_2d = 0;
//
//			_actContext->addRamp( r );
//		}
//
//		return false;
//	}
	return true;
}
//-----------------------------------------------------------------------------
// load ramp texture file
//-----------------------------------------------------------------------------
void RenderEngine::loadRamp(const TCHAR *fname )
{
	assert(false);	//no longer used, ramps are gradient objects now
}
//-----------------------------------------------------------------------------
// Compute point colour according to current settings
//-----------------------------------------------------------------------------
float g_col_geomzscale = 1.0f;
float g_col_constrast = 0.5f;
float g_col_brightness = 1.0f;
//-----------------------------------------------------------------------------
// Colour constants
//-----------------------------------------------------------------------------
void RenderEngine::updateColourConstants()
{
	g_col_geomzscale = vector3(_geomshaderParams).length();

	if (fabs(g_col_geomzscale) < 1) g_col_geomzscale = 1.0;
	g_col_geomzscale = 1.0 / g_col_geomzscale;

	g_col_constrast = _rampBlack;
	g_col_brightness = _rampWhite;
	contrastBrightnessValue(g_col_constrast, g_col_brightness);
}
//
// float version
//
void RenderEngine::computeActualColour(ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb)
{
	double dpnt[] = {pnt[0],pnt[1],pnt[2]};
	computeActualColour(col, dpnt, intensity, rgb);
}
//
// This is used for client code to render data
//
void RenderEngine::computeActualColour(ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb)
{
	/* move into integer space from float gave 3x performance boost */ 
	int pcol [] = { 256, 256, 256 };
	int components = 0;
				
	bool doIntensity = intensity && (_renderflags & Render_Intensity) ? true : false;
	bool doRGB = rgb && (_renderflags & Render_RGB) ? true : false;
	const Ramp *ramp = _actContext->ramp(_rampid);
	const Ramp *gramp = _actContext->ramp(this->_geomshaderRamp);

	/* calculate colour contributions */ 
	if (doRGB)
	{
		if (!doIntensity && !_geomshader) /* handle rgb quickly */ 
		{
			col[0] = rgb[0];
			col[1] = rgb[1];
			col[2] = rgb[2];
			return;
		}
		pcol[0] *= rgb[0];
		pcol[1] *= rgb[1];
		pcol[2] *= rgb[2];
		pcol[0] /= 256;
		pcol[1] /= 256;
		pcol[2] /= 256;

		++components;
	}
	if (doIntensity)
	{		
		/* todo: move into integer space also for performance */ 
		double _in = (*intensity);
		_in /= 65536;
		_in *= g_col_constrast;
		_in += g_col_brightness;

		/* clamp */ 
		if (_in > 1.0f) _in = 1.0f;
		else if (_in < 0) _in = 0;

		int in = (_in * (ramp->width-1));
		
		pcol[2] *= ramp->data[in*4];
		pcol[1] *= ramp->data[in*4+1];
		pcol[0] *= ramp->data[in*4+2];
		pcol[0] /= 256;
		pcol[1] /= 256;
		pcol[2] /= 256;
	}
	if (_geomshader)
	{
		vector3 ps(_geomshaderParams);
		vector3 gz(pnt[0],pnt[1], pnt[2]);
		
		float z = gz.dot(ps) + _geomshaderParams[3];
		bool isblack = false;

		/*edge behaviour*/ 
		if (z > 1.0 || z < 0)
		{
			switch (geomShaderEdgeMode())
			{
			case RepeatEdge: 
				z = fmod(z, 1.0f);				
				while (z<0) z += 1.0f;
				break;
			case ClampEdge:
				if (z > 1.0f) z = 1.0f;
				else if (z < 0) z = 0;
				break;
			case BlackEdge:
				pcol[0] = pcol[1] = pcol[2] = 0;
				isblack = true;
				break;
			}
		}
		if (!isblack)
		{
			int zp = (z * gramp->width);
			
			pcol[2] *= gramp->data[zp*4];
			pcol[1] *= gramp->data[zp*4+1];
			pcol[0] *= gramp->data[zp*4+2];
			pcol[0] /= 256;
			pcol[1] /= 256;
			pcol[2] /= 256;
		}
	}
	else if (!doIntensity && !doRGB)
	{
		col[0] = 255;
		col[1] = 255;
		col[2] = 255;
		return;
	}

	/* assign and clamp */ 
	col[0] = pcol[0];
	col[1] = pcol[1];
	col[2] = pcol[2];
}
//-----------------------------------------------------
//
//-----------------------------------------------------
void RenderEngine::computeActualColours(int numPnts, ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb)
{
	assert(0); /* not implemented */ 	
}
//-----------------------------------------------------
//
//-----------------------------------------------------
void RenderEngine::computeActualColours(int numPnts, ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb)
{
	assert(0); /* not implemented */ 	
}
//-----------------------------------------------------
//
//-----------------------------------------------------
void RenderEngine::renderVoxelPerPoint( pcloud::Voxel * vox, uint size ) 
{
	bool wholeSel = vox->flag(WholeSelected);

	if (wholeSel) glColor3ub(SEL_COL_RGB);
	else glColor3ubv( vox->pointCloud()->displayBaseColor() );

	bool is16bit = vox->channel(pcloud::PCloud_Geometry)->typesize() == 4 ? false : true;
	short *intensity = (short*)_chrenderers[ pcloud::PCloud_Intensity ]->channelArray;
	ubyte *filter = (ubyte*)_chrenderers[ pcloud::PCloud_Filter ]->channelArray;
	pt::vector3s *normals = (pt::vector3s*)_chrenderers[ pcloud::PCloud_Normal ]->channelArray;
	pt::vec3<ubyte> *rgb = (pt::vec3<ubyte>*)_chrenderers[ pcloud::PCloud_RGB ]->channelArray;

	glBegin( GL_POINTS );
	if (is16bit)
	{
		pt::vector3s *geom = (pt::vector3s*)_chrenderers[ pcloud::PCloud_Geometry ]->channelArray;

		for (int p=0;p<size * _density; p++)
		{
//			if (filter && filter[p] & 0) continue;
			if (filter && filter[p] & SELECTED_PNT_BIT)
				glColor3ub(SEL_COL_RGB);
			else if (rgb && !wholeSel) glColor3ubv((GLubyte*)&rgb[p].x);
			if (intensity) glTexCoord1s(intensity[p]);
			if (normals) glNormal3sv(normals[p]);

			glVertex3sv(geom[p]);
		}
	}
	else
	{
		pt::vector3 *geom = (pt::vector3*)_chrenderers[ pcloud::PCloud_Geometry ]->channelArray;

		for (int p=0;p<size * _density; p++) 
		{
			if (filter && filter[p] & 0) continue;
			if (filter && filter[p] & SELECTED_PNT_BIT)
				glColor3ub(SEL_COL_RGB);
			else if (rgb && !wholeSel) glColor3ubv((GLubyte*)&rgb[p].x);
			if (intensity) glTexCoord1s(intensity[p]);
			if (normals) glNormal3sv(normals[p]);

			glVertex3fv(geom[p]);
		}
	}
	glEnd();
}