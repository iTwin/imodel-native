/*----------------------------------------------------------------------*/ 
/* PointCloud.cpp														*/ 
/* Point Cloud Implementation file										*/ 
/*----------------------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004											*/   
/*----------------------------------------------------------------------*/ 
/* Written by Faraz Ravi												*/ 
/*----------------------------------------------------------------------*/ 

#ifndef POINTOOLS_PCLOUD_IMAGE_H
#define POINTOOLS_PCLOUD_IMAGE_H


#include <pt/geomtypes.h>
#include <ptcloud2/pointcloud.h>

#pragma warning ( disable : 4805 ) // unsafe mix of bool and double

namespace pcloud
{
struct Calibration
{
	Calibration()
	{
		pinHole();
	}
	void pinHole()
	{
		k1 = k2 = k3 = k4 = p1 = p2 = 0;
		double intrin[] = {200, 0, 320, 200, 240, 0, 0, 1};
		memcpy(intrinsic, intrin, sizeof(double)*9);
		extrinsic = mmatrix4d::identity();
	}
	void setIntrinsicParameters(double fx, double fy, double cx, double cy)
	{
		intrinsic[0] = fx;
		intrinsic[2] = cx;
		intrinsic[3] = fy;
		intrinsic[4] = cy;
	}
	void setExtrinsicMatrix(const mmatrix4d &mat)
	{
		extrinsic = mat;
	}
	void setDistortionCoeffs(double p1_, double p2_, double k1_, double k2_, double k3_, double k4_)
	{
		p1 = p1_;
		p2 = p2_;
		k1 = k1_;
		k2 = k2_;
		k3 = k3_;
		k4 = k4_;
	}
	void setAngleExtents(double min_horiz, double max_horiz, double min_vert, double max_vert)
	{
		tanMaxHoriz = max_horiz;
		tanMinHoriz = min_horiz;
		tanMaxVert = max_vert;
		tanMinVert = min_vert;
	}
	Calibration &operator = (const Calibration &call)
	{
		memcpy(this, &call, sizeof(Calibration));
		return *this;
	}
	void undistort(ubyte *_dimg, int w, int h, ubyte *&new_img, int &new_w, int &new_h);

	bool hasDistortion() const { return (k1 == k2 == k3 == k4 == p1 == p2 == 0) ? true : false; }

	/* distortion coeffs */ 
	double k1;
	double k2;
	double k3;
	double k4;

	double p1;
	double p2;

	/* angle extents - these are tangents */ 
	double tanMaxHoriz;
	double tanMinHoriz;
	double tanMaxVert;
	double tanMinVert;

	/* intrinsic matrix */ 
	double		intrinsic[9];

	mmatrix4d	extrinsic;
};
class PCLOUD_API ScanImage
{
public:
	ScanImage()
	{
		_width = 0;
		_height = 0;
		_filepath = "";
		_name = "";
		_scannerDist = 5.0;
		_scannerPos = 0;
	}
	ScanImage(const ScanImage &si)
	{
		*this = si;
	}
	ScanImage(const pt::String &filepath, const Calibration &callib);
#ifdef HAVE_OPENGL
	void drawGL();
#endif
	
	static void pushGLstate();
	static void popGLstate();

	void setScannerPos(pt::Object3D *sp) { _scannerPos = sp; }
	void setImagePlaneDistance(double dist) { _scannerDist = dist; }

	pt::String name() const { if (!_name.length()) return _filepath; return _name; }

	ScanImage &operator = (const ScanImage &si);

	void setName(const pt::String name) { _name = name; }
	const pt::String &filepath() const { return _filepath; }
	const Calibration &calibration() const { return _camCalib; }

	void startTextureProjection();
	void endTextureProjection();

	void undistort(const char *new_filename);

	bool isUndistorted() const { return !_camCalib.hasDistortion(); }

private:
	Calibration		_camCalib;

	bool bindGLTexture();

	pt::String	_filepath;
	pt::String	_name;

	int	_width;
	int _height;
	
	pt::Object3D	*_scannerPos;
	double			_scannerDist;

#ifdef HAVE_OPENGL
	typedef std::map<uint, GLuint> TexIDMap;
	TexIDMap		_texids;
#endif
};

};

#endif