/*----------------------------------------------------------*/ 
/* RenderEngine.h											*/ 
/* Points Render Engine Interface file						*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004-2007						*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_RENDER_ENGINE
#define POINTOOLS_RENDER_ENGINE 1


#include <ptcloud2/defs.h>
#include <ptgl/glcamera.h>
#include <ptgl/gllight.h>
#include <ptgl/glstate.h>
#include <ptgl/glVertexProgram.h>

#include <ptcloud2/pointcloud.h>
#include <ptengine/colourRamps.h>
#include <ptengine/ptengine_api.h>
#include <ptengine/module.h>

#include <loki/assocvector.h>
#include <set>

namespace pointsengine
{
#ifndef POINTOOLS_TYPEDEFS_HEADER
	typedef unsigned int uint;
	typedef unsigned char ubyte;
	typedef unsigned short ushort;
#endif

class PTENGINE_API RenderEngine : public Module
{
public:
	enum RenderFlag
	{
		Render_Geometry = 1,
		Render_Clipping = 2,
		Render_Lighting = 4,
		Render_Intensity = 8,
		Render_RGB = 16,
		Render_Normals = 32,
		Render_Analytical = 64,
		Render_FrontBias = 128
	};

	RenderEngine();
	~RenderEngine();

	void pushClipping();
	void popClipping();
	bool initialize();

	void renderGL(int millisecs=0, bool front=false, const pcloud::Scene *scene=0);
	void renderScanners();
	void renderFrontGL();
	void renderSafeGL();
	void renderNoVertexArraysGL();
	void renderEditMode();

	void renderDebugInfo();

	void drawPostRenderGraphics(bool safe=false);

	void renderPriorities();

	enum ShaderEdgeMode
	{
		RepeatEdge	=0,
		ClampEdge	=1,
		BlackEdge	=2
	};
	enum ScanPositionStyle
	{
		ScanPosAxis = 1,
		ScanPosPoint = 2,
		ScanPosBox = 3,
		ScanPosSphere = 4,
		ScanPosScanner = 5
	};
	enum RenderMethod
	{
		RenderPoints = 1,
		RenderPointProxies = 2,
		RenderBoxProxies = 4
	};
	enum PointDensity
	{
		PointDensityViewOptimal = 1,
		PointDensityFull = 2
	};

	void renderMethod(uint method);
	uint renderMethod() { return _renderMethod; }

	/*edit mode */ 
	void enableEditMode();
	void disableEditMode();
	bool isInEditMode()	const	{ return _inEditMode; }

	/*settings*/ 
	void minDynamicOutput(float output) { _minDynamicOutput = output; }
	float minDynamicOutput() const { return _minDynamicOutput; }

	void minStaticOutput(float output) { _minStaticOutput = output; }
	float minStaticOutput() const { return _minStaticOutput; }

	void pointSize(float pixels) { _pointSize = pixels; }
	float pointSize() const { return _pointSize; }
	
	void dataRamp(int rampid) { _rampid = rampid; }
	int dataRamp() const { return _rampid; }

	void dataWhitePoint(float white);
	float dataWhitePoint() const { return _rampWhite; }

	void dataBlackPoint(float black);
	float dataBlackPoint() const { return _rampBlack; }

	ShaderEdgeMode dataEdgeMode() const { return _rampEdge; }
	void dataEdgeMode(const ShaderEdgeMode &m)  { _rampEdge = m; }

	GLuint texID(int rampID) const { return _actContext->ramp(rampID)->tex_1d; }

	bool filterUnusedChannels() const { return _filterUnusedChannels; }
	void filterUnusedChannels(bool f);

	void addRenderFlag(RenderFlag flag);
	void remRenderFlag(RenderFlag flag);

	void addBlendFlag(RenderFlag channel) { _blend |= channel; }
	void remBlendFlag(RenderFlag channel) { _blend &= ~channel; }

	/* edge shader */ 
	void enableEdgeShader()			{ _edgeShader = true; }		
	void disableEdgeShader()		{ _edgeShader = false; }
	bool isEdgeShaderEnabled() const { return _edgeShader; }
	void setEdgeShaderTolerance(float t) { _edgeShaderTolerance = t; }
	float getEdgeShaderTolerance() const { return _edgeShaderTolerance; }

	void clearBlendFlags() { _blend = 0; }

	uint renderFlags() const { return _renderflags; }
	uint channelFilter() const { return _channelFilter; }

	void materialAmbient(const pt::vector3 &amb) { memcpy(_ambient, amb, sizeof(float)*3); }
	pt::vector3 materialAmbient() const { return _ambient; }

	void materialDiffuse(const pt::vector3 &diff) { memcpy(_diffuse, diff, sizeof(float)*3); }
	pt::vector3 materialDiffuse() const { return _diffuse; }

	void materialSpecular(const pt::vector3 &spec) { memcpy(_specular, spec, sizeof(float)*3); }
	pt::vector3 materialSpecular() const { return _specular; }

	void materialGlossiness(const float &gloss) { _glossiness = gloss; }
	float materialGlossiness() const { return _glossiness; }

