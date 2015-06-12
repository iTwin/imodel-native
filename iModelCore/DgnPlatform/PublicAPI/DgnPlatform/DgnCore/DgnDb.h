/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnDb.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"
#include "DgnModel.h"
#include "DgnDomain.h"
#include <Bentley/BeFileName.h>

/** @addtogroup DgnDbGroup

Classes for creating and opening a DgnDb.

A DgnDb is a database that holds graphic and non-graphic data. A DgnDb object is used to access the database.

@ref PAGE_DgnPlatform
*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum DgnDbSchemaValues : int32_t
    {
    DGNDB_CURRENT_VERSION_Major = 5,
    DGNDB_CURRENT_VERSION_Minor = 4,
    DGNDB_CURRENT_VERSION_Sub1  = 0,
    DGNDB_CURRENT_VERSION_Sub2  = 0,

    DGNDB_SUPPORTED_VERSION_Major = 5,  // oldest version of the schema supported by the current api
    DGNDB_SUPPORTED_VERSION_Minor = 2,
    };

/** @cond BENTLEY_SDK_Internal */

//=======================================================================================
//! Supplies the parameters necessary to create new DgnDbs.
// @bsiclass
//=======================================================================================
struct CreateDgnDbParams : BeSQLite::Db::CreateParams
{
public:
    bool            m_overwriteExisting;
    Utf8String      m_name;
    Utf8String      m_description;
    Utf8String      m_client;
    BeFileName      m_seedDb;
    BeDbGuid        m_guid;

    //! ctor for CreateDgnDbParams.
    //! @param[in] guid The BeDbGuid to store in the newly created DgnDb. If invalid (the default), a new BeDbGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateDgnDbParams(BeDbGuid guid=BeDbGuid()) : BeSQLite::Db::CreateParams(), m_guid(guid) {if (!m_guid.IsValid()) m_guid.Create(); }

    //! Set the value to be stored as the ProjectName property in the new DgnDb created using this CreateDgnDbParams/
    //! @note This value is stored as a property in the project. It does *not* refer to a file name.
    void SetProjectName(Utf8CP name) {m_name.AssignOrClear(name);}

    //! Set the value to be stored as the ProjectDescription property in the new DgnDb created using this CreateDgnDbParams.
    void SetProjectDescription(Utf8CP description) {m_description.AssignOrClear(description);}

    //! Set the value to be stored as the Client property in the new DgnDb created using this CreateDgnDbParams.
    void SetClient(Utf8CP client) {m_client = client;}

    //! Set the filename of an existing, valid, SQLite db to be used as the "seed database" for the new DgnDb created using this CreateDgnDbParams.
    //! If a SeedDb is specified, it is merely copied verbatim to the filename supplied to DgnDb::CreateDgnDb. Then, the DgnDb
    //! tables are added to the copy of the seed database.
    //! @note The default is to create a new database from scratch (in other words, no seed database).
    //! @note When using a seed database, it determines the page size, encoding, compression, user_version, etc, for the database. Make sure they are appropriate
    //! for your needs.
    void SetSeedDb(BeFileNameCR seedDb) {m_seedDb = seedDb;}

    //! Get the BeDbGuid to be stored in the newly created DgnDb. This is only necessary if you don't supply a valid BeDbGuid
    //! to the ctor.
    BeDbGuid GetGuid() const {return m_guid;}

    //! Determine whether to overwrite an existing file in DgnDb::CreateDgnDb. The default is to fail if a file by the supplied name
    //! already exists.
    void SetOverwriteExisting(bool val) {m_overwriteExisting = val;}
};

/** @endcond */

//=======================================================================================
//! A 4-digit number that specifies a serializable version of something in a DgnDb.
// @bsiclass
//=======================================================================================
struct DgnVersion : BeSQLite::SchemaVersion
{
    DgnVersion(uint16_t major, uint16_t minor, uint16_t sub1, uint16_t sub2) : SchemaVersion(major, minor, sub1, sub2) {}
    DgnVersion(Utf8CP val) : SchemaVersion(val){}
};

//=======================================================================================
//! A DgnDb is an in-memory object to access the information in a .dgndb database.
//! @ingroup DgnDbGroup
// @bsiclass
//=======================================================================================
struct DgnDb : RefCounted<BeSQLite::EC::ECDb>
{
    DEFINE_T_SUPER(BeSQLite::EC::ECDb)

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   05/13
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE OpenParams : BeSQLite::Db::OpenParams
    {
        explicit OpenParams(OpenMode openMode, BeSQLite::StartDefaultTransaction startDefaultTxn=BeSQLite::DefaultTxn_Yes) : Db::OpenParams(openMode, startDefaultTxn) {}
        virtual ~OpenParams() {}

        BeSQLite::DbResult UpgradeSchema(DgnDbR) const;
        DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _DoUpgrade(DgnDbR, DgnVersion& from) const;
    };

private:
    void Destroy();

protected:
    friend struct Txns;
    friend struct DgnElement;

    Utf8String      m_fileName;
    DgnElements     m_elements;
    DgnModels       m_models;
    DgnVersion      m_schemaVersion;
    DgnDomains      m_domains;
    DgnFonts        m_fonts;
    DgnColors       m_colors;
    DgnCategories   m_categories;
    DgnStyles       m_styles;
    DgnUnits        m_units;
    DgnViews        m_views;
    DgnGeomParts    m_geomParts;
    DgnMaterials    m_materials;
    DgnLinks        m_links;
    TxnManagerPtr   m_txnManager;

