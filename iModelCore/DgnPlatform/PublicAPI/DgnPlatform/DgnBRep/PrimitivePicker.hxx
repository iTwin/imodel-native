#ifndef _PrimitivePicker_Header
#define _PrimitivePicker_Header

//#include <GeometryCache.hxx>
//#include <gp_Pnt.hxx>
//#include <gp_Vec.hxx>
//#include <NCollection_Array1.hxx>
//#include <Precision.hxx>
//#include <Standard.hxx>
//#include <Standard_Type.hxx>
//#include <Standard_Transient.hxx>
//#include <TopoDS_Shape.hxx>
//#include <TopTools_IndexedMapOfShape.hxx>

typedef NCollection_Vec3<Standard_Real> PrimitivePicker_Vec3;

//! A wrapper for picking ray and its defined vicinity.
//! It pre-computes and caches the data that will be
//! used multiple times in detection algorithm, such as
//! square of the radius and inversed direction of the ray.
class Cylinder
{
public:

  //! Creates an empty cylinder
  Cylinder() {}

  //! Creates the cylinder with given characteristics
  //! and pre-computes inversed ray direction and square
  //! of the radius
  Cylinder (const gp_Pnt& theOrigin,
            const gp_Vec& theDir,
            const Standard_Real theRadius)
  : myOrigin (theOrigin),
    myDir (theDir),
    myRadius (theRadius)
  {
    // handle zeros and very small numbers in coordinates of
    // direction vector
    Standard_Real aXInv = 1.0 / Max (Abs (theDir.X()), 1e-30);
    Standard_Real aYInv = 1.0 / Max (Abs (theDir.Y()), 1e-30);
    Standard_Real aZInv = 1.0 / Max (Abs (theDir.Z()), 1e-30);
    aXInv = std::copysign (aXInv, theDir.X());
    aYInv = std::copysign (aYInv, theDir.Y());
    aZInv = std::copysign (aZInv, theDir.Z());
    myInvDir = gp_Vec (aXInv, aYInv, aZInv);

    mySqRadius = theRadius * theRadius;
  }

  const gp_Pnt& Origin() const
  {
    return myOrigin;
  }

  const gp_Vec& Dir() const
  {
    return myDir;
  }

  const gp_Vec& InvDir() const
  {
    return myInvDir;
  }

  const Standard_Real Radius() const
  {
    return myRadius;
  }

  const Standard_Real SquareRadius() const
  {
    return mySqRadius;
  }

private:

  gp_Pnt        myOrigin;
  gp_Vec        myDir;
  Standard_Real myRadius;
  Standard_Real mySqRadius; //!< Square of the cylinder's radius
  gp_Vec        myInvDir;   //!< Inverse of cylinder's direction line
};

//! This class implements the algorithms that detect overlapping
//! between the ray in the given vicinity with edges and vertices
//! of some shape. It contains the following overlap detection
//! methods:
//! - for edges:
//!    1. non-parallel straighforward implementation (exchaustive search);
//!    2. parallelized exchaustive search (or parallel brute force method);
//!    3. BVH-based search;
//!    4. BVH-based search with parallelized traverse routine;
//! - for vertices:
//!    1. non-parallel straighforward implementation (exchaustive search);
//!    2. parallelized exchaustive search (or parallel brute force method);
//!    3. parallelized search, based on sorting vertices using radix sort,
//!       combining sequences of vertices into patches with fixed size and
//!       sieving improper patches of points using overlap test between the
//!       cylinder and AABB of the patch.
//! The class also contains interface for choosing overlap detection method
//! and setting up the size of the patch.
class PrimitivePicker : public Standard_Transient
{
public:

  //! Lists implemented overlap detection methods
  enum PrimitivePicker_TypeOfAlgo
  {
    PrimitivePicker_NON_PARALLEL,  //! exchaustive search for both edges and vertices
    PrimitivePicker_BRUTE_FORCE,   //! parallelized exchaustive search
    PrimitivePicker_CACHE,         //! BVH-based search for edges and parallelized search with sorting for vertices
    PrimitivePicker_PCACHE         //! BVH-based search with parallelized traverse routine and parallelized search with sorting for vertices
  };

public:

