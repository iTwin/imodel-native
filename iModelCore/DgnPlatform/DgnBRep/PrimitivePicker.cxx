#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/OCBRep.h>

//#include <BRepBuilderAPI_MakeEdge.hxx>
//#include <BRepExtrema_DistShapeShape.hxx>
//#include <BRep_Tool.hxx>
//#include <BVH_RadixSorter.hxx>
//#include <gp_Lin.hxx>
//#include <OSD_Parallel.hxx>
//#include <PrimitivePicker.hxx>
//#include <TopoDS.hxx>
//#include <TopExp.hxx>

IMPLEMENT_STANDARD_RTTIEXT (PrimitivePicker, Standard_Transient)

namespace PickerTools
{
  //! Returns true if the given AABB is overlapped by the cylinder.
  //! To speed up the test, the following approximation is used:
  //! the AABB is extended to the size of cylinder's radius and
  //! the algo searches for overlap between cylinder's direction line
  //! and the extended AABB. Therefore, it is not a precise test!
  Standard_Boolean IsOverlapped (const PrimitivePicker_Vec3& theMinPnt,
                                 const PrimitivePicker_Vec3& theMaxPnt,
                                 const Cylinder& theCyl)
  {
    PrimitivePicker_Vec3 aTime0 = PrimitivePicker_Vec3 (
      (theMinPnt.x() - theCyl.Radius() - theCyl.Origin().X()) * theCyl.InvDir().X(),
      (theMinPnt.y() - theCyl.Radius() - theCyl.Origin().Y()) * theCyl.InvDir().Y(),
      (theMinPnt.z() - theCyl.Radius() - theCyl.Origin().Z()) * theCyl.InvDir().Z());

    PrimitivePicker_Vec3 aTime1 = PrimitivePicker_Vec3 (
      (theMaxPnt.x() + theCyl.Radius() - theCyl.Origin().X()) * theCyl.InvDir().X(),
      (theMaxPnt.y() + theCyl.Radius() - theCyl.Origin().Y()) * theCyl.InvDir().Y(),
      (theMaxPnt.z() + theCyl.Radius() - theCyl.Origin().Z()) * theCyl.InvDir().Z());

    PrimitivePicker_Vec3 aTimeMin = aTime0.cwiseMin (aTime1);
    PrimitivePicker_Vec3 aTimeMax = aTime0.cwiseMax (aTime1);

    Standard_Real aTimeOut = Min (aTimeMax.x(), Min (aTimeMax.y(), aTimeMax.z()));
    Standard_Real aTimeIn  = Max (aTimeMin.x(), Max (aTimeMin.y(), aTimeMin.z()));

    return aTimeIn < aTimeOut;
  }
}

//! An API class to perform parallelized exchaustive search for vertices.
class BruteForceVertexPicker
{
public:

  BruteForceVertexPicker (const TopTools_IndexedMapOfShape& theVertices,
                          const gp_Lin& theRay,
                          const Standard_Real theSqRadius,
                          NCollection_Array1<Standard_Character>& theResult)
    : myVerts (theVertices),
      myCylAxis (theRay),
      myCylSqRadius (theSqRadius),
      myResult (theResult) {}

  //! Implements a precise test to detect overlap between the cylinder and point
  //! with index theThreadId in the map and stores the result
  void operator() (const Standard_Integer theThreadId) const
  {
    Standard_Real aSqDist = myCylAxis.SquareDistance (BRep_Tool::Pnt (TopoDS::Vertex (myVerts.FindKey (theThreadId))));
    if (aSqDist < myCylSqRadius)
    {
      myResult.SetValue (theThreadId, 1);
    }
  }

private:

  //! Assignment operator.
  void operator =(const BruteForceVertexPicker& /*theOther*/)
  {
  }

private:

  TopTools_IndexedMapOfShape              myVerts;
  gp_Lin                                  myCylAxis;
  Standard_Real                           myCylSqRadius;
  NCollection_Array1<Standard_Character>& myResult;
};

//! An API class to perform parallelized exchaustive search for edges.
class BruteForceEdgePicker
{
public:

  BruteForceEdgePicker (const TopTools_IndexedMapOfShape& theEdges,
                        const TopoDS_Edge& theAxis,
                        const Standard_Real theRadius,
                        NCollection_Array1<Standard_Character>& theResult,
                        NCollection_Array1<Standard_Real>&      theCurveParams)
    : myEdges (theEdges),
      myCylAxis (theAxis),
      myCylRadius (theRadius),
      myResult (theResult),
      myCurveParams (theCurveParams) {}

