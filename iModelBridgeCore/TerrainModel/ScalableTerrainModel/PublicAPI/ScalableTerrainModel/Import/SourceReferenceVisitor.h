/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/SourceReferenceVisitor.h $
|    $RCSfile: SourceReferenceVisitor.h,v $
|   $Revision: 1.11 $
|       $Date: 2011/10/21 17:32:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct SourceRefBase;
struct LocalFileSourceRef;
struct DGNElementSourceRef;
struct DGNLevelByIDSourceRef;
struct DGNReferenceLevelByIDSourceRef;
struct DGNLevelByNameSourceRef;
typedef DGNLevelByNameSourceRef DGNLevelByNameSourceRef;
struct DGNReferenceLevelByNameSourceRef;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ISourceRefVisitor
    {
    virtual                         ~ISourceRefVisitor     () = 0 {}

    virtual void                    _Visit2                 (const SourceRefBase&                    sourceRef) { /* Do nothing*/ }

    virtual void                    _Visit                 (const LocalFileSourceRef&               sourceRef) = 0;
    virtual void                    _Visit                 (const DGNElementSourceRef&              sourceRef) = 0;
    virtual void                    _Visit                 (const DGNLevelByIDSourceRef&            sourceRef) = 0;
    virtual void                    _Visit                 (const DGNReferenceLevelByIDSourceRef&   sourceRef) = 0;
    virtual void                    _Visit                 (const DGNLevelByNameSourceRef&          sourceRef) = 0;
    virtual void                    _Visit                 (const DGNReferenceLevelByNameSourceRef& sourceRef) = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  Special source ref visitor that implement default behavior calling
*               automatically SourceRef visit overload.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceRefVisitor : public ISourceRefVisitor
    {
    virtual                         ~SourceRefVisitor      () = 0 {}

    virtual void                    _Visit                 (const LocalFileSourceRef&               sourceRef) override { /* Do nothing*/ }
    virtual void                    _Visit                 (const DGNElementSourceRef&              sourceRef) override { /* Do nothing*/ }
    virtual void                    _Visit                 (const DGNLevelByIDSourceRef&            sourceRef) override { /* Do nothing*/ }
    virtual void                    _Visit                 (const DGNReferenceLevelByIDSourceRef&   sourceRef) override { /* Do nothing*/ }
    virtual void                    _Visit                 (const DGNLevelByNameSourceRef&          sourceRef) override { /* Do nothing*/ }
    virtual void                    _Visit                 (const DGNReferenceLevelByNameSourceRef& sourceRef) override { /* Do nothing*/ }
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE