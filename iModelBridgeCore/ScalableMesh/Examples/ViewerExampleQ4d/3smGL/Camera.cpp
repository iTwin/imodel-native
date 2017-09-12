#include "Camera.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GLU.h>

static Bentley::DPoint3d wc;

CoordinateSystem::CoordinateSystem()
{
	wc.Zero();
}
CoordinateSystem &CoordinateSystem::instance()
{
	static CoordinateSystem cs;
	return cs;
}
const Bentley::DPoint3d &CoordinateSystem::worldCenter()
{
	return wc;
}
void CoordinateSystem::setWorldCenter(const Bentley::DPoint3d & pnt)
{
	wc = pnt;
}
Camera::Camera()
{
	m_targetShift.zero();
	m_positionShift.zero();

	m_target.zero();

	m_position.x = 30.0;
	m_position.y = 20.0;
	m_position.z = 50.0;

	m_up.x = 0;
	m_up.y = 0;
	m_up.z = 1.0;

	m_viewportW = 1280;
	m_viewportH = 800;

	m_nearPlane = 1.0;
	m_farPlane = 1000;
	m_fov = 200;

	m_basisX.Zero();
	m_basisY.Zero();
	m_basisZ.Zero();

	m_modelviewMatrix.initIdentity();
	m_projectionMatrix.initIdentity();
	m_screenMatrix.initIdentity();

	m_dirty = true;
}

void Camera::pushGLModel()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	Bentley::DPoint3d current_target = m_targetShift;
	Bentley::DPoint3d current_position = m_positionShift;
	current_target.Add(m_target);
	current_position.Add(m_position);

	gluLookAt(current_position.x, current_position.y, current_position.z,
		current_target.x, current_target.y, current_target.z,
		m_up.x, m_up.y, m_up.z);

	getBasisVectorsFromGL(m_basisX, m_basisY, m_basisZ);
}

// helper func
static void MultiplyMatrix(DMatrix4d& Res, const DMatrix4d &Mat1, const DMatrix4d &Mat2)
{
	Res = DMatrix4d::FromZero();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int c = 0; c < 4; c++)
			{
				Res.coff[i][j] += Mat1.coff[i][c] * Mat2.coff[c][j];
			}
		}
	}
}

void Camera::createCameraFrustumClips(bvector<ClipPlane> &clipPlanes)
{
	DPoint3d worldEye(m_position);
	worldEye.Add(CoordinateSystem::instance().worldCenter());

	DVec3d camDir = getDirection();
	camDir.Normalize();
	DVec3d camX, camY;
	camX.crossProduct(&m_up, &camDir);
	camY.crossProduct(&camX, &camDir);

	double size = m_farPlane;
	double tangent = tan(0.5 * m_fov * PI / 180.0);
	double sizeX = size * tangent * 2.0;
	double sizeY = sizeX * m_viewportH / m_viewportW;

	DPoint3d P0 = m_position; // recentered
	DPoint3d P1 = P0 + size * camDir - sizeX * camX - sizeY * camY;
	DPoint3d P2 = P0 + size * camDir + sizeX * camX - sizeY * camY;
	DPoint3d P3 = P0 + size * camDir + sizeX * camX + sizeY * camY;
	DPoint3d P4 = P0 + size * camDir - sizeX * camX + sizeY * camY;

	DVec3d V12; V12.NormalizedCrossProduct(P2 - P0, P1 - P0);
	DVec3d V23; V23.NormalizedCrossProduct(P3 - P0, P2 - P0);
	DVec3d V34; V34.NormalizedCrossProduct(P4 - P0, P3 - P0);
	DVec3d V41; V41.NormalizedCrossProduct(P1 - P0, P4 - P0);

	P0 = worldEye;

	clipPlanes.push_back(ClipPlane(V12, P0));  // normal needs to be inward
	clipPlanes.push_back(ClipPlane(V23, P0));
	clipPlanes.push_back(ClipPlane(V34, P0));
	clipPlanes.push_back(ClipPlane(V41, P0));
	clipPlanes.push_back(ClipPlane(camDir, P0)); // keep in front of camera

	// hack in a little test here
	static Bentley::bvector<Bentley::ClipPlane> clipPlanesStore;

	if (0 && ::GetAsyncKeyState(VK_SHIFT))
	{
		clipPlanes.push_back(ClipPlane(V12, P0));  // normal needs to be inward
		clipPlanes.push_back(ClipPlane(V23, P0));
		clipPlanes.push_back(ClipPlane(V34, P0));
		clipPlanes.push_back(ClipPlane(V41, P0));
		clipPlanes.push_back(ClipPlane(camDir, P0)); // keep in front of camera

		clipPlanesStore.clear();
		clipPlanesStore.push_back(clipPlanes[0]);
		clipPlanesStore.push_back(clipPlanes[1]);
		clipPlanesStore.push_back(clipPlanes[2]);
		clipPlanesStore.push_back(clipPlanes[3]);
		clipPlanesStore.push_back(clipPlanes[4]);
	}
	else
	{
		for (int i = 0; i < clipPlanesStore.size(); i++)
		{
			clipPlanes.push_back(clipPlanesStore[i]);
		}
	}
}
void Camera::updateViewMatrix()
{
	DPoint3d XAxis, YAxis, ZAxis = m_target - m_position;
	ZAxis.Normalize();

	XAxis.crossProduct(&m_up, &ZAxis); // X axis dir
	XAxis.Normalize();
	YAxis.crossProduct(&ZAxis, &XAxis);

	DPoint3d cen(CoordinateSystem::instance().worldCenter());
	DPoint3d eye(m_position);
	eye.Add(cen);

	double XX = -XAxis.DotProduct(eye);
	double YY = -YAxis.DotProduct(eye);
	double ZZ = -ZAxis.DotProduct(eye);

	m_modelviewMatrix.InitFromRowValues(
		XAxis.x, YAxis.x, ZAxis.x, 0.0,
		XAxis.y, YAxis.y, ZAxis.y, 0.0,
		XAxis.z, YAxis.z, ZAxis.z, 0.0,
		XX, YY, ZZ, 1.0
	);
}

