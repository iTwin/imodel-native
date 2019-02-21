/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnProvenance.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#define DGN_TABLE_ProvenanceFile "dgn_ProvenanceFile"
#define DGN_TABLE_ProvenanceModel "dgn_ProvenanceModel"
#define DGN_TABLE_ProvenanceElement "dgn_ProvenanceElement"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Utility to create and query provenance of files originating from V8 dgn/.i.dgn-s
// @bsiclass                                                  Ramanujam.Raman   08/16
//=======================================================================================
struct DgnV8FileProvenance
{
private:
    static Utf8String ExtractEmbeddedFileName(Utf8StringCR v8Pathname);

public:
    //! Create the provenance table
    DGNPLATFORM_EXPORT static void CreateTable(DgnDbR dgndb);

    //! Insert an entry
    DGNPLATFORM_EXPORT static void Insert(BeSQLite::BeGuidCR fileId, Utf8StringCR v8Pathname, Utf8StringCR v8UniqueName, DgnDbR dgndb);

    //! Delete an entry
    DGNPLATFORM_EXPORT static void Delete(BeSQLite::BeGuidCR v8FileId, DgnDbR dgndb);

    //! Find an entry
    DGNPLATFORM_EXPORT static BentleyStatus Find(Utf8StringP v8Name, Utf8StringP v8UniqueName, BeSQLite::BeGuidCR v8FileId, DgnDbCR dgndb);

    //! Find an entry
    DGNPLATFORM_EXPORT static BentleyStatus FindFirst(BeSQLite::BeGuid* v8FileId, Utf8CP v8NameOrUniqueName, bool findByUniqueName, DgnDbCR dgndb);
};

//=======================================================================================
//! Utility to create and query provenance of models originating from V8 dgn/.i.dgn-s
// @bsiclass                                                  Ramanujam.Raman   08/16
//=======================================================================================
struct DgnV8ModelProvenance
    {
    //! Create the provenance table
    DGNPLATFORM_EXPORT static void CreateTable(DgnDbR dgndb);

    struct ModelProvenanceEntry
        {
        DgnModelId m_modelId;
        int        m_dgnv8ModelId;
        Utf8String m_modelName;
        Transform  m_trans;
        };

    //! Insert an entry
    DGNPLATFORM_EXPORT static void Insert(BeSQLite::BeGuidCR v8FileId, ModelProvenanceEntry const& entry, DgnDbR dgndb);

    //! Delete an entry
    DGNPLATFORM_EXPORT static void Delete(DgnModelId modelId, DgnDbR dgndb);

    //! Find an entry
    DGNPLATFORM_EXPORT static BentleyStatus FindFirst(BeSQLite::BeGuid* v8FileId, int* v8ModelId, Utf8StringP v8ModelName, DgnModelId modelId, DgnDbCR dgndb);

    //Find all the entries for a file
    DGNPLATFORM_EXPORT static BentleyStatus FindAll(bvector <ModelProvenanceEntry> &entries, BeSQLite::BeGuidCR v8FileId,  DgnDbCR dgndb);
    };

//=======================================================================================
//! Utility to create and query provenance of elements originating  originating from V8
// @bsiclass                                                  Ramanujam.Raman   08/16
//=======================================================================================
struct DgnV8ElementProvenance
    {
    //! Create the element provenance table
    DGNPLATFORM_EXPORT static void CreateTable(DgnDbR dgndb);

    //! Insert an entry
    DGNPLATFORM_EXPORT static void Insert(DgnElementId elementId, BeSQLite::BeGuidCR v8FileId, int v8ModelId, int64_t v8ElementId, DgnDbR dgndb);

    //! Delete an entry
    DGNPLATFORM_EXPORT static void Delete(DgnElementId elementId, DgnDbR dgndb);

    //! Find an entry
    DGNPLATFORM_EXPORT static BentleyStatus FindFirst(BeSQLite::BeGuid* v8FileId, int* v8ModelId, int64_t* v8ElementId, DgnElementId elementId, DgnDbCR dgndb);

    //! Find an entry
    DGNPLATFORM_EXPORT static BentleyStatus FindFirst(DgnElementId* elementId, BeSQLite::BeGuidCR v8FileId, int64_t v8ElementId, DgnDbCR dgndb);
    };

END_BENTLEY_DGN_NAMESPACE

