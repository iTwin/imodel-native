// Viewport.h: interface for the CViewport class.
//
// (c) igloosoft 2002 all rights reserved
// abstracts GL viewport.
// 1. Initiates Rendering context
// 2. builds viewport and projection matrices
// 3. manages current view settings
//
// Faraz Ravi Oct 2002
//----------------------------------------------------------------------------

#ifndef POINTOOLS_GLVIEWPORT_DEFINITION
#define POINTOOLS_GLVIEWPORT_DEFINITION

#ifndef uint
#define uint unsigned int
#define dword unsigned int
#define word unsigned short
#endif

/*includes*/ 
#include <gl/glew.h>
#include <gl/glu.h>

#include <pt/rect.h>
#include <ptgl/glCamera.h>
#include <ptgl/glLight.h>

#include <ptgl/ptgl.h>
#include <ptgl/color.h>

//defines *****************************

/*status							*/ 
	#define VPS_INITITALIZING		1
	#define VPS_READY				2
	#define VPS_DESTROYING			3
	#define VPS_NORC				4

/*settings							*/ 
	#define VP_DEFAULT_FRAME_RATE	15
	#define VP_DEFAULT_FOV			45.0f

	#define VP_PROJ_PERSPECTIVE		1
	#define VP_PROJ_ORTHO			2

/*blit operations					*/ 
	#define VP_BLIT_NONE			0
	#define VP_BLIT_STORE			1
	#define VP_BLIT_USE				2

/*view operations					*/ 
	#define VP_OP_ROTATE			1
	#define VP_OP_PAN				2
	#define VP_OP_ZOOM				3

/*create result						*/ 
	/*failure codes					*/ 
	#define VPR_FAIL				1
	#define VPR_PIXELFORMAT_FAIL	2
	#define VPR_INIT_GL_FAIL		4
	#define VPR_CREATE_RC_FAIL		8
	#define VPR_RETRIEVE_PF_FAIL	16

	/*success						*/ 
	#define VPR_SUCCESS				32

	/*palette						*/ 
	#define VPR_NEEDS_PALETTE		64

/*interface							*/ 
	#define VP_CTRL_BAR_HT			20
	#define VP_FRAME_THICKNESS		0
	#define VP_FRAME_COL			RGB(160,160,160)

/*mouse state flags					*/ 
	#define VP_MOUSE_LEFT		1
	#define VP_MOUSE_RIGHT		2
	#define VP_MOUSE_MOVE		4
	#define VP_MOUSE_MIDDLE		8
	#define VP_MOUSE_WHEEL_UP	16
	#define VP_MOUSE_WHEEL_DOWN	32
/*key flags							*/ 
	#define VP_KEY_ALT		1
	#define VP_KEY_SHIFT	2
	#define VP_KEY_CONTROL	4

/*navigate options					*/ 
	#define VP_PAN_WITH_ZOOM	1

/*navigate state					*/ 
	#define VP_NONE				0
	#define VP_PANNING			1
	#define VP_ZOOMING			2
	#define VP_ROTATING			3

/*navigation mode					*/ 
	#define VP_FREE				1
	#define VP_TUMBLE			2
	#define VP_ZOOM				3
	#define VP_PAN				4
	#define VP_ROTATE			5

/*level of detail					*/ 
	#define VP_LOD_FULL			1
	#define VP_LOD_NAV			2
	#define VP_LOD_QUICK		4
	#define VP_LOD_REFRESH		8

/*view locking						*/ 
	#define VP_HALF_LOCK		1
	#define VP_FULL_LOCK		2
/*bar item style					*/ 
	#define VP_BI_FLAT			1
	#define VP_BI_GRAD			2
	#define VP_BI_SUNKEN		3
/*viewa								*/ 
	#define VP_VIEW_DEFAULT		0
	#define VP_VIEW_FRONT		1
	#define VP_VIEW_BACK		2
	#define VP_VIEW_RIGHT		3
	#define VP_VIEW_LEFT		4
	#define VP_VIEW_TOP			5
	#define VP_VIEW_BOTTOM		6


namespace ptgl
{

//--------------------------------------------------
// Custom pallette structure
//--------------------------------------------------
typedef struct tagLogicalPalette
{
	word wVersion;
	word wNumEntries;
	PALETTEENTRY peaEntries[256];
} LOGICALPALETTE;
//
// viewsetup
//
struct GL_VIEW_SETUP {
	GLint			viewport[4];
	GLdouble		model_mat[16];
	GLdouble		proj_mat[16];
};
//--------------------------------------------------
// CViewport class header
//--------------------------------------------------
class PTGL_API Viewport
{
public:
	/*callbacks*/ 
	typedef void (*paintCB)(void*, int);

	Viewport();
	virtual ~Viewport();

	/*sizes viewport*/ 
	void setSize(const pt::Recti &rWin);

	/*next viewport*/ 
	Viewport *m_next;

	/*register ploting callback*/ 
	void	setDrawCallback(paintCB cb);

	/*set up back color for viewport*/ 
	void	setBackColor(const Color &col);
	void	getBackColor(Color &p) const;

	void	setBackColor2(const Color &col);
	void	getBackColor2(Color &p) const;

	void	setUseGradient(bool use);
	bool	getUseGradient() const;

	void	setGridColor(const Color &col);
	void	getGridColor(Color &col) const;

	void	setFrameRate(int fr);
	int	getFrameRate() const;

