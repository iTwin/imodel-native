/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnIModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Supplies the parameters necessary to create new .imodel file.
// @bsiclass
//=======================================================================================
struct CreateIModelParams : BeSQLite::Db::CreateParams
{
public:
    Utf8String m_description;
    Utf8String m_client;
    BeSQLite::BeGuid   m_guid;
    uint32_t   m_chunkSize;
    bool       m_overwriteExisting;

    //! @param[in] guid The BeProjectGuid to store in the newly created .imodel database. If invalid (the default), a new BeSQLite::BeGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateIModelParams(BeSQLite::BeGuid guid=BeSQLite::BeGuid()) : m_guid(guid), m_overwriteExisting(false), m_chunkSize(1024 * 1024) {}

    //! Determine whether to overwrite an existing file in DgnDb::CreateDgnDb. The default is to fail if a file by the supplied name
    //! already exists.
    void SetOverwriteExisting(bool val) {m_overwriteExisting = val;}

    //! Specifies the size of a block to be compressed and stored in a distinct blob in the package file.  It will be rounded up to an
    //! appropriate value.
    void SetChunkSize(uint32_t chunkSize) {m_chunkSize = chunkSize;}

    //! Get the BeProjectGuid to be stored in the newly created DgnDb. This is only necessary if you don't supply a valid BeProjectGuid
    //! to the ctor.
    BeSQLite::BeGuid GetGuid() const {return m_guid;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnIModel
{
public:
    //! Extracts a DgnDb file from an imodel.
    //! @param[out] dbResult When Extract returns DgnDbStatus::SQLiteError, the actual DB error is stored in dbResult.
    //! @param[in] outputDirectory Where to store the extracted DgnDb file.
    //! @param[in] dbName Combined with the outputDirectory parameter to form the full path name of the extracted file.
    //! @param[in] imodelFile Name (full path) of the input file.
    //! @param[in] overwriteExisting  Pass true to enable overwriting an existing file.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @remarks See BeSQLite::DbEmbeddedFileTable::ExportDbFile for information about the expected BeSQLite::DbResult error codes.
    //! @if BENTLEY_SDK_Publisher
    //!     @ingroup DgnDbGroup
    //! @endif
    DGNPLATFORM_EXPORT static DgnDbStatus Extract(BeSQLite::DbResult& dbResult, Utf8CP outputDirectory, Utf8CP dbName, BeFileNameCR imodelFile, bool overwriteExisting, BeSQLite::ICompressProgressTracker* progress = NULL);

    //! Extracts a DgnDb file from an imodel.
    //! @param[out] dbResult When ExtractUsingDefaults returns DgnDbStatus::SQLiteError, the actual DB error is stored in dbResult.
    //! @param[in] dgndbFile Name of the output file.
    //! @param[in] imodelFile Name of the input file.
    //! @param[in] overwriteExisting  Pass true to enable overwriting an existing file.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @remarks ExtractUsingDefaults uses the base file name and extension from dgndbFileName as the name to use when searching the embedded file table.
    //! If it gets a match, it uses that embedded file. If it does not get a match and the package file contains just one embedded DGNDB,
    //! it uses that embedded file.  Regardless of how it finds the embedded file to use, it uses the dgndbFile parameter as the output filename.
    //! @remarks See BeSQLite::ExtractDb for information about the expected BeSQLite::DbResult error codes.
    //! @see BeSQLite::DbEmbeddedFileTable
    //! @private
    DGNPLATFORM_EXPORT static DgnDbStatus ExtractUsingDefaults(BeSQLite::DbResult& dbResult, BeFileNameCR dgndbFile, BeFileNameCR imodelFile, bool overwriteExisting, BeSQLite::ICompressProgressTracker* progress = NULL);

    //! Creates a compressed file that contains a DgnDb database file.
    //! @param[in] outFile The name of the file to create.  The name is used exactly as specified. CreatePackageUsingDefaults does not add an extension or directory.
    //! @param[in] dgndbFile The name of the source file.
    //! @param[in] params Specifies parameters that control the creation of the output file.  Call params.SetOverwriteExisting(true) to enable overwriting an existing file.
    //! @remarks The compressed image is stored as a BeSQLite embedded file. Many of the properties of the compressed image are copied into the properties table
    //! of the output file making it possible to interrogate these properties without extracting the compressed file.
    //! @remarks Create deletes the output file if it encounters an error other than DgnDbStatus::FileAlreadyExists.  
    //! @remarks If Create returns DgnDbStatus::SharingViolation it means that the file specified via dgndbFile has a pending SQLite write or is open with DefaultTxn_Immediate
    //! or DefaultTxn_Exclusive transaction mode. 
    //! @remarks Create returns DgnDbStatus::CorruptFile if the input file if not a valid SQLite database or if there is a journal file for the DgnDb file.
    //! @remarks Create can also return DgnDbStatus::ReadError, DgnDbStatus::WriteError and any standard status values that DgnDb::OpenDgnDb returns.
    //! @see ExtractUsingDefaults, BeSQLite::DbEmbeddedFileTable
    //! @private
    DGNPLATFORM_EXPORT static BeSQLite::DbResult Create(BeFileNameCR outFile, BeFileNameCR dgndbFile, CreateIModelParams params);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
