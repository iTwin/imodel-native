/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnProject.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnProjectTables.h"
#include "DgnModel.h"
#include "DgnDomain.h"
#include <Bentley/BeFileName.h>

//__PUBLISH_SECTION_END__
#include "DgnCoreEvent.h"
//__PUBLISH_SECTION_START__

/** @addtogroup DgnProjectGroup

Classes for working with a DgnProject.

A DgnDb is a database that holds graphic and non-graphic data. A DgnProject object is used to access the database.
@ref DgnFileGroup are used to access data in DgnModels.

*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum ProjectSchemaValues : int
    {
    PROJECT_CURRENT_VERSION_Major = 5,
    PROJECT_CURRENT_VERSION_Minor = 4,
    PROJECT_CURRENT_VERSION_Sub1  = 0,
    PROJECT_CURRENT_VERSION_Sub2  = 0,

    PROJECT_SUPPORTED_VERSION_Major = 5,  // oldest version of the project schema supported by the current api
    PROJECT_SUPPORTED_VERSION_Minor = 2,
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum class DgnFileOpenMode
    {
    ReadOnly                    = 1,        //!< open readonly
    ReadWrite                   = 2,        //!< open for both read and write.
    PreferablyReadWrite         = 3,        //!< try read-write, if that doesn't work, try read-only
    ReadWriteFromCopiedFile     = 4
    };

typedef struct BeGuid BeProjectGuid;

//=======================================================================================
//! Specifies a set of DgnModels that are selected into a DgnView
//=======================================================================================
struct DgnModelSelection : NonCopyableClass
{
private:
    friend struct DgnModelSelectors;
    friend struct DgnProject;
    DgnProjectR             m_project;
    DgnModelSelectorId      m_id;
    bvector<DgnModelId>     m_explicitModels;

    typedef bvector<DgnModelId>::iterator ExplicitModelsIterator;
    ExplicitModelsIterator FindExplicitModel(DgnModelId);

public:
    DgnModelSelection (DgnProjectR project, DgnModelSelectorId id) : m_project(project), m_id(id) {}

    DgnModelSelectorId  GetId() const {return m_id;}
    DGNPLATFORM_EXPORT BentleyStatus AddExplicitModel(DgnModelId);
    DGNPLATFORM_EXPORT BentleyStatus DropExplicitModel(DgnModelId);
    DGNPLATFORM_EXPORT bool HasExplicitModel(DgnModelId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult Load();
    size_t GetExplictModelCount() const {return m_explicitModels.size();}

    typedef bvector<DgnModelId>::const_iterator const_iterator;
    typedef bvector<DgnModelId>::const_iterator iterator;
    DGNPLATFORM_EXPORT const_iterator begin() const;
    DGNPLATFORM_EXPORT const_iterator end() const;
};

/** @cond BENTLEY_SDK_Internal */

//=======================================================================================
//! Supplies the parameters necessary to create new DgnProjects.
// @bsiclass
//=======================================================================================
struct CreateProjectParams : BeSQLite::Db::CreateParams
{
public:
    bool    m_overwriteExisting;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_client;
    WString m_seedDb;
    BeProjectGuid m_guid;

    //! ctor for CreateProjectParams.
    //! @param[in] guid The BeProjectGuid to store in the newly created DgnProject. If invalid (the default), a new BeProjectGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateProjectParams (BeProjectGuid guid=BeProjectGuid()) : BeSQLite::Db::CreateParams(), m_guid(guid) {if (!m_guid.IsValid()) m_guid.Create(); }

    //! Set the value to be stored as the ProjectName property in the new DgnProject created using this CreateProjectParams/
    //! @note This value is stored as a property in the project. It does *not* refer to a file name.
    void SetProjectName(Utf8CP name) {m_name.AssignOrClear (name);}

    //! Set the value to be stored as the ProjectDescription property in the new DgnProject created using this CreateProjectParams.
    void SetProjectDescription (Utf8CP description) {m_description.AssignOrClear (description);}

    //! Set the value to be stored as the Client property in the new DgnProject created using this CreateProjectParams.
    void SetClient(Utf8CP client) {m_client = client;}

