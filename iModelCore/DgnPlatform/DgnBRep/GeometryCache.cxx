#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/OCBRep.h>

//#include <BRepBndLib.hxx>
//#include <BRep_Tool.hxx>
//#include <Bnd_Box.hxx>
//#include <BVH_LinearBuilder.hxx>
//#include <GeometryCache.hxx>
//#include <TopoDS.hxx>
//#include <TopoDS_Edge.hxx>
//#include <TopoDS_Vertex.hxx>

//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
GeometryCache::GeometryCache (const TopTools_IndexedMapOfShape& theShapes)
  : myShapes (theShapes),
    myIndexes (0, theShapes.Extent() - 1)
{
  for (Standard_Integer aIdx = 0; aIdx < myIndexes.Size(); ++aIdx)
  {
    myIndexes.SetValue (aIdx, aIdx + 1);
  }

  myBuilder = new BVH_LinearBuilder<Standard_Real, 3> (1, 32);
  MarkDirty();
}

//=======================================================================
//function : Size
//purpose  :
//=======================================================================
Standard_Integer GeometryCache::Size() const
{
  return myIndexes.Size();
}

//=======================================================================
//function : Box
//purpose  :
//=======================================================================
BVH_Box<Standard_Real, 3> GeometryCache::Box (const Standard_Integer theIdx) const
{
  const TopoDS_Shape aShape = myShapes.FindKey (myIndexes.Value (theIdx));
  TopAbs_ShapeEnum aShapeType = aShape.ShapeType();
  if (aShapeType == TopAbs_EDGE)
  {
    Bnd_Box aBndBox;
    BRepBndLib::Add (aShape, aBndBox);
    const gp_Pnt aBoxMin = aBndBox.CornerMin();
    const gp_Pnt aBoxMax = aBndBox.CornerMax();
    return BVH_Box<Standard_Real, 3> (NCollection_Vec3<Standard_Real> (aBoxMin.X(), aBoxMin.Y(), aBoxMin.Z()),
                                      NCollection_Vec3<Standard_Real> (aBoxMax.X(), aBoxMax.Y(), aBoxMax.Z()));
  }
  else if (aShapeType == TopAbs_VERTEX)
  {
    const gp_Pnt aVertex = BRep_Tool::Pnt (TopoDS::Vertex (aShape));

    return BVH_Box<Standard_Real, 3> (NCollection_Vec3<Standard_Real> (aVertex.X(), aVertex.Y(), aVertex.Z()));
  }
  else
  {
    return BVH_Box<Standard_Real, 3> (NCollection_Vec3<Standard_Real> (DBL_MAX, DBL_MAX, DBL_MAX));
  }
}

//=======================================================================
//function : Center
//purpose  :
//=======================================================================
Standard_Real GeometryCache::Center (const Standard_Integer theIdx,
                                     const Standard_Integer theAxis) const
{
  const TopoDS_Shape aShape = myShapes.FindKey (myIndexes.Value (theIdx));
  TopAbs_ShapeEnum aShapeType = aShape.ShapeType();
  if (aShapeType == TopAbs_EDGE)
  {
    BVH_Box<Standard_Real, 3> aBox = Box (theIdx);
    NCollection_Vec3<Standard_Real> aCenter = (aBox.CornerMax() + aBox.CornerMin()) * 0.5;

    return theAxis == 0 ? aCenter.x() : (theAxis == 1 ? aCenter.y() : aCenter.z());
  }
  else if (aShapeType == TopAbs_VERTEX)
  {
    const gp_Pnt aVertex = BRep_Tool::Pnt (TopoDS::Vertex (aShape));

    return theAxis == 0 ? aVertex.X() : (theAxis == 1 ? aVertex.Y() : aVertex.Z());
  }
  else
  {
    return DBL_MAX;
  }
}

//=======================================================================
//function : Swap
//purpose  :
//=======================================================================
void GeometryCache::Swap (const Standard_Integer theIdx1,
                              const Standard_Integer theIdx2)
{
  const Standard_Integer anEdgeIdx1 = myIndexes.Value (theIdx1);
  const Standard_Integer anEdgeIdx2 = myIndexes.Value (theIdx2);
  myIndexes.SetValue (theIdx1, anEdgeIdx2);
  myIndexes.SetValue (theIdx2, anEdgeIdx1);
}

//=======================================================================
//function : GetItemById
//purpose  :
//=======================================================================
void GeometryCache::GetItemById (const Standard_Integer theItemIdx,
                                 TopoDS_Edge& theEdge,
                                 Standard_Integer& theIdxInMap) const
{
  theIdxInMap = myIndexes.Value (theItemIdx);
  theEdge = TopoDS::Edge (myShapes.FindKey (theIdxInMap));
}

//=======================================================================
//function : GetItemById
//purpose  :
//=======================================================================
void GeometryCache::GetItemById (const Standard_Integer theItemIdx,
                                 TopoDS_Vertex& theVertex,
                                 Standard_Integer& theIdxInMap) const
{
  theIdxInMap = myIndexes.Value (theItemIdx);
  theVertex = TopoDS::Vertex (myShapes.FindKey (theIdxInMap));
}

//=======================================================================
//function : BoundingBox
//purpose  :
//=======================================================================
BVH_Box<Standard_Real, 3> GeometryCache::BoundingBox() const
{
  TopAbs_ShapeEnum aShapeType = myShapes.FindKey (1).ShapeType();
  BVH_Box<Standard_Real, 3> aBox;

  for (TopTools_IndexedMapOfShape::Iterator aShapeIter (myShapes); aShapeIter.More(); aShapeIter.Next())
  {
    if (aShapeType == TopAbs_EDGE)
    {
      Bnd_Box aBndBox;
      BRepBndLib::Add (aShapeIter.Value(), aBndBox);
      const gp_Pnt aBoxMin = aBndBox.CornerMin();
      const gp_Pnt aBoxMax = aBndBox.CornerMax();
      aBox.Combine (BVH_Box<Standard_Real, 3> (NCollection_Vec3<Standard_Real> (aBoxMin.X(), aBoxMin.Y(), aBoxMin.Z()),
                                               NCollection_Vec3<Standard_Real> (aBoxMax.X(), aBoxMax.Y(), aBoxMax.Z())));
    }
    else if (aShapeType == TopAbs_VERTEX)
    {
      const gp_Pnt aVertex = BRep_Tool::Pnt (TopoDS::Vertex (aShapeIter.Value()));

      aBox.Add (NCollection_Vec3<Standard_Real> (aVertex.X(), aVertex.Y(), aVertex.Z()));
    }
  }

  return aBox;
}
