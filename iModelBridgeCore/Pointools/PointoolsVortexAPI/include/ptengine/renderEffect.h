#pragma once

#include <pt/typedefs.h>

#include <ptengine/rendersettings.h>
#include <ptengine/renderPointsBuffer.h>
#include <ptengine/renderEnvironment.h>
#include <ptengine/renderResourceManager.h>

namespace ptgl
{
	class Shader;
}
namespace pointsengine
{
	class RenderContext;

	enum ShaderStandardUniforms
	{
		Uniform_Quantize_O			= 0x01,		
		Uniform_Quantize_S			= 0x02,			
		Uniform_RegMatrix			= 0x04,	
		Uniform_CloudToPrjMatrix	= 0x08,	
		Uniform_EyeMatrix			= 0x10,	 	
		Uniform_EyeMatrixInverted	= 0x20,
		Uniform_ClipBoxMatrix		= 0x40
	};

	typedef void *	ShaderObj;

	class RenderEffectI
	{
	public:
		/* traits */ 
		virtual bool			compatibleEnvironment( RenderEnvironment e ) const =0;
		virtual bool			isEnabled( const RenderSettings *settings ) const = 0;
		virtual uint			requiredBuffers( ) const=0;
		
		/* fixed pipeline implementation */ 
		virtual void			startFixedFuncFrame( const RenderContext *context )=0;
		virtual void			endFixedFuncFrame( const RenderContext *context )=0;

		
		/* GLSL based implementation */ 
								/// preprocessor define to enable code in vert/frag shader script
		virtual const char*		shaderDefine() const=0;

								/// start of the frame rendering. Set up shader etc
		virtual void			startShaderFrame( const RenderContext *context, ShaderObj *shader )=0;

								/// end of frame rendering
		virtual void			endShaderFrame( const RenderContext *context, ShaderObj *shader )=0;
		
								/// start of sub-frame, ie. buffer rendering. Do buffer specific setup
		virtual void			startShaderBuffer( const RenderContext *context, ShaderObj *shader ){};

								/// end of sub-frame rendering
		virtual void			endShaderBuffer( const RenderContext *context, ShaderObj *shader )	{};

		virtual uint			requiredStandardUniforms()const										{ return 0; }
	};

	/** Per Context effects manager */  
	class RenderEffectsManager
	{
	public:
		int						numOfEffects() const;
		RenderEffectI			*renderEffect( int index );
		const RenderEffectI		*renderEffect( int index ) const;
		
		void					startFrame( const RenderContext *context, uint buffersAvailable, ShaderObj *shader );
		void					endFrame(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader );
		void					startBuffer( const RenderContext *context, uint buffersAvailable, ShaderObj *shader );
		void					endBuffer(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader );


		uint					requiredBuffers( const RenderContext *context ) const;

		int						addEffect( RenderEffectI *effect );

	private:
		typedef std::vector<RenderEffectI*>	EffectsContainer;
		EffectsContainer					m_effects;
	};
}