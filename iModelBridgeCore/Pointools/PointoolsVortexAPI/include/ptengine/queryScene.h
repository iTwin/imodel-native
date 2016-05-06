/*----------------------------------------------------------*/ 
/* QueryScene.h												*/ 
/* Point Scene Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2005								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTOOLS_POINTCLOUD_QUERY_SCENE
#define POINTOOLS_POINTCLOUD_QUERY_SCENE 1
#include <ptcloud2/pointcloud.h>
#include <ptengine/pointsvisitor.h>
#include <pt/plane.h>
#include <pt/fence.h>
#include <mutex>

namespace pointsengine
{


	struct NoLock {
		inline NoLock(std::mutex &m){};
		inline bool locked() const { return true; }
	};
	//
	// Conditional Filter
	//
	struct IterateFullVoxelPoints
	{
		template <class ConditionFunctor, class Receptor>
			static void iteratePoints(pcloud::Voxel *vox, ConditionFunctor &C, Receptor &R, pt::CoordinateSpace cs = pt::ProjectSpace)
		{
			ConditionalPointsFilter<ConditionFunctor, Receptor> cr(C, R, vox);
			vox->iterateFullTransformedPoints(cr, cs, /* false for now */false);
		}
		template <class ConditionFunctor, class Receptor, class Transformer>
			static void iteratePoints(pcloud::Voxel *vox, ConditionFunctor &C, Receptor &R, Transformer &T)
		{
			ConditionalPointsFilter<ConditionFunctor, Receptor> cr(C, R, vox);
			vox->_iterateFullTransformedPoints(cr, T, /* false for now */false);
		}

	private:
		template <class ConditionFunctor, class Receptor>
		struct ConditionalPointsFilter
		{
			ConditionalPointsFilter(ConditionFunctor &c, Receptor &r, pcloud::Voxel *vox)
				: C(c), R(r)
			{ R.setVoxel(vox); }

			template <class T> 
				inline void point(const T &pt, int index, const ubyte &filter, short *inten, 
					pt::vec3<unsigned char> *rgb, pt::vector3s *norm )
			{ if (C(pt)) R.point(pt, index, filter, inten, rgb, norm); }

		private:
			Receptor			&R;
			ConditionFunctor	&C;
		};
	};
	struct IterateVoxelPoints
	{
		template <class ConditionFunctor, class Receptor>
			static void iteratePoints(pcloud::Voxel *vox, ConditionFunctor &C, Receptor &R, pt::CoordinateSpace cs = pt::ProjectSpace)
		{
			ConditionalPointsFilter<ConditionFunctor, Receptor> cr(C, R, vox);
			vox->iterateTransformedPoints(cr, cs, true);
		}
		template <class ConditionFunctor, class Receptor, class Transformer>
			static void iteratePoints(pcloud::Voxel *vox, ConditionFunctor &C, Receptor &R, Transformer &T)
		{
			ConditionalPointsFilter<ConditionFunctor, Receptor> cr(C, R, vox);
			vox->_iterateTransformedPoints(cr, T, true);
		}
	private:
		template <class ConditionFunctor, class Receptor>
		struct ConditionalPointsFilter
		{
			ConditionalPointsFilter(ConditionFunctor &c, Receptor &r, pcloud::Voxel *vox)
				: C(c), R(r)
			{
				R.setVoxel(vox);
			}

			template <class T> 
				inline void point(const T &pt, int index, ubyte &f) { if (C(pt)) R.point(pt, index); }

		private:
			Receptor			&R;
			ConditionFunctor	&C;
		};
	};
	struct AnyPointCondition
	{
		bool operator() (const pt::vector3 &p) { return true;	}
		bool operator() (const pt::vector3d &p){ return true;	}
	};
	//
	// Visible Points (not clipped and visible property = true)
	//
	struct PointVisibleCondition
	{
		PointVisibleCondition(const mmatrix4d &clipboxmatrix) : B(clipboxmatrix) {};

		bool operator() (const pt::vector3 &p)	
		{ 
			f.set(p);
			B.vec3_multiply_mat4f(f, f1); 
			
			return (f1.x >= -1 && f1.y >= -1 && f1.z >= -1 && 
				f1.x <= 1 && f1.y <= 1 && f1.z <= 1);
		}
		bool operator() (const pt::vector3d &p)
		{ 
			f.set(p);
			B.vec3_multiply_mat4f(f, f1); 
			
			return (f1.x >= -1 && f1.y >= -1 && f1.z >= -1 && 
				f1.x <= 1 && f1.y <= 1 && f1.z <= 1);
		}
		pt::vector3 f, f1;

		mmatrix4d B;
	};
	template <class Receptor, class IterationMode = IterateVoxelPoints, class Lock = NoLock>
	struct VisibleQuery : public PointsVisitor
	{
		VisibleQuery(Receptor &r, const mmatrix4d &clipboxmatrix, bool useclip = true, pt::CoordinateSpace cs = pt::ProjectSpace)
			: PointsVisitor(cs), receptor(r), use_clip(useclip), viscond(clipboxmatrix)
		{}

		bool scene(pcloud::Scene *sc)		{ return sc->displayInfo().visible(); }
		bool cloud(pcloud::PointCloud *pc)	{ return pc->displayInfo().visible(); }
		bool voxel(pcloud::Voxel *vox)
		{
			Lock lock(vox->mutex());
			if (!lock.locked()) return false;

			/* iterate through points */ 
			if (use_clip)
			{
				IterationMode::iteratePoints(vox, viscond, receptor, cspace);
			}
			else
			{
				AnyPointCondition anycond;
				IterationMode::iteratePoints(vox, anycond, receptor, cspace);
			}
			return true;
		}
		PointVisibleCondition	viscond;
		Receptor				&receptor;
		bool use_clip;
	};
	//
	// Point In Sphere
	//
	struct PointInSphere
	{
		PointInSphere(pt::BoundingSphere &bs) : sphere(bs) {};
		bool operator() (const pt::vector3 &p )	{ return sphere.contains(p); }
		bool operator() (const pt::vector3d &p )	{ return sphere.contains(pt::vector3(p.x, p.y, p.z)); }
		const pt::BoundingSphere &sphere;
	};

