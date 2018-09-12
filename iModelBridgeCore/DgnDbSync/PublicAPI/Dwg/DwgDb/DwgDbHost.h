/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Dwg/DwgDb/DwgDbHost.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Dwg/DwgDb/DwgDbDatabase.h>

BEGIN_DWGDB_NAMESPACE

enum class AcadFileType
    {
    Default                 = 0,
    FontFile                = 1,    // Could be either
    CompiledShapeFile       = 2,    // shx
    TrueTypeFontFile        = 3,    // ttf
    EmbeddedImageFile       = 4,
    XRefDrawing             = 5,
    PatternFile             = 6,
    ARXApplication          = 7,
    FontMapFile             = 8,
    UnderlayFile            = 9,
    DataLinkFile            = 10,
    PhotometricWebFile      = 11,
    MaterialMapFile         = 12,
    CloudOrProjectFile      = 13,
    };

enum class PasswordChoice
    {
    Default                 = 0,
    UpperCase               = 1,
    ExternalReference       = 2,
    };

enum class FileShareMode
    {
    DenyReadWrite           = 0x10,     // deny read/write mode
    DenyWrite               = 0x20,     // deny write mode
    DenyRead                = 0x30,     // deny read mode
    DenyNo                  = 0x40,     // deny none mode
    };

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct IDwgDbProgressMeter
    {
public:
    virtual void        _Start (WStringCR displayString = WString()) { /* do nothing */ }
    virtual void        _Stop () { /* do nothing */ }
    virtual void        _Progress () { /* do nothing */ }
    virtual void        _SetLimit (int max) { /* do nothing */ }
    };  // IDwgDbProgressMeter

/*=================================================================================**//**
This interface is a gate way to/from toolkit's host, and consists of only an essential 
subset of the toolkit.  Most of the other stuff is handled in DwgDb layer thus shielded from
the app which does not have to deal with toolkit specific issues/requirements.
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct IDwgDbHost : NonCopyableClass
{
public:
    enum class RootRegistry
        {
        Unknown,
        Machine,
        User,
        };

    //! Must override this method to return true after a singleton instance is created.
    virtual bool    _IsValid () const { return false; }

    // methods called from DWGTOOLKIT
    //! Find DBX, pattern, font, etc.
    virtual DwgDbStatus     _FindFile (WStringR fullpathOut, WCharCP filenameIn, DwgDbDatabaseP dwg = nullptr, AcadFileType hint = AcadFileType::Default) { return DwgDbStatus::FileNotFound; }
    //! Supply an alternate font.
    virtual bool            _GetAlternateFontName (WStringR altFont) const { altFont.assign(L"simplex.shx"); return true; }
    //! Supply DWG file password.
    virtual bool            _GetPassword (WCharCP dwgName, PasswordChoice choice, WCharP password, const size_t bufSize) { return false; }
    //! Supply a registry product root key.  Default is "SOFTWARE\\Autodesk\\ObjectDBX\\R<Version.Number>".
    virtual WCharCP         _GetRegistryProductRootKey (RootRegistry type) { return nullptr; }
    //! Supply a locale for the product from Microsoft assigned ID's.
    virtual LCID            _GetRegistryProductLCID () { return 0x0409; /*English*/ }
    //! Supply a product name.
    virtual WCharCP         _Product () const { return L"DwgDbExchange"; }
    //! Handle fatal error initiated in the toolkit.
    virtual void            _FatalError (WCharCP format, ...) { }
    //! Display an alert message.
    virtual void            _Alert (WCharCP message) const { }
    //! Display a message.
    virtual void            _Message (WCharCP message, int numChars) const { }

    // methods used by DwgDb layer and apps
    //! Display DwgDb debugging information
    virtual void            _DebugPrintf (WCharCP format, ...) const { }
    //! Read a DWG file
    DWGDB_EXPORT DwgDbDatabasePtr       ReadFile (WStringCR filename, bool convCodepage = false, bool partialLoad = false, FileShareMode mode = FileShareMode::DenyNo, WStringCR password = WString());
    //! Returns the layout manager to access layouts in a DWG file.
    DWGDB_EXPORT DwgDbLayoutManagerPtr  GetLayoutManager () const;

    // methods to call the toolkit.
    //! Supply your app's host - must be called exactly once per session.
    DWGDB_EXPORT static void            InitializeToolkit (IDwgDbHost& appHost);
    //! Uninitialize the toolkit - must be called exactly once per session
    DWGDB_EXPORT static void            TerminateToolkit ();
    //! Tell the toolkit about a new DWG database to be used - optional call.
    DWGDB_EXPORT static void            SetWorkingDatabase (DwgDbDatabaseP dwg);
    //! Supply a progress meter which should be always available for the toolkit - optional call.
    DWGDB_EXPORT static void            SetWorkingProgressMeter (IDwgDbProgressMeter* newMeter);
    //! Sniff the first few bytes to see if it smells like a DXF or DXB file:
    DWGDB_EXPORT static bool            IsDxfFile (WStringCR filename);
    //! Explicitly load a DBX(RealDWG) or TX(OpenDWG) module by name(including file extension)
    DWGDB_EXPORT static bool            LoadObjectEnabler (WStringCR filename);
    //! Download an URL file to local cache if not already done, return the cached file back:
    DWGDB_EXPORT static bool            GetCachedLocalFile (WStringR localFile, WStringCR url);
};  // IDwgDbHost

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__

