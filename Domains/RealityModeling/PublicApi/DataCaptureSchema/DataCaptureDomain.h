/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/DataCaptureDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//=======================================================================================
//! Domain concrete class for data-stores
//=======================================================================================
struct SchemaUpdateDataCaptureDgnDbParams
    {
    public:
        BeFileName    m_assetsRootDir;
        Dgn::DgnDbPtr m_dgnDb;

        SchemaUpdateDataCaptureDgnDbParams(Dgn::DgnDbR dgnDb, BeFileNameCR assetsRootDir) : m_assetsRootDir(assetsRootDir), m_dgnDb(&dgnDb) {}

    }; // SchemaUpdateDataCaptureDgnDbParams


//=======================================================================================
//! The DgnDomain for the DataCapture schema.
//! @ingroup GROUP_DataCapture
//=======================================================================================
struct DataCaptureDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DataCaptureDomain, DATACAPTURE_EXPORT)

private:
    static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbCR, Utf8CP);
    static void InsertCategory(Dgn::DgnDbR, Dgn::ColorDef const&, int const, Utf8CP);

protected:
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;


public:
    DataCaptureDomain();
	enum SchemaOperation { None, Import, MinorSchemaUpdate, MajorSchemaUpgrade, UnsupportedSchema, Undetermined };

    DATACAPTURE_EXPORT static Dgn::DgnAuthorityId QueryDataCaptureAuthorityId(Dgn::DgnDbCR dgndb);
    DATACAPTURE_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    //! Format a BeBriefcaseBasedId as BeBriefcaseId-LocalId 
    DATACAPTURE_EXPORT static Utf8String FormatId(BeSQLite::BeBriefcaseBasedId id);

    //! Build a default name from the specified prefix and BeBriefcaseBasedId
    DATACAPTURE_EXPORT static Utf8String BuildDefaultName(Utf8CP prefix, BeSQLite::BeBriefcaseBasedId id);

    DATACAPTURE_EXPORT Dgn::DgnDbStatus UpdateSchema(SchemaUpdateDataCaptureDgnDbParams& params) const;
	DATACAPTURE_EXPORT SchemaOperation SchemaOperationNeeded(Dgn::DgnDbCR db) const;

    static uint32_t GetExpectedSchemaVersionDigit1() { return 1; }
    static uint32_t GetExpectedSchemaVersionDigit2() { return 0; }
    static uint32_t GetExpectedSchemaVersionDigit3() { return 0; } // "01.00"
    }; // DataCaptureDomain

END_BENTLEY_DATACAPTURE_NAMESPACE
