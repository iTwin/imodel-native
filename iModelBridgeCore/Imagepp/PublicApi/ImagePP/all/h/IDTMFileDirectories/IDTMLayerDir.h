//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMLayerDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <Imagepp/all/h/HTGFFDirectory.h>

#include <Imagepp/all/h/IDTMFileDirectories/IDTMMetadataDir.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <Imagepp/all/h/HTGFFSubDirIter.h>
#include <Imagepp/all/h/HCPWellKnownText.h>

namespace IDTMFile {




/*---------------------------------------------------------------------------------**//**
* @description  The layer directory. Used to access all information pertaining to a
*               layer of DTM data.
*
*               A single spatial reference system can be stored per layer. This system
*               is always stored using Unicode WKT representation. Overload helper for
*               MB WKT is provided only as a conversion facility (to Unicode rep.).
*
*               Iterator interface for mixed and uniform feature directories is provided
*               as an optimized way of accessing the whole content via STL algorithms.
*
*               Use the mixed feature dir interface when handling features
*               of mixed types.
*
*               Use the uniform feature dir interface when handling features of a
*               specific type. Uniform feature dir can be optimized for point features
*               by creating it with the special CreatePointsOnlyUniformFeatureDir. In
*               this case, no header directory will be created.
*
* @see          FeatureDir
* @see          MetadataDir
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerDir : public HTGFF::Directory
    {
private:
    friend class                    HTGFF::Directory;
    explicit                        LayerDir                           ();

    mutable FeatureHeaderTypeID     m_featureHeaderType;
    mutable PointTypeID             m_featurePointType;

    class                           FeatureDirEditor;
    class                           UniformFeatureDirEditor;
    class                           MixedFeatureDirEditor;


    void                            InitTypes                          () const;

    const FeatureDir*               GetFirstFeatureDir                 () const;

    bool                            IsPointTypeCompatible              (PointTypeID                 pi_PointType) const;




public:
    _HDLLg static LayerDir*         Create                             ();      // Should be private, Android problem.

    typedef HTGFF::SubDirIter<const FeatureDirEditor>
                                    FeatureDirCIter;
    typedef HTGFF::SubDirIter<FeatureDirEditor>
                                    FeatureDirIter;

    typedef HTGFF::SubDirIter<const UniformFeatureDirEditor>
                                    UniformFeatureDirCIter;
    typedef HTGFF::SubDirIter<UniformFeatureDirEditor>
                                    UniformFeatureDirIter;

    typedef HTGFF::SubDirIter<const MixedFeatureDirEditor>
                                    MixedFeatureDirCIter;
    typedef HTGFF::SubDirIter<MixedFeatureDirEditor>
                                    MixedFeatureDirIter;

    _HDLLg static uint32_t          s_GetVersion                       ();

    virtual                         ~LayerDir                          ();

    size_t                          GetIndex                           () const;

    _HDLLg FeatureHeaderTypeID      GetFeatureHeaderType               () const;
    _HDLLg PointTypeID              GetFeaturePointType                () const;


    _HDLLg Extent3d64f              ComputeExtent                      () const;

    // TDORAY: Add layer name accessors?

    /*---------------------------------------------------------------------------------**//**
    * Spatial reference system accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg bool                     HasWkt                             () const;

    _HDLLg bool                     SetWkt                             (const HCPWKT&               pi_wkt);
    _HDLLg HCPWKT                   GetWkt                             () const;


    /*---------------------------------------------------------------------------------**//**
    * Metadata accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg const DTMMetadataDir*    GetMetadataDir                     () const;
    _HDLLg DTMMetadataDir*          GetMetadataDir                     ();

    _HDLLg DTMMetadataDir*          CreateMetadataDir                  ();

    /*---------------------------------------------------------------------------------**//**
    * Uniform feature directories interface
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg size_t                   CountUniformFeatureDirs            () const;
    _HDLLg size_t                   CountPointOnlyUniformFeatureDirs   () const;

    _HDLLg bool                     HasUniformFeatureDir               (FeatureType                 pi_FeatureType) const;

    _HDLLg const UniformFeatureDir* GetUniformFeatureDir               (FeatureType                 pi_FeatureType) const;
    _HDLLg UniformFeatureDir*       GetUniformFeatureDir               (FeatureType                 pi_FeatureType);

    _HDLLg UniformFeatureDirCIter   UniformFeatureDirsBegin            () const;
    _HDLLg UniformFeatureDirCIter   UniformFeatureDirsEnd              () const;

    _HDLLg UniformFeatureDirIter    UniformFeatureDirsBegin            ();
    _HDLLg UniformFeatureDirIter    UniformFeatureDirsEnd              ();


    _HDLLg UniformFeatureDir*       CreateUniformFeatureDir            (FeatureType                 pi_FeatureType,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());

    _HDLLg UniformFeatureDir*       CreatePointsOnlyUniformFeatureDir  (FeatureType                 pi_FeatureType,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());


    /*---------------------------------------------------------------------------------**//**
    * Mixed feature directories interface
    +---------------+---------------+---------------+---------------+---------------+------*/
    _HDLLg size_t                   CountMixedFeatureDirs              () const;

    _HDLLg MixedFeatureDirCIter     MixedFeatureDirsBegin              () const;
    _HDLLg MixedFeatureDirCIter     MixedFeatureDirsEnd                () const;

    _HDLLg MixedFeatureDirIter      MixedFeatureDirsBegin              ();
    _HDLLg MixedFeatureDirIter      MixedFeatureDirsEnd                ();



    _HDLLg bool                     HasMixedFeatureDir                 (size_t                      pi_Index) const;

    _HDLLg const MixedFeatureDir*   GetMixedFeatureDir                 (size_t                      pi_Index) const;
    _HDLLg MixedFeatureDir*         GetMixedFeatureDir                 (size_t                      pi_Index);

    _HDLLg MixedFeatureDir*         CreateMixedFeatureDir              (size_t                      pi_Index,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());

    _HDLLg MixedFeatureDir*         AddMixedFeatureDir                 (PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());




    // TDORAY: Deprecate. Dangerous.
    typedef HTGFF::SubDirIter2<const UniformFeatureDir> 
                                    UniformFeatureDirCIter2;
    typedef HTGFF::SubDirIter2<const MixedFeatureDir> 
                                    MixedFeatureDirCIter2;

    // TDORAY: Deprecate. Dangerous.
    _HDLLg UniformFeatureDirCIter2  UniformFeatureDirsBegin2           () const;
    _HDLLg UniformFeatureDirCIter2  UniformFeatureDirsEnd2             () const;

    // TDORAY: Deprecate. Dangerous.
    _HDLLg MixedFeatureDirCIter2    MixedFeatureDirsBegin2             () const;
    _HDLLg MixedFeatureDirCIter2    MixedFeatureDirsEnd2               () const;


    };

#pragma warning( push )
#pragma warning( disable : 4265 )

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerDir::FeatureDirEditor : public HTGFF::SubDirEditorBase<HTGFF::SubDirManagerBase>
    {
    virtual const FeatureDir*   _Get                               () const = 0;
    virtual FeatureDir*         _Get                               () = 0;

protected:
    explicit                    FeatureDirEditor                   ();
                                FeatureDirEditor                   (const FeatureDirEditor&         rhs);


    // Not meant to be held polymorphically
                                ~FeatureDirEditor                  () {}
public:
    _HDLLg const FeatureDir*    Get                                () const;
    _HDLLg FeatureDir*          Get                                ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerDir::UniformFeatureDirEditor : public FeatureDirEditor
    {
    virtual const UniformFeatureDir*   
                                _Get                               () const override;
    virtual UniformFeatureDir*  _Get                               () override;

public:
    _HDLLg explicit             UniformFeatureDirEditor            ();
    _HDLLg                      UniformFeatureDirEditor            (const UniformFeatureDirEditor&  rhs);

    // Using default assignment

    _HDLLg const UniformFeatureDir* 
                                Get                                () const;
    _HDLLg UniformFeatureDir*   Get                                ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LayerDir::MixedFeatureDirEditor : public FeatureDirEditor
    {
    virtual const MixedFeatureDir*   
                                _Get                               () const override;
    virtual MixedFeatureDir*    _Get                               () override;

public:
    _HDLLg explicit             MixedFeatureDirEditor              ();
    _HDLLg                      MixedFeatureDirEditor              (const MixedFeatureDirEditor&    rhs);

    // Using default assignment

    _HDLLg const MixedFeatureDir* 
                                Get                                () const;
    _HDLLg MixedFeatureDir*     Get                                ();
    };


#pragma warning( pop )

inline size_t LayerDir::GetIndex () const
    {
    return GetSubDirIndex();
    }

} //End namespace IDTMFile