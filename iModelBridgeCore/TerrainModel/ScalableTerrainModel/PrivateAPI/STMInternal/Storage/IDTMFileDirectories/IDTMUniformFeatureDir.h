//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Directory that store only features of uniform type. The type of data
*               stored in this directory can be qualified as point only or complex
*               data. When point only applies (IsPointOnly), user can mount optimized
*               queries by using directly the underlying point directory (GetPointDir).
*               As this directory can store features of only a single type, this type
*               can be queried for the whole directory via GetFeatureType.
*
*               Use appropriate TileHandler depending on stored point type (GetPointType)
*               and other (TDORAY: to be determined) factors.
*
*
* @see          FeatureDir
* @see          FeatureTileHandler
* @see          PointDir
* @bsiclass                                                  Raymond.Gauthier   9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class UniformFeatureDir : public FeatureDir
    {
public:
    struct Options : public UserOptions
        {
         explicit             Options                            (bool                        pi_PointOnly);

         bool                 IsPointOnly                        () const;

    private:
        bool                        m_PointOnly;
        };

     FeatureType              GetFeatureType                     () const;

    /*---------------------------------------------------------------------------------**//**
    * Direct access to underlying point directory.
    +---------------+---------------+---------------+---------------+---------------+------*/
     PointDir*                GetPointDir                        () const;

     static UniformFeatureDir*
                                    Create                             ();      // Should be private, Android problem.


private:
    friend class                    HTGFF::Directory;
    explicit                        UniformFeatureDir                  ();


    virtual bool                    _IsUniform                         () const override {
        return true;
    }

    virtual bool                    _CreateFeatureHeaderDir            (HeaderDir*&                 po_rpHeaderDir,
                                                                        const CreateConfig&         pi_rCreateConfig,
                                                                        const UserOptions*          pi_pUserOptions) override;
    virtual bool                    _LoadFeatureHeaderDir              (HeaderDir*&                 po_rpHeaderDir,
                                                                        const UserOptions*          pi_pUserOptions) override;

    FeatureType                     m_FeatureType;
    };



inline FeatureType UniformFeatureDir::GetFeatureType () const
    {
    return static_cast<FeatureType>(GetSubDirIndex());
    }


} //End namespace IDTMFile