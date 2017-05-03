//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMLayerDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <STMInternal/Storage/HTGFFSubDirIter.h>
#include <Imagepp/all/h/HCPWellKnownText.h>

namespace IDTMFile {


    enum WktFlavor
    {
        WktFlavor_Oracle9 = 1, 
        WktFlavor_Autodesk, 
        WktFlavor_End, 
    };



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
     static LayerDir*         Create                             ();      // Should be private, Android problem.

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

     static uint32_t          s_GetVersion                       ();

    virtual                         ~LayerDir                          ();

    size_t                          GetIndex                           () const;

     FeatureHeaderTypeID      GetFeatureHeaderType               () const;
     PointTypeID              GetFeaturePointType                () const;


     Extent3d64f              ComputeExtent                      () const;

    // TDORAY: Add layer name accessors?

    /*---------------------------------------------------------------------------------**//**
    * Spatial reference system accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool                     HasWkt                             () const;

     bool                     SetWkt                             (const BentleyApi::ImagePP::HCPWKT&               pi_wkt);
     BentleyApi::ImagePP::HCPWKT                   GetWkt                             () const;


    /*---------------------------------------------------------------------------------**//**
    * Metadata accessors
    +---------------+---------------+---------------+---------------+---------------+------*/
     const DTMMetadataDir*    GetMetadataDir                     () const;
     DTMMetadataDir*          GetMetadataDir                     ();

     DTMMetadataDir*          CreateMetadataDir                  ();

    /*---------------------------------------------------------------------------------**//**
    * Uniform feature directories interface
    +---------------+---------------+---------------+---------------+---------------+------*/
     size_t                   CountUniformFeatureDirs            () const;
     size_t                   CountPointOnlyUniformFeatureDirs   () const;

     bool                     HasUniformFeatureDir               (FeatureType                 pi_FeatureType) const;

     const UniformFeatureDir* GetUniformFeatureDir               (FeatureType                 pi_FeatureType) const;
     UniformFeatureDir*       GetUniformFeatureDir               (FeatureType                 pi_FeatureType);

     UniformFeatureDirCIter   UniformFeatureDirsBegin            () const;
     UniformFeatureDirCIter   UniformFeatureDirsEnd              () const;

     UniformFeatureDirIter    UniformFeatureDirsBegin            ();
     UniformFeatureDirIter    UniformFeatureDirsEnd              ();


     UniformFeatureDir*       CreateUniformFeatureDir            (FeatureType                 pi_FeatureType,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());

     UniformFeatureDir*       CreatePointsOnlyUniformFeatureDir  (FeatureType                 pi_FeatureType,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());


    /*---------------------------------------------------------------------------------**//**
    * Mixed feature directories interface
    +---------------+---------------+---------------+---------------+---------------+------*/
     size_t                   CountMixedFeatureDirs              () const;

     MixedFeatureDirCIter     MixedFeatureDirsBegin              () const;
     MixedFeatureDirCIter     MixedFeatureDirsEnd                () const;

     MixedFeatureDirIter      MixedFeatureDirsBegin              ();
     MixedFeatureDirIter      MixedFeatureDirsEnd                ();



     bool                     HasMixedFeatureDir                 (size_t                      pi_Index) const;

     const MixedFeatureDir*   GetMixedFeatureDir                 (size_t                      pi_Index) const;
     MixedFeatureDir*         GetMixedFeatureDir                 (size_t                      pi_Index);

     MixedFeatureDir*         CreateMixedFeatureDir              (size_t                      pi_Index,
                                                                        PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());

     MixedFeatureDir*         AddMixedFeatureDir                 (PointTypeID                 pi_PointType = POINT_TYPE_XYZf64,
                                                                        const Compression&          pi_CompressType = Compression::None::Create());




    // TDORAY: Deprecate. Dangerous.
    typedef HTGFF::SubDirIter2<const UniformFeatureDir> 
                                    UniformFeatureDirCIter2;
    typedef HTGFF::SubDirIter2<const MixedFeatureDir> 
                                    MixedFeatureDirCIter2;

    // TDORAY: Deprecate. Dangerous.
     UniformFeatureDirCIter2  UniformFeatureDirsBegin2           () const;
     UniformFeatureDirCIter2  UniformFeatureDirsEnd2             () const;

    // TDORAY: Deprecate. Dangerous.
     MixedFeatureDirCIter2    MixedFeatureDirsBegin2             () const;
     MixedFeatureDirCIter2    MixedFeatureDirsEnd2               () const;


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
     const FeatureDir*    Get                                () const;
     FeatureDir*          Get                                ();
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
     explicit             UniformFeatureDirEditor            ();
                          UniformFeatureDirEditor            (const UniformFeatureDirEditor&  rhs);

    // Using default assignment

     const UniformFeatureDir* 
                                Get                                () const;
     UniformFeatureDir*   Get                                ();
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
     explicit             MixedFeatureDirEditor              ();
                          MixedFeatureDirEditor              (const MixedFeatureDirEditor&    rhs);

    // Using default assignment

     const MixedFeatureDir* 
                                Get                                () const;
     MixedFeatureDir*     Get                                ();
    };


#pragma warning( pop )

inline size_t LayerDir::GetIndex () const
    {
    return GetSubDirIndex();
    }

} //End namespace IDTMFile