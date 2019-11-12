/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct AttachmentsConfig : public ImportConfigComponentMixinBase<AttachmentsConfig>
    {
private:
    bool                                    m_imported;
    void*                                   m_implP; // Reserve space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    AttachmentsConfig              (bool                            imported);
    IMPORT_DLLE virtual                     ~AttachmentsConfig             ();

    IMPORT_DLLE                             AttachmentsConfig              (const AttachmentsConfig&        rhs);


    bool                                    AreImported                    () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE