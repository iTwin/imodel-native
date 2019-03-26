/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/TypeConversionFilterFactory.h $
|    $RCSfile: TypeConversionFilterFactory.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/09/07 14:21:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Filter.h>

#include <ScalableTerrainModel/Import/Plugin/CustomFilterFactoryV0.h>
#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct TypeConversionFilterRegistry;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


struct DataType;

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterFactory : private Uncopyable
    {
private:
    friend struct                           TypeConversionCustomFilterFactory;

    typedef Plugin::TypeConversionFilterRegistry
                                            TypeConversionRegistry;

    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_pImpl;

public:
    explicit                                TypeConversionFilterFactory        (Log&                            log = GetDefaultLog());
    explicit                                TypeConversionFilterFactory        (const TypeConversionRegistry&   registry,
                                                                                Log&                            log = GetDefaultLog());

                                            TypeConversionFilterFactory        (const TypeConversionFilterFactory& rhs);

                                            ~TypeConversionFilterFactory       ();

    // TDORAY: Consider error codes here

    FilterCreatorCPtr                       FindCreatorFor                     (const DataType&                 srcType,
                                                                                const DataType&                 dstType) const;
    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionCustomFilterFactory : public Import::Plugin::V0::CustomFilterFactoryBase
    {
    TypeConversionFilterFactory             m_factory;
    DataType                                m_targetType;

    explicit                                TypeConversionCustomFilterFactory      (const TypeConversionFilterFactory&  factory,
                                                                                    const DataType&                     targetType);
                                            ~TypeConversionCustomFilterFactory     ();

    virtual const FilterCreatorBase*        _FindCreatorFor                        (const DataType&                     sourceType,
                                                                                    Log&                                log) const override;

public:
    static CustomFilterFactory              CreateFrom                             (const TypeConversionFilterFactory&  factory,
                                                                                    const DataType&                     targetType);
    };



END_BENTLEY_MRDTM_IMPORT_NAMESPACE