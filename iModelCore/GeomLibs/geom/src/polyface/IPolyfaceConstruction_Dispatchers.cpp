/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#define DISPATCH_VOIDMETHOD(_class_,_publicName_,_sigArgs_,_callArgs_) \
void _class_::_publicName_ _sigArgs_ {_##_publicName_ _callArgs_;}

#define DISPATCH_METHOD(_returnType_,_class_,_publicName_,_sigArgs_,_callArgs_) \
_returnType_ _class_::_publicName_ _sigArgs_ {return _##_publicName_ _callArgs_;}


BEGIN_BENTLEY_GEOMETRY_NAMESPACE


// To be called by all internal EndFace sites until confirmed that they apply param options.
void IPolyfaceConstruction::EndFace_internal ()
    {
    EndFace ();
    }
DISPATCH_VOIDMETHOD(IPolyfaceConstruction, Clear, (), ())
DISPATCH_VOIDMETHOD(IPolyfaceConstruction, EndFace, (), ())
//DISPATCH_VOIDMETHOD(IPolyfaceConstruction, BuildParameterLocalToWorld, (DRange2dCR targetRange), (targetRange))
DISPATCH_VOIDMETHOD(IPolyfaceConstruction, CollectCurrentFaceRanges, (), ())

DISPATCH_METHOD(size_t, IPolyfaceConstruction, IncrementFaceIndex, (), ())
DISPATCH_METHOD(size_t, IPolyfaceConstruction, GetFaceIndex, () const, ())
DISPATCH_METHOD(FacetFaceData, IPolyfaceConstruction, GetFaceData, () const, ())
DISPATCH_VOIDMETHOD(IPolyfaceConstruction, SetFaceData, (FacetFaceDataCR data), (data))
DISPATCH_VOIDMETHOD(IPolyfaceConstruction, SetFaceIndex, (size_t index), (index))



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::FindOrAddPoint (DPoint3dCR point)
    {
    return _FindOrAddPoint (point);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::FindOrAddNormal (DVec3dCR normal)
    {
    return _FindOrAddNormal (normal);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::FindOrAddParam (DPoint2dCR param)
    {
    return _FindOrAddParam (param);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::FindOrAddIntColor (uint32_t color)
    {
    return _FindOrAddIntColor (color);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddPointIndex (size_t zeroBasedIndex, bool visible){return _AddPointIndex (zeroBasedIndex, visible);}
size_t IPolyfaceConstruction::AddNormalIndex (size_t zeroBasedIndex){return _AddNormalIndex (zeroBasedIndex);}
size_t IPolyfaceConstruction::AddParamIndex (size_t zeroBasedIndex){return _AddParamIndex (zeroBasedIndex);}
size_t IPolyfaceConstruction::AddColorIndex (size_t zeroBasedIndex){return _AddColorIndex (zeroBasedIndex);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddSignedOneBasedPointIndex (int oneBasedIndex) {return _AddSignedOneBasedPointIndex (oneBasedIndex);}
size_t IPolyfaceConstruction::AddSignedOneBasedNormalIndex (int oneBasedIndex){return _AddSignedOneBasedNormalIndex (oneBasedIndex);}
size_t IPolyfaceConstruction::AddSignedOneBasedParamIndex (int oneBasedIndex) {return _AddSignedOneBasedParamIndex (oneBasedIndex);}
size_t IPolyfaceConstruction::AddSignedOneBasedColorIndex (int oneBasedIndex) {return _AddSignedOneBasedColorIndex (oneBasedIndex);}



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddPointIndexTerminator ( )
    {
    return _AddPointIndexTerminator ( );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddNormalIndexTerminator ( )
    {
    return _AddNormalIndexTerminator ( );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddParamIndexTerminator ()                        {return _AddParamIndexTerminator ( );}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddColorIndexTerminator ()                        {return _AddColorIndexTerminator ( );}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderR IPolyfaceConstruction::GetClientMeshR ()                        {return _GetClientMeshR ( );}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PolyfaceHeaderPtr IPolyfaceConstruction::GetClientMeshPtr ()                    {return _GetClientMeshPtr ( );}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IFacetOptionsR IPolyfaceConstruction::GetFacetOptionsR ()                       {return _GetFacetOptionsR ( );}



#ifdef CompileParamRemap

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::RemapCurrentFaceParamsToRange (DRange2dCR targetRange)
    {
    return _RemapCurrentFaceParamsToRange (targetRange);
    }
#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddPointIndexTriangle (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2)
    {
    return _AddPointIndexTriangle (index0, visible0, index1, visible1, index2, visible2);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddNormalIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    return _AddNormalIndexTriangle (index0, index1, index2);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddParamIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    return _AddParamIndexTriangle (index0, index1, index2);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddColorIndexTriangle (size_t index0, size_t index1, size_t index2)
    {
    return _AddColorIndexTriangle (index0, index1, index2);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddPointIndexQuad (size_t index0, bool visible0, size_t index1, bool visible1, size_t index2, bool visible2, size_t index3, bool visible3)
    {
    return _AddPointIndexQuad (index0, visible0, index1, visible1, index2, visible2, index3, visible3);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddNormalIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    return _AddNormalIndexQuad (index0, index1, index2, index3);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddParamIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    return _AddParamIndexQuad (index0, index1, index2, index3);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t IPolyfaceConstruction::AddColorIndexQuad (size_t index0, size_t index1, size_t index2, size_t index3)
    {
    return _AddColorIndexQuad (index0, index1, index2, index3);
    }
                     
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::AddPolyface (PolyfaceQueryCR polyface, size_t drawMethodIndex) { return _AddPolyface (polyface, drawMethodIndex); }



END_BENTLEY_GEOMETRY_NAMESPACE