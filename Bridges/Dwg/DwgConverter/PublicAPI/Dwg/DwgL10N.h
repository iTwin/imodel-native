/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <iModelBridge/iModelBridge.h>
#include <Dwg/DwgImporter.h>

BEGIN_DWG_NAMESPACE

//! Categories for issues
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START (IssueCategory, dwg_issueCategory)
    L10N_STRING(Compatibility)      // =="Compatibility"==
    L10N_STRING(ConfigXml)          // =="Config"==
    L10N_STRING(CorruptData)        // =="Corrupt Data"==
    L10N_STRING(DigitalRights)      // =="Digital Rights"==
    L10N_STRING(DiskIO)             // =="Disk I/O"==
    L10N_STRING(Filtering)          // =="Filtering"==
    L10N_STRING(InconsistentData)   // =="Inconsistent Data"==
    L10N_STRING(MissingData)        // =="Missing Data"==
    L10N_STRING(UnexpectedData)     // =="Unexpected Data"==
    L10N_STRING(Sync)               // =="SyncInfo"==
    L10N_STRING(ToolkitError)       // =="Toolkit Error"==
    L10N_STRING(ToolkitMessage)     // =="From Toolkit"==
    L10N_STRING(Unknown)            // =="Unknown"==
    L10N_STRING(Unsupported)        // =="Unsupported"==
    L10N_STRING(VisualFidelity)     // =="Visual Fidelity"==
    L10N_STRING(Briefcase)          // =="Briefcase"==
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

//! A problem in the conversion process
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START (Issue, dwg_issue)
    L10N_STRING(CannotCreateChangesFile)     // =="Cannot create changes file"==
    L10N_STRING(CannotEmbedFont)             // =="Could not embed font type/name %i/'%s'; a different font will be used for display."==
    L10N_STRING(CannotOpenModelspace)        // =="Cannot open modelspace for file [%s]"==
    L10N_STRING(CannotUseStableIds)          // =="Cannot use DgnElementIds for this kind of file"==
    L10N_STRING(CantCreateModel)             // =="Cannot create model [%s]"==
    L10N_STRING(CantCreateProject)           // =="Cannot create project file [%s]"==
    L10N_STRING(CantCreateSyncInfo)          // =="Cannot create sync info [%s]"==
    L10N_STRING(CantOpenSyncInfo)            // =="Cannot open sync info [%s]"==
    L10N_STRING(CantOpenObject)              // =="Cannot open DwgDb object [%s]"==
    L10N_STRING(ChangesFileInconsistent)     // =="The changes file [%s] is inconsistent with the syncinfo file"==
    L10N_STRING(ChangesFileInvalid)          // =="The changes file exists but cannot be opened. It may be invalid."==
    L10N_STRING(ConfigFileError)             // =="[%s] error at [%d,%d], %s"==
    L10N_STRING(ConfigUsingDefault)          // =="Using default configuration."==
    L10N_STRING(ImportFailure)               // =="Failed to import [%s]"==
    L10N_STRING(UpdateFailure)               // =="Failed to update [%s]"==
    L10N_STRING(ElementFilteredOut)          // =="Element [%s] was not converted."==
    L10N_STRING(EmptyGeometry)               // ==" No geometry created for entity %s."== <<Leading space is necessary>>
    L10N_STRING(Error)                       // =="Error: %s"==
    L10N_STRING(Exception)                   // =="Exception thrown: %s"==
    L10N_STRING(FatalError)                  // =="A fatal error is stopping the conversion: %s"==
    L10N_STRING(ProgramExits)                // =="The importer must exit due to a fatal error!"==
    L10N_STRING(XrefFileFilteredOut)         // =="Xref file [%s] was not converted."==
    L10N_STRING(XrefFileSkipped)             // =="Unreferenced Xref file [%s] was skipped."==
    L10N_STRING(FileInUse)                   // =="File [%s] is in use"==
    L10N_STRING(FileNotFound)                // =="File [%s] was not found"==
    L10N_STRING(FileReadOnly)                // =="The file is read-only"==
    L10N_STRING(FontEmbedError)              // =="Could not embed %s font '%s'. Some elements may not display properly."==
    L10N_STRING(FontIllegalNumber)           // =="Illegal font number %u."==
    L10N_STRING(FontMissing)                 // =="Missing %s font '%s'. Some elements may not display properly."==
    L10N_STRING(FontNotEmbedded)             // =="Did not embed %s font '%s' due to importer configuration. Some elements may not display properly."==
    L10N_STRING(GroupError)                  // =="Group import error [%s]"==
    L10N_STRING(InvalidLayer)                // =="Invalid Layer [%s] could not be converted"==
    L10N_STRING(InvalidRange)                // =="Invalid Range"==
    L10N_STRING(InvisibleElementFilteredOut) // =="Element [%s] is invisible. It was not converted."==
    L10N_STRING(LayerDefinitionChange)       // =="Layer [%s] has changed in [%s]: %s. Update is not possible. You must do a full conversion."==
    L10N_STRING(LayerDisplayInconsistent)    // =="Layer [%s] is turned on for some attachments but is turned off for [%s]"==
    L10N_STRING(LayerNotFoundInRoot)         // =="Layer [%s] found in tile file [%s] but not in root file [%s]. The root file must define all Layers."==
    L10N_STRING(LayerSymbologyInconsistent)  // =="Layer [%s] has a different definition in [%s] than in other files or attachments: %s"==
    L10N_STRING(MissingCategory)             // =="Missing category for layer [%s]"==
    L10N_STRING(LinetypeError)               // =="Linetype import error %s: "==
    L10N_STRING(Message)                     // =="%s"==
    L10N_STRING(MissingLinetype)             // =="Could not find linetype [%s]. Some elements may not display properly."==
    L10N_STRING(MaterialError)               // =="Material import error [%s]"==
    L10N_STRING(MaterialNoFile)              // =="Material [%lls] has no texture file attached"==
    L10N_STRING(ModelAlreadyImported)        // =="Model [%s] has already been converted."==
    L10N_STRING(ModelFilteredOut)            // =="Model [%s] was not imported."==
    L10N_STRING(NotADgnDb)                   // =="The file is not a DgnDb"==
    L10N_STRING(NotRecognizedFormat)         // =="File [%s] is not in a recognized format"==
    L10N_STRING(NewerDwgVersion)             // =="File [%s] is a newer version not currently supported"==
    L10N_STRING(CantCreateRaster)            // =="Cannot create raster attachment [%s]."==
    L10N_STRING(RootModelChanged)            // =="The original root model was deleted or has changed units."==
    L10N_STRING(RootModelMustBePhysical)     // =="Root model [%s] is not a physical model."==
    L10N_STRING(SaveError)                   // =="An error occurred when saving changes (%s)"==
    L10N_STRING(SeedFileMismatch)            // =="Seed file [%s] does not match target [%s]"==
    L10N_STRING(SyncInfoInconsistent)        // =="The syncInfo file [%s] is inconsistent with the project"==
    L10N_STRING(SyncInfoTooNew)              // =="Sync info was created by a later version"==
    L10N_STRING(ViewNoneFound)               // =="No view was found"==
    L10N_STRING(ViewportError)               // =="Modelspace viewport error %s"==
    L10N_STRING(ImageNotAJpeg)               // =="Sky box image is not a jpeg file, %s"==
    L10N_STRING(UpdateDoesNotChangeClass)    // =="Update cannot change the class of an element. Element: %s. Proposed class: %s."==
    L10N_STRING(MissingJobDefinitionModel)   // =="Missing JobDefinitionModel for %s"==
    L10N_STRING(CircularXrefIgnored)         // =="Circular xRef %s is ignored"==
    L10N_STRING(CannotUpdateName)            // =="Unable to change name from %s to %s"==
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