#ifdef NEEDS_WORK_VORTEX_DGNDB 
	//
	// Spherical Query
	//
	template <class Receptor, class Lock = NoLock>
	struct SphericalQuery : public PointsVisitor
	{
		SphericalQuery(Receptor &r, const pt::vector3 &cen, float rad) : receptor(r)
		{
			sphere.center(cen);
			sphere.radius(rad);
		}
		bool scene(pcloud::Scene *sc)	{
			return boxSphereIntersects(objCsBounds);
		}
		bool cloud(pcloud::PointCloud *pc)	{
			return boxSphereIntersects(objCsBounds)
		}
		bool voxel(pcloud::Voxel *vox)
		{
			if (!boxSphereIntersects(objCsBounds)) return false;
		
			Lock lock(vox->mutex());
			if (!lock.locked()) return false;

			/* iterate through points */ 
			PointInSphere P(sphere);
			IterateVoxelPoints::iteratePoints(vox, P, receptor);
			return true;
		}
	private:
		bool boxSphereIntersects(const pt::BoundingBox &bb)
		{
			vector3 v;
			for (int c=0; c<8; c++)
			{
				bb.getExtrema(c, v);
				if (sphere.contains(v))	return true;
			}		
			return false;
		}
		pt::BoundingSphere   sphere;
		Receptor		 &receptor;
	};
#endif
	//
	// Point In Box
	//
	struct PointInBox
	{
		PointInBox(pt::BoundingBox &bx) : box(bx) {};
		bool operator() (const pt::vector3 &p )	{ return box.inBounds(p); }
		bool operator() (const pt::vector3d &p)	{ return box.inBounds(pt::vector3(p.x, p.y, p.z)); }
		const pt::BoundingBox &box;
	};
#ifdef NEEDS_WORK_VORTEX_DGNDB 
	//
	// Box Query
	//
	template <class Receptor, class Lock = NoLock>
	struct BoxQuery : public PointsVisitor
	{
		BoxQuery(Receptor &r, const pt::BoundingBox &bx) : receptor(r)	{
			box = bx;
		}
		bool scene(pcloud::Scene *sc)		{ return box.intersects(&objCsBounds); }
		bool cloud(pcloud::PointCloud *pc)	{ return box.intersects(&objCsBounds); }
		bool voxel(pcloud::Voxel *vox)
		{
			if (!box.intersects(&objCsBounds)) return false;

			Lock lock(vox->mutex());
			if (!lock.locked()) return false;

			/* iterate through points */ 
			PointInBox P(box);
			IterateVoxelPoints::iteratePoints(vox, P, receptor);
			return true;
		}
	private:
		pt::BoundingBox		box;
		Receptor			&receptor;
	};