  //! Implements a precise test to detect overlap between the cylinder and edge
  //! with index theThreadId in the map and stores the result
  void operator() (const Standard_Integer theThreadId) const
  {
    const TopoDS_Edge anEdge = TopoDS::Edge (myEdges.FindKey (theThreadId));
    BRepExtrema_DistShapeShape anIntersector (myCylAxis, anEdge, Extrema_ExtFlag_MIN);
    if (anIntersector.IsDone())
    {
      if (anIntersector.Value() < myCylRadius)
      {
        myResult.SetValue (theThreadId, 1);
        TopoDS_Shape aDetectedShape = anIntersector.SupportOnShape2 (1);
        if (aDetectedShape.ShapeType() == TopAbs_VERTEX)
        {
          myCurveParams.SetValue (theThreadId, BRep_Tool::Parameter (TopoDS::Vertex (aDetectedShape), anEdge));
        }
        else
        {
          Standard_Real aParam = DBL_MAX;
          anIntersector.ParOnEdgeS2 (1, aParam);
          myCurveParams.SetValue (theThreadId, aParam);
        }
        return;
      }
    }
  }

private:

  //! Assignment operator.
  void operator =(const BruteForceEdgePicker& /*theOther*/)
  {
  }

private:

  TopTools_IndexedMapOfShape              myEdges;
  TopoDS_Edge                             myCylAxis;
  Standard_Real                           myCylRadius;
  NCollection_Array1<Standard_Character>& myResult;
  NCollection_Array1<Standard_Real>&      myCurveParams;
};

//! An API class to perform parallelized testing for overlap between
//! the patch of vertices and the cylinder.
class ParallelVertexPicker
{
public:

  ParallelVertexPicker (const NCollection_Handle<GeometryCache>& theVertCache,
                        const Standard_Integer thePatchSize,
                        const Cylinder& theCylinder,
                        NCollection_Array1<Standard_Character>& theResult)
    : myVertsCache (theVertCache.get()),
      myPatchSize (thePatchSize),
      myCylinder (theCylinder),
      myResult (theResult) {}

  //! Handles the patch with index theThreadId.
  //! Combines AABB of the all vertices in patch, then performs a slab
  //! test to check if there may be a vertex that is overlapped by the
  //! cylinder. If the test is true, iterates through all vertices of
  //! the patch and performs precise overlap test.
  void operator() (const Standard_Integer theThreadId) const
  {
    const Standard_Integer aLower = theThreadId * myPatchSize;
    const Standard_Integer anUpper = Min (myVertsCache->Size(), myPatchSize * (theThreadId + 1));
    BVH_Box<Standard_Real, 3> aPatchBnd;
    for (Standard_Integer aPointIdx = aLower; aPointIdx < anUpper; ++ aPointIdx)
    {
      Standard_Integer aIdxInMap;
      TopoDS_Vertex aVertex;
      myVertsCache->GetItemById (aPointIdx, aVertex, aIdxInMap);
      const gp_Pnt aVert = BRep_Tool::Pnt (aVertex);
      aPatchBnd.Add (PrimitivePicker_Vec3 (aVert.X(), aVert.Y(), aVert.Z()));
    }

    gp_Lin aRay (myCylinder.Origin(), myCylinder.Dir());
    if (PickerTools::IsOverlapped (aPatchBnd.CornerMin(), aPatchBnd.CornerMax(), myCylinder))
    {
      for (Standard_Integer aPointIdx = aLower; aPointIdx < anUpper; ++ aPointIdx)
      {
        TopoDS_Vertex aVert;
        Standard_Integer aIdxInMap;
        myVertsCache->GetItemById (aPointIdx, aVert, aIdxInMap);
        Standard_Real aSqDist = aRay.SquareDistance (BRep_Tool::Pnt (aVert));
        if (aSqDist < myCylinder.SquareRadius())
        {
          myResult.SetValue (aIdxInMap, 1);
        }
      }
    }
  }

private:

  //! Assignment operator.
  void operator =(const ParallelVertexPicker& /*theOther*/)
  {
  }

private:

  const GeometryCache*                    myVertsCache;
  Standard_Integer                        myPatchSize;
  Cylinder                                myCylinder;
  NCollection_Array1<Standard_Character>& myResult;
};

