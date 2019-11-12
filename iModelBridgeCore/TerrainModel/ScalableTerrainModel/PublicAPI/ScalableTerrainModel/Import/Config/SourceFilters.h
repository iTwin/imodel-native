/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>

#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceFiltersConfig : public ImportConfigComponentMixinBase<SourceFiltersConfig>
    {
private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_implP;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    SourceFiltersConfig            ();
    IMPORT_DLLE explicit                    SourceFiltersConfig            (const CustomFilterFactory&              filterFactory);

    IMPORT_DLLE virtual                     ~SourceFiltersConfig           ();

    IMPORT_DLLE                             SourceFiltersConfig            (const SourceFiltersConfig&              rhs);

    IMPORT_DLLE void                        push_back                      (const CustomFilterFactory&              filterFactory);

    const CustomFilteringSequence&          GetSequence                    () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE