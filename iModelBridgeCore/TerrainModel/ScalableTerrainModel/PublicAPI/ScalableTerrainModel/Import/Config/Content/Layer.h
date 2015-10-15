/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Content/Layer.h $
|    $RCSfile: Layer.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/11/22 21:28:22 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Config/Content/Base.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ILayerConfigVisitor;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LayerConfig : public ContentConfigComponentMixinBase<LayerConfig>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    struct Impl;
    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;

    UInt                                m_layerID;
    uint32_t                              m_flags;
    ImplPtr                             m_implP;
public:
    typedef ContentConfigComponent      value_type;
    typedef const value_type&           const_reference;
    typedef value_type&                 reference;

    typedef const std::type_info*       ComponentClassID;

    IMPORT_DLLE static ClassID          s_GetClassID                   ();

    IMPORT_DLLE explicit                LayerConfig                    (UInt                                layerID);
    IMPORT_DLLE virtual                 ~LayerConfig                   ();

    IMPORT_DLLE                         LayerConfig                    (const LayerConfig&                  rhs);

    UInt                                GetID                          () const;

    IMPORT_DLLE LayerConfig&            push_back                      (const ContentConfigComponent&       config);
    IMPORT_DLLE LayerConfig&            push_back                      (const ContentConfigComponentBase&   config);

    IMPORT_DLLE void                    Accept                         (ILayerConfigVisitor&                 visitor) const;

    IMPORT_DLLE bool                    IsEmpty                        () const;
    IMPORT_DLLE size_t                  GetCount                       () const;

    IMPORT_DLLE void                    RemoveAllOfType                (ComponentClassID                        classID);
    };


inline UInt LayerConfig::GetID () const
    { return m_layerID; }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE