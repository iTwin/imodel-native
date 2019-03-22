/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Content/ScalableMeshConfig.h $
|    $RCSfile: ScalableMeshConfig.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/22 21:58:02 $
|     $Author: Thomas.Butzbach $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ScalableMeshConfigImpl;


struct ScalableMeshConfig //: public ContentConfigComponentMixinBase<TypeConfig>
    {

    private:
        RefCountedPtr<ScalableMeshConfigImpl>
            m_pImpl;
    public:
        //    IMPORT_DLLE static ClassID          s_GetClassID                       ();
        IMPORT_DLLE explicit                ScalableMeshConfig();
        IMPORT_DLLE explicit                ScalableMeshConfig(const ScalableMeshData&             type);
        IMPORT_DLLE virtual                 ~ScalableMeshConfig();

        IMPORT_DLLE                         ScalableMeshConfig(const ScalableMeshConfig&           rhs);

        const ScalableMeshData&                     GetScalableMeshData() const;
        bool                                        IsSet() const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
