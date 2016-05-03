#ifndef POINTOOLS_GLCG_SUPPORT_HEADER
#define POINTOOLS_GLCG_SUPPORT_HEADER
#ifdef PTGL_USING_CG

#include <ptgl/ptgl.h>
#include <cg/cg.h>

namespace ptgl
{
	class PTGL_API CgShader
	{
	public:
		CgShader(const char*id, const char *vpath, const char *ppath=0);
		virtual ~CgShader();
		
		virtual bool begin();
		virtual void end();
		
		virtual const char* shaderID() const { return 0; };
		virtual const char* vertexShaderFile() const { return _vfile; }
		virtual const char* pixelShaderFile() const { return _pfile; }

		void setQuantizeMatrix(const float*scaler=0, const float*offset=0);
		void setEyeMatrix();
		void setTransformMatrix();
		void setTextureMatrix();

		void setLight(	const float *ambient, 
						const float *diffuse, 
						const float *specular, 
						const float *pos, 
						const float *dir);

		void setMaterial(	const float *ambient, 
							const float *diffuse, 
							const float *specular, 
							const float glossiness);

		void setTexture( int unit, GLuint id);

		virtual void setUserParameter(const char *param, float *value);
		
		virtual bool initialize();

		bool active() const { return _active; }

		/*Management functions*/ 
		static void addShader(const char*id, CgShader *shader);		
		static CgShader* getShader(const char*id);

	protected:
		void* _verprogram;
		void* _pixprogram;
		bool _active;

		const char * _vfile;
		const char * _pfile;

		struct Param
		{
			CGparameter modelViewProj;
			CGparameter modelView;
			CGparameter modelViewIT;
			CGparameter texMatrix;

			CGparameter	quanScale;
			CGparameter quanOffset;

			CGparameter lightVec;
			CGparameter lightPos;
			CGparameter lightAmb;
			CGparameter lightSpec;
			CGparameter lightDiff;

			CGparameter matAmb;
			CGparameter matSpec;
			CGparameter matDiff;
			CGparameter matGloss;

			CGparameter texture0;
			CGparameter texture1;
			CGparameter texture2;
			CGparameter texture3;

		} _param;
	};

	class PTGL_API CgShaderObj
	{
	public:
		CgShaderObj(const char*id);
		~CgShaderObj();	
		
		bool valid() const { return _shader ? true : false; }
		virtual void setQuantizeMatrix()	{ if (_shader) _shader->setQuantizeMatrix(); }
		virtual void setTransformMatrix() { if (_shader) _shader->setTransformMatrix(); }
		virtual void setTextureMatrix() { if (_shader) _shader->setTextureMatrix(); }

		CgShader *shaderInstance() { return _shader; }
	private:
		CgShader *_shader;
	};
}
#endif
#endif