    //! Set the filename of an existing, valid, SQLite db to be used as the "seed database" for the new DgnProject created using this CreateProjectParams.
    //! If a SeedDb is specified, it is merely copied verbatim to the filename supplied to DgnProject::CreateProject. Then, the DgnProject
    //! tables are added to the copy of the seed database.
    //! @note The default is to create a new database from scratch (in other words, no seed database).
    //! @note When using a seed database, it determines the page size, encoding, compression, user_version, etc, for the database. Make sure they are appropriate
    //! for your needs.
    void SetSeedDb (BeFileNameCR seedDb) {m_seedDb = seedDb;}

    //! Get the BeProjectGuid to be stored in the newly created DgnProject. This is only necessary if you don't supply a valid BeProjectGuid
    //! to the ctor.
    BeProjectGuid GetGuid() const {return m_guid;}

    //! Determine whether to overwrite an existing file in DgnProject::CreateProject. The default is to fail if a file by the supplied name
    //! already exists.
    void SetOverwriteExisting (bool val) {m_overwriteExisting = val;}
};

//=======================================================================================
//! Supplies the parameters necessary to create new package file.
// @bsiclass
//=======================================================================================
struct CreatePackageParams : BeSQLite::Db::CreateParams
{
public:
    Utf8String m_packageFormat;
    Utf8String m_description;
    Utf8String m_client;
    BeDbGuid m_guid;
    UInt32  m_chunkSize;
    bool    m_overwriteExisting;

    //! ctor for CreateBoundProjectParams.
    //! @param[in] guid The BeProjectGuid to store in the newly created DgnProject. If invalid (the default), a new BeProjectGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreatePackageParams (BeProjectGuid guid=BeProjectGuid()) : m_guid(guid), m_overwriteExisting(false), m_chunkSize(1024 * 1024) {}

    //! Determine whether to overwrite an existing file in DgnProject::CreateProject. The default is to fail if a file by the supplied name
    //! already exists.
    void SetOverwriteExisting (bool val) {m_overwriteExisting = val;}

    //! Specifies the size of a block to be compressed and stored in a distinct blob in the package file.  It will be rounded up to an
    //! appropriate value.
    void SetChunkSize(UInt32 chunkSize) { m_chunkSize = chunkSize; }

    //! Get the BeProjectGuid to be stored in the newly created DgnProject. This is only necessary if you don't supply a valid BeProjectGuid
    //! to the ctor.
    BeDbGuid GetGuid() const {return m_guid;}

    //! Set the value to be stored as the BoundProjectFormat property in the new DgnProject created using this object.
    //! The value can be any string you want, and is not used for anything internally. Its purpose is to recognize DgnProjects created
    //! for a specific purpose.
    void SetPackageFormat (Utf8CP format) {m_packageFormat.assign (format);}
};
/** @endcond */