	/*draw*/ 
	void drawAxis() const;
	void drawGrid() const;
	void drawTarget() const;
	void drawBackdrop();
	void drawLightDirection();

	void start2d();
	void end2d();

	void paint(uint lod);

	/*zooms*/ 
	void zoomTo(const pt::BoundingBox *bb, bool paint = true);
	void zoomTo(float *lower, float *upper, float* cen, int tol, bool paint = true);
	void zoomWindow(const pt::Recti &window, bool paint = true);
	void zoomWindow(const pt::Rectf &window, bool paint = true);
	void zoomIn();
	void zoomOut();
	void zoomLeft();
	void zoomRight();
	void zoomFront();
	void zoomBack();
	void zoomTop();
	void zoomBottom();
	void zoomDefault();

	void setUCSview(uint view);

	void useOrtho();
	void usePerspective();

	void notifyManager(bool notify = true) { m_notify = notify; };
	void syncLayers(bool sync = true) { m_sync = sync; };

	/*navigation events*/ 
	bool mouse_move(const Point &p, uint flags);
	bool mouse_left_down(const Point &p, uint flags);
	bool mouse_left_up(const Point &p, uint flags);
	bool mouse_right_down(const Point &p, uint flags);
	bool mouse_right_up(const Point &p, uint flags);
	bool mouse_left_dbl(const Point &p, uint flags);
	bool mouse_mid_up(const Point &p, uint flags);
	bool mouse_mid_down(const Point &p, uint flags);
	bool mouse_roll_up(const Point &p, uint flags);
	bool mouse_roll_down(const Point &p, uint flags);

	/*direct navigation*/ 
	void setUseConstraints(bool use = true) { m_camera.setUseConstraints(use); };
	bool getUseConstraints() const { return m_camera.getUseConstraints(); };
	void setConstraintAxis(int i) { m_camera.setConstraintAxis(i); };
	int  getConstraintAxis() const { return m_camera.getConstraintAxis(); };

	void setNavLock(int lock);
	int getNavLock() const;

	void nav_free();
	void nav_zoom();
	void nav_pan();
	void nav_rotate();
	void nav_tumble();
	void nav_end();
	int nav_mode() const { return m_nav_mode; }
	/*view rotation*/ 
	void rotationFromAxisAngle(float angle, float *v);
	void rotationFromMatix(const float *mat);
	void rotationFromEulerZYX(float z, float y, float x);
	void rotationFromLookAt(const float *loc, const float *up);

	/*locking*/ 
	void lock_nav(uint lock);
	void unlock_nav();

	uint getLOD();
	void setValid();

	void setFocus(bool f = TRUE);
	void getViewRect(pt::Recti &r);

	/*get Plot Rect*/ 
	void getPlotRect(pt::Recti &r, float *lower, float *upper);

	/*build gl setup*/ 
	void setup2dproj();
	void setup2dview();
	void setupPixel();

	void alignLayers();

	/*camera*/ 
	inline const Camera &camera() const { return m_camera; };
	inline Camera &camera() { return m_camera; };

	inline Light &light(int idx) { return m_lights[idx]; };
	inline const Light &light(int idx) const { return m_lights[idx]; };

	/*culling frustum*/ 
	bool popClipFrustum();
	void pushClipFrustum();
	bool inFrustum(const pt::BoundingBox *bb) const;
	bool inFrustum(const float *v) const;

	void invalidateView() { m_viewState = 0; }
	void invalidateView(const pt::Recti &r) { m_viewState = 0; m_invalidRect = r; }
	int viewState() const { return m_viewState; }
	const pt::Recti &invalidRect() const { return m_invalidRect; }

	void	storeViewPixels();
	void	restoreViewPixels();

	int		blitMode() const { return m_blitmode; }
protected:
	void	timestamp();
	bool	inTime();

	void	color_gl(uint &col);

	/*palette*/ 
	void	setupPalette();

	/*viewport*/ 
	void	pushViewport();
	void	popViewport();

	GLint	m_viewport[4];
	bool	m_in2d;
	int		m_up;

	Color	m_backcolor;
	Color	m_backcolor2;
	bool	m_usegradient;
	
	int		m_blitmode;
	Color	m_gridcolor;

	/*registered callback for painting*/ 
	paintCB m_paintcb;
	virtual void paint();

	void setBlitMode(int blit);

	/*invalidation and store of view*/ 
	uint		m_viewState;
	pt::Recti	m_invalidRect;
	GLubyte*	m_colorPixels;
	GLfloat*	m_depthPixels;

	/*view locking*/ 
	uint	m_view_lock;
	bool	m_override_lock;

	/*Level of Detail*/ 
	uint	m_LOD;

	/*setup status*/ 
	uint	m_status;

	/*viewSettings*/ 
	int		m_time;

	/*camera*/ 
	Camera	m_camera;

	/*viewport settings*/ 
	pt::Recti	m_viewRect;

	/*notify manager of view change*/ 
	bool	m_notify;

	/*sync 2d and 3d layers*/ 
	bool	m_sync;

	/*mouse state*/ 
	uint	m_op_state;
	uint	m_mouse_state;
	uint	m_nav_mode;

	void	release_mouse(uint release_op);

	/*mouse -> operation map*/ 
	uint	m_op_map[4];
	Light   m_lights[4];

	/*PLOT*/ 
	bool	m_plot;

	/*FOCUS*/ 
	bool m_hasFocus;

	/*frame rate*/ 
	float m_framerate;
	float m_actualrate;
};
}
#endif
