#pragma once

#include <pt/geomtypes.h>

namespace pt
{
struct ViewParams
{
	enum ViewportDims { Left=0, Bottom=1, Width=2, Height=3 };

	/* matrices are col major (like gl), vec on left */ 
	double	eye_matrix[16]; 
	double	proj_matrix[16];
	double  pipeline_matrix[16];
	double  invpipeline_matrix[16];
	int		viewport[4];
	double	depth_range[2];

	const ViewParams &operator = (const ViewParams &vp) 
	{
		memcpy(this, &vp, sizeof(ViewParams) );
		return *this;
	}

	void updatePipeline()
	{
		_multMatricesd( eye_matrix, proj_matrix, pipeline_matrix );
		_invertMatrixd( pipeline_matrix, invpipeline_matrix );
	}
	bool isPerspective() const { return (proj_matrix[15] < 1.0) ? true : false; }

	bool project3v(const double *obj, double *win) const
	{
		double in[] = { obj[0], obj[1], obj[2], 1.0 };
		double out[4];
		
		_multMatrixVecd(pipeline_matrix, in, out);

	    if (out[3] == 0.0) return false;

		out[0] /= out[3];
		out[1] /= out[3];
		out[2] /= out[3];
		
		in[0] = out[0] * 0.5 + 0.5;
		in[1] = out[1] * 0.5 + 0.5;
		in[2] = out[2] * 0.5 + 0.5;

		win[0] = in[0] * viewport[2] + viewport[0];
		win[1] = in[1] * viewport[3] + viewport[1];
		win[2] = in[2];

		return true;
	}
	bool project4v(const double *obj, double *win) const
	{
		double out[4];
		
		_multMatrixVecd(pipeline_matrix, obj, out);

	    if (out[3] == 0.0) return(false);

		out[0] /= out[3];
		out[1] /= out[3];
		out[2] /= out[3];
		
		win[0] = out[0] * 0.5 + 0.5;
		win[1] = out[1] * 0.5 + 0.5;
		win[2] = out[2] * 0.5 + 0.5;

		win[0] = win[0] * viewport[2] + viewport[0];
		win[1] = win[1] * viewport[3] + viewport[1];
		win[2] = win[2];

		return true;
	}
	template <class V, class W>
	bool project3vT(const V *obj, W *win) const
	{
		double obj_d[] = { (double)obj[0], (double)obj[1], (double)obj[2] };
		double win_d[3];

		bool res = project3v(obj_d, win_d);

		win[0] = (W)win_d[0];
		win[1] = (W)win_d[1];
		win[2] = (W)win_d[2];
		
		return res;
	}
	bool	unproject3v(double *obj, const double *win) const
	{
		double in[] = { win[0], win[1], win[2], 1.0 };
		double out[4];

		in[0] = (in[0] - viewport[0]) / viewport[2];
		in[1] = (in[1] - viewport[1]) / viewport[3];

		in[0] = in[0] * 2 - 1;
		in[1] = in[1] * 2 - 1;
		in[2] = in[2] * 2 - 1;

		_multMatrixVecd(invpipeline_matrix, in, out);

		if (out[3] == 0.0) return(false);
		obj[0] = out[0] / out[3];
		obj[1] = out[1] / out[3];
		obj[2] = out[2] / out[3];

		return(true);
	}
	template <class V, class W>
	bool	unproject3vT(V *obj, const W *win) const
	{
		double obj_d[3];
		double win_d[3] = { win[0], win[1], win[2] };

		bool res = unproject3v(obj_d, win_d);

		obj[0] = (W)obj_d[0];
		obj[1] = (W)obj_d[1];
		obj[2] = (W)obj_d[2];
		
		return res;
	}

	template <class V, class W>
	bool	unproject3vTV( pt::vec3<V> &obj, const pt::vec3<W> &win) const
	{
		return unproject3vT( &obj.x, &win.x );
	}

	void	transform(const pt::vector4d &v, pt::vector4d &res) const
	{
		res[0] = pipeline_matrix[0] * v.x + pipeline_matrix[1] * v.y + pipeline_matrix[2]* v.z + pipeline_matrix[3]* v.w;
		res[1] = pipeline_matrix[4] * v.x + pipeline_matrix[5] * v.y + pipeline_matrix[6]* v.z + pipeline_matrix[7]* v.w;
		res[2] = pipeline_matrix[8] * v.x + pipeline_matrix[9] * v.y + pipeline_matrix[10]* v.z + pipeline_matrix[11]* v.w;
		res[3] = pipeline_matrix[12] * v.x + pipeline_matrix[13] * v.y + pipeline_matrix[14]* v.z + pipeline_matrix[15]* v.w;
	}
	//
	static void _multMatrixVecd(const double matrix[16], const double in[4],
				  double out[4])
	{
		int i;

		for (i=0; i<4; i++) {
		out[i] = 
			in[0] * matrix[0*4+i] +
			in[1] * matrix[1*4+i] +
			in[2] * matrix[2*4+i] +
			in[3] * matrix[3*4+i];
		}
	}
	static void _multMatricesd(const double a[16], const double b[16],
					double r[16])
	{
		int i, j;

		for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			r[i*4+j] = 
			a[i*4+0]*b[0*4+j] +
			a[i*4+1]*b[1*4+j] +
			a[i*4+2]*b[2*4+j] +
			a[i*4+3]*b[3*4+j];
		}
		}
	}
	static bool _invertMatrixd(const double m[16], double invOut[16])
	{
		double inv[16], det;
		int i;

		inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
				 + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
		inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
				 - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
		inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
				 + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
		inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
				 - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
		inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
				 - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
		inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
				 + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
		inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
				 - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
		inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
				 + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
		inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
				 + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
		inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
				 - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
		inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
				 + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
		inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
				 - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
		inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
				 - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
		inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
				 + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
		inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
				 - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
		inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
				 + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

		det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
		if (det == 0)
			return false;

		det = 1.0 / det;

		for (i = 0; i < 16; i++)
			invOut[i] = inv[i] * det;

		return true;
	}
	static void _transposeMatrix(const double m[16], double t[16])
	{
		t[0+4*0] = m[0+4*0]; t[0+4*1] = m[1+4*0]; t[0+4*2] = m[2+4*0]; t[0+4*3] = m[3+4*0];
		t[1+4*0] = m[0+4*1]; t[1+4*1] = m[1+4*1]; t[1+4*2] = m[2+4*1]; t[1+4*3] = m[3+4*1];
		t[2+4*0] = m[0+4*2]; t[2+4*1] = m[1+4*2]; t[2+4*2] = m[2+4*2]; t[2+4*3] = m[3+4*2];
		t[3+4*0] = m[0+4*3]; t[3+4*1] = m[1+4*3]; t[3+4*2] = m[2+4*3]; t[3+4*3] = m[3+4*3];
	}
};
}