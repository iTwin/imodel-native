#ifndef POINTOOLS_TRANSFORM_H
#define POINTOOLS_TRANSFORM_H

#include <pt/flags.h>
#include <math/matrix_math.h>

namespace pt
{
enum CoordinateSpace
{
	QuantizedSpace,
	LocalSpace,
	SceneSpace,
	ProjectSpace,
	WorldSpace
};
//-------------------------------------------------------------------------
/// Transformation class encapsulates Object transformation to parent
//-------------------------------------------------------------------------
/*!
 *	Transformation objects are linked to provide visibility from Local to  
 *  World Space. The 'compiled' matrix can be compiled from this object in its
 *  Space to a specified Space above it.
 */
class Transform
{
public:
	enum FlagValues { UpdatedSinceCompile = 1 };

	Transform(Transform *parent=0) : 
				m_cs(LocalSpace), m_ccs(LocalSpace), 
				m_parent(parent), m_flags(UpdatedSinceCompile), 
				m_matrix(mmatrix4d::identity()), 
				m_imatrix(mmatrix4d::identity()), 
				m_tmatrix(mmatrix4d::identity()), 
				m_cmatrix(mmatrix4d::identity()) {};

	~Transform() {};
	inline mmatrix4d &matrix()					{ return m_matrix;  }

	inline const mmatrix4d &matrix() const		{ return m_matrix;  }
	inline const mmatrix4d &invMatrix() const	{ return m_imatrix; }
	inline const mmatrix4d &traMatrix() const	{ return m_tmatrix; }
	inline const mmatrix4d &cmpMatrix() const	{ return m_cmatrix; }
	inline const mmatrix4d &invcmpMatrix() const	{ return m_icmatrix; }

	inline const Transform *parent() const		{ return m_parent; }
	inline void  parent(Transform* p)			{ m_parent = p; }

	void update()
	{
		m_imatrix = m_matrix; m_imatrix.invert();
		m_tmatrix = m_matrix; m_tmatrix.transpose();
		m_flags.set(UpdatedSinceCompile);
	}
	void matrix(const mmatrix4d &m)
	{ 
		memcpy(&m_matrix, &m, sizeof(mmatrix4d)); 
		m_imatrix = m_matrix; m_imatrix.invert();
		m_tmatrix = m_matrix; m_tmatrix.transpose();
	}
	void matrix(const double *m)	
	{ 
		memcpy(&m_matrix, m, sizeof(mmatrix4d)); 
		m_imatrix = m_matrix; m_imatrix.invert();
		m_tmatrix = m_matrix; m_tmatrix.transpose();
	}

#if defined (HAVE_OPENGL)
	inline void pushGL() const { glPushMatrix(); glMultMatrixd((double*)&m_tmatrix); };
#endif
	
	/* Coordinate space that is transformed FROM						*/ 
	inline CoordinateSpace coordinateSpace() const { return m_cs; } 
	inline void coordinateSpace(const CoordinateSpace &cs) { m_cs = cs; }

	/* get a parent tranformation in a given coordinate space			*/ 
	const Transform *getTransform(const CoordinateSpace &cs) const
	{
		if (coordinateSpace() == cs) return this;
		else if (parent()) return parent()->getTransform(cs);
		else return 0;
	}
	void identity() 
	{
		m_matrix = mmatrix4d::identity();
		m_imatrix = mmatrix4d::identity();
		m_cmatrix = mmatrix4d::identity();
		m_tmatrix = mmatrix4d::identity();
		m_ccs = LocalSpace;
	}
	inline void transform(const pt::vector3 &v, pt::vector3 &t)	const {
		m_matrix.vec3_multiply_mat4f(v,t);	}
	inline void transform(const pt::vector3d &v, pt::vector3d &t) const	{
		m_matrix.vec3_multiply_mat4(v,t);	}
	inline void transform(CoordinateSpace cs, const vector3d &v, vector3d &t)
	{
		if (cs!=m_ccs) compileMatrix(cs);
		m_cmatrix.vec3_multiply_mat4(v,t);
	}
	inline void transform(CoordinateSpace cs, const vector3 &v, vector3 &t)
	{
		if (cs!=m_ccs) compileMatrix(cs);
		m_cmatrix.vec3_multiply_mat4f(v,t);
	}
	inline void transformCS(const pt::vector3d &v, pt::vector3d &t) const	{
		m_cmatrix.vec3_multiply_mat4(v,t);	}

