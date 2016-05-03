/*------------------------------------------------*/ 
/* Pointools ARB Vertex Program Class			  */ 
/*------------------------------------------------*/ 
/* Copyright (c) 2004 Pointools Ltd				  */ 
/*------------------------------------------------*/ 

#ifndef POINTOOLS_PTGLVERTEX_PROGRAM_SUPPORT_HEADER
#define POINTOOLS_PTGLVERTEX_PROGRAM_SUPPORT_HEADER


#include <gl/glew.h>
#include <ptgl/ptgl.h>

#include <math/matrix_math.h>
#include <map>
 
namespace ptgl
{
	class PTGL_API VertexProgram
	{
	public:
		VertexProgram(const char*id, const char *vpath);
		virtual ~VertexProgram();
		
		virtual bool begin();
		virtual void end();
		
		virtual GLuint glProgramID() const { return _id; }
		virtual const char* programID() const { return 0; };
		virtual const char* file() const { return _vfile; }

		/* quantize matrix */ 
		inline void setQuantizeMatrix(const double*scaler=0, const double*offset=0) const
		{
			double sc[4]; memcpy(&sc, scaler, sizeof(double)*3);	sc[3] = 1.0;
			double off[4]; memcpy(&off, offset, sizeof(double)*3);	off[3] = 0.0;
			double isc[4]; memcpy(&isc, sc, sizeof(double)*4);	
			
			isc[0] = 1.0/ sc[0];
			isc[1] = 1.0/ sc[1];
			isc[2] = 1.0/ sc[2];

			setProgramVector(false, 0, sc);
			setProgramVector(false, 1, off);
			setProgramVector(false, 3, isc);
		}

		/* eye matrix. Input vertex to camera space*/ 
		void setEyeMatrix();

		/* input vertex to actual position tranformation*/ 
		virtual void setTransformMatrix(const mmatrix4d &m) const;
		void pushTransformMatrix(const mmatrix4d &m) const;
		void popTransformMatrix() const;

		inline void setProgramVector(bool local, int index, const double *v) const
		{
			if (!_active) return;

			if (local)	glProgramLocalParameter4dvARB(GL_VERTEX_PROGRAM_ARB, index, v);	
			else		glProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, index, v);
		}
		inline void setProgramVector(bool local, int index, const float *v) const
		{
			if (!_active) return;

			if (local)	glProgramLocalParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, v);	
			else		glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, v);
		}

		bool active() const { return _active; }

	protected:
		bool _active;

		virtual bool initialize();
		bool initializeContext();

		char *loadScript();

		std::map<HGLRC, GLuint> _rcid;
		GLuint _id;

		const char * _vfile;
	};
}
#endif