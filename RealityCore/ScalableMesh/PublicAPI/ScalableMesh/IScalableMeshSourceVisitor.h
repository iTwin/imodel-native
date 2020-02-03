/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#ifdef VANCOUVER_API
struct                              IDTMDgnTerrainModelSource;
#endif

struct IDTMSourceVisitor
    {
    virtual                         ~IDTMSourceVisitor             () {}

    //virtual void                    _Visit                         (const IDTMSource&                   source) =0;
    virtual void                    _Visit                         (const IDTMLocalFileSource&          source) = 0;   
    virtual void                    _Visit                         (const IDTMDgnModelSource&               source) = 0;
    virtual void                    _Visit                         (const IDTMDgnLevelSource&               source) = 0;
    virtual void                    _Visit                         (const IDTMDgnReferenceSource&           source) = 0;
    virtual void                    _Visit                         (const IDTMDgnReferenceLevelSource&      source) = 0;
    virtual void                    _Visit                         (const IDTMSourceGroup&              source) = 0;
#ifdef VANCOUVER_API
    virtual void                    _Visit                         (const IDTMDgnTerrainModelSource&               source) = 0;
#endif
    };




END_BENTLEY_SCALABLEMESH_NAMESPACE