#ifdef HAVE_OPENGL
    void setLight(const ptgl::Light *light) { _light = light; }
	void setCamera(const ptgl::Camera *camera) { _camera = camera; }
#endif
	bool autoDynamicPointSize() const { return _autoPointSize; }
	void autoDynamicPointSize(bool a) { _autoPointSize = a; }

	bool showScanners() const { return _renderScanners; }
	void showScanners(bool r) { _renderScanners = r; }

	const ScanPositionStyle &scanPosStyle() const	{ return _scanPosStyle; }
	void scanPosStyle(ScanPositionStyle sp)			{ _scanPosStyle = sp;	}

	void enableClipbox()	{ _clip = true; }
	void disableClipbox()	{ _clip = false; }

	void globalDensity(float density);
	float globalDensity() const			{ return _density; }

	/* geometry shader */ 
	enum GeomShaderMode
	{
		GeomShaderPlane		=0,
		GeomShaderCylinder	=1,
		GeomShaderSphere	=2
	};

	bool initialisationFailed() const { return _initialisationFailure; }
	const pt::String &getErrorMessage() const { return _lastError; }

	void enableGeometryShader()		{ _geomshader = true; }
	void disableGeometryShader()	{ _geomshader = false; }
	void showGeometryShaderScale() { _geomshaderShowScale = true; }
	void hideGeometryShaderScale() { _geomshaderShowScale = false; }
	bool geometryShaderScaleShown() const { return _geomshaderShowScale; }
	
	void enableUserChannelRender(bool enable) { _userChannelRender = enable; }
	bool isUserChannelRenderEnabled() const		{ return _userChannelRender; }
	
	void geomShaderEdgeMode(ShaderEdgeMode e) { _geomedge = e; }
	ShaderEdgeMode geomShaderEdgeMode() const { return _geomedge; }

	bool isGeomShaderEnabled()	{ return _geomshader; }
	void geomShaderRamp(int ramp)	{ _geomshaderRamp = ramp; }
	int geomShaderRamp()			{ return _geomshaderRamp; }
	void geomShaderParams(int num, const double *params)	{ 
		memcpy(_geomshaderParams, params, num*sizeof(double));
	}
	const double *geomShaderParams() const	{ return _geomshaderParams; }

	void clipBox(const pt::BoundingBox *bb, const pt::vector3 *angles);
	
	void getClipBoxMatrix(mmatrix4d &mat);
	void setClipBoxMatrixOverride(const mmatrix4d &mat);
	void cancelClipBoxOverride();

	inline bool isClipboxEnabled() const { return _clip; }

	void allowInterrupt(bool allow = true);

	/* cancel safeGL render - need to call this from another thread */ 
	void cancelRender() { _cancelRender = true; }

	ptgl::VertexProgram *vertexProgram(uint mode) 
	{	
		Loki::AssocVector <uint, ptgl::VertexProgram*>::iterator i = _vertexPrograms.find(mode);
		return i != _vertexPrograms.end() ? i->second : 0;
	}

	/* compute affect of shaders on a single point */ 
	void computeActualColour(ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb);
	void computeActualColour(ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb);
	void computeActualColours(int numPnts, ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb);
	void computeActualColours(int numPnts, ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb);
	void updateColourConstants();

	void setFocalPoint(const pt::vector3 &pnt) { _hasFocalPoint = true; _focalPoint = pnt; }
	void cancelFocalPoint() { _hasFocalPoint = false; }

	void setAnalysisPoint( const pt::vector3 &pnt) { _hasAnalysisPoint = true; _analysisPoint = pnt; }
	void setPointDensity( const PointDensity &d ) { _pointDensity = d; }
	PointDensity pointDensity() const { return _pointDensity; }
	
	enum DiagnosticDisplay
	{
		DiagnosticLayers = 1,
		DiagnosticStructure = 2
	};
	void setDiagnosticDisplay(uint display) { _diagnosticDisplay = display; }
	void cancelAnalysisPoint() { _hasAnalysisPoint = false; }	

	void useVertexArrays(bool use) { _useVertexArray = use; }
	bool useVertexArrays() const { return _useVertexArray; }

	bool initializeContext();

	const ColourGradientManager &colourGradientManager() const { return _rampsManager; }