//! An API class to perform parallelized traverse of BVH for edges.
//! During BVH traverse, the precise overlap detection tests are not
//! performed due to performance of BRepExtrema_DistShapeShape. To
//! boost the routine, each precise detection test will be performed
//! right after BVH traverse in parallel mode.
class ParallelEdgePicker
{
public:

  ParallelEdgePicker (const NCollection_Handle<GeometryCache>&   theCache,
                      const NCollection_Vector<Standard_Integer> theEdgesToCheck,
                      const TopoDS_Edge&                         theEdge,
                      const Standard_Real                        theRadius,
                      NCollection_Array1<Standard_Character>&    thePickedEdges,
                      NCollection_Array1<Standard_Real>&         theCurveParams)
  : myCache        (theCache.get()),
    myEdgesToCheck (theEdgesToCheck),
    myCylAxis      (theEdge),
    myCylRadius    (theRadius),
    myPickedEdges  (thePickedEdges),
    myCurveParams  (theCurveParams)
{}

  //! Performs a precise check for overlap between the edge with index theEdgeIdx
  //! in BVH and the cylinder.
  void operator() (const Standard_Integer theThreadId) const
  {
    TopoDS_Edge anEdge;
    Standard_Integer aMapIdx;
    const Standard_Integer aIdxToCheck = myEdgesToCheck.Value (theThreadId);
    myCache->GetItemById (aIdxToCheck, anEdge, aMapIdx);
    BRepExtrema_DistShapeShape anIntersector (myCylAxis, anEdge);
    if (anIntersector.IsDone())
    {
      if (anIntersector.Value() < myCylRadius)
      {
        myPickedEdges.SetValue (aMapIdx, 1);
        TopoDS_Shape aDetectedShape = anIntersector.SupportOnShape2 (1);
        if (aDetectedShape.ShapeType() == TopAbs_VERTEX)
        {
          myCurveParams.SetValue (aMapIdx, BRep_Tool::Parameter (TopoDS::Vertex (aDetectedShape), anEdge));
        }
        else
        {
          Standard_Real aParam = DBL_MAX;
          anIntersector.ParOnEdgeS2 (1, aParam);
          myCurveParams.SetValue (aMapIdx, aParam);
        }
      }
    }
  }

private:

  //! Assignment operator.
  void operator = (const ParallelEdgePicker& /*theOther*/) {};

private:

  const GeometryCache*                    myCache;
  NCollection_Vector<Standard_Integer>    myEdgesToCheck;
  TopoDS_Edge                             myCylAxis;
  Standard_Real                           myCylRadius;
  NCollection_Array1<Standard_Character>& myPickedEdges;
  NCollection_Array1<Standard_Real>&      myCurveParams;
};


//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
PrimitivePicker::PrimitivePicker (const TopTools_IndexedMapOfShape& theVerts,
                                  const TopTools_IndexedMapOfShape& theEdges,
                                  const PrimitivePicker_TypeOfAlgo theType)
  : myPatchSize  (512),
    myIsSorted   (Standard_False),
    myTypeOfAlgo (theType)
{
  if (myTypeOfAlgo == PrimitivePicker_CACHE || myTypeOfAlgo == PrimitivePicker_PCACHE)
  {
    myCaches[PrimitivePicker_EDGE] = new GeometryCache (theEdges);
    myCaches[PrimitivePicker_VERTICE] = new GeometryCache (theVerts);
  }
  else
  {
    myVertices = theVerts,
    myEdges = theEdges;
  }
}

//=======================================================================
//function : Destructor
//purpose  :
//=======================================================================
PrimitivePicker::~PrimitivePicker()
{
  myVertices.Clear();
  myEdges.Clear();
}

//=======================================================================
//function : SetPatchSize
//purpose  :
//=======================================================================
void PrimitivePicker::SetPatchSize (const Standard_Integer theSize)
{
  if (theSize <= 0)
    return;

  myPatchSize = theSize;
}