//=======================================================================================
//! A 4-digit number that specifies a serializable version of something in a DgnProject.
// @bsiclass
//=======================================================================================
struct DgnVersion : BeSQLite::SchemaVersion
{
    DgnVersion (UInt16 major, UInt16 minor, UInt16 sub1, UInt16 sub2) : SchemaVersion(major, minor, sub1, sub2) {}
    DgnVersion (Utf8CP val) : SchemaVersion(val){}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnProjectPackage
{
public:
    //! Extracts a DgnDb file from a package.
    //! @param[out] dbResult When Extract returns DGNFILE_ERROR_SQLiteError, the actual DB error is stored in dbResult.
    //! @param[in] outputDirectory Where to store the extracted DgnDb file.
    //! @param[in] dbName Combined with the outputDirectory parameter to form the full path name of the extracted file.
    //! @param[in] packageFile Name (full path) of the input file.
    //! @param[in] overwriteExisting  Pass true to enable overwriting an existing file.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @if BENTLEY_SDK_Publisher
    //!     @ingroup DgnProjectGroup
    //! @endif
    DGNPLATFORM_EXPORT static DgnFileStatus Extract (BeSQLite::DbResult& dbResult, Utf8CP outputDirectory, Utf8CP dbName, BeFileNameCR packageFile, bool overwriteExisting, BeSQLite::ICompressProgressTracker* progress = NULL);

    //! Extracts a DgnDb file from a package.
    //! @param[out] dbResult When ExtractUsingDefaults returns DGNFILE_ERROR_SQLiteError, the actual DB error is stored in dbResult.
    //! @param[in] dgndbFile Name of the output file.
    //! @param[in] packageFile Name of the input file.
    //! @param[in] overwriteExisting  Pass true to enable overwriting an existing file.
    //! @param[in] progress the interface to call to report progress.  May be NULL.
    //! @remarks ExtractUsingDefaults uses the base file name and extension from dgndbFileName as the name to use when searching the embedded file table.
    //! If it gets a match, it uses that embedded file. If it does not get a match and the package file contains just one embedded DGNDB,
    //! it uses that embedded file.  Regardless of how it finds the embedded file to use, it uses the dgndbFile parameter as the output filename.
    //! @see BeSQLite::DbEmbeddedFileTable
    //! @private
    DGNPLATFORM_EXPORT static DgnFileStatus ExtractUsingDefaults (BeSQLite::DbResult& dbResult, BeFileNameCR dgndbFile, BeFileNameCR packageFile, bool overwriteExisting, BeSQLite::ICompressProgressTracker* progress = NULL);

    //! Creates a compressed file that contains a DgnProject database file.
    //! @param[in] outFile The name of the file to create.  The name is used exactly as specified. CreatePackageUsingDefaults does not add an extension or directory.
    //! @param[in] dgndbFile The name of the source file.
    //! @param[in] params Specifies parameters that control the creation of the output file.  Call params.SetOverwriteExisting(true) to enable overwriting an existing file.
    //! @remarks The compressed image is stored as a BeSQLite embedded file. Many of the properties of the compressed image are copied into the properties table
    //! of the output file making it possible to interrogate these properties without extracting the compressed file.
    //! @see ExtractUsingDefaults, BeSQLite::DbEmbeddedFileTable
    //! @private
    DGNPLATFORM_EXPORT static DgnFileStatus CreatePackage (BeFileNameCR outFile, BeFileNameCR dgndbFile, CreatePackageParams params);
};

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass
//=======================================================================================
struct IElementLoadedEvent
{
virtual ~IElementLoadedEvent () {}
virtual void _OnElementLoaded (PersistentElementRefR) = 0;
};

//__PUBLISH_SECTION_START__
//=======================================================================================
//! A DgnProject is an in-memory object to access the information in a project database.
//! @ingroup DgnProjectGroup
// @bsiclass
//=======================================================================================
struct DgnProject : RefCounted<BeSQLite::EC::ECDb>
{
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   05/13
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams : BeSQLite::Db::OpenParams
    {
        explicit OpenParams(OpenMode openMode, BeSQLite::StartDefaultTransaction startDefaultTxn=BeSQLite::DefaultTxn_Yes) : Db::OpenParams(openMode, startDefaultTxn) {}
        virtual ~OpenParams() {}

        BeSQLite::DbResult UpgradeSchema(DgnProjectR) const;
        DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _DoUpgrade(DgnProjectR, DgnVersion& from) const;
        DGNPLATFORM_EXPORT virtual DgnFileStatus _DoDgnFileUpgrade(DgnProjectR, DgnVersion& from) const;
    };

    DEFINE_T_SUPER(BeSQLite::EC::ECDb)
//__PUBLISH_SECTION_END__
private:
    void Destroy();
    static DgnProjectPtr OpenProjectUnchecked (DgnFileStatus* status, BeFileNameCR filename, OpenParams const& openParams);


protected:
    friend struct ITxnManager;
    friend struct PersistentElementRef;
    friend struct DgnElementRef;

    DgnVersion    m_schemaVersion;
    DgnDomains    m_domains;
    DgnFonts      m_fonts;
    DgnColors     m_colors;
    DgnLevels     m_levels;
    DgnStyles     m_styles;
    DgnModels     m_models;
    DgnUnits      m_units;
    DgnStamps     m_stamps;
    StampQvElemMapP m_stampQvElemMap;
    BeFileName    m_fileName;
    ITxnManagerP  m_txnManager;

    EventHandlerList<IElementLoadedEvent>* m_elementLoadedListeners;

    BeSQLite::DbResult CreateVirtualDbTables();
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _VerifySchemaVersion (BeSQLite::Db::OpenParams const& params) override;
    DgnFileStatus CreateNewProject (BeFileNameCR boundFileName, CreateProjectParams const& params);

public:
    static void InitializeTableHandlers();
    BeSQLite::DbResult InitializeProject (CreateProjectParams const& params);
    BeSQLite::DbResult CreateProjectTables();
    DGNPLATFORM_EXPORT void Create3dRTree();

    DgnProject();
    virtual ~DgnProject();

    DgnFileStatus DoOpenProject (BeFileNameCR projectNameIn, OpenParams const&);
    DGNPLATFORM_EXPORT virtual void _OnDbClose() override;
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _OnDbCreated (CreateParams const& params) override;
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _OnDbOpened() override;

