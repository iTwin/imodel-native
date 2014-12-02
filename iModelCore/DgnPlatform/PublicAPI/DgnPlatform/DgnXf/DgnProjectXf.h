/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnXf/DgnProjectXf.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnXfLib/DgnXfLib.h>
#include <DgnPlatform/DgnCore/PropertyContext.h>
#include <DgnPlatform/DgnCore/DgnProjectTables.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnProjectFromDgnXfMessageProcessor;

/** @addtogroup DgnXf Importing and Exporting Data in DgnXf Format
*
* DgnXf is a format for capturing, storing, and transmitting engineering graphics. The DgnXf format can be used to export data from a DgnDb 
* project to an external process or storage medium. DgnXf can also be used to import data into a DgnDb project
* from an external process or storage medium. DgnXf is not a file format. It is a binary
* data format. Data in DgnXf format can be written to a file, or stored in a database, 
* or transmitted to a remote process. In a similar way, data in DgnXf format can be read from an external
* source and then processed. DgnXf data is typically processed by classes that implement the DgnXfLib::IMessageProcessor interface.
* An importer or exporter obtains data in DgnXf format from some source and then passes it to an implementation of IMessageProcessor to process it.
* 
* The key DgnDb import and export classes are:
* * DgnDbToDgnXf
* * DgnProjectFromDgnXfMessageProcessor
*
* <h2>Export</h2>
* DgnDbToDgnXf::Export exports the data in a DgnProject in the form of DgnXf entries.
* The caller supplies an implementation DgnXfLib::IMessageProcessor to process the entries.
*
* Here is an example of how to use an implementation of IMessageProcessor from DgnXfLib to export data from a DgnDb project file 
* to a file that stores the data in DgnXf format.
*
* @verbatim
    // Given a DgnProject that is open for reading
    DgnProjectR dgnProject = ...

    BeFileName dgnXfFile = ... path to file to be created
    if (BeFileStatus::Success != outputFile.Create (dgnXfFile))
        return  -1;

    DgnXfLib::DgnXfFileWriter xfFile (outputFile, DgnXfLib::DgnXfFile::DATA_FORMAT_ZIP);
    xfFile.WriteHeader ();

    DgnDbToDgnXf::Export (xfFile.GetMessageWriter(), dgnProject);
*
* <h2>Import</h2>
* The caller obtains a stream of DgnXf entries from some source and passes them to an instance of DgnProjectFromDgnXfMessageProcessor.
*
* Here is an example of how to use DgnXfLib utilities in conjunction with DgnProjectFromDgnXfMessageProcessor to import data from a DgnXF 
* file and write it to a DgnDb project:
* 
* @verbatim
    // Given a DgnProject that is open for writing
    DgnProjectR dgnProject = ...

    BeFileName dgnXfFileName = ... file containing DgnXf data
    BeFile dgnXfFile;
    dgnXfFile.Open (dgnXfFileName, BeFileAccess::Read);
    DgnXfLib::DgnXfFileReader xfFileReader (dgnXfFile);
    if ((status = xfFileReader.ReadHeader()) != BSISUCCESS)
        {
        fwprintf (stderr, L"%s - not a valid DgnXf file.\n", inputFileName.GetName());
        return status;
        }
        
    DgnProjectFromDgnXfMessageProcessor writeToDb (dgnProject);
    if ((status = xfFileReader.Export (writeToDb)) != BSISUCCESS)
        return status;

    writeToDb.OnImportComplete();
@endverbatim
*
*/

#if !defined (DOCUMENTATION_GENERATOR)

