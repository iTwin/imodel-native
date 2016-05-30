#include "PointoolsVortexAPIInternal.h"
#include <ptedit/fencefilter.h>

#ifdef PI
#undef PI
#undef DEG_TO_RAD
#undef RAD_TO_DEG
#ifdef HAVE_WILDMAGIC
#include <wildmagic/math/Wm5convexhull2.h>
#endif
#endif
using namespace ptedit;
using namespace pt;

/*****************************************************************************/
/**
* @brief
* @param f
* @param view
* @param unitsPerMetre
* @return void
*/
/*****************************************************************************/
void FenceSelect::buildFromScreenFence(const pt::Fence<int> &f, const pt::ViewParams &view, double unitsPerMetre )
{
	memcpy(modelm.data(), view.eye_matrix, sizeof(double) * 16);
	modelm.transpose();
	units = unitsPerMetre;

	clear();
	vector3d a, ar, art;

	isPerspective = (view.proj_matrix[15] < 1.0) ? true : false;

	int size = f.numPoints();
	for (int i=0; i<size; i++)
	{
        a.set(static_cast<float>(f.point(i).x), static_cast<float>(f.point(i).y), 0.5);
		view.unproject3v(ar, a);
		modelm.vec3_multiply_mat4(ar, art);

		if (isPerspective)
		{
			/* eye coords - if perspective */ 
			art.x /= art.z;
			art.y /= art.z;
		}
		fence.addPoint(vec2<double>(art.x, art.y));
	}
	isValid = generateHullPlanes();
}
#ifdef HAVE_OPENGL
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void FenceSelect::draw()
{
	glPolygonMode(GL_BACK, GL_LINE);
	glPolygonMode(GL_FRONT, GL_FILL);

	if (hullPlanes.size() > 2)
	{
		float step = 1.0f / hullPlanes.size();

		pt::vector3d p0,p1,p2,p3;

		for (uint i=0; i<hullPlanes.size(); i++)
		{
			glColor3f(1.0f, step * i, 1.0f - step * i);
			
			double s = 2;
			if (i == hullPlanes.size() - 1) s = 100;

			const pt::Planed &plane = hullPlanes[i];
			plane.to3D( pt::vec2<double>(-s,-s), p0 );
			plane.to3D( pt::vec2<double>(-s,s), p1 );
			plane.to3D( pt::vec2<double>(s,s), p2 );
			plane.to3D( pt::vec2<double>(s,-s), p3 );

			glBegin(GL_QUADS);
				glVertex3dv( p0 );
				glVertex3dv( p1 );
				glVertex3dv( p2 );
				glVertex3dv( p3 );
			glEnd();
		}
	}
}
#endif
/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool FenceSelect::generateHullPlanes()
{
	if (!fence.numPoints()) return false;

	hullPlanes.clear();

	mmatrix4d invmdl(modelm);
	invmdl.invert();

	/* get 2D hull */ 
#ifdef HAVE_WILDMAGIC
	Wm5::Vector2f *vertices = new Wm5::Vector2f[fence.numPoints()];
#else
    pt::vector2 *vertices = new pt::vector2[fence.numPoints()];
#endif
	for (int i=0; i<fence.numPoints(); i++)
	{
#ifdef HAVE_WILDMAGIC
		vertices[i].X() = static_cast<float>(fence[i].x);
		vertices[i].Y() = static_cast<float>(fence[i].y);
#else
        vertices[i].x = static_cast<float>(fence[i].x);
		vertices[i].y = static_cast<float>(fence[i].y);
#endif
	}
#ifdef HAVE_WILDMAGIC
	Wm5::ConvexHull2f chull( fence.numPoints(), vertices, 1e-5, false, Wm5::Query::QT_INTEGER);
	if (chull.GetDimension() >= 2)
	{

		int numEdges = chull.GetNumSimplices();
		isConvex = numEdges == fence.numPoints() ? true : false;

		pt::vector3d p30,p31, p32;

		/* create planes */ 
		for (int i=0; i<numEdges; i++)
		{
			Wm5::Vector2f p0 = vertices[ chull.GetIndices()[i] ];
			Wm5::Vector2f p1 = vertices[ chull.GetIndices()[(i+1) % numEdges] ];

			if (isPerspective)
			{
				p30.set(p0.X()*-1,p0.Y()*-1, -1.0);
				p31.set(p1.X()*-1,p1.Y()*-1,-1.0);
				p32.set(p1.X()*-3.0f,p1.Y()*-3.0f,-3.0);
			}
			else
			{
				p30.set(p0.X(), p0.Y(), -0.5);
				p31.set(p1.X(), p1.Y(), -0.5);
				p32.set(p1.X(), p1.Y(), -1.0);
			}
			invmdl.vec3_multiply_mat4(p30);
			invmdl.vec3_multiply_mat4(p31);
			invmdl.vec3_multiply_mat4(p32);

			pt::Planed plane(p30,p31,p32);
			hullPlanes.push_back(plane);
		}
		
		p30.set(modelm.extractZVector().data());
		p30.normalize();

		pt::Planed nearp( p31, -p30 );
        double constantd = nearp.constant();
		nearp.constant(constantd - 1);

		if (isPerspective) hullPlanes.push_back(nearp);
	}
	else
	{
		delete [] vertices;
		return false;
	}
#else
    DPoint3dP chull = new DPoint3d[fence.numPoints()];
    int hullNbPts = -1;
    // &&RB The following geomlibs function call must be tested
    if ( false == bsiDPoint3dArray_convexHullXY(chull, &hullNbPts, reinterpret_cast<DPoint3dP>(vertices), fence.numPoints()))
    {
        assert(hullNbPts <= fence.numPoints());

        int numEdges = hullNbPts;
        isConvex = numEdges == fence.numPoints();

        pt::vector3d p30,p31, p32;

        /* create planes */ 
        for (int i=0; i<numEdges; i++)
        {
            auto p0 = chull[i];
            auto p1 = chull[i+1];

            if (isPerspective)
            {
                p30.set(p0.x*-1,p0.y*-1, -1.0);
                p31.set(p1.x*-1,p1.y*-1,-1.0);
                p32.set(p1.x*-3.0f,p1.y*-3.0f,-3.0);
            }
            else
            {
                p30.set(p0.x, p0.y, -0.5);
                p31.set(p1.x, p1.y, -0.5);
                p32.set(p1.x, p1.y, -1.0);
            }
            invmdl.vec3_multiply_mat4(p30);
            invmdl.vec3_multiply_mat4(p31);
            invmdl.vec3_multiply_mat4(p32);

            pt::Planed plane(p30,p31,p32);
            hullPlanes.push_back(plane);
        }

        p30.set(modelm.extractZVector().data());
        p30.normalize();

        pt::Planed nearp( p31, -p30 );
        nearp.constant(nearp.constant() - 1);
        if (isPerspective) hullPlanes.push_back(nearp);
    }
    else 
    {
        delete[] vertices;
        return false;
    }
#endif
	delete [] vertices;

	return true;
}
