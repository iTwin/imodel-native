#version 130
#define TOTAL_FOV_IN_DEGREE	200.0
uniform	int			objectID;
uniform	int			useSphericalProjection;
void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 viewCoordPos = gl_ModelViewMatrix * gl_Vertex;
	if (useSphericalProjection != 0) //Do not use spherical projection
	{
		gl_Position = gl_ProjectionMatrix * viewCoordPos;
		return;
	}

	if (objectID != 13) //not the scalable mesh
	{
		gl_Position = gl_ProjectionMatrix * viewCoordPos;
		return;
	}

	float rad2deg = 57.295;

	float r = sqrt(viewCoordPos.x*viewCoordPos.x + viewCoordPos.y*viewCoordPos.y + viewCoordPos.z*viewCoordPos.z);
	float theta = rad2deg*acos(viewCoordPos.y / r);
	float phi = rad2deg*atan(viewCoordPos.x / (-viewCoordPos.z));

	if (viewCoordPos.x <= 0.0 && (-viewCoordPos.z) <= 0.0)
		phi = -180.0 + phi;
	else
	{
		if (viewCoordPos.x > 0.0 && (-viewCoordPos.z) <= 0.0)
			phi = 180.0 + phi;
	}

	float	ValTestW = viewCoordPos.w;
	float FOV_H_eachViewport = TOTAL_FOV_IN_DEGREE;
	float phiMin = -(FOV_H_eachViewport / 2.0);

	float phiMax = +(FOV_H_eachViewport / 2.0);
	//vertical angle range
	float thetaMin = 82.0;
	float thetaMax = 114.0;

	//radius range; i.e., the depth range.
	float rMin = 0.05;// -0.5;
	float rMax = 50.0;// 256.0;

	float xPrime = (phi - phiMin) / (phiMax - phiMin);
	float yPrime = (theta - thetaMin) / (thetaMax - thetaMin);
	float zPrime = (r - rMin) / (rMax - rMin);

	float xDoublePrime = 2.0 * xPrime - 1.0;
	float yDoublePrime = 2.0 * yPrime - 1.0;
	float zDoublePrime = 2.0 * zPrime - 1.0;

	float fAdjustY = 0.25;
	gl_Position = vec4(xDoublePrime, -(yDoublePrime + fAdjustY), zDoublePrime, ValTestW);

}