#define PI 3.14159265

void Camera::updateProjectionMatrix()
{
	float aspectRatio = (float)m_viewportW / m_viewportH;

	// update the projection and screen matrices
	// from DirectX sdk documentation
    
	//double xScale = 1.0 / tan(m_fov * 0.5);
    double xScale = 1.0 / tan(m_fov * PI / 180.0 / 0.5);
	double yScale = xScale * aspectRatio;
	double Q = m_farPlane / (m_nearPlane - m_farPlane);

	m_projectionMatrix.InitFromRowValues(
		xScale, 0.0, 0.0, 0.0,
		0.0, yScale, 0.0, 0.0,
		0.0, 0.0, Q, -1.0,
		0.0, 0.0, m_nearPlane*Q, 0.0
	);

	//screen matrix
	m_screenMatrix.InitFromColumnVectors(
		DVec3d::From(0.5*m_viewportW, 0, 0),
		DVec3d::From(0, 0.5*m_viewportH, 0),
		DVec3d::From(0, 0, 1),
		DVec3d::From(0, 0, 0)
	);
}

void Camera::getRHModelviewMatrix(Bentley::DMatrix4d & matrix, bool worldSpace /* = true */)
{
	GLdouble gl_model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, gl_model);

	DPoint3d center = CoordinateSystem::instance().worldCenter();
	gl_model[12] += center.x;
	gl_model[13] += center.y;
	gl_model[14] += center.z;

	memcpy(matrix.coff, gl_model, sizeof(double) * 16);
	// flip some signs to make it RH
	matrix.coff[0][2] *= -1.0;
	matrix.coff[1][2] *= -1.0;
	matrix.coff[2][0] *= -1.0;
	matrix.coff[2][1] *= -1.0;
	matrix.coff[3][2] *= -1.0;
}

void Camera::getTransformationMatrix(Bentley::DMatrix4d & matrix)
{
	matrix.InitIdentity();

	updateViewMatrix();
	updateProjectionMatrix();
	
	DMatrix4d ViewProjMat, AllMat;
	AllMat.InitIdentity(); 
	ViewProjMat.InitIdentity();
	MultiplyMatrix(ViewProjMat, m_modelviewMatrix, m_projectionMatrix);
	MultiplyMatrix(AllMat, ViewProjMat, m_screenMatrix);

	matrix.TransposeOf(AllMat);
}

