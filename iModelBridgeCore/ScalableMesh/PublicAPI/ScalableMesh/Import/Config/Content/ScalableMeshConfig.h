/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
