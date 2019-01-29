//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

#include <STMInternal/Storage/HTGFFFile.h>
#include <STMInternal/Storage/HTGFFTagFile.h>

#include <STMInternal/Storage/HTGFFSubDirManager.h>


namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct File::Impl
    {
    TagFile                         m_file;
    mutable Directory::Ptr          m_pRootDir;

    explicit                        Impl                       (const WString&                  pi_rInputFilePath,
                                                                const HFCAccessMode&            pi_rAccessMode,
                                                                const TagFile::Definition&      pi_rFileDefinition,
                                                                uint32_t                         pi_version)
        :   m_file(pi_rInputFilePath, pi_rAccessMode, pi_rFileDefinition),
            m_pRootDir(0)
        {

        // Check for errors
        if (m_file.GetFilePtr() == 0)
            throw runtime_error("Could not open file");

        if (m_file.GetFilePtr()->GetLastException() != 0)
            throw runtime_error("An exception occurred in the file");

        HTIFFError* pErr;

        if (!m_file.IsValid(&pErr) && ((pErr == 0) || pErr->IsFatal()))
            throw runtime_error("File is not valid");

        if (pi_rAccessMode.m_HasCreateAccess)
            {
            // Check that root directory is not already present
            if (0 != m_file.NumberOfDirectory())
                throw runtime_error("Trying to recreate an existing file");

            // Append root directory
            if (!m_file.AppendDirectory())
                throw runtime_error("Could not append root directory");

            if (!m_file.SetVersion(pi_version))
                throw runtime_error("Could not set the file version");
            }
        else
            {
            // Check if a root directory is present
            if (0 == m_file.NumberOfDirectory())
                throw runtime_error("File was not created");

            // Set the current dir at least once so that it is not null
            m_file.SetDirectory(HTagFile::MakeDirectoryID(HTagFile::STANDARD, 0));

            const uint32_t version = m_file.GetVersion();
            if (version > pi_version)
                throw HFCUnsupportedFileVersionException();            
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::File (const char*                     pi_rInputFilePath,
            const HFCAccessMode&            pi_rAccessMode,
            const TagFile::Definition&      pi_rFileDefinition,
            uint32_t                         pi_version)
    {
    WString InputStr;
    BeStringUtilities::CurrentLocaleCharToWChar(InputStr, pi_rInputFilePath);
    m_pImpl.reset(new Impl(InputStr, pi_rAccessMode, pi_rFileDefinition, pi_version));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::File (WCharCP                         pi_rInputFilePath,
            const HFCAccessMode&            pi_rAccessMode,
            const TagFile::Definition&      pi_rFileDefinition,
            uint32_t                         pi_version)
    :   m_pImpl(new Impl(
                    pi_rInputFilePath,
                    pi_rAccessMode,
                    pi_rFileDefinition,
                    pi_version
                ))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::~File ()
    {
    Close();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool File::Save ()
    {
    if (0 == m_pImpl.get())
        return false;

    if (IsReadOnly()) // Do not save the file when in read only mode
        return false;

    bool Success = true;

    Directory* pRootDir = GetRootDir();
    // Notifies sub-directories of file save
    if (0 != pRootDir)
        Success &= pRootDir->OnSaved();

    m_pImpl->m_file.Save();
    // TDORAY: Check if we can check for implemention Save failure?

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool File::Close ()
    {
    if (0 == m_pImpl.get())
        return true; // Already closed

    bool Success = true;

    // Try saving the file
    if (!IsReadOnly())
        Success &= Save();

    Directory* pRootDir = GetRootDir();
    // Notifies sub-directories of file closure
    if (0 != pRootDir)
        Success &= pRootDir->OnClosed();

    // Invalidate file
    m_pImpl.reset();

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const File::AccessMode& File::GetAccessMode () const
    {
    return GetFileHandle().GetAccessMode();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool File::IsReadOnly () const
    {
    return !GetAccessMode().m_HasCreateAccess && !GetAccessMode().m_HasWriteAccess;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TagFile& File::GetFileHandle () const
    {
    return m_pImpl->m_file;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void File::SetRoot (Directory* pi_pRoot)
    {
    m_pImpl->m_pRootDir = pi_pRoot;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Directory* File::GetRootDir ()
    {
    return m_pImpl->m_pRootDir.GetPtr();
    }

} //End namespace HTGFF