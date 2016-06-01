#include "PointoolsVortexAPIInternal.h"
#include <ptcloud2/pcimage.h>
#include <ptcloud2/scene.h>
//#include <il/il.h>
//#include <il/ilu.h>
//#include <il/ilut.h>
//#include <bmg/bmgdll.h>

using namespace pcloud;
using namespace pt;
//
// Constructor
//
ScanImage::ScanImage(const pt::String &filepath, const Calibration &calib) : _filepath(filepath)
{
	_camCalib = calib;
	_scannerPos = 0;
	_scannerDist = 1.0;
}
//
// assignment operator
//
ScanImage &ScanImage::operator = (const ScanImage &si)
{
	_filepath = si._filepath;
	_scannerPos = si._scannerPos;
	_scannerDist = si._scannerDist;

	_width = si._width;
	_height = si._height;

	_camCalib	= si._camCalib;

	return *this;
}
//
// bindGLTexture
// generates a GL texture from the image 
// into the current context
//
bool ScanImage::bindGLTexture()
{
#ifndef POINTOOLS_API
	static bool initializeILUT = false;
	if (!initializeILUT)
	{
		ilutRenderer(ILUT_OPENGL);
		initializeILUT = true;
	}
	/* assumes GL context is valid */ 
	uint context = (uint)wglGetCurrentContext();
	if (!context) return false;

	int numContexts = _texids.size();
	TexIDMap::iterator i = _texids.find(context);

	GLuint texid;

	if (i == _texids.end())
	{
		texid = ilutGLLoadImage((ILstring)_filepath.c_str());
		if (!texid)
		{
			/* try the path relative to the pod path */ 
			/*ptfs::FilePath path(_pointCloud->scene()->filepath());
			path.stripFilename();
			
			ptfs::FilePath imgfile(_filepath.c_str());
			imgfile.setParent(&path);
			imgfile.setRelative();

			wchar_t fullpath[260];
			imgfile.fullpath(fullpath);

			texid = ilutGLLoadImage( (ILstring)(String(imgfile.path()).c_str()) );

			if (!texid)
				return false;*/
		}
		_texids.insert(TexIDMap::value_type(context, texid));
	}
	else texid = i->second;

	glBindTexture(GL_TEXTURE_2D, texid);
	return true;
#else
	return false;
#endif
}
//
// start texture projection
//
void ScanImage::startTextureProjection()
{
#ifndef POINTOOLS_API

	if (bindGLTexture())
	{
		// Initialise automatic tex coord generation
		GLfloat eyePlaneS[] = {1.0, 0.0, 0.0, 0.0}; 
		GLfloat eyePlaneT[] = {0.0, 1.0, 0.0, 0.0}; 
		GLfloat eyePlaneR[] = {0.0, 0.0, 1.0, 0.0}; 
		GLfloat eyePlaneQ[] = {0.0, 0.0, 0.0, 1.0}; 
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); 
		glTexGenfv(GL_S, GL_EYE_PLANE, eyePlaneS); 
		glTexGenfv(GL_T, GL_EYE_PLANE, eyePlaneT); 
		glTexGenfv(GL_R, GL_EYE_PLANE, eyePlaneR); 
		glTexGenfv(GL_Q, GL_EYE_PLANE, eyePlaneQ); 
		glEnable(GL_TEXTURE_GEN_S); 
		glEnable(GL_TEXTURE_GEN_T); 
		glEnable(GL_TEXTURE_GEN_R); 
		glEnable(GL_TEXTURE_GEN_Q);	

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		/* coordinate normalisation from -1,1 to 0, 1 */ 
		glMatrixMode(GL_TEXTURE); 
		//glPushMatrix();
		glLoadIdentity(); 
		glTranslatef(0.5,0.5,0); 
		glScalef(-0.5,0.5,1); 

		/* persepective mat */ 
		glFrustum(_camCalib.tanMinHoriz, _camCalib.tanMaxHoriz, _camCalib.tanMinVert, _camCalib.tanMaxVert, 1.0, 100.0);

		/* camera view */ 
		mmatrix4d ext(_camCalib.extrinsic);
		ext.invert();

		glMultMatrixd( (double*)&ext ); 

		/* current modelview - inverted */ 
		mmatrix4d mview;
		mview.loadGLmodel();
		mview.invert();

		//glMultMatrixd((double*)&mview); 
	}
