//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DQuadrilaterMesh.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Mesh to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterMesh<ATTRIBUTE>::HVE2DQuadrilaterMesh(const HVE2DQuadrilaterMesh<ATTRIBUTE>& pi_rObj)
    : HVE2DMesh<ATTRIBUTE>(pi_rObj)
    {
    }



/**----------------------------------------------------------------------------
 PRIVATE
 DefaultConstructor for this class
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterMesh<ATTRIBUTE>::HVE2DQuadrilaterMesh()
    {
    }


/**----------------------------------------------------------------------------
 Constructor for this class
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterMesh<ATTRIBUTE>::HVE2DQuadrilaterMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DMesh<ATTRIBUTE>(pi_rpCoordSys)
    {
    }


/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterMesh<ATTRIBUTE>::~HVE2DQuadrilaterMesh()
    {
    }


/**----------------------------------------------------------------------------
 Assignement operator

 @param pi_rObj Mesh to copy from.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterMesh<ATTRIBUTE>& HVE2DQuadrilaterMesh<ATTRIBUTE>::operator=(const HVE2DQuadrilaterMesh<ATTRIBUTE>& pi_rObj)
    {
    HVE2DMesh<ATTRIBUTE>::operator=(pi_rObj);
    }



/**----------------------------------------------------------------------------
 This method adds a facet to the mesh.

 @param pi_rFacet Constant reference to facet to add to mesh. The facet
                  is duplicated. The new facet added must be located outside
                  all other facets located in the mesh, though these facets
                  can be contiguous by many points and to many facets at
                  the same time.

 @return true if the facet could be added and false otherwise.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DQuadrilaterMesh<ATTRIBUTE>::AddFacet(const HVE2DQuadrilaterFacet<ATTRIBUTE>& pi_rFacet)
    {
    // Now that the facet type is validated (by compiler) we ask ancester to process
    return(HVE2DMesh<ATTRIBUTE>::AddFacet(pi_rFacet));
    }

END_IMAGEPP_NAMESPACE
