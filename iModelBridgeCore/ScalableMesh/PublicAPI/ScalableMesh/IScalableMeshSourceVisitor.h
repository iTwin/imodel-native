/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceVisitor.h $
|    $RCSfile: IScalableMeshSourceVisitor.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/10/26 17:55:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct                              IDTMSource;
struct                              IDTMLocalFileSource;
struct                              IDTMDgnModelSource;
struct                              IDTMDgnLevelSource;
struct                              IDTMDgnReferenceSource;
struct                              IDTMDgnReferenceLevelSource;
struct                              IDTMSourceGroup;


struct IDTMSourceVisitor
    {
    virtual                         ~IDTMSourceVisitor             () = 0 {}

    //virtual void                    _Visit                         (const IDTMSource&                   source) =0;
    virtual void                    _Visit                         (const IDTMLocalFileSource&          source) = 0;   
    virtual void                    _Visit                         (const IDTMDgnModelSource&               source) = 0;
    virtual void                    _Visit                         (const IDTMDgnLevelSource&               source) = 0;
    virtual void                    _Visit                         (const IDTMDgnReferenceSource&           source) = 0;
    virtual void                    _Visit                         (const IDTMDgnReferenceLevelSource&      source) = 0;
    virtual void                    _Visit                         (const IDTMSourceGroup&              source) = 0;
    };




END_BENTLEY_SCALABLEMESH_NAMESPACE