#endif
}
//
// Helper structures used in image calibration
//
struct Pix
{
	template <class T> 
	void set(T _r, T _g, T _b) { r = (ubyte)_r; g = (ubyte)_g; b = (ubyte)_b; }
	ubyte r;
	ubyte g;
	ubyte b;
};
//
struct Img
{
	Img(int w, int h, ubyte *data)
	{
		_data = data;
		_w = w;//((int)w + 3) & ~3; /* 4 pix boundary, unused */ 
		_h = h;
	}
#ifndef POINTOOLS_API

	Img(int w, int h)
	{
		_h = h;
		_w = (w + 3) & ~3;

		_data = new ubyte[_w*_h*3];
		ZeroMemory(_data, _w*_h*3);

		ilGenImages(1, &_id);
		ilBindImage(_id);
		ilTexImage(_w, _h, 1, 3, IL_BGR, IL_UNSIGNED_BYTE, 0);
	}
	~Img() {}
	
	inline void clamp(int &x, int &y) const
	{
		if (x < 0)  x = 0; 
		else if (x > _w-1) x = _w-1; 
		if (y < 0)  y = 0; 
		else if (y > _h-1) y = _h-1;
	}
	inline ubyte *get(int x, int y) 
	{ 
		clamp(x,y); 
		return &_data[3*(_w * y + x)]; 
	}
	
	void get(double x, double y, Pix &px)
	{
		double cx = fmod(x, 1.0);
		double cy = fmod(y, 1.0);
		double area = 0;
		double overlap = 0;

		double ix = 1.0 - cx;
		double iy = 1.0 - cy;

		/* get 3 x 3 matrix */ 
		ubyte *pix = get(x, y);
		ubyte *pixx1 = get(x+1, y);
		ubyte *pixx1y1 = get(x+1, y+1);
		ubyte *pixy1 = get(x, y+1);

		pt::vector3d pxv[4];
		
		pxv[0].set(pix[0] * ix * iy, pix[1] * ix * iy, pix[2] * ix * iy);
		pxv[1].set(pixx1[0] * cx * iy, pixx1[1] * cx * iy, pixx1[2] * cx * iy);
		pxv[2].set(pixx1y1[0] * cx * cy, pixx1y1[1] * cx * cy, pixx1y1[2] * cx * cy);
		pxv[3].set(pixy1[0] * cy * ix, pixy1[1] * cy * ix, pixy1[2] * cy * ix);

		pxv[0] += pxv[1] + pxv[2] + pxv[3];

		px.set(pxv[0].x, pxv[0].y, pxv[0].z);
	}
	ILuint _id;
#endif
	ubyte *_data;

	int _w;
	int _h;
};
//
// Undistort an image
//
void Calibration::undistort(ubyte *_dimg, int w, int h, ubyte *&new_img, int &new_w, int &new_h)
{
#ifndef POINTOOLS_API

	double fx = intrinsic[0];
	double cx = intrinsic[2];
	double fy = intrinsic[3];
	double cy = intrinsic[4];

	double uMin  = tanMinHoriz * fx + cx;
	double uMax  = tanMaxHoriz * fx + cx;
	int uSize = uMax-uMin+1;

	double vMin  = tanMinVert * fy + cy;
	double vMax  = tanMaxVert * fy + cy;
	int vSize = vMax-vMin+1;
	
	double ud, vd;
	double r2, r4, r6, r8, x, y, x2, y2;

	Img dimg(w, h, _dimg);

	int edge_x = uSize;
	int edge_y = vSize;

	int v, u;

#define COMPUTE_DISTORTED(u,v) \
		x = (u - cx)/fx; \
		y = (v - cy)/fy; \
		x2 = x*x; \
		y2 = y*y; \
		r2 = x2 + y2; \
		r4 = r2 * r2; \
		r6 = r2 * r4; \
		r8 = r4 * r4; \
		ud = u + x*fx*(k1*r2+k2*r4+k3*r6+k4*r8) + 2*fx*x*y*p1 + p2*fx*(r2 + 2*x2); \
		vd = v + y*fy*(k1*r2+k2*r4+k3*r6+k4*r8) + 2*fy*x*y*p2 + p1*fy*(r2 + 2*y2); \


	/* determine true extents of undistorted image*/ 
	for (v=0 ; v<vSize; v++)
	{
		COMPUTE_DISTORTED(0,v);
		if (vd >= h) 
		{
			edge_y = v;
			break;
		}
	}
	for (u=0 ; u<uSize; u++)
	{
		COMPUTE_DISTORTED(u,0);
		if (ud >= w)
		{
			edge_x = u;
			break;
		}
	}
	//uSize = edge_x;
	//vSize = edge_y;

	Img img(uSize, vSize);

	/* do undistortion */ 
	for (v=0 ; v<vSize; v++)
	{
		for (u=0; u<uSize; u++)
		{
			COMPUTE_DISTORTED(u,v);
			
			Pix px;
			dimg.get(ud, vd, px);
			
			memcpy(img.get(u,vSize-v), &px, 3);

			if (ud >= w) edge_x = u;
		}
	}
	/* its now undistorted */ 
	k1 = k2 = k3 = k4 = p1 = p2 = 0;

	new_img = img._data;
	new_w = img._w;
	new_h = img._h;

	ilBindImage(img._id);
	ilSetPixels(0,0,0,img._w,img._h,1,IL_RGB, IL_UNSIGNED_BYTE, img._data);
#endif
}
//
// undistort image
//
void ScanImage::undistort(const char *new_filename)
{
#ifndef POINTOOLS_API

	if (!_camCalib.hasDistortion()) return;

	/* load the current image */ 
	unsigned char*pBitmap=0;
	
	ILuint imgid = iluLoadImage((ILstring)filepath().c_str());

	if (imgid)
	{
		ubyte *data = ilGetData();
		if (data)
		{
			ubyte *newimg = 0;
			int nw=0, nh = 0;
			int w = ilGetInteger(IL_IMAGE_WIDTH);
			int h = ilGetInteger(IL_IMAGE_HEIGHT);;

			_camCalib.undistort(data, w, h, newimg, nw, nh);

			if (newimg)
			{
				ilEnable(IL_FILE_OVERWRITE);
				ilSaveImage(new_filename);
				_filepath = new_filename;
			}
		}
	}
#endif
	/* TODO: undistort it */ 

	/* TODO: save it */ 

}

#ifdef HAVE_OPENGL
//
// end texture projection
//
void ScanImage::endTextureProjection()
{
	//glMatrixMode(GL_TEXTURE);
	//glPopMatrix();

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
}
//
// set up GL renderer
//
void ScanImage::pushGLstate()
{
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	glShadeModel(GL_FLAT);
	glEnable(GL_TEXTURE_2D);
}
//
// set up GL renderer
//
void ScanImage::popGLstate()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
}
//
// drawGL. Draw the image on an image plane
//
void ScanImage::drawGL()
{
	if (bindGLTexture())
	{
		if (_scannerPos)
			_scannerPos->registration().pushGL();
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); 
		glMultMatrixd((double*)&_camCalib.extrinsic);
		
		glColor3ub(255,255,255);
		
		glBegin(GL_QUADS);
			glTexCoord2d(1,1);
			glVertex3f(_camCalib.tanMaxHoriz * _scannerDist,_camCalib.tanMinVert * _scannerDist, _scannerDist);

			glTexCoord2d(0,1);
			glVertex3f(_camCalib.tanMinHoriz * _scannerDist,_camCalib.tanMinVert * _scannerDist, _scannerDist);

			glTexCoord2d(0,0);
			glVertex3f(_camCalib.tanMinHoriz * _scannerDist,_camCalib.tanMaxVert * _scannerDist, _scannerDist);

			glTexCoord2d(1,0);
			glVertex3f(_camCalib.tanMaxHoriz * _scannerDist,_camCalib.tanMaxVert * _scannerDist, _scannerDist);


		glEnd();
		
		glMatrixMode(GL_MODELVIEW);
		
		glPopMatrix();

		if (_scannerPos)
			glPopMatrix();
	}
}
#endif