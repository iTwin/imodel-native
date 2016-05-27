#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL
#include <ptapi/PointoolsVortexAPI.h>

#include <ptengine/PointsFilter.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/RenderSettings.h>
#include <ptengine/RenderContext.h>
#include <ptengine/engine.h>

#include <pt/timestamp.h>


using namespace pt;
using namespace pointsengine;

extern int				g_timeOut;
extern int				setLastErrorCode( int );
extern TimeStamp		g_startTime;
extern RenderContext	*g_currentRenderContext;

extern pcloud::Scene*		sceneFromHandle(PThandle handle);
extern pcloud::PointCloud*	cloudFromHandle(PThandle cloud);

//-------------------------------------------------------------------------------
// Shader settings for each viewport
//-------------------------------------------------------------------------------
struct ShaderSetup
{
	PTbool	rgb;
	PTbool	intensity;
	PTbool	blending;
	PTbool	plane;
	PTbool	lighting;
	PTbool	adaptivePntSize;
	PTbool	clipping;
	PTbool	frontBias;
	PTbool	res[2];

	PTfloat	pointSize;
	PTfloat	planeDistance;
	PTfloat	planeOffset;
	pt::vector3	planeVector;
	PTint	planeRamp;
	PTint	planeEdge;

	PTfloat intensityContrast;
	PTfloat	intensityBrightness;
	PTint	intensityRamp;

	PTfloat materialDiffuse;
	PTfloat materialAmbient;
	PTfloat materialSpecular;
	PTfloat materialGlossiness;
	
	PTfloat globalDensity;

	bool	channelRender;