	/* translation */ 
	template <class T> void translate(const pt::vec3<T> &v)
	{
		m_matrix(3,0) += v[0];		m_matrix(3,1) += v[1];		m_matrix(3,2) += v[2];
		m_imatrix(3,0) -= v[0];		m_imatrix(3,1) -= v[1];		m_imatrix(3,2) -= v[2];
		m_tmatrix(0,3) += v[0];		m_tmatrix(1,3) += v[1];		m_tmatrix(2,3) += v[2];
		m_flags.set(UpdatedSinceCompile);
	}
	template <class T> void translation(const pt::vec3<T> &v)
	{
		m_matrix(3,0) = v[0];		m_matrix(3,1) = v[1];		m_matrix(3,2) = v[2];
		m_imatrix(3,0) = -v[0];		m_imatrix(3,1) = -v[1];		m_imatrix(3,2) = -v[2];
		m_tmatrix(0,3) = v[0];		m_tmatrix(1,3) = v[1];		m_tmatrix(2,3) = v[2];
		m_flags.set(UpdatedSinceCompile);		
	}
	vector3d translation() const
	{
		return vector3d(m_matrix(3,0), m_matrix(3,1), m_matrix(3,2));
	}
	/* rotation */ 
	template <class T> void rotation(const pt::vec3<T> &v)
	{
		m_matrix.rotation(v.x, v.y, v.z);
		update();
	}
	template <class T> void rotate(const pt::vec3<T> &v)
	{
		m_matrix.concat(mmatrix4d::rotation(v.x, v.y, v.z));
		update();
	}

	const inline Flags &flags() const { return m_flags; }

	/* matrix compilation */ 
	inline void compileMatrix(CoordinateSpace cs = ProjectSpace, bool force = false)
	{
		if (m_ccs == m_cs && !force) return;
		m_cmatrix = m_matrix;
		if (m_parent) m_parent->_compileMatrix(m_cmatrix, cs);
		m_icmatrix = m_cmatrix;
		m_icmatrix.invert();
		m_ccs = cs;
	}
	bool compileMatrix(mmatrix4d &m, CoordinateSpace cs= ProjectSpace) const
	{
		if (m_ccs == cs && !m_flags.flag(UpdatedSinceCompile))
		{m = m_cmatrix; return true; }
		m = mmatrix4d::identity();
		return _compileMatrix(m, cs);
	}	

protected:
	/* combine matrices up to the coordinate space in m_cmatrix			*/ 
	bool _compileMatrix(mmatrix4d &m, CoordinateSpace cs = ProjectSpace) const
	{
		if (m_cs == cs) return true;
		m >>= m_matrix;
		if (m_parent) m_parent->_compileMatrix(m, cs);
		else return cs == WorldSpace ? true : false;
		return false;
	}
private:
	/*try to keep this aligned on 16byte boundary for MMX usage*/ 
	mmatrix4d		m_matrix;
	mmatrix4d		m_imatrix;
	mmatrix4d		m_tmatrix;

	mmatrix4d		m_cmatrix;
	mmatrix4d		m_icmatrix;

	Transform		*m_parent;
	CoordinateSpace	m_cs;
	CoordinateSpace	m_ccs;
	Flags			m_flags;
};

//-----------------------------------------------------------------------------
class TransformHelper
{
public:
	static void transformBounds( const Transform *transform, BoundingBox &box )
	{
		const mmatrix4d &mat = transform->matrix();
		vector3 pt, ptt;
		BoundingBox newBox;

		for (int i=0; i<8; i++)
		{
			box.getExtrema(i, pt);
			mat.vec3_multiply_mat4f(pt, ptt);
			newBox.expand( ptt );
		}
		box = newBox;
	}

	static void transformBounds( const Transform *transform, BoundingBoxD &box )
	{
		const mmatrix4d &mat = transform->matrix();
		vector3d pt, ptt;
		BoundingBoxD newBox;

		for (int i=0; i<8; i++)
		{
			box.getExtrema(i, pt);
			mat.vec3_multiply_mat4(pt, ptt);
			newBox.expand( ptt );
		}
		box = newBox;
	}

	static void transformBounds( const mmatrix4d& mat, BoundingBoxD &box )
	{
		vector3d pt, ptt;
		BoundingBoxD newBox;

		for (int i=0; i<8; i++)
		{
			box.getExtrema(i, pt);
			mat.vec3_multiply_mat4(pt, ptt);
			newBox.expand( ptt );
		}
		box = newBox;
	}

};
}
#endif