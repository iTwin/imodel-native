/*--------------------------------------------------------------------------*/ 
/*	Pointools GLcamera class definition and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_CAMERA_INTERFACE
#define POINTOOLS_CAMERA_INTERFACE

#include <stack>

#include <pt/geomTypes.h>
#include <ptgl/glFrustum.h>
#include <ptgl/glViewstore.h>
#include <ptgl/glArcball.h>
#include <ptgl/glLight.h>
#include <pt/BoundingBox.h>

#include <pt/Rect.h>
#include <ptgl/ptgl.h>

namespace pt { namespace datatree { class Branch; } }

namespace ptgl
{

class PTGL_API Camera  
{
public:
	Camera();
	Camera(const Camera &c);
	Camera(const pt::datatree::Branch *b);

	virtual ~Camera();

	/*navigation states*/ 
	enum GLnavstate
	{
		TUMBLE,
		PAN,
		ZOOM,
		NONE,
	};
	enum GLnavmode
	{
		INSPECT,
		WALK,
		FLY,
		LIGHT
	};

	/*frustum*/ 
	inline const Frustum &cullFrustum() const { return m_cull_stack.top(); };
	
	void setFromViewstore(const Viewstore &v);

	void	setOrtho(bool ortho);
	bool	getOrtho() const { return m_ortho; };

	/*target*/ 
	void	setTarget(const float *v);
	void	getTarget(float *v) const;
	const	float *getTarget() const;
	void	setTargetDirection(float *v);
	void	translateTarget(const float *v);
	void	centerViewAt(const float *v);
	
	/*location*/ 
	void	setLocation(const float *v);
	void	getLocation(float *v) const;
	void	translate(const float *v);

	void	setLocationAndTarget(const float *loc, const float *tar);

	/*clipping planes*/ 
	void	setFrustumDepth(float depth);
	void	fitNearFarPlanes() { computeNearFarPlanes(); }

	/*up vector*/ 
	void	setUp(const float *v);
	void	getUp(float *v) const;
	const	float *getUp() const;

	/*mode*/ 
	void	setMode(GLnavmode m)  { m_nav_mode = m; }
	GLnavmode  getMode() const { return m_nav_mode; }

	/*fov*/ 
	void	setFOV(double fov) { m_basefov = fov; m_frustum_needs_update = true; };
	double	getFOV() const { return m_basefov; };
	
	void	setZoomDistThreshold(float t) { m_zoomthreshold = t; }; 
	float	getZoomDistThreshold() const { return m_zoomthreshold; };

	void	setMinFOV(float fov) { m_minfov = fov; };
	float	getMinFOV() const { return m_minfov; };

	/*frustum*/ 
	void setFrustum(float fUpFovDegrees, float fAspectRatio, float fNear, float fFar);
	void setFrustum(float fNear, float fFar, float fLeft, float fRight, float fTop, float fBottom);
	void setFrustum(double fNear, double fFar, double fLeft, double fRight, double fTop, double fBottom);
	void setFrustumNear (float fNear);
    void setFrustumFar (float fFar);

	void  getFrustum (float& rfNear, float& rfFar, float& rfLeft, float& rfRight, float& rfTop, float& rfBottom) const;
	void  getFrustum (double& rfNear, double& rfFar, double& rfLeft, double& rfRight, double& rfTop, double& rfBottom) const;
    float getMaxCosSqrFrustumAngle () const;

    inline float getFrustumNear () const	{ return m_frustumN; };
    inline float getFrustumFar () const		{ return m_frustumF; };
    inline float getFrustumLeft () const	{ return m_frustumL; };
    inline float getFrustumRight () const	{ return m_frustumR; };
    inline float getFrustumTop () const		{ return m_frustumT; };
    inline float getFrustumBottom () const	{ return m_frustumB; };

	/*viewport*/ 
    void  setViewport (float fLeft, float fRight, float fTop, float fBottom);
    void  getViewport (float& rfLeft, float& rfRight, float& rfTop, float& rfBottom);
    void  getViewport (pt::Rectd &vp);

    inline double getViewportLeft () const		{ return 0; };
    inline double getViewportRight () const		{ return (double)m_width; };;
    inline double getViewportTop () const		{ return (double)m_height; };;
    inline double getViewportBottom () const	{ return 0; };;

	bool update(bool use_flag = true);
	bool updateFrustum(bool use_flag = true);
	bool updateTransform(bool use_flag = true, bool identity = true);
	bool updateViewport(bool use_flag = true);
	void updateCullingFrustum();

	void pushCullingFrustum();
	bool popCullingFrustum();

	/*events*/ 
	void onResize(int width, int height);

	void resetRotation();

	/*Navigation Control*/ 
	float distanceToTarget() const;

	void draw();

	void getRotation(float &x, float &y, float &z) const;
	void pushGLrotation() const;

	bool onAxis();

	void setConstraintAxis(int axis);
	int  getConstraintAxis() const { return m_constrained_up_axis; };
	void setUseConstraints(bool use);
	bool getUseConstraints() const { return m_constrained; }; 

	void rotationFromAxisAngle(float angle, const pt::vector3 &axis);
	void rotationFromMatrix(const float* m);
	void rotationFromEulerZYX(float z, float y, float x);
	void rotationFromLookAt(const pt::vector3 &location, const pt::vector3 &up);

	/*all navigation handlers are dependent on this*/ 
	bool moveControl(int x, int y);
	void resetControl();

	void setNavigation(GLnavstate nav);
	GLnavstate getNavigationState() const { return m_nav_state; };

	/* pan*/ 
	void panStatic(int dx, int dy);

	/* zoom */ 
	double getZoomFactor() const;
	
	void zoomStatic(float factor);
	void zoomWindow(const pt::Rectd &from, const pt::Rectd &to);
	void zoomWindow(const pt::Rectd &from);
	void zoomTo(const pt::BoundingBox *bb, const pt::Rectd &win, int tol);
	void zoomTo(const pt::BoundingBox *bb, int tol);

	/*  dolly	*/ 
	void dollyStatic(float distance);

	void resetLocation();

	/*data*/ 
	void setDataSphere(const float *center, float radius);
	void getDataSphere(float *center, float &radius) const;
	void setDataBounds(const pt::BoundingBox &box);
	const pt::BoundingBox &getDataBounds() const;

	/*projections*/ 
	inline bool project4vb(const pt::vector4d &obj,  pt::vector4d &win) const { return m_vstore.project4vb(obj, win); };
	inline void project4v(const pt::vector4d &obj,  pt::vector4d &win) const { m_vstore.project4v(obj, win); };
	inline bool project3v(const double *obj, GLdouble *win) const { return m_vstore.project3v(obj, win); };
	inline bool project3v(const float *obj, GLdouble *win) const { return m_vstore.project3v(obj, win); };
	inline bool project3v(const double *obj, GLint *win) const { return m_vstore.project3v(obj, win); };
	bool project3v(const float *obj, GLfloat *win) const;
	inline void unproject3v(const GLdouble *px, GLdouble *v) const { m_vstore.unproject3v(px, v);};
	inline void unproject3v(const GLfloat *px, GLdouble *v) const { m_vstore.unproject3v(px, v);};
	inline float unprojectLength(int pixels, float dist) const { return m_vstore.unprojectLength(pixels, dist);};
	void projectedBoxExtents(const pt::BoundingBox *bb, pt::Rectd &rect) const;

	const Camera &operator = (const Camera &cam);
	const Camera &operator /= (const float &v);
	const Camera &operator *= (const float &v);
	const Camera &operator += (const Camera &cam);
	const Camera &operator -= (const Camera &cam);

	void setLight(Light *light);
	Light *getLight();

	/*persistance*/ 
	void writeBranch(pt::datatree::Branch *b);

	inline const Viewstore	&viewstore() const { return m_vstore; }

	void reset();
 