void Camera::getEyeTargetUpFromGL(Bentley::DPoint3d & eye, Bentley::DPoint3d & target, Bentley::DPoint3d & up)
{
	Bentley::DVec3d xvec, yvec, zvec, trans;

	// extract the GL modelview matrix and calc from there
	// we could just use the rotations and target, but using the
	// modelview matrix provides a more generalised method for
	// any GL setup
	GLdouble modelview[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	// extract vectors - of course we can just read in the array directly
	// but this shows the make up of a GL matrix
	getBasisVectorsFromGL(xvec, yvec, zvec);

	trans.x = modelview[12];
	trans.y = modelview[13];
	trans.z = modelview[14];

	// compute eye position
	Bentley::DMatrix4d mmodel, immodel;
	Bentley::DPoint4d eye4, target4;

	Bentley::DPoint3d current_target = m_targetShift;
	current_target.Add(target);
	target4.x = current_target.x;
	target4.y = current_target.y;
	target4.z = current_target.z;
	target4.w = 1.0;

	mmodel.InitIdentity();
	mmodel.InitFromColumnVectors(xvec, yvec, zvec, trans);

	// invert the matrix to compute the camera position
	immodel.QrInverseOf(mmodel);
	immodel.GetColumn(eye4, 3);
	eye.InitFromArray(&eye4.x);

	// calc target position
	target = eye;

	// this scaling cannot be generalised, we can do this 
	// because we know distance of camera from the target
	zvec.Scale(-target.Magnitude());
	target += zvec;

	up = yvec;
}

void Camera::getBasisVectorsFromGL(Bentley::DPoint3d &xvec, Bentley::DPoint3d &yvec, Bentley::DPoint3d &zvec)
{
	GLdouble modelview[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	xvec.x = modelview[0];
	xvec.y = modelview[4];
	xvec.z = modelview[8];
	yvec.x = modelview[1];
	yvec.y = modelview[5];
	yvec.z = modelview[9];
	zvec.x = modelview[2];
	zvec.y = modelview[6];
	zvec.z = modelview[10];
}

void Camera::setView( const Bentley::DPoint3d & local_eye, const Bentley::DPoint3d & local_target)
{
	m_position = local_eye;
	m_target = local_target;

	setDirtyFlag();
}

void Camera::fitView(const Bentley::DRange3d & range)
{
	Bentley::DPoint3d center = DPoint3d::From(range.low.x + range.XLength() / 2, range.low.y + range.YLength() / 2, range.low.z + range.ZLength() / 2);

	// set target to center (most likely zero)
	m_target = center - CoordinateSystem::instance().worldCenter();

	DVec3d CamDir = m_target - m_position; 
	CamDir.Normalize();

	double tangent = tan(m_fov * PI / 360.0);
	double distance = (range.high - range.low).magnitude() / (2 * tangent);

	// new eye
	m_position = m_target - distance * CamDir;

	setDirtyFlag();
}

void Camera::setDirtyFlag()
{
	m_dirty = true;
}

void Camera::resetDirtyFlag()
{
	m_dirty = false;
}

bool Camera::isDirty() const
{
	return m_dirty;
}

bool Camera::operator==(const Camera & camera)
{
	return (
		m_target == camera.m_target	&&
		m_position == camera.m_position	&&
		m_up == camera.m_up &&
		m_viewportW == camera.m_viewportW &&
		m_viewportH == camera.m_viewportH &&
		m_nearPlane == camera.m_nearPlane &&
		m_farPlane == camera.m_farPlane &&
		m_fov == camera.m_fov
		)
		? true : false;
}

Camera & Camera::operator=(const Camera & camera)
{
	m_target = camera.m_target;
	m_position = camera.m_position;
	m_up = camera.m_up;
	m_viewportW = camera.m_viewportW;
	m_viewportH = camera.m_viewportH;
	m_nearPlane = camera.m_nearPlane;
	m_farPlane = camera.m_farPlane;
	m_fov = camera.m_fov;

	return (*this);
}

Bentley::DVec3d Camera::getDirection() const
{
	return Bentley::DVec3d(m_target-m_position);
}
