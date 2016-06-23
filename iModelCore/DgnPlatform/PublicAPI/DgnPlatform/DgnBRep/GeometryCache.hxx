#ifndef _GeometryCache_Header
#define _GeometryCache_Header

//#include <BVH_PrimitiveSet.hxx>
//#include <NCollection_Array1.hxx>
//#include <TopTools_IndexedMapOfShape.hxx>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
/*
    .../GeometryCache.hxx:23:53: error: 'GeometryCache::Box' hides overloaded virtual function [-Werror,-Woverloaded-virtual]
    .../BVH_PrimitiveSet.hxx:48:25: note: hidden overloaded virtual function 'BVH_PrimitiveSet<double, 3>::Box' declared here: different number of parameters (0 vs 1)
    Mantis 0027634
*/
#endif

//! This class implements a primitive set that will be used for BVH build
//! or sorting using radix sort.
class GeometryCache : public BVH_PrimitiveSet<Standard_Real, 3>
{
public:

  //! Initializes indexes array with indexes from 0 to theShapes.Size() - 1
  Standard_EXPORT GeometryCache (const TopTools_IndexedMapOfShape& theShapes);

  Standard_EXPORT virtual ~GeometryCache() {};

  //! Returns the length of set of sensitives
  Standard_EXPORT virtual Standard_Integer Size() const Standard_OVERRIDE;

  //! Returns bounding box of sensitive with index theIdx
  Standard_EXPORT virtual BVH_Box<Standard_Real, 3> Box (const Standard_Integer theIdx) const Standard_OVERRIDE;

  //! Returns center of sensitive with index theIdx in the set along the
  //! given axis theAxis
  Standard_EXPORT virtual Standard_Real Center (const Standard_Integer theIdx,
                                                const Standard_Integer theAxis) const Standard_OVERRIDE;

  //! Swaps items with indexes theIdx1 and theIdx2 in the set
  Standard_EXPORT virtual void Swap (const Standard_Integer theIdx1,
                                     const Standard_Integer theIdx2) Standard_OVERRIDE;

  //! Returns the edge and its index in map of shapes by the given index in BVH
  Standard_EXPORT void GetItemById (const Standard_Integer theItemIdx,
                                    TopoDS_Edge& theEdge,
                                    Standard_Integer& theIdxInMap) const;

  //! Returns the vertex and its index in map of shapes by the given index in BVH
  Standard_EXPORT void GetItemById (const Standard_Integer theItemIdx,
                                    TopoDS_Vertex& theVertex,
                                    Standard_Integer& theIdxInMap) const;

  //! Returns AABB of the whole primitive set (for radix sort)
  Standard_EXPORT BVH_Box<Standard_Real, 3> BoundingBox() const;

private:

  TopTools_IndexedMapOfShape           myShapes;
  NCollection_Array1<Standard_Integer> myIndexes; //!< An index array for BVH, since it uses indexation from 0 (unlike the TopTools_IndexedMapOfShape map)
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // _GeometryCache_Header