/*=================================================================================**//**
* Remaps DgnDb ids to DgnDb ids
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DgnXfToDbRemapper : IEditProperties
    {
private:
    DgnProjectFromDgnXfMessageProcessor& m_importer;
    bmap<UInt32,DgnTrueColorId> m_extendedColorRemap;
    bmap<UInt32,DgnStyleId> m_displayStyleRemap;
    bmap<UInt32,UInt32>     m_fontRemap;
    bmap<UInt64,DgnModelId> m_modelIdRemap;
    bmap<UInt64,DgnModelSelectorId> m_modelSelectorIdRemap;
    bmap<UInt64,ElementId>  m_xgraphicsSymbolRemap;
    bmap<WString,bool>      m_reportedMissing;

public:
    DgnXfToDbRemapper (DgnProjectFromDgnXfMessageProcessor& i) : m_importer(i) {;}
    virtual ~DgnXfToDbRemapper() {}

    void RecordLevelId (UInt32, LevelId);
    LevelId RemapLevelId (UInt32 oldId);

    void RecordFontId (UInt32 oldId, UInt32 newId);
    UInt32 RemapFontId (UInt32 oldId);

    void RecordDisplayStyleId (UInt32 oldId, DgnStyleId newId);
    DgnStyleId RemapDisplayStyleId (UInt32 oldId);

    void RecordExtendedColor (UInt32 oldId, DgnTrueColorId newId);
    DgnTrueColorId RemapExtendedColor (UInt32 oldId);

    UInt32 RemapElementColor (UInt32 oldColor);

    void RecordSymbolId (UInt64 oldId, ElementId newId);
    ElementId RemapSymbolId (UInt64 oldId);

    void RecordModelId (UInt64 oldId, DgnModelId newId);
    DgnModelId RemapModelId (UInt64 oldId);

    void RecordModelSelectorId (UInt64 oldId, DgnModelSelectorId newId);
    DgnModelSelectorId RemapModelSelectorId (UInt64 oldId);

    DgnStyleId RemapLineStyle (UInt32 v) {return (DgnStyleId)v;} // *** WIP_REMAP_LINESTYLES
    
    DgnMaterialId RemapMaterialId (UInt64 v) {return (DgnMaterialId)v;} // *** WIP_REMAP_MATERIALID

    virtual ElementProperties _GetEditPropertiesMask () override;
    virtual EditPropertyPurpose _GetEditPropertiesPurpose ();
    virtual DgnModelP _GetDestinationDgnModel () override;

    virtual void _EachColorCallback (EachColorArg& arg) override;
    virtual void _EachLevelCallback (EachLevelArg& arg) override;
    virtual void _EachFontCallback (EachFontArg& arg) override ;
    virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override;
    virtual void _EachMaterialCallback (EachMaterialArg& arg) override;
    virtual void _EachTextStyleCallback (EachTextStyleArg& arg) override;
    virtual void _EachDimStyleCallback (EachDimStyleArg& arg) override;
    virtual void _EachMLineStyleCallback (EachMLineStyleArg& arg) override;

    void ReportTBD (WCharCP item);
    };

#endif

#if !defined (DOCUMENTATION_GENERATOR)

/*=================================================================================**//**
* This class handles the job of importing context-section data.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DgnXfToDbContextSection
{
    // Used during context import, so that various phases can assert their prerequisites. 
    struct 
        {
        UInt32 header:1;
        UInt32 units:1;
        UInt32 colors:1;
        UInt32 lineStyles:1;
        UInt32 materials:1;
        UInt32 displayStyles:1;
        UInt32 levels:1;
        UInt32 views:1;
        UInt32 models:1;
        UInt32 modelSelectors:1;
        }   m_imported;

    //  The importer
    DgnProjectFromDgnXfMessageProcessor& m_importer;

    // Context section entries read during phase 1 and processed during phase 2.
    DgnXfLib::Units         m_unitsEntry;
    DgnXfLib::Fonts         m_fontsEntry;
    DgnXfLib::DgnHeader     m_hdrEntry;
    DgnXfLib::Levels        m_levelsEntry;
    DgnXfLib::Colors        m_colorsEntry;
    bvector<DgnXfLib::View> m_viewEntries;
    DgnXfLib::ModelSelectors m_modelSelectorsEntry;
    DgnXfLib::Models        m_modelsEntry;
    DgnXfLib::Styles        m_stylesEntry;

    //  Helper functions
    void SaveViewProperties (DgnViews::View const& entry, DgnXfLib::View const& view);

    //  Gather context-related messages
    void ProcessContextSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg);

    //  Process the messages
    void OnContextSectionEnd ();
    void ImportDgnHeader();
    void ImportModelSelectors();
    void ImportModel (DgnXfLib::Models_Model const&);
    void ImportModels();
    void ImportUnits();
    void ImportDisplayStyle (DgnXfLib::Styles_Style const&);
    void ImportStyles();
    void ImportLevel (DgnXfLib::Levels_Level const&);
    void ImportLevels();
    void ImportFont (DgnXfLib::Fonts_Font const&);
    void ImportFonts();
    void ImportColors();
    void ImportView (DgnXfLib::View const&);
    void ImportViews();
    
public:
    DgnXfToDbContextSection (DgnProjectFromDgnXfMessageProcessor& i) : m_importer(i) {memset (&m_imported, 0, sizeof(m_imported));} 
};

#endif

//=======================================================================================
//! Imports data in DgnXf format into a DgnProject. This class implements IMessageProcessor,
//! allowing it to be used as an outlet by any process that obtains DgnXf messages from some source.
//! @ingroup DgnXf
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnProjectFromDgnXfMessageProcessor : DgnXfLib::IMessageProcessor
{
    friend struct DgnXfToDbContextSection;
    friend struct DgnXfToDbRemapper;

protected:
    //  ----------------------------------------------------
    //  Member types
    //  ----------------------------------------------------

    //! Import issues
    enum ImportIssue
        {
        IMPORT_ISSUE_ParseError,            //!< "Failed to parse item \"%ls\". The data in the input stream may be corrupt.",
        IMPORT_ISSUE_UnrecognizedMessage,   //!< "Unrecognized message in the DgnXf stream - \"%ls\". The data in the input stream may be corrupt or may have been created by a newer version.",
        IMPORT_ISSUE_IncorrectData,         //!< "Unrecognized or incorrect data in a DgnXf message - \"%ls\". The data in the input stream may be corrupt or may have been created by a newer version.",
        IMPORT_ISSUE_ImportFailure,         //!< "Cannot create \"%ls\" in the output file.",
        IMPORT_ISSUE_MissingSection,        //!< "Section \"%ls\" is missing from the DgnXf stream.",
        IMPORT_ISSUE_TBDStyles,             //!< "Not yet importing \"%ls\". Elements using these styles may not display correctly.",
        IMPORT_ISSUE_MissingRscFont,        //!< "Missing RSC font \"%ls\" - remapped to RSC font 0",
        };

    //! Import phases
    enum ImportPhase
        {
        IMPORT_PHASE_Context,               //!< "Importing context information (styles, etc.)"
        IMPORT_PHASE_ECSchemas,             //!< "Importing ECSchemas"
        IMPORT_PHASE_ECInstances,           //!< "Importing ECInstances"
        IMPORT_PHASE_Elements,              //!< "Importing Elements"
        };

    enum InSection {IN_SECTION_Neutral, IN_SECTION_Context, IN_SECTION_ECSchema, IN_SECTION_ECInstances, IN_SECTION_Elements};

    //  ----------------------------------------------------
    //  Member variables
    //  ----------------------------------------------------

    DgnProjectR m_project;
    DgnXfToDbContextSection* m_context;
    DgnXfToDbRemapper* m_remapper;
    InSection m_inSection;
    struct {UInt32 context:1; UInt32 ecschema:1;} m_imported;

    //  ----------------------------------------------------
    //  Member Functions
    //  ----------------------------------------------------
        
    //  This callback is invoked by the host message producer to a process a message
    DGNDBTOXF_EXPORT virtual void _ProcessMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg) override;

/** @name Virtual methods that sub-class can override. */
/** @{ */
    //! This callback is invoked when the DgnDb importer encounters a problem.
    DGNDBTOXF_EXPORT virtual void _ReportIssue (ImportIssue error, WCharCP item);

    //! This callback is invoked when the DgnDb importer starts a new major phase
    DGNDBTOXF_EXPORT virtual void _ReportImportPhase (ImportPhase phase);

    //! This callback is invoked periodically by the DgnDb importer to indicate that it is making progress processing messages
    DGNDBTOXF_EXPORT virtual void _ReportImportProgress();
