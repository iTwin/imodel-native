//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/FeatureHeaderTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


namespace IDTMFile {



typedef uint32_t                       FeatureType;



/*---------------------------------------------------------------------------------**//**
* @description  Listing of all supported header types. This is also the sub type for
*               IDTMDirectoryType::HEADER.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
enum FeatureHeaderTypeID
    {
    FEATURE_HEADER_TYPE_FEATURE,
    FEATURE_HEADER_TYPE_QTY,
    FEATURE_HEADER_TYPE_INVALID = FEATURE_HEADER_TYPE_QTY,
    FEATURE_HEADER_TYPE_NONE = FEATURE_HEADER_TYPE_QTY,
    };


/*---------------------------------------------------------------------------------**//**
* @description  Compile time mapping between feature header types and feature header
*                types ids.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FeatureHeaderT>
struct FeatureHeaderTypeIDTrait
    {
    // TDORAY: Crash instead
    static const FeatureHeaderTypeID value = FEATURE_HEADER_TYPE_QTY;
    };




#ifdef _WIN32
#pragma pack(push, IDTMFileIdent, 4)
#else
#pragma pack(push, 4)
#endif


/*---------------------------------------------------------------------------------**//**
* @description Native IDTM feature header
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct FeatureHeader
    {
    // Move to a trait
    typedef uint32_t                   feature_type;
    typedef uint32_t                   group_id_type;
    typedef uint32_t                   index_type;
    typedef uint32_t                   size_type;

     static group_id_type         GetNullID                      ();

    feature_type                        type;
    index_type                          offset;     // In points
    size_type                           size;       // In points
    group_id_type                       groupId;    // Reference to the metadata for this feature
    };

template <> struct FeatureHeaderTypeIDTrait<FeatureHeader> {
    static const FeatureHeaderTypeID value = FEATURE_HEADER_TYPE_FEATURE;
    };





#ifdef _WIN32
#pragma pack(pop, IDTMFileIdent)
#else
#pragma pack(pop)
#endif



} //End namespace IDTMFile