	ShaderSetup()
	{
		defaults();
	}
	void defaults()
	{
		rgb = true;
		intensity = true;
		blending = true;
		plane = false;
		lighting = false;
		pointSize = 1.0f;
		intensityContrast = 50.0f;
		intensityRamp = 0;
		intensityBrightness = 180.0f;
		planeVector.set(0,0,1.0f);
		planeDistance = 20.0f;
		planeOffset = 0;
		planeRamp = 0;
		adaptivePntSize = true;
		frontBias = false;
		clipping = false;
		materialDiffuse = 0.8f;
		materialAmbient = 0.2f;
		materialSpecular = 1.0f;
		materialGlossiness = 0.5f;
		globalDensity = 1.0f;
		planeEdge = PT_EDGE_REPEAT;
	}
	void operator = (const ShaderSetup &s)
	{
		if (&s != this)
		{
			rgb = s.rgb;
			intensity = s.intensity;
			blending = s.blending;
			plane = s.plane;
			lighting = s.lighting;
			adaptivePntSize = s.adaptivePntSize;
			clipping = s.clipping;

			pointSize = s.pointSize;
			planeDistance = s.planeDistance;
			planeOffset = s.planeOffset;
			planeVector = s.planeVector;
			planeRamp = s.planeRamp;
			planeEdge = s.planeEdge;

			intensityContrast = s.intensityContrast;
			intensityBrightness = s.intensityBrightness;
			intensityRamp = s.intensityRamp;

			materialDiffuse = s.materialDiffuse;
			materialAmbient = s.materialAmbient;
			materialSpecular = s.materialSpecular;
			materialGlossiness = s.materialGlossiness;

			globalDensity = s.globalDensity;
			channelRender = s.channelRender;

			frontBias = s.frontBias;
		}
	}	
	void apply()
	{
		RenderSettings *settings = 0;

		if (g_currentRenderContext)
			settings = g_currentRenderContext->settings();	
		else return;

		settings->enableRGB( rgb );
		settings->enableIntensity( intensity );
		settings->enableLighting( lighting );
		settings->intensityWhite( intensityBrightness/360.0f );
		settings->intensityBlack( intensityContrast/360.0f );
		settings->intensityGradient( intensityRamp );
		settings->enableGeomShader( plane );
		settings->pointSize( pointSize );
		settings->dynamicAdaptivePntSize( adaptivePntSize );
		settings->enableUserChannelRender( channelRender );
		settings->clippingEnabled( clipping );

		/*
		if (blending)
		{
			theRenderEngine().addBlendFlag(RenderEngine::Render_Intensity);
			theRenderEngine().addBlendFlag(RenderEngine::Render_RGB);
		}
		else
		{
			theRenderEngine().remBlendFlag(RenderEngine::Render_Intensity);
			theRenderEngine().addBlendFlag(RenderEngine::Render_RGB);
		}
		*/		
		
		/* density reduction in timeout mode */ 
		float red_density = globalDensity;

		if (g_timeOut > 0)
		{
			static int lastMultiple = 0;

			TimeStamp tnow; tnow.tick();
			int s = static_cast<int>(TimeStamp::delta_s(g_startTime, tnow));
			s /= g_timeOut;
			
			float factor = 0.6f;

			for (int t=0;t<s;t++)
			{
				red_density *= factor;
				factor *= factor;
			}
			if (s > lastMultiple)
			{
				g_timeOut = static_cast<int>(g_timeOut * 0.8);
				lastMultiple = s;
			}
		}
		else if (g_timeOut == 0)
			red_density = 0.001;

		settings->overallDensity(red_density);

		pt::vector3 materialprop(materialAmbient, materialAmbient, materialAmbient);
		settings->lightAmbient( materialprop );

		materialprop.set(materialDiffuse, materialDiffuse, materialDiffuse);
		settings->materialDiffuse(materialprop);

		materialprop.set(materialSpecular, materialSpecular, materialSpecular);
		settings->materialSpecular(materialprop);

		settings->materialGlossiness(materialGlossiness);

		settings->dynamicFrontBias( frontBias );

		/* plane shader */ 
		vector4d plane;
		
		if (abs(planeDistance) < 0.01f) planeDistance = planeDistance < 0 ? -0.01f : 0.01f;

		if (planeVector.length2() > 0.1f)
		{
			planeVector.normalize();

			// get distance in project space
			pt::vector3d origin(planeVector.x, planeVector.y, planeVector.z);
			pt::vector3d coordinateBase;
			ptGetCoordinateBase( coordinateBase );

			origin *= planeOffset;
			origin -= coordinateBase;

			planeOffset = static_cast<PTfloat>(origin.dot( pt::vector3d(planeVector) ));

			plane.x = planeVector.x;
			plane.y = planeVector.y;
			plane.z = planeVector.z;
			plane /= planeDistance;
		}
		else
		{
			plane.x = 0; plane.y = 0; plane.z = 1.0f;
		}
		plane.w = -(planeOffset / planeDistance);
		settings->geomShaderGradient( planeRamp );
		settings->geomShaderParams( 4, plane );
		
		settings->geomShaderEdge( (pointsengine::ShaderEdgeMode)planeEdge );
	}
	void initialise()
	{
		ptGetShaderOptioni(PT_INTENSITY_SHADER_RAMP, &intensityRamp);
		ptGetShaderOptioni(PT_PLANE_SHADER_RAMP, &planeRamp);
	}
};
ShaderSetup	g_shaders[PT_MAX_VIEWPORTS];

#ifdef HAVE_OPENGL
extern ptgl::Light g_light;

struct LightSetup
{
	pt::vector3 ambient;
	pt::vector3 diffuse;
	pt::vector3 specular;

	PTfloat ambientStrength;
	PTfloat diffuseStrength;
	PTfloat specularStrength;

	pt::vector3 direction;
	pt::vector3 eulers;
	bool asEulers;

	LightSetup() { defaults(); }

	void defaults()
	{
		ambientStrength = 1.0f;
		diffuseStrength = 1.0f;
		specularStrength = 1.0f;

		ambient.set(0.15f, 0.15f, 0.15f);
		diffuse.set(0.7f, 0.7f, 0.7f);
		specular.set(1.0f, 1.0f, 1.0f);

		asEulers = true;
		direction.set(1.0f, 1.0f, 1.0f);
		eulers.set(0,45.0f,45.0f);
	}	

	void operator = (const LightSetup &s)
	{
		if (&s != this)
			memcpy(this, &s, sizeof(this));
	}	
	void apply()
	{
		g_light.setAmbient(ambient * ambientStrength);
		g_light.setDiffuse(diffuse * diffuseStrength);
		g_light.setSpecular(specular * specularStrength);

		if (asEulers)
		{
			g_light.directionFromEulers(eulers);
			memcpy(direction, g_light.getDirection(), sizeof(direction));
		}
		else
		{
			g_light.setDirection(direction);
			memcpy(eulers, g_light.directionAsEulers(), sizeof(eulers));
		}
	}
};
//-------------------------------------------------------------------------------
LightSetup g_lights[PT_MAX_VIEWPORTS];
#endif