#include <Geom_Line.hxx>
//=======================================================================
//function : traverseEdges
//purpose  :
//=======================================================================
void PrimitivePicker::traverseEdges (NCollection_Array1<Standard_Character>& thePickedEdges,
                                     NCollection_Array1<Standard_Real>&      theCurveParams)
{
  TopoDS_Edge aCylinderAxis = BRepBuilderAPI_MakeEdge (gp_Lin (myCylinder.Origin(), myCylinder.Dir()));
  NCollection_Vector<Standard_Integer> anIndexesToCheck;
  const NCollection_Handle<BVH_Tree<Standard_Real, 3> >& aBVH =
    myCaches[PrimitivePicker_EDGE]->BVH();

  if (myCaches[PrimitivePicker_EDGE]->Size() < 1 ||
    !PickerTools::IsOverlapped (aBVH->MinPoint (0), aBVH->MaxPoint (0), myCylinder))
  {
    return;
  }

  Standard_Integer aStack[32];
  Standard_Integer aNode =  0;
  Standard_Integer aHead = -1;
  Standard_Real aParam = DBL_MAX;

  for (;;)
  {
    const BVH_Vec4i& aData = aBVH->NodeInfoBuffer()[aNode];

    if (aData.x() == 0)
    {
      const Standard_Integer aLftIdx = aData.y();
      const Standard_Integer aRghIdx = aData.z();

      Standard_Boolean toCheckLft =
        PickerTools::IsOverlapped (aBVH->MinPoint (aLftIdx), aBVH->MaxPoint (aLftIdx), myCylinder);

      Standard_Boolean toCheckRgh =
        PickerTools::IsOverlapped (aBVH->MinPoint (aRghIdx), aBVH->MaxPoint (aRghIdx), myCylinder);

      if (toCheckLft || toCheckRgh)
      {
        aNode = toCheckLft ? aLftIdx : aRghIdx;

        if (toCheckLft && toCheckRgh)
        {
          aStack[++aHead] = aRghIdx;
        }
      }
      else
      {
        if (aHead < 0)
          break;

        aNode = aStack[aHead--];
      }
    }
    else
    {
      for (Standard_Integer anElemIdx = aData.y(); anElemIdx <= aData.z(); ++anElemIdx)
      {
        if (myTypeOfAlgo == PrimitivePicker_CACHE)
        {
          TopoDS_Edge anEdge;
          Standard_Integer aMapIdx;
          myCaches[PrimitivePicker_EDGE]->GetItemById (anElemIdx, anEdge, aMapIdx);
          BRepExtrema_DistShapeShape anIntersector (aCylinderAxis, anEdge, Extrema_ExtFlag_MIN);
          if (anIntersector.IsDone())
          {
            if (anIntersector.Value() < myCylinder.Radius())
            {
              thePickedEdges.SetValue (aMapIdx, 1);
              TopoDS_Shape aDetectedShape = anIntersector.SupportOnShape2 (1);
              if (aDetectedShape.ShapeType() == TopAbs_VERTEX)
              {
                theCurveParams.SetValue (aMapIdx, BRep_Tool::Parameter (TopoDS::Vertex (aDetectedShape), anEdge));
              }
              else
              {
                aParam = DBL_MAX;
                anIntersector.ParOnEdgeS2 (1, aParam);
                theCurveParams.SetValue (aMapIdx, aParam);
              }
            }
          }
        }
        else
        {
          anIndexesToCheck.Append (anElemIdx);
        }
      }

      if (aHead < 0)
        break;

      aNode = aStack[aHead--];
    }
  }

  if (myTypeOfAlgo == PrimitivePicker_PCACHE)
  {
    OSD_Parallel::For (0, anIndexesToCheck.Size(), ParallelEdgePicker (
                                                     myCaches[PrimitivePicker_EDGE],
                                                     anIndexesToCheck,
                                                     aCylinderAxis,
                                                     myCylinder.Radius(),
                                                     thePickedEdges,
                                                     theCurveParams));
  }
}

//=======================================================================
//function : traverseVertices
//purpose  :
//=======================================================================
void PrimitivePicker::traverseVertices (NCollection_Array1<Standard_Character>& thePickedVerts)
{
  if (!myIsSorted)
  {
    BVH_RadixSorter<Standard_Real, 3> aSorter (myCaches[PrimitivePicker_VERTICE]->BoundingBox());
    aSorter.Perform (myCaches[PrimitivePicker_VERTICE].get());
    myIsSorted = Standard_True;
  }

  const gp_Lin aCylinderAxis (myCylinder.Origin(), myCylinder.Dir());
  const Standard_Integer aNbPatches =
    (Standard_Integer)Ceiling (myCaches[PrimitivePicker_VERTICE]->Size() / (Standard_Real)myPatchSize);
  OSD_Parallel::For (0, aNbPatches, ParallelVertexPicker (myCaches[PrimitivePicker_VERTICE], myPatchSize, myCylinder, thePickedVerts));
}