private:

	void drawGeomShaderScale(bool safe=false);
	void initVertexProgramsMap();

	void renderVoxelPerPoint( pcloud::Voxel * vox, uint size );
	void renderVoxelNoVP( pcloud::Voxel * vox, uint size );

	typedef Loki::AssocVector <uint, ptgl::VertexProgram*> VPMAP;
	VPMAP _vertexPrograms;

	struct Ramp
	{
		const ubyte *data;
		int			width;
		int			height;
		int			_step;

		GLuint		tex_1d;
		GLuint		tex_2d;

		inline const GLubyte *value(const float &v) const { return &data[(int)(width *v)]; }
		inline const GLubyte *value(const GLushort &v) const { return &data[v / 768]; }
	};
	
	struct Context
	{
		inline const Ramp *ramp(int index) const { return _ramps[index]; }
		void addRamp(Ramp*r) { _ramps.push_back(r); }
		int numRamps() const { return _ramps.size(); }
	private:
		std::vector<Ramp *> _ramps;
	};
	typedef std::map<HGLRC, Context*> ContextMap;
	ContextMap _contexts;
	Context* _actContext;

	//---------------------------------------------------------
	// Settings
	//---------------------------------------------------------
	uint _rampid;
	
	float _pointSize;
	float _minDynamicOutput;
	float _minStaticOutput;
	float _rampWhite;
	float _rampBlack;
	float _edgeShaderTolerance;
	ShaderEdgeMode _rampEdge;

	bool _geomshader;
	bool _geomshaderShowScale;
	int _geomshaderRamp;
	double _geomshaderParams[16];
	GeomShaderMode _geommode;
	ShaderEdgeMode _geomedge;
	PointDensity _pointDensity;

	GLfloat _diffuse[4];
	GLfloat _specular[4];
	GLfloat _ambient[4];
	GLfloat _glossiness;

	uint  _renderflags;
	uint  _blend;
	uint  _renderflagsStore;
	uint  _renderMethod;

	void pushBlending(const pcloud::Voxel *vox);
	void popBlending();

	uint _channelFilter;
	bool _filterUnusedChannels;
	bool _autoPointSize;
	bool _renderScanners;
	bool _edgeShader;

	bool _initialisationFailure;

	ScanPositionStyle _scanPosStyle;

	const pt::BoundingBox *_clipbox;
	const pt::vector3	  *_clipboxangles;
	bool  _clip;
	bool _allowInterrupt;
	bool _cancelRender;
	bool _inEditMode;
	bool _useVertexArray;
	bool _userChannelRender;

	uint _diagnosticDisplay;

	ColourGradientManager	_rampsManager;

	//---------------------------------------------------------
	// Channel Renderer
	//---------------------------------------------------------
	struct _ChannelRenderer 
	{ 
		_ChannelRenderer() : channelArray(0) {}
		virtual bool predraw(const pcloud::Voxel* vox)=0; 
		virtual void postdraw()=0;	
		virtual void startFrame()=0; 
		virtual void endFrame()=0; 
		uint renderFlags() const;
		uint channelFilter() const;
		bool useVertexArray() const;

		const void *channelArray;
	};
	template <pcloud::Channel C, class PointerSetup, uint RenderFlags, GLenum P, GLenum S=0>
	struct ChannelRenderer : public _ChannelRenderer
	{		
		void startFrame() { if (valid()) PointerSetup::startFrame(); }
		void endFrame() { if (valid()) PointerSetup::endFrame(); }

		bool predraw(const pcloud::Voxel* vox)
		{
			active = false;
			if (valid())
			{
				PointerSetup::pushMatrix(vox->channel(C));
				channelArray = PointerSetup::setup(vox->channel(C));
				
				if (P) ptgl::ClientState::enable(P);
				if (S) ptgl::State::enable(S);
				
				active = true;
				return true;
			}
			return false;
		}
		void postdraw()
		{
			if (valid())
			{
				PointerSetup::popMatrix();

				if (P) ptgl::ClientState::disable(P);
				if (S) ptgl::State::disable(S);

				channelArray = 0;
			}
		}		
		inline bool valid() const { return renderFlags() & RenderFlags && !(channelFilter() & (1 << (C-1))); }
		GLuint clientState() const { return useVertexArray() ? P : 0; }
		GLuint state() const { return S; }

		bool active;
	};

	_ChannelRenderer *_chrenderers[MAX_CHANNELS];

	void renderVoxelOutline(const pcloud::Node*v);
	void renderProgress();
	int calcVoxelRenderDensity(const pcloud::Voxel *vox);
	void renderVoxelProxy(const pcloud::Voxel *vox);
	void renderProxies(int millisecs);

	void renderAnalysisInfo();

	void setupLighting();

	bool loadRamps();
	void loadRamp( const TCHAR* );
	bool initializeGL();

	float		_multiplier;
	int			_pointcount;
	int			_frontvoxels;
	float		_density;

#ifdef HAVE_OPENGL
    /*render frame data*/
	const ptgl::Camera *_camera;
	const ptgl::Light  *_light;
#endif
	bool		_dynamic;

	/*frame data*/ 
	std::set<pcloud::Voxel*> _missedvoxels;
	bool _lastcomplete;

	/* focal point */ 
	pt::vector3 _focalPoint;
	bool _hasFocalPoint;

	/* analysis Point */ 
	bool _hasAnalysisPoint;
	pt::vector3 _analysisPoint;

	pt::String _lastError;
};

}

#endif