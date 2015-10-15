//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFDirectory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

#include <STMInternal/Storage/HTGFFAttributeManager.h>
#include <STMInternal/Storage/HTGFFPacketManager.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>



namespace HTGFF {


/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CreateConfig::CreateConfig ()
    :   m_DataType(DataType::CreateVoid()),
        m_DataCompressType(Compression::None::Create())
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CreateConfig::CreateConfig (const DataType&    pi_DataType,
                            const Compression& pi_DataCompressType)
    :   m_DataType(pi_DataType),
        m_DataCompressType(pi_DataCompressType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class DirectoryImpl
    {
public:
    typedef HFCPtr<Directory>               DirPtr;

    typedef vector<DirPtr>                  SubDirectoriesList;
    typedef vector<DirectoryHandler*>       RegisteredHandlersList;

    TagFile&                                m_rFile;
    Directory::TagID                        m_SubDirTagID;
    Directory::ID                           m_DirectoryID;
    size_t                                  m_SubDirIndex;

    RegisteredHandlersList                  m_registeredHandlers;

    AttributeManager                        m_attributeManager;
    PacketManagerBase                       m_packetManager;
    SubDirManagerBase                       m_subDirManager;


    explicit                                DirectoryImpl                  (TagFile&                pi_rFile,
                                                                            Directory::TagID        pi_SubDirTagID,
                                                                            size_t                  pi_SubDirIndex,
                                                                            Directory::ID           pi_DirectoryID,
                                                                            Directory&              pi_rDir)
        :   m_rFile(pi_rFile),
            m_SubDirTagID(pi_SubDirTagID),
            m_DirectoryID(pi_DirectoryID),
            m_SubDirIndex(pi_SubDirIndex),
            m_attributeManager(pi_rDir),
            m_packetManager(pi_rDir),
            m_subDirManager(pi_rDir)
        {

        }

    TagFile&                                GetFile                        ();
    const TagFile&                          GetFile                        () const;



    struct NotifyDirForSave
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnSaved ();
            }
        };
    struct NotifyDirForClose
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnClosed ();
            }
        };

    struct NotifyDirForSubDirCacheRemoval
        {
        bool operator () (Directory* pi_pDirectory) const {
            return pi_pDirectory->OnRemoved ();
            }
        };

    struct SaveHandler
        {
        bool operator () (DirectoryHandler* pi_pHandler) const {
            return pi_pHandler->Save ();
            }
        };
    struct CloseHandler
        {
        bool operator () (DirectoryHandler* pi_pHandler) const {
            return pi_pHandler->Close ();
            }
        };
    };

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const AttributeManager& Directory::AttributeMgr () const
    {
    return GetImpl().m_attributeManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeManager& Directory::AttributeMgr ()
    {
    return GetImpl().m_attributeManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PacketManagerBase& Directory::PacketMgr () const
    {
    return GetImpl().m_packetManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketManagerBase& Directory::PacketMgr ()
    {
    return GetImpl().m_packetManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SubDirManagerBase& Directory::SubDirMgr () const
    {
    return GetImpl().m_subDirManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirManagerBase& Directory::SubDirMgr ()
    {
    return GetImpl().m_subDirManager;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Constructor.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Directory::Directory (uint32_t pi_version)
    :   m_version(pi_version),
        m_pImpl(0)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  Destructor.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Directory::~Directory ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Directory::GetVersion  () const
    {
    return m_version;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                            Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::InitializeRoot (TagFile&            pi_rFile,
                                const CreateConfig* pi_pCreateConfig,
                                const UserOptions*  pi_pUserOptions)
    {
    // Set the current dir at least once so that it is not null
    pi_rFile.SetDirectory(HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0));

    // Root dir version is expected to be set via the file's constructor. User should expect his version to be overridden if set.
    HPRECONDITION(0 == m_version);
    m_version = pi_rFile.GetVersion();

    return Initialize(pi_rFile,
                      0,
                      SubDirManagerBase::GetNoSubDirIndex(),
                      HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0),
                      pi_pCreateConfig,
                      pi_pUserOptions);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Initialize the current directory. Called on first create/get of the
*               directory.
* @param        void
* @return       true on success, false otherwise.
* @bsimethod                                            Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::Initialize (TagFile&                pi_rFile,
                            TagID                   pi_SubDirTagID,
                            size_t                  pi_SubDirIndex,
                            ID                      pi_DirectoryID,
                            const CreateConfig*     pi_pCreateConfig,
                            const UserOptions*      pi_pUserOptions)
    {
    m_pImpl.reset(new DirectoryImpl(pi_rFile, pi_SubDirTagID, pi_SubDirIndex, pi_DirectoryID, *this));

    if (0 != pi_pCreateConfig)
        {
        if (IsReadOnly())
            {
            HASSERT(!"Should never have been called with a create config when in read-only mode.");
            return false;
            }

        if (!GetFile().SetVersion(m_version))
            return false;

        if (!GetFile().SetDataType(pi_pCreateConfig->m_DataType))
            return false;

        if (!GetFile().SetDataCompressType(pi_pCreateConfig->m_DataCompressType))
            return false;

        if (!_Create(*pi_pCreateConfig, pi_pUserOptions))
            {
            HASSERT(!"Failed creating directory");
            return false;
            }
        }
    else
        {
        const uint32_t version = GetFile().GetVersion();
        if (version > m_version)
            {
            // Stored directory version is greater than actual directory version. We cannot 
            // support forward compatibility.
            return false;
            }
        m_version = version;
        }

    // Load current directory
    if (!_Load(pi_pUserOptions))
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the file implementation. Change current directory when needed.
*               File implementation should always be accessed through this method.
*               NOTE: If file is closed, file implementation will be invalidated
*                     so invoking this method will provoke a null ptr crash.
* @return       File implementation
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile& Directory::GetFile ()
    {
    return GetImpl().GetFile();
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the file implementation. Change current directory when needed.
*               File implementation should always be accessed through this method.
*               NOTE: If file is closed, file implementation will be invalidated
*                     so invoking this method will provoke a null ptr crash.
* @return       File implementation
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const TagFile& Directory::GetFile () const
    {
    return GetImpl().GetFile();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile& DirectoryImpl::GetFile ()
    {
    if (m_DirectoryID != m_rFile.CurrentDirectory())
        m_rFile.SetDirectory(m_DirectoryID);

    return m_rFile;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TagFile& DirectoryImpl::GetFile () const
    {
    if (m_DirectoryID != m_rFile.CurrentDirectory())
        const_cast<TagFile&>(m_rFile).SetDirectory(m_DirectoryID);

    return m_rFile;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile& Directory::GetFileHandle ()
    {
    return GetImpl().m_rFile;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the unique directory ID.
* @return       Directory ID.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Directory::ID Directory::GetID () const
    {
    return GetImpl().m_DirectoryID;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the tag id associated with this directory.
* @return       Tag id.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Directory::TagID Directory::GetTagID () const
    {
    return GetImpl().m_SubDirTagID;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Return whether this directory has a sub directory index. A directory
*               has a sub directory index only when part of a sub directory collection.
* @return       true when has a sub dir index, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::HasSubDirIndex () const
    {
    return SubDirManagerBase::GetNoSubDirIndex() != GetImpl().m_SubDirIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Return index of this sub directory. Could only be used when this
*               is part of a sub directories collection.
* @return       The sub directory index of this directory. 0 when not
*               part of a sub directories collection.
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Directory::GetSubDirIndex () const
    {
    HASSERT(SubDirManagerBase::GetNoSubDirIndex() != GetImpl().m_SubDirIndex);
    return (SubDirManagerBase::GetNoSubDirIndex() != GetImpl().m_SubDirIndex) ? GetImpl().m_SubDirIndex : 0;
    }


bool Directory::Register (DirectoryHandler& pi_rHandler)
    {
    // Ensure that handler is not already registered
    HASSERT(GetImpl().m_registeredHandlers.end() == find(GetImpl().m_registeredHandlers.begin(),
                                                         GetImpl().m_registeredHandlers.end(),
                                                         &pi_rHandler));

    GetImpl().m_registeredHandlers.push_back(&pi_rHandler);
    return true;
    }

void Directory::Unregister (DirectoryHandler& pi_rHandler)
    {
    DirectoryImpl::RegisteredHandlersList::iterator foundIt = std::find(GetImpl().m_registeredHandlers.begin(),
                                                                        GetImpl().m_registeredHandlers.end(),
                                                                        &pi_rHandler);

    if (foundIt == GetImpl().m_registeredHandlers.end())
        {
        HASSERT(!"Wrong unregistered handler or no handler to unregister");
        // TDORAY: Throw instead??
        return;
        }

    GetImpl().m_registeredHandlers.erase(foundIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Load the current directory. Called on initialization for each
*               directories when directory is accessed, starting by the parent directory.
*               Also called just after a successful _Create.
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::_Load  (const UserOptions*  pi_pUserOptions)
    {
    // Default behavior -> Do nothing.
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Create the current directory. Called on initialization for each
*               directories when creating a directory, starting by the parent directory.
*               Called just before _Load.
*
* @param        pi_rCreateConfig    Create config
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::_Create (const CreateConfig& pi_rCreateConfig,
                         const UserOptions*  pi_pUserOptions)
    {
    HPRECONDITION(!IsReadOnly());
    // Default behavior -> Do nothing.
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Notify directory of file closure. Invoke recursively all subdirectories.
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::OnClosed ()
    {
    bool Success = true;

    Success &= GetImpl().m_subDirManager.OnDirectoryClosed();

    // Notify all registered handlers that this directory is to be saved
    Success &= (count_if(GetImpl().m_registeredHandlers.begin(), GetImpl().m_registeredHandlers.end(), DirectoryImpl::CloseHandler()) == GetImpl().m_registeredHandlers.size());

    // Close the current directory
    if (!_Close())
        {
        HASSERT(!"Directory has not closed correctly");
        Success = false;
        }

    // Invalidate our directory
    m_pImpl.reset();

    return Success;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Notify directory of file saved. Invoke recursively all subdirectories.
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::OnSaved ()
    {
    HPRECONDITION(!IsReadOnly()); // Directory should not be saved in read-only mode
    bool Success = true;

    Success &= GetImpl().m_subDirManager.OnDirectorySaved();

    // Notify all registered handler that this directory is to be saved
    Success &= (count_if(GetImpl().m_registeredHandlers.begin(), GetImpl().m_registeredHandlers.end(), DirectoryImpl::SaveHandler()) == GetImpl().m_registeredHandlers.size());

    // Save the current directory to the file
    if (!_Save())
        {
        HASSERT(!"Directory could not be saved");
        Success = false;
        }
    return Success;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::OnRemoved ()
    {
    // Notify all registered handler that this directory is to be saved
    bool Success = (count_if(GetImpl().m_registeredHandlers.begin(), GetImpl().m_registeredHandlers.end(), DirectoryImpl::CloseHandler()) == GetImpl().m_registeredHandlers.size());

    Success &= GetImpl().m_subDirManager.OnDirectoryRemoved();

    // Save the current directory to the file
    if (!_Save())
        {
        HASSERT(!"Directory could not be saved");
        Success &= false;
        }

    // Close the current directory
    if (!_Close())
        {
        HASSERT(!"Directory has not closed correctly");
        Success &= false;
        }

    // Invalidate our directory
    m_pImpl.reset();

    return Success;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Load the current directory. Called before file is closed for each
*               directories, starting by leaf directories. So sub directories are
*               guaranteed to be saved before this method is called. Never called in
*               read-only mode.
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::_Save  ()
    {
    HPRECONDITION(!IsReadOnly());
    // Default behavior -> Do nothing.
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Close the current directory. Called before file is closed for each
*               directories but after the file and all its directories were saved,
*               starting by leaf directories. So sub directories are guaranteed to be
*               saved before this method is called
* @return       true on success, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool Directory::_Close ()
    {
    // Default behavior -> Do nothing.
    return true;
    }



} //End namespace HTGFF