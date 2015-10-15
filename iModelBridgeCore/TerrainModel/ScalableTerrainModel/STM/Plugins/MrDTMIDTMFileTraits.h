/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/Plugins/MrDTMIDTMFileTraits.h $
|    $RCSfile: MrDTMIDTMFileTraits.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/08/10 15:10:27 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <STMInternal/Storage/IDTMFileDirectories/PointTypes.h>

#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Type/IMrDTMLinear.h>
#include <ScalableTerrainModel/Type/IMrDTMMesh.h>
#include <ScalableTerrainModel/Type/IMrDTMTIN.h>


BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct PointTypeCreatorTrait                                            { /* Default: Fail*/ };
template <> struct PointTypeCreatorTrait<IDTMFile::Point3d64f>          { typedef PointType3d64fCreator type; };
template <> struct PointTypeCreatorTrait<DPoint3d>                      { typedef PointType3d64fCreator type; };
template <> struct PointTypeCreatorTrait<IDTMFile::Point3d64fM64f>      { typedef PointType3d64fM64fCreator type; };
template <> struct PointTypeCreatorTrait<IDTMFile::Point3d64fG32>       { typedef PointType3d64fG32Creator type; };
template <> struct PointTypeCreatorTrait<IDTMFile::Point3d64fM64fG32>   { typedef PointType3d64fM64fG32Creator type; };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct LinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct LinearTypeCreatorTrait<IDTMFile::Point3d64f>      { typedef LinearTypeTi32Pi32Pq32Gi32_3d64fCreator type; };
template <> 
struct LinearTypeCreatorTrait<DPoint3d>                  { typedef LinearTypeTi32Pi32Pq32Gi32_3d64fCreator type; };
template <> 
struct LinearTypeCreatorTrait<IDTMFile::Point3d64fM64f>  { typedef LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator type; };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct MeshAsLinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct MeshAsLinearTypeCreatorTrait<IDTMFile::Point3d64f>      { typedef MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };
template <> 
struct MeshAsLinearTypeCreatorTrait<DPoint3d>                  { typedef MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointT> 
struct TINAsLinearTypeCreatorTrait                            { /* Default: Fail*/ };
template <> 
struct TINAsLinearTypeCreatorTrait<IDTMFile::Point3d64f>      { typedef TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };
template <> 
struct TINAsLinearTypeCreatorTrait<DPoint3d>                  { typedef TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator type; };

END_BENTLEY_MRDTM_NAMESPACE