    BeSQLite::EC::ECSqlStatementCache m_ecsqlCache;

    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _VerifySchemaVersion(BeSQLite::Db::OpenParams const& params) override;
    BeSQLite::DbResult CreateNewDgnDb(BeFileNameCR boundFileName, CreateDgnDbParams const& params);

public:
    BeSQLite::DbResult InitializeDgnDb(CreateDgnDbParams const& params);
    BeSQLite::DbResult CreateProjectTables();

    DgnDb();
    virtual ~DgnDb();

    BeSQLite::DbResult DoOpenDgnDb(BeFileNameCR projectNameIn, OpenParams const&);
    DGNPLATFORM_EXPORT virtual void _OnDbClose() override;
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _OnDbOpened() override;

    BeSQLite::DbResult SaveDgnDbSchemaVersion(DgnVersion version=DgnVersion(DGNDB_CURRENT_VERSION_Major,DGNDB_CURRENT_VERSION_Minor,DGNDB_CURRENT_VERSION_Sub1,DGNDB_CURRENT_VERSION_Sub2));

    //! Get ae BeFileName for this DgnDb.
    //! @note The superclass method BeSQLite::Db::GetDbFileName may also be used to get the same value, as a Utf8CP.
    BeFileName GetFileName() const {return BeFileName(m_fileName);}

    //! Get the schema version of an opened DgnDb.
    DGNPLATFORM_EXPORT DgnVersion GetSchemaVersion();

    //! Open an existing DgnDb file.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully opened, error code otherwise. May be NULL.
    //! @param[in] filename The name of the BeSQLite::Db file from which the DgnDb is to be opened. Must be a valid filename on the local
    //! file system. It is not legal to open a DgnDb over a network share.
    //! @param[in] openParams Parameters for opening the database file
    //! @return a reference counted pointer to the opened DgnDb. Its IsValid() method will be false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnDbPtr. The project will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    //! @note A DgnDb can have an expiration date. See Db::IsExpired
    DGNPLATFORM_EXPORT static DgnDbPtr OpenDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, OpenParams const& openParams);

    //! Create and open a new DgnDb file.
    //! @param[out] status BE_SQLITE_OK if the DgnDb file was successfully created, error code otherwise. May be NULL.
    //! @param[in] filename The name of the file for the new DgnDb. Must be a valid filename on the local
    //! filesystem. It is not legal to create a DgnDb over a network share.
    //! @param[in] params Parameters that control aspects of the newly created DgnDb
    //! @return a reference counted pointer to the newly created DgnDb. Its IsValid() method will return false if the open failed for any reason.
    //! @note If this method succeeds, it will return a valid DgnDbPtr. The DgnDb will be automatically closed when the last reference
    //! to it is released. There is no way to hold a pointer to a "closed project".
    DGNPLATFORM_EXPORT static DgnDbPtr CreateDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, CreateDgnDbParams const& params);

    DGNPLATFORM_EXPORT DgnModels& Models() const;                   //!< Information about models for this DgnDb
    DGNPLATFORM_EXPORT DgnElements& Elements() const;               //!< Information about graphic elements for this DgnDb
    DGNPLATFORM_EXPORT DgnViews& Views() const;                     //!< Information about views for this DgnDb
    DGNPLATFORM_EXPORT DgnCategories& Categories() const;           //!< Information about categories for this DgnDb
    DGNPLATFORM_EXPORT DgnUnits& Units() const;                     //!< Information about the units for this DgnDb
    DGNPLATFORM_EXPORT DgnColors& Colors() const;                   //!< Information about named and indexed colors for this DgnDb
    DGNPLATFORM_EXPORT DgnStyles& Styles() const;                   //!< Information about styles for this DgnDb
    DGNPLATFORM_EXPORT DgnGeomParts& GeomParts() const;             //!< Information about the geometry parts for this DgnDb
    DGNPLATFORM_EXPORT DgnFonts& Fonts() const;                     //!< Information about fonts for this DgnDb
    DGNPLATFORM_EXPORT DgnLinks& Links() const;                     //!< Information about DgnLinks for this DgnDb
    DGNPLATFORM_EXPORT DgnDomains& Domains() const;                 //!< The DgnDomains associated with this DgnDb.
    DGNPLATFORM_EXPORT TxnManagerR Txns();

    DGNPLATFORM_EXPORT DgnFileStatus CompactFile();

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT DgnMaterials& Materials() const;       //!< Information about materials for this DgnDb

//__PUBLISH_SECTION_START__

    //! Gets a cached and prepared ECSqlStatement.
    //! @remarks This is only to be used by DgnPlatform internally to avoid misuse and unexpected caching behavior.
    //! Therefore it is not inlined.
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetPreparedECSqlStatement(Utf8CP ecsql) const;
};

#if !defined (DOCUMENTATION_GENERATOR)
inline BeSQLite::DbResult DgnViews::QueryProperty(void* value, uint32_t size, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id)const {return m_dgndb.QueryProperty(value, size, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::QueryProperty(Utf8StringR value, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id) const{return m_dgndb.QueryProperty(value, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::QueryPropertySize(uint32_t& size, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id) const{return m_dgndb.QueryPropertySize(size, spec, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::SaveProperty(DgnViewId viewId, DgnViewPropertySpecCR spec, void const* value, uint32_t size, uint64_t id) {return m_dgndb.SaveProperty(spec, value, size, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::SavePropertyString(DgnViewId viewId, DgnViewPropertySpecCR spec, Utf8StringCR value, uint64_t id) {return m_dgndb.SavePropertyString(spec, value, viewId.GetValue(), id);}
inline BeSQLite::DbResult DgnViews::DeleteProperty(DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id) {return m_dgndb.DeleteProperty(spec, viewId.GetValue(), id);}

#endif // DOCUMENTATION_GENERATOR

END_BENTLEY_DGNPLATFORM_NAMESPACE
