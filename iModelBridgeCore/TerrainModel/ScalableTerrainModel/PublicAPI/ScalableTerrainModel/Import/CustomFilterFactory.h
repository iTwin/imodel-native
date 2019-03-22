/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/CustomFilterFactory.h $
|    $RCSfile: CustomFilterFactory.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/09/01 14:07:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Filter.h>
#include <ScalableTerrainModel/Import/DataType.h>


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct CustomFilterFactoryBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilterFactory
    {
private:
    typedef Plugin::V0::CustomFilterFactoryBase
                                            Base;
    friend                                  Base;

    typedef SharedPtrTypeTrait<const Base>::type
                                            BaseCPtr;
    BaseCPtr                                m_basePtr;

    explicit                                CustomFilterFactory            (Base*                               filterP);

public:
    IMPORT_DLLE                             ~CustomFilterFactory           ();

    IMPORT_DLLE                             CustomFilterFactory            (const CustomFilterFactory&          rhs);
    IMPORT_DLLE CustomFilterFactory&        operator=                      (const CustomFilterFactory&          rhs);

    IMPORT_DLLE FilterCreatorCPtr           FindCreatorFor                 (const DataType&                     sourceType,
                                                                            Log&                                log) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* NTERAY: Consider COW
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomFilteringSequence
    {
private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_implP;

public:
    typedef const CustomFilterFactory*      const_iterator;

    IMPORT_DLLE explicit                    CustomFilteringSequence        ();

    IMPORT_DLLE explicit                    CustomFilteringSequence        (const CustomFilterFactory&          filter);
    IMPORT_DLLE                             ~CustomFilteringSequence       ();

    IMPORT_DLLE explicit                    CustomFilteringSequence        (const CustomFilteringSequence&      rhs);
    IMPORT_DLLE CustomFilteringSequence&    operator=                      (const CustomFilteringSequence&      rhs);



    IMPORT_DLLE const_iterator              begin                          () const;
    IMPORT_DLLE const_iterator              end                            () const;

    IMPORT_DLLE size_t                      GetCount                       () const;
    IMPORT_DLLE void                        push_back                      (const CustomFilterFactory&          filter);
    };

END_BENTLEY_MRDTM_IMPORT_NAMESPACE