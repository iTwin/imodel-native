//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTGFFFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HTGFF::File
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

#include <ImagePP/all/h/HPUPacket.h>

struct HFCAccessMode;
class TagFile;

namespace HTGFF {


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

    _HDLLg TagFile&             GetFileHandle                  () const;

    _HDLLg void                 SetRoot                        (Directory*                      pi_pRoot);
    _HDLLg Directory*           GetRootDir                     ();

public:
    typedef HFCAccessMode       AccessMode;

    _HDLLg explicit             File                           (const char*                     pi_rInputFilePath,
                                                                const HFCAccessMode&            pi_rAccessMode,
                                                                const FileDefinition&           pi_rFileDefinition,
                                                                uint32_t                         pi_version = 0);

    _HDLLg explicit             File                           (WCharCP                         pi_rInputFilePath,
                                                                const HFCAccessMode&            pi_rAccessMode,
                                                                const FileDefinition&           pi_rFileDefinition,
                                                                uint32_t                         pi_version = 0);

    // As class is not meant to be used polymorphicaly, destructor is protected and not virtual
    _HDLLg                      ~File                          ();


    _HDLLg bool                 IsReadOnly                     () const;
    _HDLLg const AccessMode&    GetAccessMode                  () const;

    _HDLLg bool                 Save                           ();

    _HDLLg bool                 Close                          ();

    // TDORAY: Should return const type when HFCPtr become const wise
    template <typename RootDirT>
    RootDirT*                   GetRootDir                     () const;

    template <typename RootDirT>
    RootDirT*                   GetRootDir                     ();

    };



#include <ImagePP/all/h/HTGFFFile.hpp>

} //End namespace HTGFF