//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTGFFDirectory.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HFCPtr.h>

#include <ImagePP/all/h/HTGFFTagFile.h>
#include <ImagePP/all/h/HPUPacket.h>

#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HUncopyable.h>

struct HFCAccessMode;

namespace HTGFF {

class DirectoryImpl;

class DirectoryHandler;
class DirectoryManager;

class AttributeManager;
class SubDirManagerBase;
class PacketManagerBase;

template <typename T>
class SubDirManager;
template <typename EditorT>
class SubDirIterManager;
template <typename EditorT>
class PacketIterManager;


/*---------------------------------------------------------------------------------**//**
* @description  User options base class. When creating a directory, user can specify
*               specific user options. This base class add some level of security
*               when user has to reinterpret the cast to his specific user options.
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class UserOptions
    {
public:
    virtual                         ~UserOptions                   () {};

    template <typename T>
    T&                              SafeReinterpretAs              ();
    template <typename T>
    const T&                        SafeReinterpretAs              () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct CreateConfig
    {
    _HDLLg explicit                 CreateConfig                   ();

    _HDLLg explicit                 CreateConfig                   (const DataType&    pi_DataType,
                                                                    const Compression& pi_DataCompressType = Compression::None::Create());

    const DataType                  m_DataType;
    const Compression               m_DataCompressType;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Base class for HTGFF File directories. Implements all required methods
*               to access directory data and sub directories.
*
*
* @see          File
* @see          HTGFFFile
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class Directory : private HUncopyable, public HFCShareableObject<Directory>
    {
public:
    typedef HFCPtr<Directory>   Ptr;

    _HDLLg virtual              ~Directory                 ();

    _HDLLg uint32_t             GetVersion                 () const;

protected:
    typedef uint32_t           ID;
    typedef uint32_t           TagID;

    typedef HPU::Packet         Packet;

    typedef TagFile::PacketID   PacketID;
    typedef uint32_t            AttributeID;
    typedef HFCAccessMode       AccessMode;

    typedef HTGFF::CreateConfig CreateConfig;
    typedef HTGFF::UserOptions  UserOptions;

    typedef HTGFF::Compression  Compression;
    typedef HTGFF::DataType     DataType;

    explicit                    Directory                      (uint32_t                 pi_version = 0);


    bool                        Register                       (DirectoryHandler&       pi_rHandler);
    void                        Unregister                     (DirectoryHandler&       pi_rHandler);

    bool                        IsValid                        () const;

    _HDLLg ID                   GetID                          () const;
    _HDLLg TagID                GetTagID                       () const;

    _HDLLg bool                 HasSubDirIndex                 () const;
    _HDLLg size_t               GetSubDirIndex                 () const;

    bool                        IsReadOnly                     () const;
    const AccessMode&           GetAccessMode                  () const;


    _HDLLg const AttributeManager&
                                AttributeMgr                   () const;
    _HDLLg AttributeManager&    AttributeMgr                   ();


    _HDLLg const PacketManagerBase&
                                PacketMgr                      () const;
    _HDLLg PacketManagerBase&   PacketMgr                      ();


    template <typename EditorT>
    const PacketIterManager<EditorT>&   
                                PacketIterMgr                  () const;
    template <typename EditorT>
    PacketIterManager<EditorT>& PacketIterMgr                  ();


    _HDLLg const SubDirManagerBase&
                                SubDirMgr                      () const;
    _HDLLg SubDirManagerBase&   SubDirMgr                      ();


    template <typename T>
    const SubDirManager<T>&     SubDirMgr                      () const;
    template <typename T>
    SubDirManager<T>&           SubDirMgr                      ();

    template <typename EditorT>
    const SubDirIterManager<EditorT>& 
                                SubDirIterMgr                  () const;
    template <typename EditorT>
    SubDirIterManager<EditorT>& SubDirIterMgr                  ();

//private:
    friend class                File;
    friend class                DirectoryImpl;
    friend class                DirectoryHandler;
    friend class                DirectoryManager;
    friend class                SubDirManagerBase;
    friend class                PacketManagerBase;


    template <typename DirT>
    class                       DirCreator;

    const DirectoryImpl&        GetImpl                        () const;
    DirectoryImpl&              GetImpl                        ();

    _HDLLg TagFile&             GetFile                        ();
    _HDLLg const TagFile&       GetFile                        () const;

    _HDLLg TagFile&             GetFileHandle                  ();

    template <typename T>
    static T*                   CreateDir                      ();

    static Byte*                Create                         ();

    _HDLLg bool                 InitializeRoot                 (TagFile&                pi_rFile,
                                                                const CreateConfig*     pi_pCreateConfig,
                                                                const UserOptions*      pi_pUserOptions);

    _HDLLg bool                 Initialize                     (TagFile&                pi_rFile,
                                                                TagID                   pi_SubDirTagID,
                                                                size_t                  pi_SubDirIndex,
                                                                ID                      pi_DirectoryID,
                                                                const CreateConfig*     pi_pCreateConfig,
                                                                const UserOptions*      pi_pUserOptions);

    bool                        OnClosed                       ();
    bool                        OnSaved                        ();
    bool                        OnRemoved                      ();

    virtual bool                _Load                          (const UserOptions*      pi_pUserOptions);
    virtual bool                _Create                        (const CreateConfig&     pi_rCreateConfig,
                                                                const UserOptions*      pi_pUserOptions);
    virtual bool                _Save                          ();
    virtual bool                _Close                         ();

    uint32_t                    m_version;
    std::auto_ptr<DirectoryImpl>
                                m_pImpl;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DirectoryManager : private HUncopyable
    {
private:

protected:
    typedef Directory::Ptr              DirectoryPtr;

    typedef uint32_t                   DirectoryID;
    typedef uint32_t                   DirectoryTagID;

    Directory&                          m_rDir;

    // Note: Not meant to be used polymorphically
                                        ~DirectoryManager              () {}
    explicit                            DirectoryManager               (Directory& pi_rDir) : m_rDir(pi_rDir) {}

    bool                                IsDirValid                     () const {
        return m_rDir.IsValid();
        }

    const Directory&                    GetDir                         () const {
        return m_rDir;
        }
    Directory&                          GetDir                         () {
        return m_rDir;
        }

    TagFile&                            GetFile                        () {
        return m_rDir.GetFile();
        }
    const TagFile&                      GetFile                        () const {
        return m_rDir.GetFile();
        }

    TagFile&                            GetFileHandle                  () {
        return m_rDir.GetFileHandle();
        }
    };

#include <ImagePP/all/h/HTGFFDirectory.hpp>

} //End namespace HTGFF