#endif
	//
	// Point On Plane
	//
	struct PointOnPlane
	{
		PointOnPlane(pt::Planef &pl, float tolerance) : plane(pl) {};
		bool operator() (const pt::vector3 &p)	{ 
			return (fabs(plane.distToPlane(p)) < tolerance);
		}
		bool operator() (const pt::vector3d &p)	{ 
			return (fabs(plane.distToPlane(pt::vector3(p.x, p.y, p.z))) < tolerance);
		}
		
		const pt::Planef &plane;
		float tolerance;
	};
	//
	// Plane Query
	//
	template <class Receptor, class Lock = NoLock>
	struct PlaneQuery : public PointsVisitor
	{
		PlaneQuery(Receptor &r, const pt::Planef &p, float tol)
			: receptor(r), plane(p), tolerance(tol) {}
			
		bool scene(pcloud::Scene *sc)		{ return plane.intersects(objCsBounds); }
		bool cloud(pcloud::PointCloud *c)	{ return plane.intersects(objCsBounds); }
		bool voxel(pcloud::Voxel *vox)
		{
			if (!plane.intersects(objCsBounds)) return false;

			Lock lock(vox->mutex());
			if (!lock.locked()) return false;

			/* iterate through points */ 
			PointOnPlane P(plane, tolerance);
			IterateVoxelPoints::iteratePoints(vox, P, receptor);
			return true;
		}
		pt::Planef plane;
		float tolerance;
		Receptor			&receptor;
	};
	//
	// Point In Polygon
	//
	struct PointInPolygon
	{
		PointInPolygon(pt::Planef &pl, pt::Fencef &f, float tol) : plane(pl), fence(f), tolerance(tol) {};
		
		bool operator() (const pt::vector3 &p) { 
			plane.to2D(p, point);
            return fabs(plane.distToPlane(p)) < tolerance && fence.pntInFence(point);
		}
		bool operator() (const pt::vector3d &p)	{ 
			return (*this)(pt::vector3(p.x, p.y, p.z));
		}
		
		pt::vector2 point;
		const pt::Fencef &fence;
		const pt::Planef &plane;
		float tolerance;
	};
	struct PlaneSpaceTransform
	{
		void prepare(){}
	
		inline void transform(const pt::vector3 &v0, pt::vector3 &v1)		{ 
			plane.to2D(v0, v1);
			v1.z = 0;
		};
		inline void transform(const pt::vector3d &v0, pt::vector3d &v1)		{ 
			pt::vector2 v1f;
			plane.to2D(pt::vector3(v0), v1f);
			v1.x = v1f.x;
			v1.y = v1f.y;
			v1.z = 0;
		};
		inline void transform(const pt::vector3d &v0, pt::vector3 &v1)		{ 
			plane.to2D(pt::vector3(v0), v1);
			v1.z = 0;
		};
		pt::Planef plane;
	};

#ifdef NEEDS_WORK_VORTEX_DGNDB 
	//
	// Plane Query
	//
	template <class Receptor, class Lock = NoLock>
	struct PolygonQuery : public PointsVisitor
	{
		PolygonQuery(Receptor &r, const std::vector<pt::vector3> &polygon, float tol, bool planespace = false)
			: receptor(r), tolerance(tol) 
		{
			if (polygon.size() < 3)
			{
				valid = false;
				return;
			}
			/*establish plane from first 3 points*/ 
			plane.from3points(polygon[0], polygon[1], polygon[2]);						
			transform.plane = plane;
			if (plane.normal().length2() < 0.001)
			{
				valid = false;
				return;
			}
			valid = true;

			/*create fence*/ 
			for (int i=0; i<polygon.size(); i++)
			{
				vector2 v2;
				plane.to2D(polygon[i], v2);
				fence.addPoint(v2);
			}
			planeSpace = planespace;
		}
		PolygonQuery(Receptor &r, const std::vector<pt::vector3> &polygon, const pt::Planef &plane, float tol, bool planespace = false)
			: receptor(r), tolerance(tol) 
		{
			if (polygon.size() < 3)
			{
				valid = false;
				return;
			}
			valid = true;
			plane.from3points(polygon[0], polygon[1], polygon[2]);						
			transform.plane = plane;

			/*create fence*/ 
			for (int i=0; i<polygon.size(); i++)
			{
				vector2 v2;
				plane.to2D(polygon[i], v2);
				fence.addPoint(v2);
			}
			planeSpace = planespace;
		}
		bool scene(pcloud::Scene *sc)		{ return valid ? plane.intersects(objCsBounds) : false; }
		bool cloud(pcloud::PointCloud *c)	{ return plane.intersects(objCsBounds); }
		bool voxel(pcloud::Voxel *vox)
		{
			if (!plane.intersects(objCsBounds)) return false;

			Lock lock(vox->mutex());
			if (!lock.locked()) return false;

			/* iterate through points */ 
			PointInPolygon P(plane, fence, tolerance);
			//if (planeSpace)
			//	IterateVoxelPoints::iteratePoints(vox, P, receptor, transform);
			//else
			IterateVoxelPoints::iteratePoints(vox, P, receptor);
			return true;
		}
		PlaneSpaceTransform		transform;
		pt::Fencef fence;
		pt::Planef plane;
		float tolerance;
		Receptor				&receptor;
		bool valid;
		bool planeSpace;
	};
#endif
}
#endif