//=======================================================================
//function : PickEdges
//purpose  :
//=======================================================================
void PrimitivePicker::PickEdges (const gp_Pnt& theRayOrigin,
                                 const gp_Dir& theRayDir,
                                 const Standard_Real theRadius,
                                 NCollection_Array1<Standard_Character>& thePickedEdges,
                                 NCollection_Array1<Standard_Real>&      theCurveParams)
{
  const Standard_Integer aNbEdges = myEdges.IsEmpty() ?
                                    myCaches[PrimitivePicker_EDGE]->Size() : myEdges.Size();
  if (thePickedEdges.Size() != aNbEdges || theCurveParams.Size() != aNbEdges)
    return;

  for (Standard_Integer anEdgeIdx = 1; anEdgeIdx <= aNbEdges; ++anEdgeIdx)
  {
    thePickedEdges.ChangeValue (anEdgeIdx) = 0;
    theCurveParams.ChangeValue (anEdgeIdx) = 0.0;
  }

  if (myEdges.IsEmpty())
  {
    myCylinder = Cylinder (theRayOrigin, theRayDir, theRadius);

    traverseEdges (thePickedEdges, theCurveParams);
  }
  else
  {
    const TopoDS_Edge aCylinderAxis = BRepBuilderAPI_MakeEdge (gp_Lin (theRayOrigin, theRayDir));
    if (myTypeOfAlgo == PrimitivePicker_BRUTE_FORCE)
    {
      OSD_Parallel::For (1, myEdges.Extent() + 1, BruteForceEdgePicker (myEdges, aCylinderAxis, theRadius, thePickedEdges, theCurveParams));
    }
    else
    {
      Standard_Real aParam = DBL_MAX;
      for (Standard_Integer aIdx = 1; aIdx <= myEdges.Extent(); ++aIdx)
      {
        const TopoDS_Edge anEdge = TopoDS::Edge (myEdges.FindKey (aIdx));
        BRepExtrema_DistShapeShape anIntersector (aCylinderAxis, anEdge, Extrema_ExtFlag_MIN);
        if (anIntersector.IsDone())
        {
          if (anIntersector.Value() < theRadius)
          {
            thePickedEdges.SetValue (aIdx, 1);
            TopoDS_Shape aDetectedShape = anIntersector.SupportOnShape2 (1);
            if (aDetectedShape.ShapeType() == TopAbs_VERTEX)
            {
              theCurveParams.SetValue (aIdx, BRep_Tool::Parameter (TopoDS::Vertex (aDetectedShape), anEdge));
            }
            else
            {
              aParam = DBL_MAX;
              anIntersector.ParOnEdgeS2 (1, aParam);
              theCurveParams.SetValue (aIdx, aParam);
            }
          }
        }
      }
    }
  }
}

//=======================================================================
//function : PickVertices
//purpose  :
//=======================================================================
void PrimitivePicker::PickVertices (const gp_Pnt& theRayOrigin,
                                    const gp_Dir& theRayDir,
                                    const Standard_Real theRadius,
                                    NCollection_Array1<Standard_Character>& thePickedVerts)
{
  const Standard_Integer aNbVerts = myVertices.IsEmpty() ?
                                    myCaches[PrimitivePicker_VERTICE]->Size() : myVertices.Size();
  if (thePickedVerts.Size() != aNbVerts)
    return;

  for (Standard_Integer aVertIdx = 1; aVertIdx <= aNbVerts; ++aVertIdx)
  {
    thePickedVerts.ChangeValue (aVertIdx) = 0;
  }

  if (myVertices.IsEmpty())
  {
    myCylinder = Cylinder (theRayOrigin, theRayDir, theRadius);

    traverseVertices (thePickedVerts);
  }
  else
  {
    gp_Lin aCylinderAxis (theRayOrigin, theRayDir);
    Standard_Real aSqRadius = theRadius * theRadius;
    if (myTypeOfAlgo == PrimitivePicker_BRUTE_FORCE)
    {
      OSD_Parallel::For (1, myVertices.Extent() + 1, BruteForceVertexPicker (myVertices, aCylinderAxis, aSqRadius, thePickedVerts));
    }
    else
    {
      for (Standard_Integer aIdx = 1; aIdx <= myVertices.Extent(); ++aIdx)
      {
        Standard_Real aSqDist = aCylinderAxis.SquareDistance (BRep_Tool::Pnt (TopoDS::Vertex (myVertices.FindKey (aIdx))));
        if (aSqDist < aSqRadius)
        {
          thePickedVerts.SetValue (aIdx, 1);
        }
      }
    }
  }
}