//! Progress messages for the conversion process
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START (ProgressMessage, dwg_progress)
    L10N_STRING(STEP_INITIALIZING)                 // =="Initializing"==
    L10N_STRING(STEP_CLEANUP_EMPTY_TABLES)         // =="Cleaning up empty tables"==
    L10N_STRING(STEP_OPENINGFILE)                  // =="Opening File %ls [%s]"==
    L10N_STRING(STEP_COMPACTING)                   // =="Compacting File"==
    L10N_STRING(STEP_IMPORTING_ENTITIES)           // =="Importing Entities"==
    L10N_STRING(STEP_IMPORTING_VIEWS)              // =="Importing Views"==
    L10N_STRING(STEP_CREATE_IMODEL)                // =="Creating .imodel File"==
    L10N_STRING(STEP_CREATE_THUMBNAILS)            // =="Creating Thumbnails"==
    L10N_STRING(STEP_CREATING)                     // =="Creating DgnDb [%s]"==
    L10N_STRING(STEP_EMBED_FILES)                  // =="Embedding Files"==
    L10N_STRING(STEP_EMBED_FONTS)                  // =="Embedding Fonts"==
    L10N_STRING(STEP_IMPORTING_GROUPS)             // =="Importing Groups"==
    L10N_STRING(STEP_IMPORTING_LAYERS)             // =="Importing Layers"==
    L10N_STRING(STEP_IMPORTING_TEXTSTYLES)         // =="Importing Text Styles"==
    L10N_STRING(STEP_IMPORTING_LINETYPES)          // =="Importing Line Types"==
    L10N_STRING(STEP_UPDATING)                     // =="Updating DgnDb"==
    L10N_STRING(STEP_IMPORTING_MATERIALS)          // =="Importing Materials"==
    L10N_STRING(STEP_IMPORTING_ATTRDEFSCHEMA)      // =="Importing Attribute Definition Schema [%d classes]"==
    L10N_STRING(TASK_LOADING_FONTS)                // =="Loading %s Fonts"==
    L10N_STRING(TASK_IMPORTING_MODEL)              // =="Model: %s"==
    L10N_STRING(TASK_IMPORTING_RASTERDATA)         // =="Importing raster data file: %s"==
    L10N_STRING(TASK_CREATING_THUMBNAIL)           // =="Creating thumbnail for: %s"==
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

//! Miscellaneous strings needed for DwgImporter
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_START (DataStrings, dwg_dataStrings)
    L10N_STRING(AttrdefsSchemaDescription)  // =="Block attribute definitions created from DWG file %ls"==
    L10N_STRING(BlockAttrdefDescription)    // =="Attribute definition created from DWG block %ls"==
    L10N_STRING(ModelView)                  // =="Model View"==
    L10N_STRING(LayoutView)                 // =="Layout View"==
    L10N_STRING(XrefView)                   // =="XReference View"==
    L10N_STRING(CategorySelector)           // =="Categories"==
    L10N_STRING(ModelSelector)              // =="Models"==
    L10N_STRING(DisplayStyle)               // =="Display Style"==
    L10N_STRING(Subject)                    // =="Subject"==
IMODELBRIDGEFX_TRANSLATABLE_STRINGS_END

END_DWG_NAMESPACE