private:
	/*frame*/ 
	pt::vector3		m_target;
	pt::vector3		m_actlocation;
	pt::vector3		m_up;

	/*frustum*/ 
    GLdouble	m_frustumT;
    GLdouble	m_frustumB;
    GLdouble	m_frustumR;
    GLdouble	m_frustumL;
    GLdouble	m_frustumN;
    GLdouble	m_frustumF;

	/*zoom*/ 
	double		m_zoom;

	/*data*/ 
	pt::BoundingBox m_databounds;
	pt::vector3	m_center;
	float		m_radius;
	void		computeNearFarPlanes();

	GLint		m_viewportL;
	GLint		m_viewportR;
	GLint		m_viewportT;
	GLint		m_viewportB;

	GLint		m_width;
	GLint		m_height;

	GLdouble	m_fov;
	GLdouble	m_basefov;
	GLdouble	m_minfov;
	float		m_zoomthreshold;

	/*storing view*/ 
	Viewstore	m_vstore;

	bool m_ortho;
	bool m_frustum_needs_update;
	bool m_model_needs_update;
	bool m_vport_needs_update;
	bool m_from_vstore;

	/*clear stack and place world frustum on stack*/ 
	void clearCullStack();

	/*stack stores culling frustums*/ 
	std::stack<Frustum> m_cull_stack;

	/*control*/ 
	Arcball		m_arcball;
	GLnavstate	m_nav_state;
	GLnavmode	m_nav_mode;

	/*eulers*/ 
	void	resolveEulers();
	float	m_angleX;
	float	m_angleY;
	float	m_angleZ;

	int		m_mouseX;
	int		m_mouseY;
	int		m_lastX;
	int		m_lastY;

	int		m_constrained_up_axis;
	bool	m_constrained;

	float	m_dist2target;

	/* tumble */ 
	void rotate();
	
	/* pan*/ 
	void pan();

	/* zoom */ 
	void zoom();

	/*reset arcball*/ 
	void resetArcball();

	/*light control*/ 
	Light  *m_light;
};
}
#endif