    BeSQLite::DbResult SaveProjectSchemaVersion(DgnVersion version=DgnVersion(PROJECT_CURRENT_VERSION_Major,PROJECT_CURRENT_VERSION_Minor,PROJECT_CURRENT_VERSION_Sub1,PROJECT_CURRENT_VERSION_Sub2));
    DGNPLATFORM_EXPORT DgnFileStatus LoadMyDgnFile(OpenParams const& openParams, bool upgradeIfNecessary=true);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT ITxnManagerR GetTxnManager();

    //! Get the file name for this DgnProject.
    //! @note The superclass method BeSQLite::Db::GetDbFileName may also be used to get the same value, as a Utf8CP.
    DGNPLATFORM_EXPORT BeFileNameCR GetFileName() const;

    //! Get the schema version of an opened DgnProject.
    DGNPLATFORM_EXPORT DgnVersion GetSchemaVersion();

    //! Open an existing DgnProject file.
    //! @param[out] status DGNFILE_STATUS_Success if the DgnProject file was successfully opened, error code otherwise. May be NULL.
    //! @param[in] filename The name of the BeSQLite::Db file from which the DgnProject is to be opened. Must be a valid filename on the local
    //! file system. It is not legal to open a DgnProject over a network share.
    //! @param[in] openParams Parameters for opening the database file
    //! @return a reference counted pointer to the opened DgnProject. Its IsValid() method will be false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnProjectPtr. The project will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    //! @note A DgnProject can have an expiration date. See #IsExpired
    DGNPLATFORM_EXPORT static DgnProjectPtr OpenProject (DgnFileStatus* status, BeFileNameCR filename, OpenParams const& openParams);

/** @cond BENTLEY_SDK_Publisher */
    //! Create and open a new DgnProject file.
    //! @param[out] status DGNFILE_STATUS_Success if the DgnProject file was successfully created, error code otherwise. May be NULL.
    //! @param[in] filename The name of the file for the new DgnProject. Must be a valid filename on the local
    //! files ystem. It is not legal to create a DgnProject over a network share.
    //! @param[in] params Parameters that control aspects of the newly created DgnProject
    //! @return a reference counted pointer to the newly created DgnProject. Its IsValid() method will be false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnProjectPtr. The project will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    DGNPLATFORM_EXPORT static DgnProjectPtr CreateProject (DgnFileStatus* status, BeFileNameCR filename, CreateProjectParams const& params);
/** @endcond */

    //! Get the next available (unused) value for a BeServerIssuedId for the supplied Table/Column.
    //! @param [in,out] value
    //! @param [in] tableName
    //! @param [in] columnName
    //! @param [in] minimumId
    DGNPLATFORM_EXPORT BeSQLite::DbResult GetNextServerIssuedId (BeServerIssuedId& value, Utf8CP tableName, Utf8CP columnName, UInt32 minimumId=1);

    DGNPLATFORM_EXPORT DgnModels& Models() const;             //!< Information about models for this DgnProject
    DGNPLATFORM_EXPORT DgnModelSelectors ModelSelectors();    //!< Information about ModelSelectors for this DgnProject
    DGNPLATFORM_EXPORT DgnViews Views();                      //!< Information about views for this DgnProject
    DGNPLATFORM_EXPORT DgnLevels& Levels() const;             //!< Information about levels for this DgnProject
    DGNPLATFORM_EXPORT DgnUnits& Units() const;               //!< Information about the units for this DgnProject.
    DGNPLATFORM_EXPORT DgnColors& Colors() const;             //!< Information about named and indexed colors for this DgnProject
    DGNPLATFORM_EXPORT DgnStyles& Styles() const;             //!< Information about styles for this DgnProject
    DGNPLATFORM_EXPORT DgnStamps& Stamps() const;             //!< Information about stamps for this DgnProject
    DGNPLATFORM_EXPORT DgnFonts& Fonts() const;               //!< Information about fonts for this DgnProject.
    DGNPLATFORM_EXPORT DgnMaterials Materials();              //!< Information about materials for this DgnProject
    DGNPLATFORM_EXPORT DgnProvenances Provenance();           //!< Information about provenance for this DgnProject.
    DGNPLATFORM_EXPORT DgnViewGeneratedDrawings ViewGeneratedDrawings();           //!< Information about provenance for this DgnProject.
    DGNPLATFORM_EXPORT struct DgnLinkTable DgnLinks();    //!< Information about DgnLinks for this DgnProject
    DGNPLATFORM_EXPORT DgnFileStatus CompactFile ();

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT DgnDomains& Domains() const;           //!< The DgnDomains associated with this DgnProject.
    DGNPLATFORM_EXPORT DgnSessions Sessions();                //!< The sessions table. @return DgnSessions for this DgnProject
    DGNPLATFORM_EXPORT DgnKeyStrings KeyStrings();            //!< The KeyStrings table. @return DgnKeyStrings for this DgnProject.
    StampQvElemMapR GetStampQvElemMap();                      //!< Get map for finding QvElem related to XGraphicsSymbolStamp, creating it if necessary
    StampQvElemMapP GetStampQvElemMapP();                     //!< Get map is it is already defined

