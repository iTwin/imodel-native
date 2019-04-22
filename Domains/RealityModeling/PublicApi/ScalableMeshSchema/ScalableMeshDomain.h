/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

//=======================================================================================
//! Domain concrete class for Conceptual data-stores
//=======================================================================================
struct SchemaUpdateScalableMeshDgnDbParams
{
public:
    BeFileName  m_assetsRootDir;
    Dgn::DgnDbPtr m_dgnDb;

    SchemaUpdateScalableMeshDgnDbParams(Dgn::DgnDbR dgnDb, BeFileNameCR assetsRootDir) : m_assetsRootDir(assetsRootDir), m_dgnDb(&dgnDb) {}

}; // SchemaUpdateScalableMeshDgnDbParams

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Mathieu.St-Pierre   02/16
//=======================================================================================
struct ScalableMeshDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ScalableMeshDomain, SCALABLEMESH_SCHEMA_EXPORT)

private: 

    WCharCP _GetSchemaRelativePath() const override { return BENTLEY_SCALABLEMESH_SCHEMA_PATH; }

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    
public:

    ScalableMeshDomain();

public:
    enum SchemaOperation { None, Import, MinorSchemaUpdate, MajorSchemaUpgrade, UnsupportedSchema, Undetermined };

    SCALABLEMESH_SCHEMA_EXPORT Dgn::DgnDbStatus UpdateSchema(SchemaUpdateScalableMeshDgnDbParams& params) const;

    static uint32_t GetExpectedSchemaVersionDigit1() { return 1; }
    static uint32_t GetExpectedSchemaVersionDigit2() { return 0; }
    static uint32_t GetExpectedSchemaVersionDigit3() { return 0; } // "01.00"

    SCALABLEMESH_SCHEMA_EXPORT SchemaOperation SchemaOperationNeeded(Dgn::DgnDbCR db) const;
};

END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
