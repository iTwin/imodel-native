/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/FilterFactory.h $
|    $RCSfile: FilterFactory.h,v $
|   $Revision: 1.10 $
|       $Date: 2011/09/07 14:20:49 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Filter.h>
#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
struct GCS;
struct Reprojection;
END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct TypeConversionFilterRegistry;
struct ReprojectionFilterRegistry;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


struct DataType;

struct TypeConversionFilterFactory;
struct ReprojectionFilterFactory;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct FilterFactory : private Unassignable
    {
private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_pImpl;

public:
    IMPORT_DLLE explicit                    FilterFactory              (Log&                                log = GetDefaultLog());

    IMPORT_DLLE explicit                    FilterFactory              (const TypeConversionFilterFactory&  typeConversionFactory,
                                                                        const ReprojectionFilterFactory&    reprojectionFilterFactory,
                                                                        Log&                                log = GetDefaultLog());

    IMPORT_DLLE explicit                    FilterFactory              (const TypeConversionFilterFactory&  typeConversionFactory,
                                                                        Log&                                log = GetDefaultLog());
    IMPORT_DLLE explicit                    FilterFactory              (const ReprojectionFilterFactory&    reprojectionFilterFactory,
                                                                        Log&                                log = GetDefaultLog());


    IMPORT_DLLE                             FilterFactory              (const FilterFactory&                rhs);


    IMPORT_DLLE                             ~FilterFactory             ();

    // TDORAY: Consider error codes here and no FilterCreator exposure.

    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor             (const DataType&                     sourceType,
                                                                        const DataType&                     targetType) const;

    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor             (const DataType&                     sourceType,
                                                                        const DataType&                     targetType,
                                                                        const GCS&                          sourceGCS,
                                                                        const GCS&                          targetGCS,
                                                                        const DRange3d*                     sourceExtent) const;


    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor             (const DataType&                     sourceType,
                                                                        const DataType&                     targetType,
                                                                        const GCS&                          sourceGCS,
                                                                        const GCS&                          targetGCS,
                                                                        const DRange3d*                     sourceExtent,
                                                                        const CustomFilteringSequence&      sourceFilters,
                                                                        const CustomFilteringSequence&      targetFilters) const;


    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor             (const DataType&                     sourceType,
                                                                        const CustomFilteringSequence&      filters,
                                                                        const DataType&                     targetType) const;


    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor             (const DataType&                     sourceType,
                                                                        const DataType&                     targetType,
                                                                        const Reprojection&                 reprojection) const;




    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE