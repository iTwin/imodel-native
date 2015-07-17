/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ReprojectionFilterFactory.h $
|    $RCSfile: ReprojectionFilterFactory.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/07 14:21:09 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once


#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Filter.h>

#include <ScalableTerrainModel/GeoCoords/Reprojection.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>

#include <ScalableTerrainModel/Import/Plugin/CustomFilterFactoryV0.h>
#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct ReprojectionFilterRegistry;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct DataType;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterFactory : private Unassignable
    {
private:
    friend struct                           ReprojectionCustomFilterFactory;

    typedef Plugin::ReprojectionFilterRegistry
                                            FilterRegistry;

    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_implP;

public:
    explicit                                ReprojectionFilterFactory  (Log&                            log = GetDefaultLog());

    explicit                                ReprojectionFilterFactory  (const FilterRegistry&           registry,
                                                                        Log&                            log = GetDefaultLog());

    explicit                                ReprojectionFilterFactory  (const ReprojectionFactory&      reprojectionFactory,
                                                                        Log&                            log = GetDefaultLog());

    explicit                                ReprojectionFilterFactory  (const FilterRegistry&           registry,
                                                                        const ReprojectionFactory&      reprojectionFactory,
                                                                        Log&                            log = GetDefaultLog());

                                            ReprojectionFilterFactory  (const ReprojectionFilterFactory&
                                                                                                        rhs);

                                            ~ReprojectionFilterFactory ();

    Reprojection                            CreateReprojectionFor      (const GCS&                      sourceGCS,
                                                                        const GCS&                      targetGCS,
                                                                        const DRange3d*                 sourceExtentP) const;


    // TDORAY: Consider error codes here and no FilterCreator exposure.

    FilterCreatorCPtr                       FindCreatorFor             (const DataType&                 type,
                                                                        const GCS&                      sourceGCS,
                                                                        const GCS&                      targetGCS,
                                                                        const DRange3d*                 sourceExtentP) const;

    FilterCreatorCPtr                       FindCreatorFor             (const DataType&                 type,
                                                                        const Reprojection&             reprojection) const;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionCustomFilterFactory : public Import::Plugin::V0::CustomFilterFactoryBase
    {
    ReprojectionFilterFactory               m_factory;
    Reprojection                            m_reprojection;

    explicit                                ReprojectionCustomFilterFactory        (const ReprojectionFilterFactory&    factory,
                                                                                    const Reprojection&                 reprojection);

                                            ~ReprojectionCustomFilterFactory       ();

    virtual const FilterCreatorBase*        _FindCreatorFor                        (const DataType&                     sourceType,
                                                                                    Log&                                log) const override;

public:
    static CustomFilterFactory              CreateFrom                             (const ReprojectionFilterFactory&    factory,
                                                                                    const Reprojection&                 reprojection);

    };




END_BENTLEY_MRDTM_IMPORT_NAMESPACE