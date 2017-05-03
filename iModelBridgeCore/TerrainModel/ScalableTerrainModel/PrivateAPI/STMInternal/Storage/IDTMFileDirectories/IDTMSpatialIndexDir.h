//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : SpatialIndexDir
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Listing of all supported IDTM spatial indexes types.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
enum SpatialIndexType
    {
    SPATIAL_INDEX_TYPE_SINGLE_RESOLUTION,
    SPATIAL_INDEX_TYPE_MULTI_RESOLUTION,
    SPATIAL_INDEX_TYPE_QUAD_TREE,
    SPATIAL_INDEX_TYPE_OCT_TREE,
    SPATIAL_INDEX_TYPE_QTY,
    };

/*---------------------------------------------------------------------------------**//**
* @description  Directory that stores the spatial index
*
*
* @see QuadTreeIndexHandler
*
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class SpatialIndexDir : public HTGFF::Directory
    {
public:
    struct Options : public UserOptions
        {
        virtual                 ~Options                           () = 0 {}

                                                                        SpatialIndexType        GetType                            () const;

    protected:
        explicit                Options                            (SpatialIndexType        pi_Type);
    private:
        friend class            SpatialIndexDir;

        virtual bool            _SaveTo                            (SpatialIndexDir&            pi_rIndex) const = 0;

        SpatialIndexType        m_Type;
        };


    // TDORAY: Add missing options classes here...

     static uint32_t      s_GetVersion                       ();

    virtual                     ~SpatialIndexDir                   ();


     SpatialIndexType     GetType                            () const;


     const Extent3d64f&   GetContentExtent                   () const;

    explicit                    SpatialIndexDir                    ();      // Should be private, Android problem.
    
private:
    friend class                HTGFF::Directory;

    // TDORAY: This is overkill friendship. See IDTMDirectory note...
    friend class                BTreeIndexHandler; 

    bool                        SetType                            (SpatialIndexType            pi_Type);

    virtual bool                _Create                            (const CreateConfig&         pi_rCreateConfig,
                                                                    const UserOptions*          pi_pUserOptions) override;
    virtual bool                _Load                              (const UserOptions*          pi_pUserOptions) override;
    virtual bool                _Save                              () override;


    struct                      Impl;
    struct                      DefaultImpl;
    struct                      TreeImpl;

    const Impl&                 GetImpl                            () const;
    std::auto_ptr<Impl>         m_pImpl;
    };




} //End namespace IDTMFile