  //! Sets up the algo for picking and input parameters: vertex and edges data.
  //! If parallelized search with sorting was chosen to detect vertices overlap,
  //! sets up patch size to the default value (512 vertices per patch).
  Standard_EXPORT PrimitivePicker (const TopTools_IndexedMapOfShape& theVerts,
                                   const TopTools_IndexedMapOfShape& theEdges,
                                   const PrimitivePicker_TypeOfAlgo theType);

  //! Cleans up edge and vertex maps
  Standard_EXPORT ~PrimitivePicker();

  //! Runs the algorithm for egde overlap detection, stores detection
  //! result in the given array thePickedEdges and parameter on edge in
  //! the corresponding item of given theCurveParams array.
  //! The result is a binary map - each item of the array corresponds
  //! to the edge with the same index in TopTools_IndexedMapOfShape map
  //! of edges. If the edge is overlapped by the given cylinder, 1 will
  //! be stored to the array, 0 otherwise.
  Standard_EXPORT void PickEdges (const gp_Pnt& theRayOrigin,
                                  const gp_Dir& theRayDir,
                                  const Standard_Real theRadius,
                                  NCollection_Array1<Standard_Character>& thePickedEdges,
                                  NCollection_Array1<Standard_Real>&      theCurveParams);

  //! Runs the algorithm for vertex overlap detection and stores the
  //! result in the given array thePickedVerts.
  //! The result is a binary map - each item of the array corresponds
  //! to the vertex with the same index in TopTools_IndexedMapOfShape map
  //! of vertices. If the vertex is overlapped by the given cylinder, 1 will
  //! be stored to the array, 0 otherwise.
  Standard_EXPORT void PickVertices (const gp_Pnt& theRayOrigin,
                                     const gp_Dir& theRayDir,
                                     const Standard_Real theRadius,
                                     NCollection_Array1<Standard_Character>& thePickedVerts);

  //! Changes the patch size to the given one. Is valid only if parallelized search with sorting
  //! algorithm for picking vertices was chosen.
  Standard_EXPORT void SetPatchSize (const Standard_Integer theSize);

  DEFINE_STANDARD_RTTIEXT (PrimitivePicker, Standard_Transient)

protected:

  //! Traverses BVH tree for edges, in case if BVH-based search was chosen.
  Standard_EXPORT void traverseEdges (NCollection_Array1<Standard_Character>& thePickedEdges,
                                      NCollection_Array1<Standard_Real>&      theCurveParams);

  //! Sorts vertex data in corresponding geometry cache, in case if it was not sorted yet.
  //! Splits vertex cache onto patches with fixed size. Iterates through all the patches in
  //! parallel mode and combines its AABB, then performs slab test to determine if there may be
  //! some vertexes that are overlapped by the given cylinder. If the test returns true,
  //! iterates through all the vertices of the patch and runs a precise overlap test.
  Standard_EXPORT void traverseVertices (NCollection_Array1<Standard_Character>& thePickedVerts);

  //! Lists geometry caches
  enum PrimitivePicker_CacheType
  {
    PrimitivePicker_VERTICE,
    PrimitivePicker_EDGE
  };

private:

  TopTools_IndexedMapOfShape              myVertices;
  TopTools_IndexedMapOfShape              myEdges;
  NCollection_Handle<GeometryCache>       myCaches[2];    //!< Geometry cache to store edge and vertex speedup structures
  Cylinder                                myCylinder;     //!< Cached cylinder characteristics
  Standard_Integer                        myPatchSize;    //!< The length of patch of vertices, is equal to 512 by default.
  Standard_Boolean                        myIsSorted;     //!< Is true if the geometry cache for vertices was sorted already
  PrimitivePicker_TypeOfAlgo              myTypeOfAlgo;   //!< Stores chosen detection algorithm
};

DEFINE_STANDARD_HANDLE (PrimitivePicker, Standard_Transient)

#endif // _PrimitivePicker_Header
