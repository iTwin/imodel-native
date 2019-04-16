/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#define SQLITE_HAS_CODEC 1

#include <BeSQLite/BeSQLite.h>
#include "sqlite3.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    01/18
//=======================================================================================
enum class FileHeaderSizeOf : uint32_t
{
    FormatSignature = 24,
    KeySource = sizeof(uint32_t),
    EncryptionAlgorithm = sizeof(uint32_t),
    ReservedFlags = sizeof(uint32_t),
    ExtraDataSize = sizeof(uint32_t),
    FixedSize = FormatSignature + KeySource + EncryptionAlgorithm + ReservedFlags + ExtraDataSize,
};

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    01/18
//=======================================================================================
enum class FileHeaderOffset : uint32_t
{
    FormatSignature = 0,
    KeySource = static_cast<uint32_t>(FileHeaderSizeOf::FormatSignature),
    EncryptionAlgorithm = KeySource + static_cast<uint32_t>(FileHeaderSizeOf::KeySource),
    ReservedFlags = EncryptionAlgorithm + static_cast<uint32_t>(FileHeaderSizeOf::EncryptionAlgorithm),
    ExtraDataSize = ReservedFlags + static_cast<uint32_t>(FileHeaderSizeOf::ReservedFlags),
    ExtraData = ExtraDataSize + static_cast<uint32_t>(FileHeaderSizeOf::ExtraDataSize),
};

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    01/18
//=======================================================================================
struct EncryptedDbVfs : sqlite3_vfs
{
    sqlite3_vfs* m_root; // points to the default vfs
};

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    01/18
//=======================================================================================
struct EncryptedDbVfsFile : sqlite3_file
{
    uint32_t m_keySource; // value that indicates how to retrieve the key
    uint32_t m_encryptionAlgorithm; // set to 0, means AES128-OFB 
    uint32_t m_reservedFlags; // set to 0, not currently used
    uint32_t m_extraDataSize; // number of bytes of extra data to follow (beyond the fixed part of the header)
    uint32_t m_totalHeaderSize; // sizeof fixed header + extra data

    void InitTotalHeaderSize() {m_totalHeaderSize = static_cast<uint32_t>(FileHeaderSizeOf::FixedSize) + m_extraDataSize;}
    uint32_t GetTotalHeaderSize() const {BeAssert(m_totalHeaderSize >= static_cast<uint32_t>(FileHeaderSizeOf::FixedSize)); return m_totalHeaderSize;}

