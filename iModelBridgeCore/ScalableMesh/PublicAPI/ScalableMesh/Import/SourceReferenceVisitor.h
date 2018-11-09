/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/SourceReferenceVisitor.h $
|    $RCSfile: SourceReferenceVisitor.h,v $
|   $Revision: 1.11 $
|       $Date: 2011/10/21 17:32:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct SourceRefBase;
struct LocalFileSourceRef;
struct DGNElementSourceRef;
struct DGNLevelByIDSourceRef;
struct DGNReferenceLevelByIDSourceRef;
struct DGNLevelByNameSourceRef;
typedef DGNLevelByNameSourceRef DGNLevelByNameSourceRef;
struct DGNReferenceLevelByNameSourceRef;


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
