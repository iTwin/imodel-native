/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/mxtriangle.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef _mxtriangle_h_
#define _mxtriangle_h_

#include "memory.h"
#include "msl_array.h"



//#define ArrayClassWin ArrayClass

class MXTriangle
{
public:

#if defined ( MSVERSION ) 

    typedef DPoint3d    point;

#else

	struct point
	{
		double x;
		double y;
		double z;
	};

#endif

	struct triangle
	{
		int ptNum[3];
		int adjTri[3];
//		int pts[3];
		int strings[3];
		int groupCode;
	};
	typedef ArrayBulkPtrClass<point> PointArray;
	typedef ArrayBulkPtrClass<triangle> TriangleArray;

	struct triangleEdge
	{
		int triNum;
		int side;
	};

private:

	point lastPoint;  // was rptold & rptzold

	point currentPoint;
	const point* rpts;
	int np;
	int iimax;
	int irmax;
	double xmnval;
	double ymnval;
	double xmxval;
	double ymxval;
	double cint;
	double slope;
	double slomax;
	double scint;
	double wcint;
	double zminn;
	double zmaxx;
	double rxcd;
	double rycd;

	int lastri;
	int ntrirc;

	int iptdup;
	int npt;
	int nstack;
	int link;
	int iptold;
	int isrec;
	int kprim;
	int iftswt;
	int kdfalt, kdfac,klab,kpnt,kjcp, kcnt,kpct,kdup,itxmin,itymin,itxmax,itymax,itplab,icopmd,ierrtc,npttot,itrimb;
	int nent;
	int m_startPointNum;
	int mtri;
	ArrayClassWin<int> istack;
	ArrayClassWin<int> itrace;
	MXTriangle::PointArray points;
	MXTriangle::TriangleArray triangles;
	point** pointsPtr;
	triangle** trianglesPtr;

	int rptsPointNum;
	int nptlup;

//	void fidtri(int& mtri, triangle*& marea, int& mp1, int*& mp2);
	void fidpnt(int& mtri, triangle*& marea, int*& mp2, const point& rpt, int& ndup);
	void fdpnt2(int& mtri, triangle*& marea, int*& mp2, const point& rpt, int& ndup);
	void trgerr(const int ierr) const;
	void steck(const int mtri, const int mtrio, const int* mp2, const int mside);
	void ckstak(const int mtri, const int ntri, const int k1, const int k2, const int k4, const int k5);
	void bstfet(int& ier);
	int addPoint(const double& x, const double& y, const double& z);
	int addPoint(const point& pt);
	void eddxpt(const point& a, int& m1, int& m1side, int& nxlup, int& pointNum);
	void eduppt(int& mtri, triangle*& marea, int* &mp2, int& ndup, /*int& lpts, */int& ier);
	void eddpnt(int& mtri, triangle*& marea, int*& mp2,int& ier);
	void festad(int incr);
public:

	MXTriangle()
        {
	    empty();
	    m_startPointNum = 0;
	    mtri = -1;
	    nstack = 0;
        }
    virtual ~MXTriangle() {}
	void AddTriangles(const point* array, const int array_length);
	void AddString(const point* array, const int array_length, const int isPoints, const int label = 0);
	void checkExtents(const double& xmin, const double& ymin, const double& xmax, const double& ymax);
	int getTriangle(const point& trianglePt, int mtri = 1) const;
	int intersectWith(const point& startPt, const point& endPt, ArrayClass<point>& intersectPoints, int mtri = -1) const
	{
		ArrayClass<triangleEdge> edges;
		return intersectWith(startPt, endPt, intersectPoints, edges, mtri);
	}
	int intersectWith(const point& startPt, const point& endPt, ArrayClass<point>& intersectPoints, ArrayClass<triangleEdge>& edges, int mtri = -1) const;
	const MXTriangle::PointArray& getPoints(void) const
	{
		return points;
	}
	const MXTriangle::TriangleArray& getTriangles(void) const
	{
		return triangles;
	}
	void getPtrs(MXTriangle::TriangleArray*& triPtr, MXTriangle::PointArray*& pointsPtr)
	{
		triPtr = &triangles;
		pointsPtr = &points;
	}
	void setPoints(const MXTriangle::PointArray& newPoints)
	{
		points = newPoints;
	}
	void setTriangles(const MXTriangle::TriangleArray& newTriangles)
	{
		triangles = newTriangles;
		lastri = triangles.size() - 1;
	}
	void updateLastTri()
	{
		lastri = triangles.size() - 1;
	}
	void setEstMemSize(const int numPoints);
	void increaseExtents(const double& xmin, const double& ymin, const double& xmax, const double& ymax);
	void setIFTSWT(const int value)
	{
		iftswt = value;
	}
	void empty()
	{
		triangles.empty();
		points.empty();
	}
	void getExtents(double& xmnval, double& ymnval, double& xmxval, double& ymxval)
	{
		if(points.size())
		{
			xmnval = points[0].x;
			ymnval = points[0].y;
			xmxval = points[2].x;
			ymxval = points[1].y;
		}
		else
		{
			xmnval = ymnval = xmxval = ymxval = 0;
		}
	}
	void getTriangleSlope(const int triangleNum, double& slopeVal) const;
	void setStartPointNum(const int value)
	{
		m_startPointNum = value - 1;
	}

	struct borderInfo
	{
		int triangleNumber;
		int borderEdge;
	};
	struct intersectInfo
	{
		int triangleNumber;
		int intersectEdge;
	};

	void getInnerTriangles(ArrayClass<borderInfo>& t1BorderInfo, ArrayClass<int>& triangleNumbers);
	void getBorderInfo(ArrayClass<borderInfo>& BorderInfo, ArrayClass<int>& innerBorderPoints, unsigned char outer);
	void getBorder(ArrayClass<point>& points, ArrayClass<borderInfo>& borderInfo, ArrayClass<int>& borderPoints, unsigned char outside);
	void getIntersectInfo(ArrayClass<intersectInfo>& intersectInfo, ArrayClass<point>& points);
	void pasteTriangles(ArrayClass<borderInfo>& t1BorderInfo, ArrayClass<int> reuseTriangles, MXTriangle* t2, ArrayClass<borderInfo>& t2BorderInfo, ArrayClass<int>& useTriangles);
	void replaceSurface(MXTriangle* tri, ArrayClass<point> points);
	inline void clearMtri()
	{
		mtri = -1;
	}

private:
	int getLastClockwise(int& tNum, const int ptNum) const;
protected:
	virtual void ReportPointOutsideTriangle(const double& x, const double& y)
	{
	}
	virtual void ReportError(int a, int b, int c, int d, const double& x1, const double& y1, const double& x2, const double& y2)
	{
	}

	virtual void ReportDuplicatePoint(const double& x1, const double& y1, const double& z1, long label1,
		const double& x2, const double& y2, const double& z2, long label2)
	{
	}
	virtual void ReportIntersectStrings(const double& x, const double& y, const double& z1, const double& z2, long label1, long label2)
	{
	}
	virtual void ReportBestFitFailure(const double& x1, const double& y1, const double& x2, const double& y2)
	{
	}
	void getCurrentLink(const point*& pt1, const point*& pt2)
	{
		if(nptlup == 1)
		{
			pt1 = &rpts[nptlup - 1];
			pt2 = &rpts[nptlup];
		}
		else
		{
			pt1 = &rpts[nptlup - 2];
			pt2 = &rpts[nptlup - 1];
		}
	}
};


#endif