    static DgnFileStatus ConvertFileNameStatusToPathNameStatus (BeFileNameStatus fstatus)
        {
        return (fstatus==BeFileNameStatus::UnknownError)? DGNPATHNAME_ERROR_UnknownError: (DgnFileStatus) (PATHNAME_ERROR_BASE+(UInt32)fstatus);
        }

    //! Notify listeners of element loaded from db event.
    void SendElementLoadedEvent (PersistentElementRefR elRef) const;

    //! Add element loaded from db event listener.
    DGNPLATFORM_EXPORT void AddListener (IElementLoadedEvent* listener);

    //! Drop element loaded from db event listener.
    DGNPLATFORM_EXPORT void DropListener (IElementLoadedEvent* listener);

//__PUBLISH_SECTION_START__
};

#if !defined (DOCUMENTATION_GENERATOR)
inline BeSQLite::DbResult DgnModels::QueryProperty (void* value, UInt32 size, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id) const {return m_project.QueryProperty(value, size, spec, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnModels::QueryProperty (Utf8StringR value, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id) const {return m_project.QueryProperty (value, spec, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnModels::QueryPropertySize (UInt32& size, DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id) const {return m_project.QueryPropertySize (size, spec, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnModels::SaveProperty (DgnModelId modelId, DgnModelPropertySpecCR spec, void const* value, UInt32 size, UInt64 id) {return m_project.SaveProperty (spec, value, size, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnModels::SavePropertyString (DgnModelId modelId, DgnModelPropertySpecCR spec, Utf8StringCR value, UInt64 id) {return m_project.SavePropertyString (spec, value, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnModels::DeleteProperty (DgnModelId modelId, DgnModelPropertySpecCR spec, UInt64 id) {return m_project.DeleteProperty (spec, modelId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::QueryProperty (void* value, UInt32 size, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id)const {return m_project.QueryProperty(value, size, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::QueryProperty (Utf8StringR value, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id) const{return m_project.QueryProperty (value, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::QueryPropertySize (UInt32& size, DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id) const{return m_project.QueryPropertySize (size, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::SaveProperty (DgnViewId viewId, DgnViewPropertySpecCR spec, void const* value, UInt32 size, UInt64 id) {return m_project.SaveProperty (spec, value, size, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::SavePropertyString (DgnViewId viewId, DgnViewPropertySpecCR spec, Utf8StringCR value, UInt64 id) {return m_project.SavePropertyString (spec, value, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::DeleteProperty (DgnViewId viewId, DgnViewPropertySpecCR spec, UInt64 id) {return m_project.DeleteProperty (spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::QueryProperty (void* value, UInt32 size, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id)const {return m_project.QueryProperty(value, size, spec, materialId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::QueryProperty (Utf8StringR value, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id) const{return m_project.QueryProperty (value, spec, materialId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::QueryPropertySize (UInt32& size, DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id) const{return m_project.QueryPropertySize (size, spec, materialId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::SaveProperty (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, void const* value, UInt32 size, UInt64 id) {return m_project.SaveProperty (spec, value, size, materialId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::SavePropertyString (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, Utf8StringCR value, UInt64 id) {return m_project.SavePropertyString (spec, value, materialId.GetValue(), id);}
inline BeSQLite::DbResult DgnMaterials::DeleteProperty (DgnMaterialId materialId, DgnMaterialPropertySpecCR spec, UInt64 id) {return m_project.DeleteProperty (spec, materialId.GetValue(), id);}

#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_DGNPLATFORM_NAMESPACE