/** @} */

#if !defined (DOCUMENTATION_GENERATOR)
    //  Message processing dispatcher
    void OnStartSection (InSection, ImportPhase);
    void ProcessSectionStartMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg);

    //  Elements section
    void ProcessElementsSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg);
    void ImportElement (DgnXfLib::GraphicElement const&);
    void ImportSymbolDefinition (DgnXfLib::GraphicSymbol const&);

    //  ECSChema section
    void ProcessECSchemaSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg);
    void ImportECSchema (DgnXfLib::ECSchema const&);

    //  ECInstances section
    void ProcessECInstancesSectionMessage (DgnXfLib::DgnXfMessageId messageType, ProtoBuf::MessageLite& msg);
    void ImportECInstance (DgnXfLib::ECInstance const&);
    void ImportECRelationshipInstance (DgnXfLib::ECRelationshipInstance const&);

    //  Error handling and phase reporting
    void ReportIssue (ImportIssue error, WCharCP item);
    void ReportImportPhase (ImportPhase phase);
    void ReportImportProgress ();

    //  Helper functions
    static void GetDoubleArray(double* out, ProtoBuf::RepeatedField<double> const& in, int size) {BeAssert (in.size() == size); for (int i=0; i<size; ++i) *(out++) = in.Get(i);}
    static DPoint2d GetDPoint2d (DgnXfLib::DPoint2d const& in) {return DPoint2d::From(in.pt(0), in.pt(1));}
    static DPoint3d GetDPoint3d (DgnXfLib::DPoint3d const& in) {return DPoint3d::From(in.pt(0), in.pt(1), in.pt(2));}
    static DVec3d GetDVec3d (DgnXfLib::DPoint2d const& in) {return DVec3d::From(in.pt(0), in.pt(1), 0.0);}
    static DVec3d GetDVec3d (DgnXfLib::DPoint3d const& in) {return DVec3d::From(in.pt(0), in.pt(1), in.pt(2));}
    static RotMatrix GetRotMatrix (DgnXfLib::RotMatrix const& in) {RotMatrix r; GetDoubleArray ((double*) &r, in.rot(), 9); return r;}
    static Transform GetTransform(DgnXfLib::Transform const& in)  {Transform t; GetDoubleArray ((double*) &t, in.trans(), 12); return t;}
    static DRange3d  GetDRange3d (DgnXfLib::DRange3d const& in)   {DRange3d r;  GetDoubleArray ((double*) &r, in.range(), 6); return r;}

    //! Generate thumbnails for all views. Call this function if views were imported.
    DGNDBTOXF_EXPORT void GenerateThumbnails ();

#endif

public:
/** @name Constructor and destructor. */
/** @{ */
    //! Constructs an ImportMessage object that will write to the specified project.
    DGNDBTOXF_EXPORT DgnProjectFromDgnXfMessageProcessor (DgnProjectR);
    DGNDBTOXF_EXPORT ~DgnProjectFromDgnXfMessageProcessor();
/** @} */

/** @name Functions that may be called after the import is done. */
/** @{ */
    //! Must call this after all DgnXf entries have been written
    DGNDBTOXF_EXPORT void OnImportComplete();
/** @} */
};

//=======================================================================================
//! Functions to export data from a DgnProject.
//! @ingroup DgnXf
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnProjectToDgnXf
{
    //! Export the contents of the specified DgnProject in DgnXF format.
    //! @param processor The caller's method of writing entries to the destination
    //! @param project The DgnDb file to read from
    //! @return non-zero error status if the export failed
    DGNDBTOXF_EXPORT static DgnXfLib::DgnXfStatus ProcessMessages (DgnXfLib::IMessageProcessor& processor, DgnProjectR project);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