extern PTuint g_currentViewport;
//-------------------------------------------------------------------------------
PTvoid _ptInitialiseShaders()
{
	for (int i=0; i<PT_MAX_VIEWPORTS;i++)
		g_shaders[i].initialise();
}
//-------------------------------------------------------------------------------
// Apply shader / Light: these functions are not exported
//-------------------------------------------------------------------------------
PTvoid _ptApplyShader(PTint viewport)
{
	g_shaders[viewport].apply();
}
#ifdef HAVE_OPENGL
//-------------------------------------------------------------------------------
PTvoid _ptApplyLight(PTint viewport)
{
	g_lights[viewport].apply();
}
#endif
//-------------------------------------------------------------------------------
// Enable shader option
//-------------------------------------------------------------------------------
PTvoid PTAPI ptEnable(PTenum option)
{
	setLastErrorCode( PTV_SUCCESS );

	switch(option)
	{
	case PT_RGB_SHADER:	
		g_shaders[g_currentViewport].rgb = true; break;

	case PT_INTENSITY_SHADER:
		g_shaders[g_currentViewport].intensity = true; break;	

	case PT_BLENDING_SHADER:
		g_shaders[g_currentViewport].blending = true; break;

	case PT_PLANE_SHADER:
		g_shaders[g_currentViewport].plane = true; break;

	case PT_LIGHTING:
		g_shaders[g_currentViewport].lighting = true; break;

	case PT_CLIPPING:
		g_shaders[g_currentViewport].clipping = true; break;

	case PT_CHANNEL_RENDER:
		g_shaders[g_currentViewport].channelRender = true; break;

	case PT_ADAPTIVE_POINT_SIZE:
		g_shaders[g_currentViewport].adaptivePntSize = true; break;

	case PT_FRONT_BIAS:
		g_shaders[g_currentViewport].frontBias = true; break;

	default:
		setLastErrorCode( PTV_INVALID_OPTION );
		debugAssertM(0, "Unsupported ptEnable option");
	}
}
//-------------------------------------------------------------------------------
// Disbale shader option
//-------------------------------------------------------------------------------
PTvoid PTAPI ptDisable(PTenum option)
{
	setLastErrorCode( PTV_SUCCESS );

	switch(option)
	{
	case PT_RGB_SHADER:	
		g_shaders[g_currentViewport].rgb = false; break;

	case PT_INTENSITY_SHADER:
		g_shaders[g_currentViewport].intensity = false; break;	

	case PT_BLENDING_SHADER:
		g_shaders[g_currentViewport].blending = false; break;

	case PT_PLANE_SHADER:
		g_shaders[g_currentViewport].plane = false; break;

	case PT_LIGHTING:
		g_shaders[g_currentViewport].lighting = false; break;

	case PT_CLIPPING:
		g_shaders[g_currentViewport].clipping = false; break;

	case PT_CHANNEL_RENDER:
		g_shaders[g_currentViewport].channelRender = false; break;

	case PT_ADAPTIVE_POINT_SIZE:
		g_shaders[g_currentViewport].adaptivePntSize = false; break;

	case PT_FRONT_BIAS:
		g_shaders[g_currentViewport].frontBias = false; break;

	default:
		setLastErrorCode( PTV_INVALID_OPTION );
		debugAssertM(0, "Unsupported ptDisable option");
	}
}
//-------------------------------------------------------------------------------
// Enabled check
//-------------------------------------------------------------------------------
PTbool PTAPI ptIsEnabled(PTenum option)
{
	setLastErrorCode( PTV_SUCCESS );

	switch(option)
	{
	case PT_LIGHTING:			return g_shaders[g_currentViewport].lighting; 
	case PT_PLANE_SHADER:		return g_shaders[g_currentViewport].plane; 
	case PT_INTENSITY_SHADER:	return g_shaders[g_currentViewport].intensity; 
	case PT_RGB_SHADER:			return g_shaders[g_currentViewport].rgb; 
	case PT_BLENDING_SHADER:	return g_shaders[g_currentViewport].blending; 
	case PT_CLIPPING:			return g_shaders[g_currentViewport].clipping; 
	case PT_ADAPTIVE_POINT_SIZE:return g_shaders[g_currentViewport].adaptivePntSize;
	case PT_FRONT_BIAS:			return g_shaders[g_currentViewport].frontBias;
	default:
		debugAssertM(0, "Unsupported ptIsEnabled option"); return PT_TRUE;
		setLastErrorCode( PTV_INVALID_OPTION );
	}	
}
//-------------------------------------------------------------------------------
// Default Shader
//-------------------------------------------------------------------------------
PTvoid PTAPI ptResetShaderOptions()
{
	g_shaders[g_currentViewport].defaults();	
}
//-------------------------------------------------------------------------------
// Copy settings to another viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptCopyShaderSettings(PTuint dest_viewport)
{
	g_shaders[dest_viewport] = g_shaders[g_currentViewport];		
}
//-------------------------------------------------------------------------------
// Copy settings to another viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptCopyShaderSettingsToAll()
{
	for (int i=0; i<PT_MAX_VIEWPORTS;i++)
		if (i!=g_currentViewport)
			g_shaders[i] = g_shaders[g_currentViewport];
}
//-------------------------------------------------------------------------------
// Point Size
//-------------------------------------------------------------------------------
PTres PTAPI ptPointSize(PTfloat size)
{
	if (size < 1 || size > 10)
	{
		return setLastErrorCode( PTV_VALUE_OUT_OF_RANGE );;
	}
	else
	{
		g_shaders[g_currentViewport].pointSize = size;
		return setLastErrorCode( PTV_SUCCESS );
	}
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptShaderOptionf(PTenum shader_option, PTfloat value)
{
	switch(shader_option)
	{	
	case PT_PLANE_SHADER_DISTANCE: 
		g_shaders[g_currentViewport].planeDistance = value; break;

	case PT_PLANE_SHADER_OFFSET: 
		g_shaders[g_currentViewport].planeOffset = value; break;

	case PT_INTENSITY_SHADER_CONTRAST: 
		g_shaders[g_currentViewport].intensityContrast = value; break;

	case PT_INTENSITY_SHADER_BRIGHTNESS: 
		g_shaders[g_currentViewport].intensityBrightness = value; break;
	
	case PT_MATERIAL_AMBIENT:
		g_shaders[g_currentViewport].materialAmbient = value; break;

	case PT_MATERIAL_DIFFUSE:
		g_shaders[g_currentViewport].materialDiffuse = value; break;

	case PT_MATERIAL_SPECULAR:
		g_shaders[g_currentViewport].materialSpecular = value; break;

	case PT_MATERIAL_GLOSSINESS:
		g_shaders[g_currentViewport].materialGlossiness = value; break;

	default:
		debugAssertM(0, "Invalid Float Shader Option");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptShaderOptionfv(PTenum shader_option, PTfloat *value)
{
	switch(shader_option)
	{
	case PT_PLANE_SHADER_VECTOR: 
		g_shaders[g_currentViewport].planeVector.set(value); 
		break;

	default:
		debugAssertM(0, "Invalid Float Vector Shader Option");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptGetShaderOptionfv(PTenum shader_option, PTfloat *values)
{
	switch(shader_option)
	{
	case PT_PLANE_SHADER_VECTOR: 
		g_shaders[g_currentViewport].planeVector.get(values); 
		break;

	default:
		debugAssertM(0, "Invalid Get Float Vector Shader Option");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptShaderOptioni(PTenum shader_option, PTint value)
{
	PTenum type;
	int ramp=-1, i=0;;
	int numRamps = ptNumRamps();

	switch(shader_option)
	{
	case PT_INTENSITY_SHADER_RAMP:

		for (; i<numRamps; i++)
		{
			ptRampInfo(i, &type);

			if (type & PT_INTENSITY_RAMP_TYPE)
				ramp++;

			if (ramp ==  value)
			{
				g_shaders[g_currentViewport].intensityRamp = i;
				break;
			}
		}
		break;

	case PT_PLANE_SHADER_RAMP:

		for (; i<numRamps; i++)
		{
			ptRampInfo(i, &type);
		
			if (type & PT_PLANE_RAMP_TYPE)
				ramp++;

			if (ramp ==  value)
			{
				g_shaders[g_currentViewport].planeRamp = i;
				break;
			}
		}
		break;

	case PT_PLANE_SHADER_EDGE:
		g_shaders[g_currentViewport].planeEdge = value;
		break;

	default:
		debugAssertM(0, "Invalid Int Shader Option");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptGetShaderOptioni(PTenum shader_option, PTint *value)
{
	PTenum type;
	int ramp = 0;

	switch(shader_option)
	{
	case PT_INTENSITY_SHADER_RAMP:
		{
		int iramp = g_shaders[g_currentViewport].intensityRamp;

		for (int i=0; i<=iramp; i++)
		{
			ptRampInfo(i, &type);
			if (type & PT_INTENSITY_RAMP_TYPE)
				ramp++;
		}
		(*value) = ramp-1;
		}
		break;

	case PT_PLANE_SHADER_RAMP:

		{
		int iramp = g_shaders[g_currentViewport].planeRamp;

		for (int i=0; i<=iramp; i++)
		{
			ptRampInfo(i, &type);
			if (type & PT_PLANE_RAMP_TYPE)
				ramp++;
		}
		(*value) = ramp-1;
		}
		break;

	case PT_PLANE_SHADER_EDGE:
		(*value) = g_shaders[g_currentViewport].planeEdge;
		break;

	default:
		debugAssertM(0, "Invalid Get Int Shader");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Shader Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptGetShaderOptionf(PTenum shader_option, PTfloat *value)
{
	switch(shader_option)
	{	
	case PT_PLANE_SHADER_DISTANCE: 
		(*value) = g_shaders[g_currentViewport].planeDistance; break;

	case PT_PLANE_SHADER_OFFSET:  
		(*value) = g_shaders[g_currentViewport].planeOffset; break;

	case PT_INTENSITY_SHADER_CONTRAST: 
		(*value) = g_shaders[g_currentViewport].intensityContrast; break;

	case PT_INTENSITY_SHADER_BRIGHTNESS: 
		(*value) = g_shaders[g_currentViewport].intensityBrightness; break;

	case PT_PLANE_SHADER_VECTOR: 
		memcpy(value, g_shaders[g_currentViewport].planeVector, sizeof(PTfloat)*3); 
		break;

	case PT_MATERIAL_AMBIENT:
		(*value) = g_shaders[g_currentViewport].materialAmbient; break;

	case PT_MATERIAL_DIFFUSE:
		(*value) = g_shaders[g_currentViewport].materialDiffuse; break;

	case PT_MATERIAL_SPECULAR:
		(*value) = g_shaders[g_currentViewport].materialSpecular; break;

	case PT_MATERIAL_GLOSSINESS:
		(*value) = g_shaders[g_currentViewport].materialGlossiness; break;

	default:
		debugAssertM(0, "Invalid Get Float Shader");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
#ifdef HAVE_OPENGL
//-------------------------------------------------------------------------------
// Default LIght
//-------------------------------------------------------------------------------
PTvoid PTAPI ptResetLightOptions()
{
	g_lights[g_currentViewport].defaults();	
}
//-------------------------------------------------------------------------------
// Light Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptLightOptionf(PTenum light_option, PTfloat value)
{
	if (value < 0 || value > 3.0f) 
		return setLastErrorCode( PTV_VALUE_OUT_OF_RANGE );

	switch(light_option)
	{	
	case PT_LIGHT_STRENGTH: 
		g_lights[g_currentViewport].diffuseStrength = value; 
		g_lights[g_currentViewport].specularStrength = value; 
		g_lights[g_currentViewport].ambientStrength = value; 
		break;
	
	default:
		debugAssertM(0, "Invalid Get Float Light");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Light Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptLightOptionfv(PTenum shader_option, PTfloat *value)
{
	switch(shader_option)
	{
	case PT_LIGHT_VECTOR: 
		memcpy(g_lights[g_currentViewport].direction, value, sizeof(PTfloat)*3);
		g_lights[g_currentViewport].asEulers = false;		
		break;

	case PT_LIGHT_COLOUR: 
		memcpy(g_lights[g_currentViewport].ambient, value, sizeof(PTfloat)*3);
		memcpy(g_lights[g_currentViewport].specular, value, sizeof(PTfloat)*3);
		memcpy(g_lights[g_currentViewport].diffuse, value, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_AMBIENT_COLOUR: 
		memcpy(g_lights[g_currentViewport].ambient, value, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_DIFFUSE_COLOUR: 
		memcpy(g_lights[g_currentViewport].diffuse, value, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_SPECULAR_COLOUR: 
		memcpy(g_lights[g_currentViewport].specular, value, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_ANGLE: 
		memcpy(g_lights[g_currentViewport].eulers, value, sizeof(PTfloat)*3);
		g_lights[g_currentViewport].asEulers = true;
		break;

	default:
		debugAssertM(0, "Invalid Get Float Light");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
// Light Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptLightOptioni(PTenum shader_option, PTint value)
{
	debugAssertM(0, "Invalid Int Light Option");
	return setLastErrorCode( PTV_INVALID_OPTION );
}
//-------------------------------------------------------------------------------
// Light Options
//-------------------------------------------------------------------------------
PTres	PTAPI ptGetLightOptionf(PTenum light_option, PTfloat *value)
{
	switch(light_option)
	{	
	case PT_LIGHT_VECTOR: 
		memcpy(value, g_lights[g_currentViewport].direction, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_COLOUR: 
		memcpy(value, g_lights[g_currentViewport].diffuse, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_AMBIENT_COLOUR: 
		memcpy(value, g_lights[g_currentViewport].ambient, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_DIFFUSE_COLOUR: 
		memcpy(value, g_lights[g_currentViewport].diffuse, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_SPECULAR_COLOUR: 
		memcpy(value, g_lights[g_currentViewport].specular, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_ANGLE: 
		memcpy(value, g_lights[g_currentViewport].eulers, sizeof(PTfloat)*3);
		break;

	case PT_LIGHT_STRENGTH: 
		(*value) = g_lights[g_currentViewport].diffuseStrength;
		break;

	default:
		debugAssertM(0, "Invalid Light option");
		return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}

//-------------------------------------------------------------------------------
// Light Options
//-------------------------------------------------------------------------------
PTres PTAPI ptGetLightOptioni(PTenum light_option, PTint *value)
{
	return setLastErrorCode( PTV_INVALID_OPTION );
}
//-------------------------------------------------------------------------------
// Copy settings to another viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptCopyLightSettings(PTuint dest_viewport)
{
	g_lights[dest_viewport] = g_lights[g_currentViewport];		
}
//-------------------------------------------------------------------------------
// Copy settings to another viewport
//-------------------------------------------------------------------------------
PTvoid	PTAPI ptCopyLightSettingsToAll()
{
	for (int i=0; i<PT_MAX_VIEWPORTS;i++)
		if (i!=g_currentViewport)
			g_lights[i] = g_lights[g_currentViewport];
}
#endif
//-------------------------------------------------------------------------------
PTvoid PTAPI ptGlobalDensity(PTfloat den)
{
	g_shaders[g_currentViewport].globalDensity = den;
}
//-------------------------------------------------------------------------------
PTfloat PTAPI ptGetGlobalDensity()
{
	return g_shaders[g_currentViewport].globalDensity;
}
//-------------------------------------------------------------------------------
PTuint	PTAPI ptGetPerViewportDataSize()
{
#ifdef HAVE_OPENGL
	return 4 + sizeof(ShaderSetup) + sizeof(LightSetup);
#else
    return 4 + sizeof(ShaderSetup);
#endif
}
//-------------------------------------------------------------------------------
PTuint	PTAPI ptGetPerViewportData( PTubyte *data )
{
	data[0] = 1;
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;

    memcpy(&data[4], &g_shaders[g_currentViewport], sizeof(ShaderSetup));

#ifdef HAVE_OPENGL
    memcpy(&data[4 + sizeof(ShaderSetup)], &g_lights[g_currentViewport], sizeof(LightSetup));
    return 4 + sizeof(ShaderSetup) + sizeof(LightSetup);
#else
    return 4 + sizeof(ShaderSetup);
#endif
    }
//-------------------------------------------------------------------------------
PTres	PTAPI ptSetPerViewportData( const PTubyte *data )
{
	PTubyte version[4];

	memcpy(version, data, 4);
	if (data[0] == 1 && data[1] == 0 && data[2] == 0&& data[3] == 0)
	{
		memcpy(&g_shaders[g_currentViewport], &data[4], sizeof(ShaderSetup));
#ifdef HAVE_OPENGL
		memcpy(&g_lights[g_currentViewport], &data[4+ sizeof(LightSetup)], sizeof(LightSetup));
#endif
	}
	else
	{
		return setLastErrorCode( PTV_INVALID_BLOCK_VERSION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-------------------------------------------------------------------------------
PTres PTAPI ptAddCustomRamp( const PTstr name, PTint numKeys, const PTfloat *positions, const PTubyte* colour3vals, PTbool interpolateInHSL )
{
	pt::String gname( name );

	ColourGradient *gradient = new ColourGradient( gname.c_u8str() );

	for (int i=0; i<numKeys; i++)
	{
		pt::Color col;
		col.rgb(colour3vals[i*3], colour3vals[i*3+1], colour3vals[i*3+2]);

		gradient->m_gradient.addKey( positions[i], col, interpolateInHSL ? GradColHSL : GradColRGB );
	}
	
	RenderResourceManager::instance()->gradientManager()->addGradient( gradient );

	return PTV_SUCCESS;
}
PTres	PTAPI ptSetOverrideColor( PThandle cloud_or_scene, const PTfloat *rgb3 )
{
	pcloud::PointCloud *cloud = 0;
	pcloud::Scene *scene = 0;

    typedef unsigned char ColorType;
    if (cloud = cloudFromHandle(cloud_or_scene))
	{
    ColorType rgb[] = { (ColorType)(rgb3[0]*255), (ColorType)(rgb3[1]*255), (ColorType)(rgb3[2]*255) };
		cloud->overrideColor( rgb );
		cloud->enableOverrideColor(true);
	}
	// if its a scene we set the col for every cloud
	else if (scene = sceneFromHandle(cloud_or_scene))
	{
		for (uint i=0; i<scene->size(); i++)
		{
			cloud = scene->cloud(i);
            ColorType rgb[] = { (ColorType)(rgb3[0] * 255), (ColorType)(rgb3[1] * 255), (ColorType)(rgb3[2] * 255) };
            cloud->overrideColor( rgb );
			cloud->enableOverrideColor(true);
		}
	}
	else return setLastErrorCode( PTV_INVALID_HANDLE );

	return PTV_SUCCESS;
}

PTres	PTAPI ptGetOverrideColor( PThandle cloud_or_scene, PTfloat *rgb3 )
{
	pcloud::PointCloud *cloud = 0;
	pcloud::Scene *scene = 0;

	cloud = cloudFromHandle(cloud_or_scene);

	if (!cloud)
	{
		scene = sceneFromHandle(cloud_or_scene);
		if (scene) cloud = scene->cloud(0);
	}
	if (cloud)
	{
		rgb3[0] = (float)cloud->overrideColor()[0]/255;
		rgb3[1] = (float)cloud->overrideColor()[0]/255;
		rgb3[2] = (float)cloud->overrideColor()[0]/255;
	}
	// if its a scene we set the col for every cloud
	else return setLastErrorCode( PTV_INVALID_HANDLE );

	return PTV_SUCCESS;
}
PTres	PTAPI ptRemoveOverrideColor( PThandle cloud_or_scene )
{
	pcloud::PointCloud *cloud = 0;
	pcloud::Scene *scene = 0;

	if (cloud = cloudFromHandle(cloud_or_scene))
	{
		cloud->enableOverrideColor(false);
	}
	// if its a scene we set the col for every cloud
	else if (scene = sceneFromHandle(cloud_or_scene))
	{
		for (uint i=0; i<scene->size(); i++)
		{
			scene->cloud(i)->enableOverrideColor(false);
		}
	}
	else return setLastErrorCode( PTV_INVALID_HANDLE );

	return PTV_SUCCESS;
}