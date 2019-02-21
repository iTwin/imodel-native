//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTGFF::File
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

#include <STMInternal/Storage/HPUPacket.h>

BEGIN_IMAGEPP_NAMESPACE
struct HFCAccessMode;
END_IMAGEPP_NAMESPACE

namespace HTGFF {

class TagFile;

class FileDefinition;

class Directory;

/*---------------------------------------------------------------------------------**//**
* @description
*
* NOTE: Not meant to be used polymorphicaly.
*
* @see          Directory
* @see          TagFile
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class File
    {
    struct                      Impl;
    std::auto_ptr<Impl>         m_pImpl;

    // Disable copies of any kind
    File                           (const File&             pi_rObj);
    File&                       operator=                      (const File&             pi_rObj);

     TagFile&             GetFileHandle                  () const;

     void                 SetRoot                        (Directory*                      pi_pRoot);
     Directory*           GetRootDir                     ();

public:
    typedef BentleyApi::ImagePP::HFCAccessMode       AccessMode;

     explicit             File                           (const char*                     pi_rInputFilePath,
                                                                const AccessMode&            pi_rAccessMode,
                                                                const FileDefinition&           pi_rFileDefinition,
                                                                uint32_t                         pi_version = 0);

     explicit             File                           (WCharCP                         pi_rInputFilePath,
                                                                const AccessMode&            pi_rAccessMode,
                                                                const FileDefinition&           pi_rFileDefinition,
                                                                uint32_t                         pi_version = 0);

    // As class is not meant to be used polymorphicaly, destructor is protected and not virtual
                          ~File                          ();


     bool                 IsReadOnly                     () const;
     const AccessMode&    GetAccessMode                  () const;

     bool                 Save                           ();

     bool                 Close                          ();

    // TDORAY: Should return const type when HFCPtr become const wise
    template <typename RootDirT>
    RootDirT*                   GetRootDir                     () const;

    template <typename RootDirT>
    RootDirT*                   GetRootDir                     ();

    };



#include <STMInternal/Storage/HTGFFFile.hpp>

} //End namespace HTGFF