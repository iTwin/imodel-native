#include <ptapi/pointoolsVortexAPI.h>
#include <pt/viewparams.h>
#include <pt/geomtypes.h>
#include <ptgl/glviewstore.h>
#include <iostream>
#include <math/matrix_math.h>

#include <gl/glu.h>

#define DIVIDER std::cout << "-----------------------------------" << std::endl;

using namespace pt;

PTbool _ptUnitTestViewParams()
{
	/* test correct projection */ 
	double px[] = { 100, 150, 0.45 };
	double obj[3];
	ptgl::Viewstore viewStore(true);
	viewStore.unproject3v(px, obj);
	
	/* copy sound GL matrices into viewParams without transpose */ 
	ViewParams params;
	memcpy( params.eye_matrix, viewStore.model_mat, sizeof(double) * 16);
	memcpy( params.proj_matrix, viewStore.proj_mat, sizeof(double) * 16);
	memcpy( params.viewport, viewStore.viewport, sizeof(viewStore.viewport));
	params.updatePipeline();

	/* test view params */ 
	DIVIDER
	std::cout << "Projection and Unprojection (ViewParams)" << std::endl;
	
	vector3d wo;

	gluProject( obj[0], obj[1], obj[2], params.eye_matrix, params.proj_matrix, params.viewport, 
		&wo.x, &wo.y, &wo.z );

	std::cout << "gluProject: " << obj[0] << ", " << obj[1] << ", " << obj[2] << " -> " 
		<< wo.x << ", " << wo.y << ", " << wo.z << std::endl;

	params.project3v( obj, (double*)wo );
	
	std::cout << "ptProject: " << obj[0] << ", " << obj[1] << ", " << obj[2] << " -> " 
		<< wo.x << ", " << wo.y << ", " << wo.z << std::endl;

	gluUnProject( wo.x, wo.y, wo.z, params.eye_matrix, params.proj_matrix, params.viewport, 
		&obj[0], &obj[1], &obj[2] );

	std::cout << "gluUnproject: " << obj[0] << ", " << obj[1] << ", " << obj[2] << " <- " 
		<< wo.x << ", " << wo.y << ", " << wo.z << std::endl;

	params.unproject3v( obj, (double*)wo );
	
	std::cout << "ptUnproject: " << obj[0] << ", " << obj[1] << ", " << obj[2] << " <- " 
		<< wo.x << ", " << wo.y << ", " << wo.z << std::endl;	
	
	return true;
}
extern PTbool _ptUnitTestProjection();

//
//
//
PTbool	PTAPI _ptUnitTests( PTenum test )
{
	std::cout << "Unit Tests" << std::endl;

	_ptUnitTestViewParams();
	_ptUnitTestProjection();
	return true;
}
