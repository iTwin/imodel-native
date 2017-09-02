#pragma once

/****************************************************************************

Camera classes

Copyright (c) 2017 Bentley Systems, Incorporated. All rights reserved.


CONFIDENTIAL
--------------------------
This source code is provided as confidential material. License to use this
source code is provided for the purpose of evaluation only.

****************************************************************************/

// simple camera model
// Functions intended for demonstration purposes rather than optimal implementation

#include <DgnPlatform/DgnPlatform.h>

#define VANCOUVER_API
//#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/ScalableMeshLib.h>

#include <ScalableMesh/IScalableMeshProgressiveQuery.h>

struct CoordinateSystem
{
	CoordinateSystem();
	static CoordinateSystem &instance();
	const Bentley::DPoint3d &worldCenter();
	void setWorldCenter(const Bentley::DPoint3d &pnt);
};

class Camera
{
public:
	Camera();

	void	pushGLModel();
	void	pushGLProj();

	void	createCameraFrustumClips(bvector<ClipPlane>& clipPlanes);

	void	updateViewMatrix();
	void	updateProjectionMatrix();

	void	getRHModelviewMatrix(Bentley::DMatrix4d & matrix, bool worldSpace = true);

	void	getTransformationMatrix(Bentley::DMatrix4d &matrix);
	void	getEyeTargetUpFromGL(Bentley::DPoint3d &eye, Bentley::DPoint3d &target, Bentley::DPoint3d &up);
	void	getBasisVectorsFromGL(Bentley::DPoint3d &xvec, Bentley::DPoint3d &yvec, Bentley::DPoint3d &zvec);

	void	setView( const Bentley::DPoint3d &local_eye, const Bentley::DPoint3d &local_target);
	void	fitView(const Bentley::DRange3d &range);

	void	setDirtyFlag();
	void	resetDirtyFlag();
	bool	isDirty() const;

	bool	operator==(const Camera &camera);
	Camera&	operator=(const Camera &camera);

	Bentley::DVec3d getDirection() const;

	//camera position and target in double 
	Bentley::DPoint3d	m_target;
	Bentley::DPoint3d	m_position;
	Bentley::DVec3d		m_up;

	// temporary interactive shift when moving camera
	Bentley::DPoint3d	m_targetShift;
	Bentley::DPoint3d	m_positionShift;

	int		m_viewportH;
	int		m_viewportW;

	double	m_nearPlane;
	double	m_farPlane;
	double	m_fov;

private:
	// basis vectors of the camera 
	Bentley::DPoint3d	m_basisX;
	Bentley::DPoint3d	m_basisY;
	Bentley::DPoint3d	m_basisZ;

	// for event handling
	Bentley::DVec3d	m_startEventBasisY;
	Bentley::DVec3d	m_startEventBasisX;

	Bentley::DMatrix4d m_projectionMatrix;
	Bentley::DMatrix4d m_modelviewMatrix;
	Bentley::DMatrix4d m_screenMatrix;

	bool	m_dirty;
};
