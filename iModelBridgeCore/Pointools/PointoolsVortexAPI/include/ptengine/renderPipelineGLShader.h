#pragma  once

#include <ptengine/renderPointsMethod.h>
#include <Loki/AssocVector.h>


namespace ptgl { class Shader; }

namespace pointsengine
{
	class RenderPipeline_GLShader : public RenderPipelineI
	{
	public:
		RenderPipeline_GLShader( RenderMethodI *method );
		virtual ~RenderPipeline_GLShader();

		void			renderPoints( PointsBufferI *buffer, RenderContext *context );
		void			renderSelPoints( PointsBufferI *buffer, RenderContext *context );
	
		bool			isSupportedOnPlatform() const;
		bool			useAggregateBuffers( const RenderSettings *settings, const pcloud::Voxel *voxel ) const;

		void			startFrame( RenderContext *context ); 
		void			endFrame( RenderContext *context );

	private:
		
		uint			hashSettings( const RenderSettings *settings, const PointsBufferI *buffer ) const;
		
		ptgl::Shader	*getShader( const PointsBufferI *buffer, const RenderContext *context, uint &requiredUniforms );
		
		void			setUpShaderForBuffer( const PointsBufferI *buffer, const RenderContext *context, ptgl::Shader *shader, uint reqUniforms );
		void			setupShaderForFrame( const RenderContext *context, ptgl::Shader *shader, uint requiredUniforms );

		struct ShaderInfo
		{
			ptgl::Shader *shader;
			uint		 uniforms;	
		};

		typedef Loki::AssocVector<uint, ShaderInfo*>	ShaderMapType;
		ShaderMapType	m_shaders;	
		
		uint			m_avalBuffers;
		ptgl::Shader	*m_lastShader;
	};
}