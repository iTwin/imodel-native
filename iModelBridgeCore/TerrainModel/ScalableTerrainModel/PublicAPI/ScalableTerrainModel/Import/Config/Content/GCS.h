/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Content/GCS.h $
|    $RCSfile: GCS.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/22 21:58:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Config/Content/Base.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <ScalableTerrainModel/GeoCoords/LocalTransform.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCSConfig : public ContentConfigComponentMixinBase<GCSConfig>
    {
public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }


private:
    GCS                                 m_gcs;
    void*                               m_implP; // Reserve some space for further use
public:
    IMPORT_DLLE static ClassID          s_GetClassID                       ();

    IMPORT_DLLE explicit                GCSConfig                          (const GCS&                  gcs);
    IMPORT_DLLE virtual                 ~GCSConfig                         ();

    IMPORT_DLLE                         GCSConfig                          (const GCSConfig&            rhs);

    const GCS&                          GetGCS                             () const;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCSExtendedConfig : public ContentConfigComponentMixinBase<GCSExtendedConfig>
    {

public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

private:
    struct                              Impl;
    
    enum 
        {
        FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM = 0x1,
        FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED = 0x2,
        FLAG_PRESERVE_EXISTING_IF_LOCAL_CS = 0x4,
        };

    GCS                                 m_gcs;
    UInt32                              m_flags;
    void*                               m_implP; // Reserve some space for further use


public:
    IMPORT_DLLE static ClassID          s_GetClassID                       ();

    IMPORT_DLLE explicit                GCSExtendedConfig                  (const GCS&                  gcs);
    IMPORT_DLLE virtual                 ~GCSExtendedConfig                 ();

    IMPORT_DLLE                         GCSExtendedConfig                  (const GCSExtendedConfig&    rhs);

    const GCS&                          GetGCS                             () const;
    
    GCSExtendedConfig&                  PrependToExistingLocalTransform    (bool                        prepend);
    GCSExtendedConfig&                  PreserveExistingIfGeoreferenced    (bool                        preserve);
    GCSExtendedConfig&                  PreserveExistingIfLocalCS          (bool                        preserve);    
    
    bool                                IsPrependedToExistingLocalTransform() const;
    bool                                IsExistingPreservedIfGeoreferenced () const;
    bool                                IsExistingPreservedIfLocalCS       () const;        
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct GCSLocalAdjustmentConfig : public ContentConfigComponentMixinBase<GCSLocalAdjustmentConfig>
    {
private:
    LocalTransform                      m_transform;
    UInt32                              m_flags;

    void*                               m_implP; // Reserve some space for further use


public:
    

    IMPORT_DLLE static ClassID          s_GetClassID                       ();

    IMPORT_DLLE explicit                GCSLocalAdjustmentConfig           (const LocalTransform&               transform);


    IMPORT_DLLE virtual                 ~GCSLocalAdjustmentConfig          ();

    IMPORT_DLLE                         GCSLocalAdjustmentConfig           (const GCSLocalAdjustmentConfig&     rhs);

    const LocalTransform&               GetTransform                       () const;
    };



inline const GCS& GCSConfig::GetGCS () const
    { return m_gcs; }



inline const GCS& GCSExtendedConfig::GetGCS () const
    { return m_gcs; }

inline GCSExtendedConfig& GCSExtendedConfig::PrependToExistingLocalTransform (bool prepend)
    { SetBitsTo(m_flags, FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM, prepend); return *this; }
inline GCSExtendedConfig& GCSExtendedConfig::PreserveExistingIfGeoreferenced (bool preserve)
    { SetBitsTo(m_flags, FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED, preserve); return *this; }
inline GCSExtendedConfig& GCSExtendedConfig::PreserveExistingIfLocalCS (bool preserve)
    { SetBitsTo(m_flags, FLAG_PRESERVE_EXISTING_IF_LOCAL_CS, preserve); return *this; }

inline bool GCSExtendedConfig::IsPrependedToExistingLocalTransform () const
    { return HasBitsOn(m_flags, FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM); }
inline bool GCSExtendedConfig::IsExistingPreservedIfGeoreferenced () const
    { return HasBitsOn(m_flags, FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED); }
inline bool GCSExtendedConfig::IsExistingPreservedIfLocalCS () const
    { return HasBitsOn(m_flags, FLAG_PRESERVE_EXISTING_IF_LOCAL_CS); }


inline const LocalTransform& GCSLocalAdjustmentConfig::GetTransform () const
    { return m_transform; }



END_BENTLEY_MRDTM_IMPORT_NAMESPACE