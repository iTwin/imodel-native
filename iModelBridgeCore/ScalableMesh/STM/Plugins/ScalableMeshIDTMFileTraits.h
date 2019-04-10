/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshIDTMFileTraits.h $
|    $RCSfile: ScalableMeshIDTMFileTraits.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/08/10 15:10:27 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


//#include <ImagePP/all/h/IDTMFileDirectories/PointTypes.h>

#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct PointTypeCreatorTrait                                            { /* Default: Fail*/ };
template <> struct PointTypeCreatorTrait<DPoint3d>                      { typedef PointType3d64fCreator type; };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct MeshTypeCreatorTrait                                            { /* Default: Fail*/ };
template <> struct MeshTypeCreatorTrait<DPoint3d>                      { typedef MeshType3d64fCreator type; };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct LinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct LinearTypeCreatorTrait<DPoint3d>                  { typedef LinearTypeTi32Pi32Pq32Gi32_3d64fCreator type; };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct MeshAsLinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct MeshAsLinearTypeCreatorTrait<DPoint3d>                  { typedef MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct TINAsLinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct TINAsLinearTypeCreatorTrait<DPoint3d>                  { typedef TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };

END_BENTLEY_SCALABLEMESH_NAMESPACE