    void SetKeySource(EncryptionKeySource keySource) {m_keySource = static_cast<uint32_t>(keySource);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static sqlite3_file* toRootVfsFile(sqlite3_file* inFile) {return (sqlite3_file*)((Byte*)inFile+sizeof(EncryptedDbVfsFile));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xRead(sqlite3_file* inFile, void* buffer, int amount, sqlite_int64 offset)
    {
    EncryptedDbVfsFile* encryptedDbVfsFile = (EncryptedDbVfsFile*) inFile;
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xRead(pFile, buffer, amount, offset + encryptedDbVfsFile->GetTotalHeaderSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xWrite(sqlite3_file* inFile, const void* buffer, int amount, sqlite3_int64 offset)
    {
    EncryptedDbVfsFile* encryptedDbVfsFile = (EncryptedDbVfsFile*) inFile;
    sqlite3_file* pFile = toRootVfsFile(inFile);

    if (0 == offset)
        {
        static_assert(static_cast<uint32_t>(FileHeaderSizeOf::FixedSize) == static_cast<uint32_t>(FileHeaderOffset::ExtraData), "FileHeaderSizeOf::FixedSize must equal FileHeaderOffset::ExtraData");
        static_assert(sizeof(ENCRYPTED_BESQLITE_FORMAT_SIGNATURE) <= static_cast<uint32_t>(FileHeaderSizeOf::FormatSignature), "ENCRYPTED_BESQLITE_FORMAT_SIGNATURE must be <= RESERVED_SIZEOF_FormatSignature bytes");
        pFile->pMethods->xWrite(pFile, ENCRYPTED_BESQLITE_FORMAT_SIGNATURE, static_cast<int>(FileHeaderSizeOf::FormatSignature), static_cast<sqlite3_int64>(FileHeaderOffset::FormatSignature));
        pFile->pMethods->xWrite(pFile, &encryptedDbVfsFile->m_keySource, static_cast<int>(FileHeaderSizeOf::KeySource), static_cast<sqlite3_int64>(FileHeaderOffset::KeySource));
        pFile->pMethods->xWrite(pFile, &encryptedDbVfsFile->m_encryptionAlgorithm, static_cast<int>(FileHeaderSizeOf::EncryptionAlgorithm), static_cast<sqlite3_int64>(FileHeaderOffset::EncryptionAlgorithm));
        pFile->pMethods->xWrite(pFile, &encryptedDbVfsFile->m_reservedFlags, static_cast<int>(FileHeaderSizeOf::ReservedFlags), static_cast<sqlite3_int64>(FileHeaderOffset::ReservedFlags));
        pFile->pMethods->xWrite(pFile, &encryptedDbVfsFile->m_extraDataSize, static_cast<int>(FileHeaderSizeOf::ExtraDataSize), static_cast<sqlite3_int64>(FileHeaderOffset::ExtraDataSize));
        }

    return pFile->pMethods->xWrite(pFile, buffer, amount, offset + encryptedDbVfsFile->GetTotalHeaderSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xClose(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xClose(pFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xFileSize(sqlite3_file* inFile, sqlite3_int64* fileSize)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xFileSize(pFile, fileSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xTruncate(sqlite3_file* inFile, sqlite3_int64 size)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xTruncate(pFile, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xSync(sqlite3_file* inFile, int flags)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xSync(pFile, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xLock(sqlite3_file* inFile, int lock)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xLock(pFile, lock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xUnlock(sqlite3_file* inFile, int lock)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xUnlock(pFile, lock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xCheckReservedLock(sqlite3_file* inFile, int *reservedLock)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xCheckReservedLock(pFile, reservedLock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xFileControl(sqlite3_file* inFile, int op, void *arg)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xFileControl(pFile, op, arg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xSectorSize(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xSectorSize(pFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xDeviceCharacteristics(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xDeviceCharacteristics(pFile);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xShmMap(sqlite3_file* inFile, int page, int pageSize, int isWrite, void volatile** pp)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xShmMap(pFile, page, pageSize, isWrite, pp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xShmLock(sqlite3_file* inFile, int offset, int n, int flags)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xShmLock(pFile, offset, n, flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void encryptedDbVfs_xShmBarrier(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xShmBarrier(pFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xShmUnmap(sqlite3_file* inFile, int deleteFlag)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xShmUnmap(pFile, deleteFlag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xFetch(sqlite3_file* inFile, sqlite3_int64 offset, int amount, void **pp)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xFetch(pFile, offset, amount, pp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xUnfetch(sqlite3_file* inFile, sqlite3_int64 offset, void *p)
    {
    sqlite3_file* pFile = toRootVfsFile(inFile);
    return pFile->pMethods->xUnfetch(pFile, offset, p);
    }

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    01/18
//=======================================================================================
static sqlite3_io_methods encryptedDbVfs_io_methods = 
{
  3, // version
  encryptedDbVfs_xClose,
  encryptedDbVfs_xRead,
  encryptedDbVfs_xWrite,
  encryptedDbVfs_xTruncate,
  encryptedDbVfs_xSync,
  encryptedDbVfs_xFileSize,
  encryptedDbVfs_xLock,
  encryptedDbVfs_xUnlock,
  encryptedDbVfs_xCheckReservedLock,
  encryptedDbVfs_xFileControl,
  encryptedDbVfs_xSectorSize,
  encryptedDbVfs_xDeviceCharacteristics,
  encryptedDbVfs_xShmMap,
  encryptedDbVfs_xShmLock,
  encryptedDbVfs_xShmBarrier,
  encryptedDbVfs_xShmUnmap,
  encryptedDbVfs_xFetch,
  encryptedDbVfs_xUnfetch
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int encryptedDbVfs_xOpen(sqlite3_vfs* vfs, Utf8CP name, sqlite3_file* inFile, int flags, int *outFlags)
    {
    EncryptedDbVfs* encryptedDbVfs = (EncryptedDbVfs*) vfs;
    sqlite3_vfs* rootVfs = encryptedDbVfs->m_root;

    // Don't do anything special if not opening the main database
    if (0 == (flags & SQLITE_OPEN_MAIN_DB))
        return rootVfs->xOpen(rootVfs, name, inFile, flags, outFlags);

    EncryptedDbVfsFile* encryptedDbVfsFile = (EncryptedDbVfsFile*) inFile;
    encryptedDbVfsFile->pMethods = &encryptedDbVfs_io_methods;
    sqlite3_file* pFile = toRootVfsFile(inFile);

    if (flags & SQLITE_OPEN_CREATE)
        {
        // NOTE: Hard-coded for now, but could use sqlite3_uri_parameter to get parameters from the name (URI)
        encryptedDbVfsFile->SetKeySource(EncryptionKeySource::NotSpecified);
        encryptedDbVfsFile->m_encryptionAlgorithm = 0;
        encryptedDbVfsFile->m_reservedFlags = 0;
        encryptedDbVfsFile->m_extraDataSize = 0;
        encryptedDbVfsFile->InitTotalHeaderSize();
        return rootVfs->xOpen(rootVfs, name, pFile, flags, outFlags);
        }

    if (SQLITE_OK != rootVfs->xOpen(rootVfs, name, pFile, flags, outFlags))
        return SQLITE_CANTOPEN;

    // For an existing database, read the encryption data from the file
    if ((SQLITE_OK != pFile->pMethods->xRead(pFile, &encryptedDbVfsFile->m_keySource, static_cast<int>(FileHeaderSizeOf::KeySource), static_cast<sqlite3_int64>(FileHeaderOffset::KeySource))) || 
        (SQLITE_OK != pFile->pMethods->xRead(pFile, &encryptedDbVfsFile->m_encryptionAlgorithm, static_cast<int>(FileHeaderSizeOf::EncryptionAlgorithm), static_cast<sqlite3_int64>(FileHeaderOffset::EncryptionAlgorithm))) ||
        (SQLITE_OK != pFile->pMethods->xRead(pFile, &encryptedDbVfsFile->m_reservedFlags, static_cast<int>(FileHeaderSizeOf::ReservedFlags), static_cast<sqlite3_int64>(FileHeaderOffset::ReservedFlags))) ||
        (SQLITE_OK != pFile->pMethods->xRead(pFile, &encryptedDbVfsFile->m_extraDataSize, static_cast<int>(FileHeaderSizeOf::ExtraDataSize), static_cast<sqlite3_int64>(FileHeaderOffset::ExtraDataSize))))
        return SQLITE_CORRUPT;

    encryptedDbVfsFile->InitTotalHeaderSize();
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static int registerEncryptedDbVfs(Utf8CP vfsName)
    {
    sqlite3_vfs* rootVfs = sqlite3_vfs_find(NULL); // this will be the platform-specific default vfs
    if (NULL == rootVfs)
        return SQLITE_NOTFOUND;

    int vfsNameLength = (int) strlen(vfsName) + 1;
    int vfsNumBytes = sizeof(EncryptedDbVfs) + vfsNameLength;
    EncryptedDbVfs* encryptedDbVfs = (EncryptedDbVfs*) sqlite3_malloc(vfsNumBytes);
    if (!encryptedDbVfs) return SQLITE_NOMEM;
    memset(encryptedDbVfs, 0, vfsNumBytes);

    encryptedDbVfs->m_root = rootVfs;
    encryptedDbVfs->xOpen  = encryptedDbVfs_xOpen;
    encryptedDbVfs->szOsFile = rootVfs->szOsFile + sizeof(EncryptedDbVfsFile); // see note about SQLite subclassing at toRootVfsFile
    encryptedDbVfs->zName = (char*) &encryptedDbVfs[1]; // to save a malloc
    memcpy (const_cast<char*>(encryptedDbVfs->zName), vfsName, vfsNameLength);
    encryptedDbVfs->pAppData = nullptr;
    encryptedDbVfs->iVersion = rootVfs->iVersion;
    encryptedDbVfs->mxPathname = rootVfs->mxPathname;
    encryptedDbVfs->xDelete = rootVfs->xDelete;
    encryptedDbVfs->xAccess = rootVfs->xAccess;
    encryptedDbVfs->xFullPathname = rootVfs->xFullPathname;
    encryptedDbVfs->xDlOpen = rootVfs->xDlOpen;
    encryptedDbVfs->xDlError = rootVfs->xDlError;
    encryptedDbVfs->xDlSym = rootVfs->xDlSym;
    encryptedDbVfs->xDlClose = rootVfs->xDlClose;
    encryptedDbVfs->xRandomness = rootVfs->xRandomness;
    encryptedDbVfs->xSleep = rootVfs->xSleep;
    encryptedDbVfs->xCurrentTime = rootVfs->xCurrentTime;
    encryptedDbVfs->xGetLastError = rootVfs->xGetLastError;
    encryptedDbVfs->xCurrentTimeInt64 = rootVfs->xCurrentTimeInt64;
    encryptedDbVfs->xSetSystemCall = rootVfs->xSetSystemCall;
    encryptedDbVfs->xGetSystemCall = rootVfs->xGetSystemCall;
    encryptedDbVfs->xNextSystemCall = rootVfs->xNextSystemCall;
    return sqlite3_vfs_register(encryptedDbVfs, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP loadEncryptedDbVfs()
    {
    static Utf8CP s_encryptedDbVfs = "encryptedDb";
    if (0 == sqlite3_vfs_find(s_encryptedDbVfs))
        registerEncryptedDbVfs(s_encryptedDbVfs);

    return s_encryptedDbVfs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Db::IsEncryptedDb(BeFileNameCR dbFileName)
    {
    BeFile dbFile; // destructor will close file
    BeFileStatus fileStatus = dbFile.Open(dbFileName.GetName(), BeFileAccess::Read);
    if (BeFileStatus::Success != fileStatus)
        return false;

    Utf8Char formatSignature[static_cast<uint32_t>(FileHeaderSizeOf::FormatSignature) + 1];
    uint32_t bytesRead;
    fileStatus = dbFile.Read(&formatSignature, &bytesRead, sizeof(formatSignature) - 1);
    if ((BeFileStatus::Success != fileStatus) || (bytesRead < sizeof(ENCRYPTED_BESQLITE_FORMAT_SIGNATURE)))
        return false;

    // Silence the static analyzer.
    formatSignature[sizeof(formatSignature) - 1] = 0;

    return 0 == strcmp(ENCRYPTED_BESQLITE_FORMAT_SIGNATURE, formatSignature);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::EncryptDb(BeFileNameCR originalFileName, EncryptionParams const& params)
    {
    if (!originalFileName.DoesPathExist())
        return BE_SQLITE_CANTOPEN;

    if (!params.HasKey() || IsEncryptedDb(originalFileName)) // re-encrypt not supported
        return BE_SQLITE_MISUSE;

    // encrypt a copy so original will be left alone in case of error
    BeFileName encryptedFileName = originalFileName;
    encryptedFileName.AppendExtension(L"enc_temp");
    BeFile encryptedFile;

    // write to new file so original will be left alone in case of error
    BeFileName newFileName = originalFileName;
    newFileName.AppendExtension(L"new_temp");
    BeFile newFile;

    if ((BeFileStatus::Success != newFile.Create(newFileName.GetName())) ||
        (BeFileNameStatus::Success != BeFileName::BeCopyFile(originalFileName, encryptedFileName)) ||
        (BeFileStatus::Success != encryptedFile.Open(encryptedFileName.GetName(), BeFileAccess::Read)))
        return BE_SQLITE_IOERR;

    Db db;
    DbResult rc;
    Db::OpenParams openParams(Db::OpenMode::ReadWrite);
    rc = db.OpenBeSQLiteDb(encryptedFileName, openParams);
    if (BE_SQLITE_OK == rc)
        rc = (DbResult) sqlite3_rekey(db.GetSqlDb(), params.GetKey(), params.GetKeySize());

    db.CloseDb();
    if (BE_SQLITE_OK != rc)
        {
        BeFileName::BeDeleteFile(encryptedFileName.GetName());
        return rc;
        }

    Utf8Char formatSignature[static_cast<uint32_t>(FileHeaderSizeOf::FormatSignature)];
    memset(formatSignature, 0, sizeof(formatSignature));
    strcpy(formatSignature, ENCRYPTED_BESQLITE_FORMAT_SIGNATURE);
    uint32_t bytesWritten = 0;
    uint32_t extraDataSize = params.GetExtraDataSize();

    if ((BeFileStatus::Success != newFile.Write(&bytesWritten, formatSignature, static_cast<uint32_t>(FileHeaderSizeOf::FormatSignature))) ||
        (BeFileStatus::Success != newFile.Write(&bytesWritten, &params.m_keySource, static_cast<uint32_t>(FileHeaderSizeOf::KeySource))) ||
        (BeFileStatus::Success != newFile.Write(&bytesWritten, &params.m_encryptionAlgorithm, static_cast<uint32_t>(FileHeaderSizeOf::EncryptionAlgorithm))) ||
        (BeFileStatus::Success != newFile.Write(&bytesWritten, &params.m_reservedFlags, static_cast<uint32_t>(FileHeaderSizeOf::ReservedFlags))) ||
        (BeFileStatus::Success != newFile.Write(&bytesWritten, &extraDataSize, static_cast<uint32_t>(FileHeaderSizeOf::ExtraDataSize))))
        return BE_SQLITE_IOERR;

    if (extraDataSize > 0)
        {
        if (BeFileStatus::Success != newFile.Write(&bytesWritten, params.m_extraData.data(), extraDataSize))
            return BE_SQLITE_IOERR;
        }

    uint32_t bytesRead;
    const uint32_t bufferSize = 16384;
    bvector<Byte> buffer(bufferSize);

    do
        {
        encryptedFile.Read(buffer.data(), &bytesRead, bufferSize);
        if (bytesRead > 0)
            newFile.Write(&bytesWritten, buffer.data(), bytesRead);
        }
    while ((bytesRead > 0) && (bytesWritten == bytesRead));

    encryptedFile.Close();
    newFile.Close();

    BeFileName::BeDeleteFile(encryptedFileName.GetName());
    BeFileNameStatus copyStatus = BeFileName::BeCopyFile(newFileName, originalFileName);
    BeFileName::BeDeleteFile(newFileName.GetName());
    return BeFileNameStatus::Success == copyStatus ? BE_SQLITE_OK : BE_SQLITE_IOERR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Db::PasswordProtectDb(BeFileNameCR originalFileName, Utf8CP password)
    {
    if (!password)
        return BE_SQLITE_MISUSE;

    Db::EncryptionParams encryptionParams;
    encryptionParams.SetPassword(password);
    return EncryptDb(originalFileName, encryptionParams);
    }
