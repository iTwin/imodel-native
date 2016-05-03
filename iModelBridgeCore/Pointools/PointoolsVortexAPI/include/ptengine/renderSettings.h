#pragma once

#include <pt/typedefs.h>

namespace pointsengine
{
	enum GeomShaderMode
	{
		GeomShaderPlane		=0,
		GeomShaderCylinder	=1,
		GeomShaderSphere	=2
	};

	enum PointDensity
	{
		PointDensityViewOptimal = 1,
		PointDensityFull = 2
	};

	enum ShaderEdgeMode
	{
		RepeatEdge		=0,
		ClampEdge		=1,
		BlackEdge		=2,
		MirroredRepeat = 3
	};
	
	enum LightingMode
	{
		LightSurface = 0,
		LightSilhouette = 1
	};

	class RenderSettings
	{
	public:
		RenderSettings();
		
		void			setToDefault();

		bool			dynamicFrontBias() const			{ return m_dynFrontBias; }
		void			dynamicFrontBias(bool val)			{ m_dynFrontBias = val; }

		bool			dynamicAdaptivePntSize() const		{ return m_dynAdaptivePntSize; }
		void			dynamicAdaptivePntSize(bool val)	{ m_dynAdaptivePntSize = val; }

		float			overallDensity() const				{ return m_overallDensity; }
		void			overallDensity(float val)			{ m_overallDensity = val; }
		
		int				framesPerSec() const				{ return m_fps; }
		void			framesPerSec(int val)				{ m_fps = val; }

		float			pointSize() const					{ return m_pointSize; }
		void			pointSize(float val)				{ m_pointSize = val; }

		float			minStaticOutput() const				{ return m_minStaticOutput; }
		void			minStaticOutput(float val)			{ m_minStaticOutput = val; }

		float			minDynamicOutput() const			{ return m_dynMinOutput; }
		void			minDynamicOutput(float val)			{ m_dynMinOutput = val; }

		/* rgb */ 
		bool			isRGBEnabled() const				{ return m_rgbEnabled; }
		void			enableRGB(bool val)					{ m_rgbEnabled = val; }

		/* intensity */ 
		bool			isIntensityEnabled() const				{ return m_intensityEnabled; }
		void			enableIntensity(bool val)				{ m_intensityEnabled = val; }

		float			intensityWhite() const					{ return m_intensityWhite; }
		void			intensityWhite(float val)				{ m_intensityWhite = val; }

		float			intensityBlack() const					{ return m_intensityBlack; }
		void			intensityBlack(float val)				{ m_intensityBlack = val; }

		ShaderEdgeMode	intensityGradientEdge() const				{ return m_intensityRampEdge; }
		void			intensityGradientEdge(ShaderEdgeMode val)	{ m_intensityRampEdge = val; }

		int				intensityGradient() const				{ return m_intensityGradient; }
		void			intensityGradient(int val)				{ m_intensityGradient = val; }

		/* Plane shader */ 
		bool			isGeomShaderEnabled() const				{ return m_geomShader; }
		void			enableGeomShader(bool val)				{ m_geomShader = val; }

		bool			geomShaderShowScale() const				{ return m_geomShaderShowScale; }
		void			geomShaderShowScale(bool val)			{ m_geomShaderShowScale = val; }

		int				geomShaderGradient() const				{ return m_geomShaderGradient; }
		void			geomShaderGradient(int val)				{ m_geomShaderGradient = val; }

		ShaderEdgeMode	geomShaderEdge() const					{ return m_geomShaderEdge; }
		void			geomShaderEdge(ShaderEdgeMode val)		{ m_geomShaderEdge = val; }

		const double	*geomShaderParams() const;
		void			geomShaderParams( int numValues, const double *val );

		/* lighting based effects */ 
		bool			isLightingEnabled() const				{ return m_lightEnabled; }
		void			enableLighting(bool val)				{ m_lightEnabled = val; }

		LightingMode	lightingMode() const					{ return m_lightMode; }
		void			lightingMode(LightingMode val)			{ m_lightMode = val; }
		
		const float		*materialDiffuse() const				{ return m_lightDiffuse; }
		void			materialDiffuse( const float *diff3 );

		const float		*materialSpecular() const				{ return m_lightSpecular; }
		void			materialSpecular( const float *spec3 );

		const float		materialGlossiness() const				{ return m_lightGlossiness; }
		void			materialGlossiness( float val )			{ m_lightGlossiness = val; }

		const float		*lightAmbient() const					{ return m_lightAmbient; }
		void			lightAmbient( const float *amb );

		float			silhouetteTolerance() const				{ return m_silhouetteTolerance; }
		void			silhouetteTolerance(float val)			{ m_silhouetteTolerance = val; }

		static	 const ubyte	*selectionColour();
		static	void			selectionColour(const ubyte *col3bytes);

		bool			clippingEnabled() const					{ return m_clippingEnabled; }
		void			clippingEnabled(bool val)				{ m_clippingEnabled= val; }

		const			RenderSettings		&operator=(const RenderSettings &settings);
		
		void			enableUserChannelRender(bool enable)	{ m_userChannelRender=enable; }

	private:
		int				m_fps;
		float			m_pointSize;
		float			m_dynMinOutput;
		float			m_minStaticOutput;

		float			m_overallDensity;
		bool			m_dynAdaptivePntSize;
		bool			m_dynFrontBias;
		bool			m_intensityEnabled;
		bool			m_rgbEnabled;

		float			m_intensityWhite;
		float			m_intensityBlack;
		ShaderEdgeMode	m_intensityRampEdge;
		int				m_intensityGradient;

		bool			m_geomShader;
		bool			m_geomShaderShowScale;
		int				m_geomShaderGradient;
		double			m_geomShaderParams[16];
		GeomShaderMode	m_geomShaderMode;
		ShaderEdgeMode	m_geomShaderEdge;
		PointDensity	m_pointDensity;

		bool			m_lightEnabled;
		LightingMode	m_lightMode;
		GLfloat			m_lightDiffuse[4];
		GLfloat			m_lightSpecular[4];
		GLfloat			m_lightAmbient[4];
		GLfloat			m_lightGlossiness;

		float			m_silhouetteTolerance;
		bool			m_autoPointSize;
		bool			m_allowInterrupt;
		bool			m_cancelRender;

		bool			m_useVertexArray;
		bool			m_userChannelRender;

		static ubyte	m_selectionColour[4];

		bool			m_clippingEnabled;
	};
}