/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

//
// NOTE: As of November 22, 2016, this is up to date with the object model in version [7e63c6be68f8] of dgnjsapi.d.ts
//

#pragma unmanaged

#include <DgnPlatform/DgnPlatformAPI.h>
#include <DgnPlatform/DgnJSApi.h>
#include <DgnPlatform/ElementHandler.h>
#include <ECDb/SchemaManager.h>
#include <Bentley/ReleaseMarshaller.h>

#pragma managed
#include <vcclr.h> // PtrToStringChars

[assembly:System::Runtime::CompilerServices::InternalsVisibleTo ("DTPlatform")];

#using      <system.dll>
#using      <Bentley.GeometryNET.dll>
#using      <Bentley.GeometryNET.Structs.dll>

namespace   BDGN    = BENTLEY_NAMESPACE_NAME::Dgn;
namespace   BDGNR   = BENTLEY_NAMESPACE_NAME::Dgn::Render;
namespace   SRI     = System::Runtime::InteropServices;

namespace Bentley {
namespace DgnNET {


ref struct      DgnDb;
ref struct      DgnElement;
ref struct      ECSqlValue;
ref struct      DgnModel;
ref struct      DgnModels;
ref struct      SchemaManager;
ref struct      DgnElementCollection;
ref struct      AuthorityIssuedCode;
ref struct      ECClass;
ref struct      ECValue;
ref struct      AdHocJsonPropertyValue;
ref struct      GeometrySource;
ref struct      GeometrySource3d;
ref struct      GeometrySource2d;
ref struct      GeometryCollection;
ref struct      RepositoryRequest;
ref struct      ECClassCollection;
ref struct      ECSchema;
ref struct      ECPropertyCollection;
ref struct      ECProperty;
ref struct      PrimitiveECProperty;
ref struct      ECInstance;
ref struct      Viewport;
ref struct      ViewController;
ref struct      DgnCategoryIdSet;
ref struct      DgnCategory;

/*=================================================================================**//**
* Convert class
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
ref struct Convert abstract
{
internal:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static GeometryNET::DPoint3d    DPoint3dToManaged (DPoint3dCR native)
    {
    pin_ptr <GeometryNET::DPoint3d> pinned = (GeometryNET::DPoint3d*)&native;
    return *pinned;
    }

static GeometryNET::YawPitchRollAngles YawPitchRollAnglesToManaged (YawPitchRollAnglesCR native)
    {
    // the native object stores angles in degrees, the managed stores angles in radians.
    GeometryNET::Angle yawAngle     = GeometryNET::Angle::FromDegrees (native.GetYaw().Degrees());
    GeometryNET::Angle pitchAngle   = GeometryNET::Angle::FromDegrees (native.GetPitch().Degrees());
    GeometryNET::Angle rollAngle    = GeometryNET::Angle::FromDegrees (native.GetRoll().Degrees());
    return GeometryNET::YawPitchRollAngles::FromDegrees (yawAngle.Degrees, pitchAngle.Degrees, rollAngle.Degrees);
    }

static GeometryNET::DRange3d    DRange3dToManaged (DRange3dCR native)
    {
    pin_ptr <GeometryNET::DRange3d> pinned = (GeometryNET::DRange3d*)&native;
    return *pinned;
    }

static ::DRange3d               DRange3dToNative (GeometryNET::DRange3d rangeManaged)
    {
    pin_ptr<GeometryNET::DRange3d>     rangePinned = &rangeManaged;
    ::DRange3d rangeNative = *((::DRange3d*) rangePinned);
    return rangeNative;
    }

static GeometryNET::DTransform3d TransformToManaged (TransformCR native)
    {
    pin_ptr <GeometryNET::DTransform3d> pinned = (GeometryNET::DTransform3d*)&native;
    return *pinned;
    }

static TransformCP              TransformToNative (GeometryNET::DTransform3d transformManaged)
    {
    pin_ptr <GeometryNET::DTransform3d> pinned = &transformManaged;
    return (TransformCP) pinned;
    }

static BDGN::DgnDbP             DgnDbToNative (DgnDb^ managed);

static DgnDb^                   DgnDbToManaged (BDGN::DgnDbP);

static ECSqlValue^              ECSqlValueToManaged (BeSQLite::EC::IECSqlValue* native, ECSqlValue^ owner);

static System::DateTimeKind     DateTimeKindToManaged (BENTLEY_NAMESPACE_NAME::DateTime::Kind nativeKind)
    {
    if (nativeKind == BENTLEY_NAMESPACE_NAME::DateTime::Kind::Utc)
        return System::DateTimeKind::Utc;
    else if (nativeKind == BENTLEY_NAMESPACE_NAME::DateTime::Kind::Local)
        return System::DateTimeKind::Local;
    else
        return System::DateTimeKind::Unspecified;
    }

static System::DateTime         DateTimeToManaged (BENTLEY_NAMESPACE_NAME::DateTime const& nativeDateTime)
    {
    uint64_t julianDay;
    if (SUCCESS == nativeDateTime.ToJulianDay (julianDay))
        return System::DateTime (BENTLEY_NAMESPACE_NAME::DateTime::JulianDayToCommonEraMilliseconds(julianDay) * 10000, DateTimeKindToManaged (nativeDateTime.GetInfo().GetKind()));
    else
        return System::DateTime::Now;
    }

static DgnModels^               DgnModelsToManaged (BDGN::DgnModels*, DgnDb^);

static SchemaManager^           SchemaManagerToManaged (BeSQLite::EC::SchemaManager const*, DgnDb^);

static DgnElementCollection^    ElementCollectionToManaged (BDGN::DgnElements*, DgnDb^);

static DgnModel^                DgnModelToManaged (BDGN::DgnModelP, DgnDb^, bool);

static BDGN::DgnModelP          DgnModelToNative (DgnModel^);

static DgnElement^              DgnElementToManaged (BDGN::DgnElementP);

static BDGN::DgnElementP        DgnElementToNative (DgnElement^);

static GeometrySource^          GeometrySourceToManaged (BDGN::GeometrySourceP, DgnElement^ element);

static BDGN::GeometrySourceP    GeometrySourceToNative (GeometrySource^);

static GeometrySource3d^        GeometrySource3dToManaged (BDGN::GeometrySource3dP, DgnElement^ );

static GeometrySource2d^        GeometrySource2dToManaged (BDGN::GeometrySource2dP, DgnElement^ );

static ECClass^                 ECClassToManaged (ECN::ECClassCP, System::Object^ owner);

static ECValue^                 ECValueToManaged (ECN::ECValueP);

static ECN::ECValueCP           ECValueToNative (ECValue^);

#if defined (NEEDSWORK_ECDB_AdhocJsonPropertyValue)
// removed from ECN in February 2017. What replaces it, if anything?
static AdHocJsonPropertyValue^  AdHocJsonPropertyValueToManaged (ECN::AdHocJsonPropertyValueP);
#endif

static BDGN::IBriefcaseManager::Request* RepositoryRequestToNative (RepositoryRequest^);

static GeometryCollection^      GeometrySourceToCollection (GeometrySource^);

static ECSchema^                ECSchemaToManaged (ECN::ECSchemaCP nativeSchema, System::Object^ owner);

static ECClassCollection^       ECClassCollectionToManaged (bvector<ECN::ECClassP> const*, System::Object^);

static ECProperty^              ECPropertyToManaged (ECN::ECPropertyCP, System::Object^);

static ECInstance^              ECInstanceToManaged (ECN::IECInstanceP);

static PrimitiveECProperty^     PrimitiveECPropertyToManaged (ECN::PrimitiveECPropertyCP, System::Object^);

static ViewController^          ViewControllerToManaged (BDGN::ViewControllerP, Viewport^);

static DgnCategory^             DgnCategoryToManaged (BDGN::DgnCategoryP, DgnDb^);

};


/*======================================================================================+
* DgnPlatformNETException calss
* @bsiclass                                                     John.Gooding    06/2011
+===============+===============+===============+===============+===============+======*/
[System::SerializableAttribute]
public ref struct DgnPlatformNETException : System::Exception
{
protected:
protected:
    DgnPlatformNETException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context)
        : System::Exception(info, context)
        {
        }

public:
    DgnPlatformNETException()
        {
        }

    DgnPlatformNETException(System::String^ message)
        : System::Exception(message)
        {
        }

    DgnPlatformNETException(System::String^ message, System::Exception^ inner)
        : System::Exception(message, inner)
        {
        }
}; // DgnPlatformNETException



/*=================================================================================**//**
* ECPropertyPrimitiveType enum
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public enum class ECPropertyPrimitiveType : uint32_t
    {
    Unknown                   = BDGN::ECPropertyPrimitiveType::Unknown,
    Binary                    = BDGN::ECPropertyPrimitiveType::Binary,
    Boolean                   = BDGN::ECPropertyPrimitiveType::Boolean,
    DateTime                  = BDGN::ECPropertyPrimitiveType::DateTime,
    Double                    = BDGN::ECPropertyPrimitiveType::Double,
    Integer                   = BDGN::ECPropertyPrimitiveType::Integer,
    Long                      = BDGN::ECPropertyPrimitiveType::Long,
    Point2D                   = BDGN::ECPropertyPrimitiveType::Point2D,
    Point3D                   = BDGN::ECPropertyPrimitiveType::Point3D,
    String                    = BDGN::ECPropertyPrimitiveType::String,
    IGeometry                 = BDGN::ECPropertyPrimitiveType::IGeometry,
    };

/*=================================================================================**//**
* BeSQLiteDbResult enum
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public enum class BeSQLiteDbResult
    {
    Ok          = BDGN::BeSQLiteDbResult::BE_SQLITE_OK,
    Error       = BDGN::BeSQLiteDbResult::BE_SQLITE_ERROR,
    Row         = BDGN::BeSQLiteDbResult::BE_SQLITE_ROW,
    Done        = BDGN::BeSQLiteDbResult::BE_SQLITE_DONE,
    };

/*=================================================================================**//**
* BeSQLiteDbOpCode enum
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public enum class BeSQLiteDbOpcode
    {
    Delete = BDGN::BeSQLiteDbOpcode::Delete,
    Insert = BDGN::BeSQLiteDbOpcode::Insert,
    Update = BDGN::BeSQLiteDbOpcode::Update,
    };

/*=================================================================================**//**
* LoggingSeverity level enum
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public enum class LoggingSeverity
    {
    Fatal   = NativeLogging::LOG_FATAL,
    Error   = NativeLogging::LOG_ERROR,
    Warning = NativeLogging::LOG_WARNING,
    Info    = NativeLogging::LOG_INFO,
    Debug   = NativeLogging::LOG_DEBUG,
    Trace   = NativeLogging::LOG_TRACE,
    };

public enum class RepositoryStatus
    {
    Success                 =   BDGN::RepositoryStatus::Success,
    ServerUnavailable       =   BDGN::RepositoryStatus::ServerUnavailable,
    LockAlreadyHeld         =   BDGN::RepositoryStatus::LockAlreadyHeld,
    SyncError               =   BDGN::RepositoryStatus::SyncError,
    InvalidResponse         =   BDGN::RepositoryStatus::InvalidResponse,
    PendingTransactions     =   BDGN::RepositoryStatus::PendingTransactions,
    LockUsed                =   BDGN::RepositoryStatus::LockUsed,
    CannotCreateRevision    =   BDGN::RepositoryStatus::CannotCreateRevision,
    InvalidRequest          =   BDGN::RepositoryStatus::InvalidRequest,
    RevisionRequired        =   BDGN::RepositoryStatus::RevisionRequired,
    CodeUnavailable         =   BDGN::RepositoryStatus::CodeUnavailable,
    CodeNotReserved         =   BDGN::RepositoryStatus::CodeNotReserved,
    CodeUsed                =   BDGN::RepositoryStatus::CodeUsed,
    LockNotHeld             =   BDGN::RepositoryStatus::LockNotHeld,
    RepositoryIsLocked      =   BDGN::RepositoryStatus::RepositoryIsLocked,
    };

public ref struct BeInt64Id
    {
private:
    uint64_t    m_id;

internal:
    void SetValue (uint64_t id) {m_id = id;}

public:

    //! default constructor
    BeInt64Id () : m_id(0) {}

    //! Construct a BeInt64Id from a 64 bit value.
    BeInt64Id (uint64_t u) : m_id(u) {}

    //! Construct a BeInt64Id from a 64 bit value.
    BeInt64Id (BeInt64Id^ val) : m_id (val->m_id) {}

    //! Test validity.
    bool IsValid() { return Validate(); }

    //! Get the 64 bit value of this BeInt64Id
    property uint64_t Value
        {
        uint64_t get ()
            {
            BeAssert(IsValid());
            return m_id;
            }
        }

    //! Get the 64 bit value of this BeGuid. Does not check for valid value in debug builds.
    uint64_t GetValueUnchecked() { return m_id; }

    //! Test to see whether this BeInt64Id is valid. 0 is not a valid id.
    bool Validate() { return m_id != 0; }

    //! Set this BeInt64Id to an invalid value (0).
    void Invalidate() { m_id = 0; }

    //! Converts this BeInt64Id to its string representation.
    //! @remarks Consider the overload BeInt64Id::ToString(Utf8Char*) if you want
    //! to avoid allocating Utf8Strings.
    virtual System::String^ ToString() override
        {
        return m_id.ToString();
        }

    };

public ref struct DgnModelId : BeInt64Id
    {
    // the additional methods we might want to add are those having to do with BriefcaseId.
    DgnModelId () : BeInt64Id() {}
    DgnModelId (uint64_t u) : BeInt64Id (u) {}
    };

public ref struct DgnElementId : BeInt64Id
    {
    // the additional methods we might want to add are those having to do with BriefcaseId.
    DgnElementId () : BeInt64Id () {}
    DgnElementId (uint64_t u) : BeInt64Id (u) {}
    DgnElementId (BeInt64Id^ val) : BeInt64Id (val) {}
    };

public ref struct CodeSpecId : BeInt64Id
    {
    // the additional methods we might want to add are those having to do with BriefcaseId.
    CodeSpecId () : BeInt64Id () {}
    CodeSpecId (uint64_t u) : BeInt64Id (u) {}
    };

public ref struct DgnCategoryId : DgnElementId
    {
    DgnCategoryId () : DgnElementId() {}
    DgnCategoryId (uint64_t u) : DgnElementId (u) {}
    };

public ref struct DgnSubCategoryId : DgnElementId
    {
    DgnSubCategoryId () : DgnElementId() {}
    DgnSubCategoryId (uint64_t u) : DgnElementId (u) {}
    };

public ref struct DgnMaterialId : DgnElementId
    {
    DgnMaterialId () : DgnElementId() {}
    DgnMaterialId (uint64_t u) : DgnElementId (u) {}
    };

public ref struct ECClassId : BeInt64Id
    {
    ECClassId () : BeInt64Id() {}
    ECClassId (uint64_t u) : BeInt64Id (u) {}
    };


/*=================================================================================**//**
* Logging class.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct Logging
{
public:
    /**
    * Set the severity level for the specified category
    * @param category     The logging category
    * @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
    */
    static void SetSeverity (System::String^ category, LoggingSeverity severity)
        {
        pin_ptr<wchar_t const> categoryPinned = PtrToStringChars(category);
        NativeLogging::LoggingConfig::SetSeverity (categoryPinned, (NativeLogging::SEVERITY) severity);
        }

    /**
    * Test if the specified severity level is enabled for the specified category
    * @param category     The logging category
    * @param severity     The severity to test
    */
    static bool IsSeverityEnabled (System::String^ category, LoggingSeverity severity)
        {
        pin_ptr<wchar_t const> categoryPinned = PtrToStringChars(category);
        return NativeLogging::LoggingManager::GetLogger(categoryPinned)->isSeverityEnabled ((NativeLogging::SEVERITY) severity);
        }

    /**
    * Send a message to the log
    * @param category     The logging category
    * @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
    * @param message      The message to log
    */
    static void Message (System::String^ category, LoggingSeverity severity, System::String^ message)
        {
        pin_ptr<wchar_t const> categoryPinned = PtrToStringChars(category);
        Utf8String utf8Category (categoryPinned);
        pin_ptr<wchar_t const> messagePinned = PtrToStringChars(message);
        Utf8String utf8Message (messagePinned);
        BDGN::DgnPlatformLib::GetHost().GetScriptAdmin().HandleLogMessage (utf8Category.c_str(),  (BDGN::DgnPlatformLib::Host::ScriptAdmin::LoggingSeverity) severity, utf8Message.c_str());
        }

};

/*=================================================================================**//**
* Placement3d class
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct Placement3d
{
private:
    BDGN::Placement3dP  m_native;
    Object^             m_owner;

internal:
    Placement3d (BDGN::Placement3dP native)
        {
        m_native = native;
        }

    Placement3d (BDGN::Placement3dCR native, Object^ owner)
        {
        m_native = const_cast <BDGN::Placement3dP>(&native);
        m_owner  = owner;
        }

    BDGN::Placement3dP  GetNative() { return m_native; }

public:
    /*** NATIVE_TYPE_NAME = JsPlacement3d ***/
    Placement3d ()
        {
        m_native = new BDGN::Placement3d();
        }

    Placement3d (GeometryNET::DPoint3d point, GeometryNET::YawPitchRollAngles angles)
        {
        pin_ptr<GeometryNET::DPoint3d>              pointPinned = &point;
        pin_ptr<GeometryNET::YawPitchRollAngles>    anglesPinned = &angles;
        m_native = new BDGN::Placement3d ( *(DPoint3dP)pointPinned, *(YawPitchRollAnglesP) anglesPinned);
        }

    Placement3d (GeometryNET::DPoint3d point, GeometryNET::YawPitchRollAngles angles, GeometryNET::DRange3d boundingBox)
        {
        pin_ptr<GeometryNET::DPoint3d>              pointPinned = &point;
        pin_ptr<GeometryNET::YawPitchRollAngles>    anglesPinned = &angles;
        pin_ptr<GeometryNET::DRange3d>              boundingBoxPinned = &boundingBox;
        BDGN::ElementAlignedBox3d eaBox (*(DRange3dCP) boundingBoxPinned);
        m_native = new BDGN::Placement3d ( *(DPoint3dP)pointPinned, *(YawPitchRollAnglesP) anglesPinned, eaBox);
        }

    /** The origin of the placement */
    property GeometryNET::DPoint3d Origin
        {
        GeometryNET::DPoint3d get ()
            {
            DPoint3dCR retval = m_native->GetOrigin();
            return Convert::DPoint3dToManaged (retval);
            }
         void set (GeometryNET::DPoint3d value)
            {
            DPoint3dR origin = m_native->GetOriginR();
            origin.Init (value.X, value.Y, value.Z);
            }
        }

    /** The angles of the placement */
    property GeometryNET::YawPitchRollAngles Angles
        {
        GeometryNET::YawPitchRollAngles get ()
            {
            YawPitchRollAnglesCR retval = m_native->GetAngles();
            return Convert::YawPitchRollAnglesToManaged (retval);
            }
        void set (GeometryNET::YawPitchRollAngles value)
            {
            YawPitchRollAnglesR yprAngles = m_native->GetAnglesR();
            AngleInDegrees yawDegrees = AngleInDegrees::FromDegrees (value.Yaw.Degrees);
            yprAngles.SetYaw (yawDegrees);
            AngleInDegrees pitchDegrees = AngleInDegrees::FromDegrees (value.Pitch.Degrees);
            yprAngles.SetPitch (pitchDegrees);
            AngleInDegrees rollDegrees = AngleInDegrees::FromDegrees (value.Roll.Degrees);
            yprAngles.SetPitch (rollDegrees);
            }
        }

    property GeometryNET::DRange3d ElementBox
        {
        GeometryNET::DRange3d get ()
            {
            BDGN::ElementAlignedBox3dCR retval = m_native->GetElementBox();
            return Convert::DRange3dToManaged (retval);
            }
        void set (GeometryNET::DRange3d value)
            {
            BDGN::ElementAlignedBox3dR box = m_native->GetElementBoxR();
            ::DRange3d nativeValue = Convert::DRange3dToNative (value);
            box.InitFrom (nativeValue.low, nativeValue.high);
            }
        }

    /** Calculate the AxisAlignedBox3d of this Placement3d. */
    GeometryNET::DRange3d CalculateRange()
        {
        BDGN::AxisAlignedBox3d range = m_native->CalculateRange();
        return Convert::DRange3dToManaged (range);
        }

    /** Convert the origin and YawPitchRollAngles of this Placement3d into a Transform. */
    property GeometryNET::DTransform3d Transform
        {
        GeometryNET::DTransform3d get ()
            {
            ::Transform nativeTransform = m_native->GetTransform();
            return Convert::TransformToManaged (nativeTransform);
            }
        }

    ~Placement3d ()
        {
        // Dispose method

        // if we are already disposed or we are part of another native object (m_owner not nullptr), don't do anything.
        if (nullptr != m_owner)
            return;

        // call finalizer to clean up native resources.
        this->!Placement3d();

        // don't need finalize, when the Dispose method is called, the runtime calls System::GC::SuppressFinalize for us.
        }

    !Placement3d ()
        {
        // finalize method
        if ( (nullptr == m_native) || (nullptr != m_owner) )
            return;

        delete m_native;
        m_native = nullptr;
        }

};

/*=================================================================================**//**
* File class
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct File
{
private:
    bool    m_isDisposed;
    FILE    *m_fp;

    File (FILE* fP)
        {
        m_fp = fP;
        }

public:
    /** a wrapper for fopen.
     * @param fileName  The full path to the file.
     * @param mode  The usual fopen model specifier
     * @return an opened File object if successful, or null if the file cannot be opened
     * @note Use only text mode, as there is currently no support for reading the contents of binary files.
     * @note on mobile devices, only the temp, local state, and documents directories are accessible. It is up to the caller to supply these
     * directory root paths to the script.
    */
    static File^ Fopen (System::String^ fileName, System::String^ mode)
        {
        pin_ptr<wchar_t const> fileNamePinned = PtrToStringChars (fileName);
        pin_ptr<wchar_t const> modePinned = PtrToStringChars (mode);
        Utf8String  utf8FileName (fileNamePinned);
        Utf8String  utf8Mode (modePinned);
        FILE* fp;
        if (nullptr == (fp = fopen (utf8FileName.c_str(), utf8Mode.c_str())))
            {
            perror("Fopen failed");
            return nullptr;
            }
        return gcnew File (fp);
        }

    /** explicitly close the file */
    void Close()
        {
        if (nullptr != m_fp)
            fclose (m_fp);
        m_fp = nullptr;
        }

    /** check if the next read position is at the end of the file. If so, do not attempt to read from the file. */
    bool Feof()
        {
        return 0 != feof(m_fp);
        }

    /** Reads the next line of text.
      * @return the next line of text.
      * @note This function throws an exception if you try to read past the end of the file. Call Feof before calling this function.
      */
    System::String^ ReadLine()
        {
        if (ferror(m_fp))
            {
            throw gcnew DgnPlatformNETException ("File not open");
            return "";
            }

        char buf[4096];
        buf[0] = '\0';
        auto res = fgets(buf, sizeof(buf), m_fp);
        if (nullptr == res)
            return nullptr;

        WString stringBuf (buf, false);
        return gcnew System::String (stringBuf.c_str());
        }

    /**
     * Writes a line of text at the current write position.
     * @param line  The line of text to write
     * @return non-zero if write failed.
     */
    int WriteLine (System::String^ line)
        {
        if (ferror(m_fp))
            {
            throw gcnew DgnPlatformNETException ("File not open");
            }

        pin_ptr<wchar_t const> linePinned = PtrToStringChars (line);
        Utf8String utf8Line (linePinned);

        auto res = fputs (utf8Line.c_str(), m_fp);
        if (EOF == res)
            {
            throw gcnew DgnPlatformNETException ("Past end of File");
            }
        return 0;
        }

    ~File ()
        {
        // Dispose method
        if (m_isDisposed)
            return;

        // call finalizer to clean up native resources.
        this->!File();

        m_isDisposed = true;
        System::GC::SuppressFinalize (this);
        }

    !File()
        {
        Close();
        }
};


/*=================================================================================**//**
* ECSqlValue  class
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECSqlValue
{
private:
    BeSQLite::EC::IECSqlValue const*    m_native;
    // if this is a ECSqlValue that is part of another ECSqlValue, make sure the ECSqlValue object isn't collected before this.
    ECSqlValue^                         m_owner;

internal:
    ECSqlValue (BeSQLite::EC::IECSqlValue const* native, ECSqlValue^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

public:

    /**
     * Get the value as a string
     */
    System::String^ GetText()
        {
        Utf8String utfText = m_native->GetText();
        WString wText (utfText.c_str(), true);
        return gcnew System::String (wText.c_str());
        }

    /**
     * Get the value as an integer
     */
    int     GetInt()
        {
        return m_native->GetInt();
        }

    /**
     * Get the value as a double
     */
    double GetDouble()
        {
        return m_native->GetDouble();
        }

    /**
     * Get the value as a DgnObjectId
     */
    BeInt64Id^ GetId()
        {
        return gcnew BeInt64Id (m_native->GetUInt64());
        }

    /**
     * Get the value as a DPoint3d
     */
    GeometryNET::DPoint3d GetDPoint3d()
        {
        DPoint3d nativePoint    = m_native->GetPoint3d();
        GeometryNET::DPoint3d           outPoint;
        *((DPoint3dP) &outPoint) = nativePoint;
        return outPoint;
        }

    /**
     * Get the value as a DateTime string
     */
    System::DateTime GetDateTime()
        {
        return Convert::DateTimeToManaged (m_native->GetDateTime());
        }
    /**
     * Get the value as an Array
     */
#if defined (NEEDSWORK_ECDB_ArrayValue)
    // IECSqlArrayValue was removed from the native API. Need to figure out what, if anything, replaces it.
    ECSqlArrayValue^ GetArray()
        {
        return Convert::ECSqlArrayValueToManaged (const_cast <BeSQLite::EC::IECSqlArrayValue*> (&m_native->GetArray()));
        }
#endif
};


struct ECSqlValueEnumeratorImpl
    {
    // we need this class only because a managed class can have only a pointer to a struct as a member, not a struct
    BeSQLite::EC::IECSqlValueIterable&                 m_collection;
    BeSQLite::EC::IECSqlValueIterable::const_iterator  m_current;

    ECSqlValueEnumeratorImpl (BeSQLite::EC::IECSqlValueIterable& collection) : m_collection (collection), m_current (m_collection.begin()) {}
    };

/*=================================================================================**//**
* ECSqlValueEnumerator
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECSqlValueEnumerator : System::Collections::Generic::IEnumerator <ECSqlValue^>
{
private:
    // The lifetime of the collection is controllsed by the ECSqlyValue, not by the enumerator.
    BeSQLite::EC::IECSqlValueIterable&          m_collection;
    // We keep a reference to the owning ECSqlValue so it will not get collected while the ECSqlValueEnumerator is alive.
    ECSqlValue^                                 m_owner;

    ECSqlValueEnumeratorImpl*                   m_impl;
    ECSqlValue^                                 m_currentMember;

    ECSqlValue^     GetMember()
        {
        if (NULL == m_impl)
            return nullptr;

        BeSQLite::EC::IECSqlValueIterable::const_iterator theCurrent = m_impl->m_current;
        BeSQLite::EC::IECSqlValueIterable::const_iterator theEnd     = m_impl->m_collection.end();
        if (!(theCurrent != theEnd))
            return nullptr;

        BeSQLite::EC::IECSqlValue const &native = (*m_impl->m_current);
        return Convert::ECSqlValueToManaged (const_cast <BeSQLite::EC::IECSqlValue *> (&native), m_owner);
        }

    void            SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    bool            MoveNextInternal ()
        {
        if (nullptr == m_impl)
            {
            m_impl = new ECSqlValueEnumeratorImpl (m_collection);
            return m_impl->m_current != m_impl->m_collection.end();
            }

        BeSQLite::EC::IECSqlValueIterable::const_iterator endIterator = m_impl->m_collection.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }


internal:
    ECSqlValueEnumerator (BeSQLite::EC::IECSqlValueIterable& collection, ECSqlValue^ owner) : m_collection (collection)
        {
        m_owner         = owner;
        m_impl          = nullptr;
        m_currentMember = nullptr;
        }

public:

    /// <summary> Advances the enumerator to the next ECSqlValue of the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECSqlValue in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = nullptr;
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current ECSqlValue n the collection. </summary>
    property ECSqlValue^ Current
        {
        virtual ECSqlValue^ get () = System::Collections::Generic::IEnumerator <ECSqlValue^>::Current::get
            {
            return m_currentMember;
            }
        };

    ~ECSqlValueEnumerator ()
        {
        FreeImpl ();
        }

    !ECSqlValueEnumerator ()
        {
        FreeImpl();
        }
};



/*---------------------------------------------------------------------------------**//**
* PreparedECSqlStatementDeleter class
+---------------+---------------+---------------+---------------+---------------+------*/
struct  PreparedECSqlStatementDeleter : public RefCountedBase
    {
    BeSQLite::EC::CachedECSqlStatement* m_statement;

    PreparedECSqlStatementDeleter (BeSQLite::EC::CachedECSqlStatement* statement) : m_statement (statement)
        {
        AddRef();
        }

    ~PreparedECSqlStatementDeleter() { m_statement->Release(); }
    };



/*=================================================================================**//**
* PreparedECSqlStatement
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct   PreparedECSqlStatement
{
private:
    BeSQLite::EC::CachedECSqlStatement*     m_statement;
    ReleaseMarshaller*                      m_marshaller;

internal:
    PreparedECSqlStatement (BeSQLite::EC::CachedECSqlStatement* statement)
        {
        m_statement = statement;
        m_statement->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

public:
    /**
      * Bind a DgnObjectId value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindId (int parameterIndex, BeInt64Id^ value)
        {
        BENTLEY_NAMESPACE_NAME::BeInt64Id nativeValue (value->Value);
        m_statement->BindId (parameterIndex, nativeValue);
        }

    /**
      * Bind a string value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindText (int parameterIndex, System::String^ value)
        {
        pin_ptr<wchar_t const> valuePinned = PtrToStringChars (value);
        Utf8String utf8String (valuePinned);
        m_statement->BindText (parameterIndex, utf8String.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        }

    /**
      * Bind an integer value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindInt (int parameterIndex, int value)
        {
        m_statement->BindInt (parameterIndex, value);
        }

    /**
      * Bind a double value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindDouble (int parameterIndex, double value)
        {
        m_statement->BindDouble (parameterIndex, value);
        }

    /**
      * Bind a DPoint3d value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindPoint3d (int parameterIndex, GeometryNET::DPoint3d value)
        {
        pin_ptr<GeometryNET::DPoint3d>    valuePinned = &value;
        m_statement->BindPoint3d (parameterIndex, *(DPoint3dP)valuePinned);
        }


    /**
      * Bind a DRange3d value to the specified parameter
      * @param parameterIndex Parameter index
      * @param value Value to bind.
      */
    void BindDRange3d(int parameterIndex, GeometryNET::DRange3d value)
        {
        pin_ptr<GeometryNET::DRange3d>    valuePinned = &value;
        m_statement->BindBlob(parameterIndex, valuePinned, sizeof(::DRange3d), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
        }

    /**
     * Step the statement to the next row.
     * @return false if the query is done.
     */
    BeSQLiteDbResult Step ()
        {
        return (BeSQLiteDbResult) m_statement->Step();
        }

    /**
     * Return the index of an SQL parameter given its name. The index value returned is suitable for use
     * as the second parameter to one of the Bind functions. A zero is returned if no matching parameter is found.
     * @note a named parameter is a placeholder in a WHERE clause that is identified by a name, rather than by a simple ?
     * @param parameterName   parameterName Name of the binding parameter
     */
    int GetParameterIndex (System::String^ parameterName)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (parameterName);
        Utf8String utf8String (namePinned);
        return m_statement->GetParameterIndex (utf8String.c_str());
        }

    /**
     * Get the value of the specified column as a string
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    System::String^ GetValueText (int columnIndex)
        {
        Utf8String returnVal = m_statement->GetValueText (columnIndex);
        WString returnWString (returnVal.c_str(), true);
        return gcnew System::String (returnWString.c_str());
        }

    /**
     * Get the value of the specified column as an integer
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    int GetValueInt (int columnIndex)
        {
        return m_statement->GetValueInt (columnIndex);
        }

    /**
     * Get the value of the specified column as a double
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    double GetValueDouble (int columnIndex)
        {
        return m_statement->GetValueDouble (columnIndex);
        }

    /**
     * Get the value of the specified column as a DgnObjectId
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    BeInt64Id^ GetValueId (int columnIndex)
        {
        return gcnew BeInt64Id (m_statement->GetValueUInt64 (columnIndex));
        }

    /**
     * Get the value of the specified column as a DPoint3d
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    GeometryNET::DPoint3d GetValueDPoint3d (int columnIndex)
        {
        return Convert::DPoint3dToManaged (m_statement->GetValuePoint3d (columnIndex));
        }

    /**
     * Get the value of the specified column as a DRange3d
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    GeometryNET::DRange3d GetValueDRange3d (int columnIndex)
        {
        int size;
        void const* p = m_statement->GetValueBlob(columnIndex, &size);
        if ( (nullptr == p) || (size != sizeof(DRange3d)) )
            {
            throw gcnew DgnPlatformNETException ("ECSql ColumnType is not DRange3d type");
            }

        return Convert::DRange3dToManaged (*(DRange3dP) p);
        }

    /**
     * Get the value of the specified column as a DateTime string
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    System::DateTime GetValueDateTime (int columnIndex)
        {
        return Convert::DateTimeToManaged (m_statement->GetValueDateTime (columnIndex));
        }

#if defined (NEEDSWORK_ECDB_ArrayValue)
    /**
     * Get the value of the specified column as an Array
     * @param columnIndex Index of ECSQL column in result set (0-based)
     */
    ECSqlArrayValue^ GetValueArray (int columnIndex)
        {
        return Convert::ECSqlArrayValueToManaged (const_cast <BeSQLite::EC::IECSqlArrayValue*> (&m_statement->GetValueArray (columnIndex)));
        }
#endif

    ~PreparedECSqlStatement ()
        {
        if (nullptr == m_statement)
            return;

        m_statement->Release();
        m_statement  = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !PreparedECSqlStatement ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (new PreparedECSqlStatementDeleter (m_statement));
            m_marshaller = nullptr;
            m_statement = nullptr;
            }
        }
};


//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/15
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    mutable Utf8String                      m_issue;

    void _OnIssueReported(Utf8CP message) const override
        {
        m_issue = message;
        }
    };


/*=================================================================================**//**
* DgnDb - managed version of Dgn::DgnDb
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnDb
{
private:
    BDGN::DgnDbP        m_native;

internal:
    BDGN::DgnDbP        GetNative() { return m_native; }

    DgnDb (BDGN::DgnDbP native)
        {
        m_native = native;
        }

public:
    /** The collection of models in this DgnDb */
    property DgnModels^ Models
        {
        DgnModels^ get ()
            {
            return Convert::DgnModelsToManaged (&m_native->Models(), this);
            }
        }

    property SchemaManager^ Schemas
        {
        SchemaManager^ get ()
            {
            return Convert::SchemaManagerToManaged (&m_native->Schemas(), this);
            }
        }

    property DgnElementCollection^ Elements
        {
        DgnElementCollection^ get ()
            {
            return Convert::ElementCollectionToManaged (&m_native->Elements(), this);
            }
        }

    property DgnCategoryIdSet^ Categories
        {
        DgnCategoryIdSet^ get ();
        }

    property System::String^ FileName
        {
        System::String^ get ()
            {
            BeFileName nativeFileName = m_native->GetFileName();
            return gcnew System::String (nativeFileName.c_str());
            }
        }

    /** Find or load the Category with the specified ID. @param id The ID to look up. @param db The DgnDb that contains the Category. @return The Category if found */
    DgnCategory^ GetCategory (DgnCategoryId^ categoryId)
        {
        BDGN::DgnCategoryCPtr nativeCategory = BDGN::SpatialCategory::Get (*Convert::DgnDbToNative (this), BDGN::DgnCategoryId (categoryId->Value));
        return Convert::DgnCategoryToManaged (const_cast <BDGN::DgnCategoryP>(nativeCategory.get()), this);
        }

    /** Get a prepared ECSqlStatement for selecting rows from this DgnDb
      * @param ecsql    The body of the ECSql SELECT statement that is to be executed.
      * @return a prepared ECSql statement or null if the SQL statement is invalid.
      * @see ECClass::ECSqlName
      * @note Only SELECT statements can be prepared. If you do not specify the SELECT keyword, it will be prepended automatically.
      */
    PreparedECSqlStatement^ GetPreparedECSqlSelectStatement (System::String^ ecsqlFragment)
        {
        // the string we got from the user.
        pin_ptr<wchar_t const> ecsqlFragmentPinned = PtrToStringChars (ecsqlFragment);
        Utf8String  utf8EcsqlFragment (ecsqlFragmentPinned);

        // the string we are assembling.
        Utf8String  ecsql;

        if (!utf8EcsqlFragment.StartsWithI("SELECT"))
            ecsql.append("SELECT "); // We want to prevent callers from doing INSERT, UPDATE, or DELETE. Pre-pending SELECT will guarantee a prepare error if ecsqlFragment also contains one of those keywords.

        ecsql.append(utf8EcsqlFragment);

        ECDbIssueListener issues;
        m_native->AddIssueListener (issues);
        BeSQLite::EC::CachedECSqlStatementPtr newStatement = m_native->GetPreparedECSqlStatement (ecsql.c_str());
        m_native->RemoveIssueListener();
        if (!newStatement.IsValid())
            {
            System::String^ issueString = gcnew System::String (issues.m_issue.c_str());
            System::String^ errorMsg = System::String::Format ("{0} = {1}", ecsqlFragment, issueString);
            throw gcnew DgnPlatformNETException (errorMsg);
            return nullptr;
            }

        return gcnew PreparedECSqlStatement (newStatement.get());
        }

    /** Save changes to the DgnDb, marking the end of a transaction. If undo/redo is enabled, this creates an undo point. @return non-zero if the insert failed. */
    int SaveChanges()
        {
        return m_native->SaveChanges ();
        }

};


/*=================================================================================**//**
* internal stuff needed for implementing the IdSetEnumerator.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
typedef BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>  BeInt64IdSet;

struct IdSetEnumeratorImpl
    {
    BeInt64IdSet&                   m_set;
    BeInt64IdSet::const_iterator    m_current;

    IdSetEnumeratorImpl (BeInt64IdSet& set) : m_set (set), m_current (m_set.begin()) {}
    };

/*=================================================================================**//**
* Templated class that implements IEnumerator for DgnXXXIds.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
generic <class T_idType> where T_idType : BeInt64Id
public ref struct IdSetEnumerator : System::Collections::Generic::IEnumerator <T_idType>
{
private:
    BeInt64IdSet*               m_nativeSet;
    IdSetEnumeratorImpl*        m_impl;
    T_idType                    m_currentMember;
    System::Object^             m_owner;

    T_idType        GetMember()
        {
        if ( (nullptr == m_nativeSet) || (nullptr == m_impl) )
            return T_idType();

        BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>::const_iterator theCurrent = m_impl->m_current;
        BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>::const_iterator theEnd     = m_nativeSet->end();
        if (!(theCurrent != theEnd))
            return T_idType();

        BENTLEY_NAMESPACE_NAME::BeInt64Id native = (*m_impl->m_current);
        T_idType managed = (T_idType)(System::Activator::CreateInstance (T_idType::typeid));
        managed->SetValue (native.GetValue());
        return managed;
        }


    bool MoveNextInternal ()
        {
        if (nullptr == m_nativeSet)
            return false;

        if (nullptr == m_impl)
            {
            m_impl = new IdSetEnumeratorImpl (*m_nativeSet);
            return m_impl->m_current != m_impl->m_set.end();
            }

        BeInt64IdSet::const_iterator endIterator = m_impl->m_set.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }

internal:
    IdSetEnumerator (BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>* nativeSet, System::Object^ owner)
        {
        m_nativeSet     = nativeSet;
        m_owner         = owner;
        m_impl          = nullptr;
        m_currentMember = T_idType();
        }

public:
    /// <summary> Advances the enumerator to the next T_idType in the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECSqlValue in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = T_idType();
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current T_idType in the collection. </summary>
    property T_idType Current
        {
        virtual T_idType get () = System::Collections::Generic::IEnumerator <T_idType>::Current::get
            {
            return m_currentMember;
            }
        };


    ~IdSetEnumerator ()
        {
        }
};


/*=================================================================================**//**
* Templated class supplies an IEnumerable for DgnXXXIds (DgnModelIds, DgnCategoryIds, etc.)
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
generic <class T_idType> where T_idType : BeInt64Id
public ref struct IdSet : System::Collections::Generic::IEnumerable <T_idType>
{
internal:
    BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>*  m_nativeSet;

public:
    void FromString (System::String^ in)
        {
        pin_ptr<wchar_t const> inPinned = PtrToStringChars (in);
        Utf8String utf8In (inPinned);
        m_nativeSet->FromString (utf8In.c_str());
        }

    virtual System::String^ ToString () override
        {
        Utf8String nativeString = m_nativeSet->ToString ();
        return gcnew System::String (nativeString.c_str());
        }

    virtual System::Collections::IEnumerator^ RawGetEnumerator () = System::Collections::IEnumerable::GetEnumerator
        {
        return GetEnumerator ();
        }

    virtual System::Collections::Generic::IEnumerator <T_idType>^ GetEnumerator ()
        {
        return gcnew IdSetEnumerator<T_idType> (m_nativeSet, this);
        }

    ~IdSet ()
        {
        this->!IdSet();
        }

    !IdSet ()
        {
        if (nullptr != m_nativeSet)
            {
            delete m_nativeSet;
            m_nativeSet = nullptr;
            }
        }

};




/*=================================================================================**//**
* Specialization of IdSet for DgnCategoryIds.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnCategoryIdSet : IdSet <DgnCategoryId^>
    {
internal:
    DgnCategoryIdSet (DgnDb^ dgnDb)
        {
        BDGN::DgnDbP            dgnDbNative = Convert::DgnDbToNative (dgnDb);
        BDGN::ElementIterator   iterator = BDGN::SpatialCategory::MakeIterator (*dgnDbNative, nullptr, nullptr);
        bvector<BDGN::DgnCategoryId>* idList = new bvector <BDGN::DgnCategoryId>();
        iterator.BuildIdList<BDGN::DgnCategoryId>(*idList);
        m_nativeSet = reinterpret_cast <BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>*> (idList);

        }
    };


// This method on DgnDb can't be inlined.
DgnCategoryIdSet^ DgnDb::Categories::get ()
    {
    return gcnew DgnCategoryIdSet (this);
    }

/*=================================================================================**//**
* AuthorityIssuedCode - managed version of BentleyApi::Dgn::DgnCode
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct AuthorityIssuedCode
{
private:
    BDGN::DgnCodeCP     m_native;
    System::Object^     m_owner;

internal:
    AuthorityIssuedCode (BDGN::DgnCodeCP native, Object^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

    AuthorityIssuedCode (BDGN::DgnCode native, Object^ owner)
        {
        m_native = new BDGN::DgnCode (native);
        m_owner  = owner;
        }

    BDGN::DgnCodeCP  GetNative()
        {
        return m_native;
        }

public:
    //! Get the value for this DgnCode
    property System::String^ Value
        {
        System::String^ get ()
            {
            Utf8CP value = m_native->GetValueCP();
            WString wValue (value, true);
            return gcnew System::String (wValue.c_str());
            }
        }

    //! Get the CodeSpecId of the CodeSpec that issued this DgnCode.
    property CodeSpecId^ Authority
        {
        CodeSpecId^ get ()
            {
            return gcnew CodeSpecId (m_native->GetCodeSpecId().GetValue());
            }
        }

    ~AuthorityIssuedCode ()
        {
        if (nullptr == m_owner)
            this->!AuthorityIssuedCode();
        }

    !AuthorityIssuedCode ()
        {
        // only delete if not owned by another object (for example, a DgnElement).
        if ( (nullptr != m_native) && (nullptr == m_owner) )
            {
            delete m_native;
            m_native = nullptr;
            }
        }
};

/*=================================================================================**//**
* Specialization of IdSet for DgnModelIds.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnModelIdSet : IdSet <DgnModelId^>
    {
internal:
    DgnModelIdSet (BDGN::DgnModels* nativeModels, System::String^ className, System::String^ whereClause, System::String^ orderByClause)
        {
        // return empty DgnModelIdSet
        if (nullptr == className)
            m_nativeSet = nullptr;
        else
            {
            pin_ptr<wchar_t const> classNamePinned = PtrToStringChars(className);
            Utf8String utf8ClassName (classNamePinned);

            Utf8String  utf8WhereClause;
            Utf8CP      utf8WhereClauseP = nullptr;
            if (nullptr != whereClause)
                {
                pin_ptr<wchar_t const> whereClausePinned = PtrToStringChars(whereClause);
                utf8WhereClause.Assign (whereClausePinned);
                utf8WhereClauseP = utf8WhereClause.c_str();
                }

            Utf8String  utf8OrderByClause;
            Utf8CP      utf8OrderByClauseP = nullptr;
            if (nullptr != orderByClause)
                {
                pin_ptr<wchar_t const> orderByClausePinned = PtrToStringChars(orderByClause);
                utf8OrderByClause.Assign (orderByClausePinned);
                utf8OrderByClauseP = utf8OrderByClause.c_str();
                }

            BDGN::ModelIterator iterator = nativeModels->MakeIterator (utf8ClassName.c_str(), utf8WhereClauseP, utf8OrderByClauseP);

            bvector<BDGN::DgnModelId>* idList = new bvector <BDGN::DgnModelId>();
            iterator.BuildIdList (*idList);
            m_nativeSet = reinterpret_cast <BeSQLite::IdSet <BENTLEY_NAMESPACE_NAME::BeInt64Id>*> (idList);
            }
        }
    };



/*=================================================================================**//**
*  DgnModels - managed version of Dgn::DgnModels
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnModels
{
private:
    // this is a member of the DgnDb, and is never deleted.
    BDGN::DgnModels*    m_native;

    // keep a reference to the DgnDb so it doesn't get collected out from under us.
    DgnDb^              m_dgnDb;

internal:
    DgnModels (BDGN::DgnModels* models, DgnDb^ dgnDb)
        {
        m_native = models;
        m_dgnDb  = dgnDb;
        }

public:
    /** Find or load the model identified by the specified ID. @param id The model ID. @return The loaded model or null if not found */
    DgnModel^ GetModel (DgnModelId^ modelId)
        {
        BDGN::DgnModelPtr nativeModel = m_native->GetModel (BDGN::DgnModelId (modelId->Value));
        return Convert::DgnModelToManaged (nativeModel.get(), m_dgnDb, true);
        }

    //! Return a collection of models of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name.  For example: BIS_SCHEMA(BIS_CLASS_PhysicalModel)
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DgnModelIdSet^ GetModels (System::String^ className, System::String^ whereClause, System::String^ orderByClause)
        {
        return gcnew DgnModelIdSet (m_native, className, whereClause, orderByClause);
        }

    //! Return a collection of models of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name.  For example: BIS_SCHEMA(BIS_CLASS_PhysicalModel)
    DgnModelIdSet^ GetModels (System::String^ className)
        {
        return GetModels (className, nullptr, nullptr);
        }



};


/*=================================================================================**//**
*  DgnElementCollection - managed version of Dgn::DgnElements
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
/** DgnElements - Projection of BentleyApi::Dgn::DgnElements */
public ref struct DgnElementCollection
{
private:
    // DgnElements is a member of DgnDb, so it is never deleted.
    BDGN::DgnElements*  m_native;

    // Keep a reference to the DgnDb so it isn't collected out from under us.
    DgnDb^              m_owner;

internal:
    DgnElementCollection (BDGN::DgnElements* elements, DgnDb^ owner)
        {
        m_native = elements;
        m_owner  = owner;
        }

public:
    /** The DgnDb that contains this collection of elements */
    property DgnNET::DgnDb^  DgnDb
        {
        DgnNET::DgnDb^ get ()
            {
            return m_owner;
            }
        };

    /**
     * Look up an element in the pool of loaded elements for this DgnDb.
     * @return A pointer to the element, or nullptr if the is not in the pool.
     * @note This method will return null if the element is not currently loaded. That does not mean the element doesn't exist in the database.
     */
    DgnElement^     FindLoadedElement (DgnElementId^ elementId)
        {
        BDGN::DgnElementCP nativeElement = m_native->FindLoadedElement (BDGN::DgnElementId (elementId->Value));
        return Convert::DgnElementToManaged (const_cast <BDGN::DgnElementP>(nativeElement));
        }

    /** Look for the element that has the specified code
      * @param codeSpecName    The name of the CodeSpec
      * @param codeValue            The name portion of the Code
      * @param nameSpace            The namespace portion of the Code
      * @return the DgnElementId of the element found or null if no element with that specified code was found
      */
    DgnElementId^ QueryElementIdByCode (System::String^ codeSpecName, System::String^ codeValue, System::String^ nameSpace)
        {
        pin_ptr<wchar_t const> codeSpecPinned  = PtrToStringChars (codeSpecName);
        pin_ptr<wchar_t const> codeValuePinned      = PtrToStringChars (codeValue);
        pin_ptr<wchar_t const> nameSpacePinned      = PtrToStringChars (nameSpace);
        Utf8String utf8CodeSpec (codeSpecPinned);
        Utf8String utf8CodeValue (codeValuePinned);
        Utf8String utf8NameSpace (nameSpacePinned);
        BDGN::DgnElementId nativeElementId = m_native->QueryElementIdByCode (utf8CodeSpec.c_str(), utf8CodeValue.c_str(), utf8NameSpace.c_str());
        return gcnew DgnElementId (nativeElementId.GetValue());
        }

    /**
     * Get a DgnElement from the DgnDb by its DgnElementId.
     * @param elementId             The element's DgnElementId
     * @remarks The element is loaded from the database if necessary.
     * @return Invalid if the element does not exist.
     */
    DgnElement^ GetElement (DgnElementId^ elementId)
        {
        BDGN::DgnElementCPtr nativeElement = m_native->GetElement (BDGN::DgnElementId (elementId->Value));
        return Convert::DgnElementToManaged (const_cast <BDGN::DgnElementP>(nativeElement.get()));
        }
};

/*=================================================================================**//**
* DgnCategory - managed version of Dgn::DgnCategory
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnCategory
{
private:
    BDGN::DgnCategoryP      m_native;
    DgnDb^                  m_owner;

internal:
    DgnCategory (BDGN::DgnCategoryP native, DgnDb^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

public:
    /** The DgnDb that contains this Category */
    property DgnNET::DgnDb^ DgnDb
        {
        DgnNET::DgnDb^  get ()
            {
            return m_owner;
            }
        };

    /** The ID of this Category */
    property DgnCategoryId^ CategoryId
        {
        DgnCategoryId^ get ()
            {
            return gcnew DgnCategoryId (m_native->GetCategoryId().GetValue());
            }
        };

    /** The ID of the default SubCategory of this Category */
    property DgnCategoryId^ DefaultSubCategoryId
        {
        DgnCategoryId^ get ()
            {
            return gcnew DgnCategoryId (m_native->GetDefaultSubCategoryId().GetValue());
            }
        };


    /** The name of this Category */
    property System::String^    Name
        {
        System::String^ get ()
            {
            Utf8String  nativeName  = m_native->GetCategoryName();
            WString     wNativeName (nativeName.c_str(), true);
            return gcnew System::String (nativeName.c_str());
            }
        };

    property bool IsUser    { bool get () { return m_native->IsUserCategory(); } }
    property bool IsSystem  { bool get () { return m_native->IsSystemCategory(); } }

    /** Look up the ID of the Category with the specified name. @param name The name to look up. @param db The DgnDb that contains the Category. @return The ID of the Category if found */
    static DgnCategoryId^ QueryCategoryId (DgnNET::AuthorityIssuedCode^ code, DgnNET::DgnDb^ dgnDb)
        {
        if ( (nullptr == code) || (nullptr == dgnDb) )
            return nullptr;

        BDGN::DgnCodeCP dgnCode = code->GetNative();
        if (nullptr == dgnCode)
            return nullptr;

        BDGN::DgnCategoryId nativeCategoryId = BDGN::SpatialCategory::QueryCategoryId (*Convert::DgnDbToNative (dgnDb), *dgnCode);
        return gcnew DgnCategoryId (nativeCategoryId.GetValue());
        }

    /** Find or load the Category with the specified ID. @param id The ID to look up. @param db The DgnDb that contains the Category. @return The Category if found */
    static DgnCategory^ QueryCategory (DgnCategoryId^ categoryId,  DgnNET::DgnDb^ dgnDb)
        {
        BDGN::DgnCategoryCPtr nativeCategory = BDGN::SpatialCategory::Get (*Convert::DgnDbToNative (dgnDb), BDGN::DgnCategoryId (categoryId->Value));
        return gcnew DgnCategory (const_cast <BDGN::DgnCategoryP>(nativeCategory.get()), dgnDb);
        }

    /** Get the set of all DgnCategoryIDs in the Db. @param db The DgnDb to query. @return the set of all category IDs. */
    static DgnCategoryIdSet^ QueryCategories (DgnNET::DgnDb^ dgnDb)
        {
        return gcnew DgnCategoryIdSet (dgnDb);
        }

};

/*---------------------------------------------------------------------------------**//**
* DgnElementDeleter class
+---------------+---------------+---------------+---------------+---------------+------*/
struct  DgnElementDeleter : public RefCountedBase
    {
    BDGN::DgnElementP   m_element;

    DgnElementDeleter (BDGN::DgnElementP element) : m_element (element)
        {
        AddRef();
        }

    ~DgnElementDeleter () { m_element->Release(); }
    };

/*=================================================================================**//**
* DgnElement - managed version of Dgn::DgnElement
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnElement
    {
private:
    BDGN::DgnElementP       m_native;
    ReleaseMarshaller*      m_marshaller;

internal:

    DgnElement (BDGN::DgnElementP native)
        {
        // keep it and add a reference.
        m_native = native;
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    BDGN::DgnElementP GetNative() { return m_native; }


public:
    property DgnElementId^ ElementId
        {
        DgnElementId^ get()
            {
            BDGN::DgnElementId nativeId = m_native->GetElementId();
            return gcnew DgnElementId (nativeId.GetValue());
            }
        }

    /** The Element's Code */
    property AuthorityIssuedCode^ Code
        {
        AuthorityIssuedCode^ get ()
            {
            BDGN::DgnCode const& nativeCode = m_native->GetCode();
            return gcnew AuthorityIssuedCode (&nativeCode, this);
            }
        }

    /** The Model that contains the Element */
    property DgnModel^  Model
        {
        DgnModel^ get ()
            {
            // NEEDSWORK_DgnPlatformNET_Model may need to worry more about returning the same managed Model object for a given nativeModel.
            BDGN::DgnModelPtr nativeModel = m_native->GetModel ();
            DgnDb^ dgnDb                  = Convert::DgnDbToManaged (&nativeModel->GetDgnDb());
            return Convert::DgnModelToManaged (nativeModel.get(), dgnDb, false);
            }
        }

    /** The ModelId of the Model that contains the Element */
    property DgnModelId^  ModelId
        {
        DgnModelId^ get ()
            {
            // NEEDSWORK_DgnPlatformNET_Model may need to worry more about returning the same managed Model object for a given nativeModel.
            BDGN::DgnModelId nativeModelId = m_native->GetModelId ();
            return gcnew DgnModelId (nativeModelId.GetValue());
            }
        }


    /** The ECClass of this element */
    property ECClass^ ElementClass
        {
        ECClass^    get ()
            {
            ECN::ECClassCP nativeClass = m_native->GetElementClass();
            return Convert::ECClassToManaged (const_cast <ECN::ECClassP> (nativeClass), this);
            }
        }


    /** Insert this Element into its Model in the DgnDb. @return non-zero if the insert failed. */
    int Insert()
        {
        BDGN::DgnDbStatus   status;
        BDGN::DgnElementCPtr newElement = m_native->Insert (&status);
        // free the old element and substitute the new one.
        m_native->Release();
        m_native = const_cast <BDGN::DgnElementP>(newElement.get());
        m_native->AddRef();
        return (int) status;
        }

    /** Update this Element in its Model in the DgnDb. @return non-zero if the update failed. */
    int Update()
        {
        BDGN::DgnElementCPtr newElement = m_native->Update ();
        m_native->Release();
        if (!newElement.IsValid())
            {
            m_native = nullptr;
            return -2;
            }

        m_native = const_cast <BDGN::DgnElementP>(newElement.get());
        m_native->AddRef();
        return 0;
        }

#if defined (NEEDSWORK_DesktopPlatform_DgnClassId)
    /** Set the Parent of this Element. @param parent The parent element. */
    void SetParent(DgnElement^ parent, )
        {
        if (nullptr == parent)
            m_native->SetParentId (BDGN::DgnElementId());
        else
            m_native->SetParentId (parent->m_native->GetElementId());
        }
#endif

    /**
     * Get the value of a property that is defined by the element's class.
     * @param name The name of property
     * @return the value of the property or null if the property is not found
     */
    ECValue^ GetPropertyValue (System::String^ name)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (name);
        Utf8String utf8Name (namePinned);
        ECN::ECValue nativeValue;
        BDGN::DgnDbStatus status;
        if (BDGN::DgnDbStatus::Success != (status = m_native->GetPropertyValue (nativeValue, utf8Name.c_str())))
            return nullptr;

        return Convert::ECValueToManaged (new ECN::ECValue (nativeValue));
        }

    /**
     * Set the value of a property that is defined by the element's class
     * @param name      The name of property
     * @param value     The new value for the property
     * @return non-zero error status if the property is not found
     */
    int SetPropertyValue (System::String^ name, ECValue^ value)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (name);
        Utf8String utf8Name (namePinned);
        ECN::ECValueCP nativeValue = Convert::ECValueToNative (value);
        return (int) m_native->SetPropertyValue (utf8Name.c_str(), *nativeValue);
        }

#if defined (BENTLEY_CHANGE)
    /**
     * Get a handle to a user property on this element.
     * @note If the user property does not already exist, this function will create it.
     * You can use the returned object to both get and set the property's value and metadata.
     * @note Call DgnElement::Update after modifying a user property's value or metadata in order to save your changes.
     * @param name The name of the property
     * @return an object that accesses the value and metadata of the specified user property on this element.
     * @see ContainsUserProperty
     */
    AdHocJsonPropertyValue^ GetUserProperty (System::String^ name)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (name);
        Utf8String utf8Name (namePinned);
        ECN::AdHocJsonPropertyValue nativeValue = m_native->GetUserProperty (utf8Name.c_str());
        return Convert::AdHocJsonPropertyValueToManaged (&nativeValue);
        }

    /**
     * Check to see if the element has the specified user property
     * @param name The name of the property
     */
    bool ContainsUserProperty (System::String^ name)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (name);
        Utf8String utf8Name (namePinned);
        return m_native->ContainsUserProperty (utf8Name.c_str());
        }

    /** Remove the specified user property from this element
     * @param name  The name of the property
     */
    void RemoveUserProperty (System::String^ name)
        {
        pin_ptr<wchar_t const> namePinned = PtrToStringChars (name);
        Utf8String utf8Name (namePinned);
        m_native->RemoveUserProperty (utf8Name.c_str());
        }
#endif

    /** Cast this element to GeometrySource if possible */
    GeometrySource^ ToGeometrySource()
        {
        BDGN::GeometrySourceP nativeGeometrySource = m_native->ToGeometrySourceP();
        if (nullptr == nativeGeometrySource)
            return nullptr;
        return Convert::GeometrySourceToManaged (nativeGeometrySource, this);
        }

    /** Cast this element to GeometrySource3d if possible */
    GeometrySource3d^ ToGeometrySource3d()
        {
        BDGN::GeometrySource3dP nativeGeometrySource = m_native->ToGeometrySource3dP();
        if (nullptr == nativeGeometrySource)
            return nullptr;
        return Convert::GeometrySource3dToManaged (nativeGeometrySource, this);
        }

    /** Cast this element to GeometrySource2d if possible */
    GeometrySource2d^ ToGeometrySource2d()
        {
        BDGN::GeometrySource2dP nativeGeometrySource = m_native->ToGeometrySource2dP();
        if (nullptr == nativeGeometrySource)
            return nullptr;
        return Convert::GeometrySource2dToManaged (nativeGeometrySource, this);
        }

    /**
    * Add any required locks and/or codes to the specified request in preparation for the specified operation
    * @param request    The request to which to add the required locks and/or codes
    * @param opcode     The operation to be performed
    */
    RepositoryStatus PopulateRequest (RepositoryRequest^ request, BeSQLiteDbOpcode opcode)
        {
        BDGN::IBriefcaseManager::Request* nativeRequest = Convert::RepositoryRequestToNative (request);
        return (DgnNET::RepositoryStatus)m_native->PopulateRequest (*nativeRequest, (BeSQLite::DbOpcode) opcode);
        }

    /**
     * Create a new DgnElement
     * @param model The model that is to contain the new element
     * @param elementClassName The name of the element's ECClass.
     * @return a new, non-persistent DgnElement or null if one of the parameters is invalid
     * @see Insert
    */

internal:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      06/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    // copied from dgnjsapi.cpp
    static BDGN::DgnElementPtr CreateElementByClass (BDGN::DgnModelR model, Utf8CP ecSqlClassName)
        {
        if (!ecSqlClassName || !*ecSqlClassName)
            return nullptr;

        Utf8CP dot = strchr(ecSqlClassName, '.');
        if (nullptr == dot)
            {
            // *** NEEDS WORK: BDGN::DgnPlatformLib::GetHost().GetScriptAdmin().HandleScriptError(BDGN::DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "malformed ECSql ecclass name", ecSqlClassName);
            return nullptr;
            }
        Utf8String ecschema(ecSqlClassName, dot);
        Utf8String ecclass(dot+1);
        BDGN::DgnDbR db = model.GetDgnDb();
        BDGN::DgnClassId pclassId = BDGN::DgnClassId(db.Schemas().GetClassId(ecschema.c_str(), ecclass.c_str()));
        if (!pclassId.IsValid())
            {
            // *** NEEDS WORK: BDGN::DgnPlatformLib::GetHost().GetScriptAdmin().HandleScriptError(BDGN::DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "ECClass not found", ecSqlClassName);
            return nullptr;
            }
        BDGN::dgn_ElementHandler::Element* handler = BDGN::dgn_ElementHandler::Element::FindHandler(model.GetDgnDb(), pclassId);
        if (nullptr == handler)
            {
            // *** NEEDS WORK: BDGN::DgnPlatformLib::GetHost().GetScriptAdmin().HandleScriptError(BDGN::DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "handler not found", ecSqlClassName);
            return nullptr;
            }
        BDGN::DgnElementPtr el = handler->Create(BDGN::DgnElement::CreateParams(db, model.GetModelId(), pclassId));
        if (!el.IsValid())
            {
            Utf8PrintfString details ("class: %s", ecSqlClassName);
            // *** NEEDS WORK: BDGN::DgnPlatformLib::GetHost().GetScriptAdmin().HandleScriptError(BDGN::DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, "dgn_ElementHandler::Geometric3d::GetHandler().Create failed", details.c_str());
            return nullptr;
            }
        return el;
        }

public:
    static DgnElement^ Create (DgnModel^ model, System::String^ elementClassName)
        {
        pin_ptr<wchar_t const> classNamePinned = PtrToStringChars (elementClassName);
        Utf8String utf8ClassName (classNamePinned);
        BDGN::DgnModelP     nativeModel   = Convert::DgnModelToNative (model);
        BDGN::DgnElementPtr nativeElement = CreateElementByClass (*nativeModel, utf8ClassName.c_str());
        return Convert::DgnElementToManaged (nativeElement.get());
        }

    ~DgnElement ()
        {
        if (nullptr == m_native)
            return;
        m_native->Release();

        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !DgnElement ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (new DgnElementDeleter (m_native));
            m_marshaller = nullptr;
            m_native = nullptr;
            }
        }
};

/*=================================================================================**//**
* GeometrySource - managed equivalent of Dgn::GeometrySource
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometrySource
{
private:
    BDGN::GeometrySourceP   m_native;

protected:
    DgnElement^             m_element;

internal:
    GeometrySource (BDGN::GeometrySourceP native, DgnElement^ element)
        {
        // the GeometrySource retains a pointer to the element so the element isn't garbage collected.
        m_native    = native;
        m_element   = element;
        }

    BDGN::GeometrySourceP  GetNative() { return m_native; }

public:
    /** Get the element's DgnCategoryId */
    property DgnCategoryId^ CategoryId
        {
        DgnCategoryId^ get ()
            {
            return gcnew DgnCategoryId (m_native->GetCategoryId().GetValue());
            }
        }

    /** The element's geometry (read-only) */
    /** NEEDSWORK - do we really want this to be a property? **/
    property GeometryCollection^ Geometry
        {
        GeometryCollection^ get ()
            {
            return Convert::GeometrySourceToCollection (this);
            }
        }

    /** Cast this element to DgnElement */
    DgnElement^ ToDgnElement()
        {
        return m_element;
        }

    GeometrySource2d^ ToGeometrySource2d()
        {
        return m_element->ToGeometrySource2d();
        }

    GeometrySource3d^ ToGeometrySource3d()
        {
        return m_element->ToGeometrySource3d();
        }
 };

/*=================================================================================**//**
* GeometrySource2d - managed equivalent of Dgn::GeometrySource
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometrySource2d : public GeometrySource
{
internal:
    GeometrySource2d (BDGN::GeometrySource2dP native, DgnElement^ element) : GeometrySource (native, element) {}
};

public ref struct GeometrySource3d : public GeometrySource
{
internal:
    GeometrySource3d (BDGN::GeometrySource3dP native, DgnElement^ element) : GeometrySource (native, element) {}

    /** Get the placement of this element */
    property Placement3d^ Placement
        {
        Placement3d^ get ()
            {
            return gcnew Placement3d (const_cast <BDGN::Placement3dP> ( &((BDGN::GeometrySource3dCP)GetNative())->GetPlacement()));
            }
        }

    /** Transform the element's Placement
     * @param transform The transform to apply to the element's Placement. The transform must be pure rotation and/or translation.
     * @return non-zero error status if the element could not be transformed or if \a transform is invalid.
    */
    int Transform (GeometryNET::DTransform3d transform)
        {
        TransformCP nativeTransform = Convert::TransformToNative (transform);
        BDGN::DgnElementP nativeElement = Convert::DgnElementToNative (m_element);
        return (int) BDGN::DgnElementTransformer::ApplyTransformTo (*nativeElement, *nativeTransform);
        }

};


/*=================================================================================**//**
* GeometricElement
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometricElement : public DgnElement
{
internal:
    GeometricElement (BDGN::GeometricElement* native) : DgnElement (native)
        {
        }

public:
    /** Get the element's DgnCategoryId */
    property DgnCategoryId^ CategoryId
        {
        DgnCategoryId^ get ()
            {
            BDGN::GeometrySourceCP      geometrySourceNative = GetNative()->ToGeometrySource();
            if (nullptr == geometrySourceNative)
                return gcnew DgnCategoryId (0);

            return gcnew DgnCategoryId (geometrySourceNative->GetCategoryId().GetValue());
            }
        }

    /** The element's geometry (read-only) */
    /** NEEDSWORK - do we really want this to be a property? **/
    property GeometryCollection^ Geometry
        {
        GeometryCollection^ get ()
            {
            GeometrySource^ geometrySource;
            if (nullptr == (geometrySource = this->ToGeometrySource()))
                return nullptr;

            return Convert::GeometrySourceToCollection (geometrySource);
            }
        }

    /** Cast this element to DgnElement */
    DgnElement^     ToDgnElement()
        {
        return this;
        }
};


/*=================================================================================**//**
* GeometricElement3d
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometricElement3d : public GeometricElement
{
internal:
    GeometricElement3d (BDGN::GeometricElement3d* native) : GeometricElement (native)
        {
        }

public:
    /** Get the placement of this element */
    property Placement3d^     Placement
        {
        Placement3d^ get ()
            {
            BDGN::GeometricElement3d*     geometricElementNative = dynamic_cast <BDGN::GeometricElement3d*> (GetNative());
            if (nullptr == geometricElementNative)
                return nullptr;

            return gcnew Placement3d (geometricElementNative->GetPlacement(), this);
            }
        }

    /** Transform the element's Placement
     * @param transform The transform to apply to the element's Placement. The transform must be pure rotation and/or translation.
     * @return non-zero error status if the element could not be transformed or if \a transform is invalid.
    */
    int Transform (GeometryNET::DTransform3d transform)
        {
        BDGN::GeometricElement3d*     geometricElementNative = dynamic_cast <BDGN::GeometricElement3d*> (GetNative());
        if (nullptr == geometricElementNative)
            return -1;
        TransformCP transformNative = Convert::TransformToNative (transform);
        return (BDGN::DgnDbStatus::Success == BDGN::DgnElementTransformer::ApplyTransformTo (*geometricElementNative, *transformNative)) ? 0: -2;
        }

    /**
     * Create a new GeometricElement3d
     * @param model The model that is to contain the new element
     * @param categoryId The new element's DgnCategoryId
     * @param elementClassName The name of the element's ECClass. Must be a subclass of GeometricElement3d.
     * @return a new, non-persistent GeometricElement3d or null if one of the parameters is invalid
     * @see Insert
    */
    static GeometricElement3d^ CreateGeometricElement3d (DgnModel^ model, DgnCategoryId^ categoryId, System::String^ elementClassName)
        {
        pin_ptr<wchar_t const> classNamePinned = PtrToStringChars (elementClassName);
        Utf8String utf8ClassName (classNamePinned);
        BDGN::DgnModelP     nativeModel   = Convert::DgnModelToNative (model);

        BDGN::DgnElementPtr         nativeElement = CreateElementByClass (*nativeModel, utf8ClassName.c_str());
        BDGN::GeometricElement3d*   geometricElementNative = dynamic_cast <BDGN::GeometricElement3d*> (nativeElement.get());
        if (nullptr == geometricElementNative)
            {
            System::String^ exceptionString = System::String::Format (L"{0} does not represent a GeometryElement3D", elementClassName);
            throw gcnew DgnPlatformNETException (exceptionString);
            }

        BDGN::DgnCategoryId         nativeCategoryId (categoryId->Value);
        BDGN::GeometrySourceP       geometrySource = geometricElementNative->ToGeometrySourceP();
        geometrySource->SetCategoryId (nativeCategoryId);

        return dynamic_cast <GeometricElement3d^>(Convert::DgnElementToManaged (nativeElement.get()));
        }
};

/*=================================================================================**//**
* DgnModel - Managed equivalent of DgnModel
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnModel
{
private:
    BDGN::DgnModelP     m_native;
    DgnDb^              m_dgnDb;
    ReleaseMarshaller*  m_marshaller;

internal:
    DgnModel (BDGN::DgnModelP native, DgnDb^ dgnDb, bool needRelease)
        {
        m_native = native;
        m_dgnDb  = dgnDb;

        // some models we create (for example from enumerating the collection, some we just come across and don't add reference to.
        if (needRelease)
            {
            m_marshaller = ReleaseMarshaller::GetMarshaller();
            m_native->AddRef();
            }
        }

    BDGN::DgnModelP     GetNative() { return m_native; }


    // called from DesktopPlatform
    static DgnModel^    GetModel (System::IntPtr modelPointer)
        {
        BDGN::DgnModelP     nativeDgnModel  = (BDGN::DgnModelP) modelPointer.ToPointer();
        DgnNET::DgnDb^      dgnDbManaged    = Convert::DgnDbToManaged (&nativeDgnModel->GetDgnDb());

        return gcnew DgnModel (nativeDgnModel, dgnDbManaged, true);
        }

public:
    property DgnModelId^ ModelId
        {
        DgnModelId^ get ()
            {
            return gcnew DgnModelId (m_native->GetModelId().GetValue());
            }
        }

    /** The DgnDb that contains this model */
    property DgnNET::DgnDb^     DgnDb
        {
        DgnNET::DgnDb^ get()
            {
            return m_dgnDb;
            }
        }


    /** The name of this model */
    property System::String^     Name
        {
        System::String^ get ()
            {
            Utf8String nativeName = m_native->GetName();
            return gcnew System::String (nativeName.c_str());
            }
        }

    property bool IsGeometricModel      { bool get () { return m_native->IsGeometricModel();    } }
    property bool IsSpatialModel        { bool get () { return m_native->IsSpatialModel();      } }
    property bool IsPhysicalModel       { bool get () { return m_native->IsPhysicalModel();     } }
    property bool Is2dModel             { bool get () { return m_native->Is2dModel();           } }
    property bool Is3dModel             { bool get () { return m_native->Is3dModel();           } }
    property bool IsRoleModel           { bool get () { return m_native->IsRoleModel();         } }
    property bool IsInformationModel    { bool get () { return m_native->IsInformationModel();  } }
    property bool IsDefinitionModel     { bool get () { return m_native->IsDefinitionModel();   } }
    property bool IsSheetModel          { bool get () { return m_native->IsSheetModel();        } }
    property bool IsDictionaryModel     { bool get () { return m_native->IsDictionaryModel();   } }


    /**
    * Add any required locks and/or codes to the specified request in preparation for the specified operation
    * @param request    The request to which to add the required locks and/or codes
    * @param opcode     The operation to be performed
    */
    RepositoryStatus PopulateRequest (RepositoryRequest^ request, BeSQLiteDbOpcode opcode)
        {
        BDGN::IBriefcaseManager::Request* nativeRequest = Convert::RepositoryRequestToNative (request);
        return (DgnNET::RepositoryStatus)m_native->PopulateRequest (*nativeRequest, (BeSQLite::DbOpcode) opcode);
        }

    ~DgnModel ()
        {
        // see if it needs to be released.
        if ( (nullptr == m_marshaller) || (nullptr == m_native) )
            return;

        m_native->Release();
        m_native     = nullptr;
        m_marshaller = nullptr;
        }

    !DgnModel ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (m_native);
            m_marshaller = nullptr;
            m_native     = nullptr;
            }
        }
};

/**
 * ColorDef - Projection of BentleyApi::Dgn::ColorDef
 */
[SRI::StructLayout (SRI::LayoutKind::Explicit)]
public value struct ColorDef
{
private:
    [SRI::FieldOffset(0)]System::Byte     m_red;
    [SRI::FieldOffset(1)]System::Byte     m_green;
    [SRI::FieldOffset(2)]System::Byte     m_blue;
    [SRI::FieldOffset(3)]System::Byte     m_alpha;

internal:
    ColorDef (BDGN::ColorDefCR native)
        {
        m_red   = native.GetRed();
        m_green = native.GetGreen();
        m_blue  = native.GetBlue();
        m_alpha = native.GetAlpha();
        }

public:
    /**
     * Construct a new ColorDef
     * @param red   The red value. 0-255.
     * @param green   The green value. 0-255.
     * @param blue   The blue value. 0-255.
     * @param alpha   The alpha value. 0-255.
     */
    ColorDef (byte red, byte green, byte blue, byte alpha) : m_red (red), m_green (green), m_blue (blue), m_alpha (alpha)
        {
        }


    property System::Byte Red
        {
        System::Byte get ()
            {
            return m_red;
            }
        void set (System::Byte red)
            {
            m_red = red;
            }
        }

    property System::Byte Green
        {
        System::Byte get ()
            {
            return m_green;
            }
        void set (System::Byte green)
            {
            m_green = green;
            }
        }

    property System::Byte Blue
        {
        System::Byte get ()
            {
            return m_blue;
            }
        void set (System::Byte blue)
            {
            m_blue = blue;
            }

        }

    property System::Byte Alpha
        {
        System::Byte get ()
            {
            return m_alpha;
            }
        void set (System::Byte alpha)
            {
            m_alpha = alpha;
            }

        }
};


    /** Projection of BentleyApi::Dgn::Render::FillDisplay */
public enum class FillDisplay
    {
    Never       = BDGNR::FillDisplay::Never,
    ByView      = BDGNR::FillDisplay::ByView,
    Always      = BDGNR::FillDisplay::Always,
    Blanking    = BDGNR::FillDisplay::Blanking,
    };


    /** Projection of BentleyApi::Dgn::Render::DgnGeometryClass */
public enum class DgnGeometryClass
    {
    Primary      = BDGNR::DgnGeometryClass::Primary,
    Construction = BDGNR::DgnGeometryClass::Construction,
    Dimension    = BDGNR::DgnGeometryClass::Dimension,
    Pattern      = BDGNR::DgnGeometryClass::Pattern,
    };

/**
 * RenderGeometryParams - Projection of BentleyApi::Dgn::Render::GeometryParams
 */
public ref struct RenderGeometryParams
{
private:
    BDGNR::GeometryParamsP      m_native;
    System::Object^             m_owner;

internal:
    RenderGeometryParams (BDGNR::GeometryParamsCP native, System::Object^ owner)
        {
        m_native = const_cast <BDGNR::GeometryParamsP>(native);
        m_owner = owner;
        }

    BDGNR::GeometryParamsP      GetNative() { return m_native; }

public:
    /** The geometry's Category */
    property DgnCategoryId^ CategoryId
        {
        DgnCategoryId^ get ()
            {
            return gcnew DgnCategoryId (m_native->GetCategoryId().GetValue());
            }
        void set (DgnCategoryId^ value)
            {
            m_native->SetCategoryId (BDGN::DgnCategoryId (value->Value));
            }
        }

    /** The geometry's SubCategory */
    property DgnSubCategoryId^ SubCategorId
        {
        DgnSubCategoryId^ get ()
            {
            return gcnew DgnSubCategoryId (m_native->GetSubCategoryId().GetValue());
            }
        void set (DgnSubCategoryId^ value)
            {
            m_native->SetSubCategoryId (BDGN::DgnSubCategoryId (value->Value));
            }
        }


    /** The geometry's weight. Must be an integer between 0 and ... */
    property unsigned int Weight
        {
        unsigned int get ()
            {
            return m_native->GetWeight();
            }
        void set (unsigned int value)
            {
            m_native->SetWeight (value);
            }
        }


    /** The geometry's line color. */
    property ColorDef LineColor
        {
        ColorDef get ()
            {
            return ColorDef (m_native->GetLineColor());
            }
        void set (ColorDef value)
            {
            pin_ptr <ColorDef> pinned = &value;
            m_native->SetLineColor (*((BDGN::ColorDef*) pinned));
            }
        }

    /** Specify if or when the geometry should be filled. */
    property DgnNET::FillDisplay    FillDisplay
        {
        DgnNET::FillDisplay get ()
            {
            return (DgnNET::FillDisplay)(m_native->GetFillDisplay());
            }
        void set (DgnNET::FillDisplay value)
            {
            return m_native->SetFillDisplay ((BDGNR::FillDisplay) (value));
            }
        }

    /** The geometry's fill color. */
    property ColorDef FillColor
        {
        ColorDef get ()
            {
            return ColorDef (m_native->GetFillColor());
            }
        void set (ColorDef value)
            {
            pin_ptr <ColorDef> pinned = &value;
            m_native->SetFillColor (*((BDGN::ColorDef*) pinned));
            }
        }

    /** Set the geometry's fill color to match the view background. */
    void SetFillColorFromViewBackground()
        {
        m_native->SetFillColorFromViewBackground();
        }

    /** The geometry class */
    property DgnGeometryClass GeometryClass
        {
        DgnGeometryClass get ()
            {
            return (DgnGeometryClass)(m_native->GetGeometryClass());
            }
        void set (DgnGeometryClass value)
            {
            m_native->SetGeometryClass ((BDGNR::DgnGeometryClass)(value));
            }
        }

    /** The geometry's transparency. Must be a floating point number between ... */
    property double Transparency
        {
        double get ()
            {
            return m_native->GetTransparency();
            }
        void set (double value)
            {
            m_native->SetTransparency (value);
            }
        }

    /** The geometry's fill transparency. Must be a floating point number between ... */
    property double FillTransparency
        {
        double get ()
            {
            return m_native->GetFillTransparency();
            }
        void set (double value)
            {
            m_native->SetFillTransparency (value);
            }
        }

    /** The geometry's display priority. Must be an integer between ... */
    property int DisplayPriority
        {
        int get ()
            {
            return m_native->GetDisplayPriority();
            }
        void set (int value)
            {
            m_native->SetDisplayPriority (value);
            }
        }

    /** The geometry's material. */
    property DgnMaterialId^     MaterialId
        {
        DgnMaterialId^ get ()
            {
            return gcnew DgnMaterialId (m_native->GetMaterialId().GetValue());
            }
        void set (DgnMaterialId^ value)
            {
            m_native->SetMaterialId (BDGN::DgnMaterialId (value->Value));
            }
        }

    ~RenderGeometryParams ()
        {
        // Dispose method
        // if m_owner not null, then m_native is a pointer into owner.
        if ( (nullptr == m_native) || (nullptr != m_owner) )
            return;

        this->!RenderGeometryParams();
        }

    !RenderGeometryParams ()
        {
        // Finalizer
        if ( (nullptr == m_native) || (nullptr != m_owner) )
            return;

        delete m_native;
        m_native = nullptr;
        }

};


/*=================================================================================**//**
* TextString - managed equivalent of Dgn::TextString
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct TextString
{
private:
    BDGN::TextStringP   m_native;
    bool                m_isDisposed;

internal:
    TextString (BDGN::TextStringP native) : m_native (native)
        {
        }

public:
    ~TextString ()
        {
        // Dispose method

        // if we are already disposed or we are part of another native object (m_owner not nullptr), don't do anything.
        if (m_isDisposed)
            return;

        // call finalizer to clean up native resources.
        this->!TextString();
        m_isDisposed = true;

        // don't need finalize, when the Dispose method is called, the runtime calls System::GC::SuppressFinalize for us.
        }

    !TextString()
        {
        // finalize method
        delete m_native;
        }

};


/*=================================================================================**//**
* GeometricPrimitive    managed version of Dgn::GeometricPrimitive
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometricPrimitive
{
private:
    BDGN::GeometricPrimitiveP     m_native;
    ReleaseMarshaller*            m_marshaller;

internal:
    GeometricPrimitive (BDGN::GeometricPrimitiveP native)
        {
        m_native = native;
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

public:
    // NOTE: Brien suggests that this class be made more closely parallel to the native version rather than this.
    //  A current problem with it is that it will not return OpenCascade-based solids.

    /** If the primitive is pure geometry, then this property returns its geometry */
    property GeometryNET::AnyGeometry^  Geometry
        {
        GeometryNET::AnyGeometry^ get ()
            {
            if (BDGN::GeometricPrimitive::GeometryType::TextString == m_native->GetGeometryType())
                return nullptr;

            ICurvePrimitivePtr curvePrimitive = m_native->GetAsICurvePrimitive();
            if (curvePrimitive.IsNull())
                return nullptr;

            // Brien points out that IGeometry is not complete and thus essentially obsolete.
            IGeometryPtr geometryPtr = IGeometry::Create (curvePrimitive);
            if (geometryPtr.IsNull())
                return nullptr;

            System::IntPtr geometryIntPtr (geometryPtr.get());
            return GeometryNET::AnyGeometry::CreateStrongTypeFromNativeIGeometry (geometryIntPtr, GeometryNET::CreateAction::Clone);
            }
        }

    /** If the primitive is a text string, then this property returns its text string */
    property DgnNET::TextString^    TextString
        {
        DgnNET::TextString^ get ()
            {
            if (BDGN::GeometricPrimitive::GeometryType::TextString != m_native->GetGeometryType())
                return nullptr;

            BDGN::TextStringPtr nativeTS = m_native->GetAsTextString();
            return gcnew DgnNET::TextString (nativeTS.get());
            }
        }

    ~GeometricPrimitive ()
        {
        if (nullptr == m_native)
            return;

        m_native->Release();
        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !GeometricPrimitive ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (m_native);
            m_native     = nullptr;
            m_marshaller = nullptr;
            }
        }

};


/*=================================================================================**//**
* DgnGeometryPart managed version of Dgn::DgnGeometryPart
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct DgnGeometryPart
{
// A GeometryPart is a nested group of GeometricPrimitives (so they can be shared between GeometrySources). You can't nest one GeometryPart in another.
private:
    BDGN::DgnGeometryPartP  m_native;
    ReleaseMarshaller*      m_marshaller;

internal:
    DgnGeometryPart (BDGN::DgnGeometryPartP native) : m_native (native)
        {
        m_native = native;
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    BDGN::DgnGeometryPartP  GetNative() { return m_native; }

public:
    /**
      * Create a new DgnGeometryPart object.
      * @param db   The DgnDb that will hold the DgnGeometryPart
      * @return the DgnGeometryPart object
      * @see InsertGeometryPart
      */
    static DgnGeometryPart^ Create(DgnDb^ dgnDb)
        {
        BDGN::DgnGeometryPartPtr geometryPartNative = BDGN::DgnGeometryPart::Create (*Convert::DgnDbToNative (dgnDb));
        return gcnew DgnGeometryPart (geometryPartNative.get());
        }

    /**
     * Insert this DgnGeometryPart into the DgnDb.
     * @return non-zero error status if the DgnGeometryPart could not be inserted.
     */
    int Insert()
        {
        return -2;
        }

    ~DgnGeometryPart ()
        {
        if (nullptr == m_native)
            return;

        m_native->Release();
        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !DgnGeometryPart ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (new DgnElementDeleter (m_native));
            m_native     = nullptr;
            m_marshaller = nullptr;
            }
        }

};


/*=================================================================================**//**
* GeometryCollectionIterator - managed version of Dgn:GeometryCollectionIterator
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
struct GeometryCollectionIteratorImpl
{
    BDGN::GeometryCollection::Iterator  m_iterator;

    GeometryCollectionIteratorImpl (BDGN::GeometryCollection::Iterator iterator) : m_iterator (iterator) {}
};

/*=================================================================================**//**
* GeometryCollectionIterator - managed version of Dgn:GeometryCollectionIterator
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometryCollectionIterator
{
internal:
    GeometryCollectionIteratorImpl*     m_impl;

    GeometryCollectionIterator (BDGN::GeometryCollection::Iterator iterator)
        {
        m_impl = new GeometryCollectionIteratorImpl (iterator);
        }

    GeometricPrimitive^ GetGeometry ()
        {
        BDGN::GeometricPrimitivePtr geometricPrimitiveNative = m_impl->m_iterator.GetGeometryPtr();
        if (geometricPrimitiveNative.IsNull())
            return nullptr;

        return gcnew GeometricPrimitive (geometricPrimitiveNative.get());
        }

    DgnGeometryPart^ GetGeometryPart ()
        {
        BDGN::DgnGeometryPartPtr geometryPartPtr = m_impl->m_iterator.GetGeometryPartPtr();
        if (geometryPartPtr.IsNull())
            return nullptr;

        return gcnew DgnGeometryPart (geometryPartPtr.get());
        }

    GeometryNET::DTransform3d^ GetGeometryToWorld ()
        {
        TransformCR     transform = m_impl->m_iterator.GetGeometryToWorld();
        return Convert::TransformToManaged (transform);
        }

    RenderGeometryParams^ GetGeometryParams ()
        {
        BDGNR::GeometryParamsCR params = m_impl->m_iterator.GetGeometryParams();
        return gcnew RenderGeometryParams (&params, this);
        }

};

/*=================================================================================**//**
* GeometryCollection - managed version of Dgn::GeometryCollection
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometryCollection
{
private:
    BDGN::GeometryCollection*   m_collection;
    GeometrySource^             m_source;

internal:
    GeometryCollection (GeometrySource^ geometrySource)
        {
        m_collection = new BDGN::GeometryCollection (*Convert::GeometrySourceToNative (geometrySource));
        // make sure the source stays in memory while the collection is accessible.
        m_source     = geometrySource;
        }

    /**
     * Get an interator that points to the first item in this collection
     */
    GeometryCollectionIterator^ Begin()
        {
        return gcnew GeometryCollectionIterator (m_collection->begin());
        }

    /**
     * Test if this iterator is not at the end of the collection
     * @param iter  The iterator
     * @return true if the iterator is not at the end of the collection
     */
    bool IsValid (GeometryCollectionIterator^ iterator)
        {
        return m_collection->end() != iterator->m_impl->m_iterator;
        }

    /**
     * Move to the next item in this collection.
     * @param iter  The iterator
     * @return true if the new position of the iterator is not at the end of the collection
     */
    bool ToNext (GeometryCollectionIterator^ iterator)
        {
        ++iterator->m_impl->m_iterator;
        return m_collection->end() != iterator->m_impl->m_iterator;
        }

    /**
     * Get the GeometricPrimitive at the specified position in this collection.
     * @param iter  The iterator
     * @return a GeometricPrimiive or null if the current item is not a primitive.
     * @see GetGeometryPart
     * @see GetGeometryToWorld
     */
    GeometricPrimitive^ GetGeometry (GeometryCollectionIterator^ iterator)
        {
        return iterator->GetGeometry ();
        }

    /**
     * Get the DgnGeometryPart at the specified position in this collection.
     * @param iter  The iterator
     * @return a DgnGeometryPart or null if the current item is not a DgnGeometryPart reference.
     * @see GetGeometry
     * @see GetGeometryToWorld
     */
    DgnGeometryPart^ GetGeometryPart (GeometryCollectionIterator^ iterator)
        {
        return iterator->GetGeometryPart ();
        }

    /**
     * Get the transform that relates the GeometricPrimitive at the specified position in this collection to the world coordinate system.
     * @param iter  The iterator
     * @return The transform
     */
    GeometryNET::DTransform3d^ GetGeometryToWorld (GeometryCollectionIterator^ iterator)
        {
        return iterator->GetGeometryToWorld ();
        }

    /**
     * Get the RenderGeometryParams that apply to geometry at the specified position in this collection.
     * @param iter  The iterator
     */
    RenderGeometryParams^ GetGeometryParams (GeometryCollectionIterator^ iterator)
        {
        return iterator->GetGeometryParams ();
        }
};

/*=================================================================================**//**
* GeometryBuilder - managed version of Dgn::GeometryBuilder
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct GeometryBuilder
{
private:
    BDGN::GeometryBuilderP       m_native;
    ReleaseMarshaller*           m_marshaller;

internal:
    GeometryBuilder (GeometrySource^ source)
        {
        BDGN::GeometryBuilderPtr nativeGB = BDGN::GeometryBuilder::Create (*Convert::GeometrySourceToNative (source));
        m_native = nativeGB.get();
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    GeometryBuilder (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DTransform3d transform)
        {
        BDGN::DgnCategoryId nativeCategoryId (categoryId->Value);
        TransformCP nativeTransform = Convert::TransformToNative (transform);

        BDGN::GeometryBuilderPtr nativeGB = BDGN::GeometryBuilder::Create (*Convert::DgnModelToNative (model), nativeCategoryId, *nativeTransform);
        m_native = nativeGB.get();
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    GeometryBuilder (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DPoint3d origin, GeometryNET::YawPitchRollAngles angles)
        {
        BDGN::DgnCategoryId                         nativeCategoryId (categoryId->Value);
        pin_ptr<GeometryNET::DPoint3d>              originPinned = &origin;
        pin_ptr<GeometryNET::YawPitchRollAngles>    anglesPinned = &angles;

        BDGN::GeometryBuilderPtr nativeGB = BDGN::GeometryBuilder::Create (*Convert::DgnModelToNative (model), nativeCategoryId, *(DPoint3dP)originPinned, *(YawPitchRollAnglesP)anglesPinned);
        m_native = nativeGB.get();
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    GeometryBuilder (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DPoint2d origin, double angle)
        {
        BDGN::DgnCategoryId                         nativeCategoryId (categoryId->Value);
        pin_ptr<GeometryNET::DPoint2d>              originPinned = &origin;
        AngleInDegrees                              degrees = AngleInDegrees::FromDegrees (angle);


        BDGN::GeometryBuilderPtr nativeGB = BDGN::GeometryBuilder::Create (*Convert::DgnModelToNative (model), nativeCategoryId, *(DPoint2dP)originPinned, degrees);
        m_native = nativeGB.get();
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    GeometryBuilder (DgnDb^ dgnDb, bool is3d)
        {
        BDGN::DgnDbP    nativeDgnDb = Convert::DgnDbToNative (dgnDb);
        BDGN::GeometryBuilderPtr nativeGB = BDGN::GeometryBuilder::CreateGeometryPart (*nativeDgnDb, is3d);
        m_native = nativeGB.get();
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

public:
    /**
     * Construct a new GeometryBuilder to prepare geometry using the category and placement of an existing element.
     * @note This is just a short cut for calling CreateForModel.
     * @param el The element to get the category and placement origin/angle(s) from.
     * @return a GeometryBuilder object
     * @see CreateForModel
     */
    static GeometryBuilder^ CreateForElement (GeometrySource^ source)
        {
        return gcnew GeometryBuilder (source);
        }

    /**
     * Construct a new GeometryBuilder to prepare geometry for elements in the specified model and category
     * @param model The model where the geometry will ultimately be stored
     * @param catid The category of the element that will ultimatley contain the geometry
     * @param transform The placement represented by a transform
     * @return a GeometryBuilder object
     */
    static GeometryBuilder^  CreateForModelWithTransform (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DTransform3d transform)
        {
        return gcnew GeometryBuilder (model, categoryId, transform);
        }

    /**
     * Construct a new GeometryBuilder to prepare 3d geometry for elements in the specified model and category
     * @param model The model where the geometry will ultimately be stored
     * @param catid The category of the element that will ultimatley contain the geometry
     * @param o     The placement origin
     * @param angles The placement angles
     * @return a GeometryBuilder object
     */
    static GeometryBuilder^  CreateFor3dModel (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DPoint3d origin, GeometryNET::YawPitchRollAngles angles)
        {
        return gcnew GeometryBuilder (model, categoryId, origin, angles);
        }

    /**
     * Construct a new GeometryBuilder to prepare 2d geometry for elements in the specified model and category
     * @param model The model where the geometry will ultimately be stored
     * @param catid The category of the element that will ultimatley contain the geometry
     * @param o     The placement origin
     * @param angle The placement angle
     * @return a GeometryBuilder object
     */
    static GeometryBuilder^  CreateFor2dModel (DgnModel^ model, DgnCategoryId^ categoryId, GeometryNET::DPoint2d origin, double angle)
        {
        return gcnew GeometryBuilder (model, categoryId, origin, angle);
        }

    /**
     * Construct a new GeometryBuilder to prepare geometry for a DgnGeometryPart
     * @param db    The DgnDb that will hold the DgnGeometryPart
     * @param is3d  Will the DgnGeometryPart hold 3-D geometry?
     * @return a GeometryBuilder object
     */
    static GeometryBuilder^  CreateGeometryPart (DgnDb^ dgnDb, bool is3d)
        {
        return gcnew GeometryBuilder (dgnDb, is3d);
        }

    /**
     * Append a copy of each geometric primitive in the specified builder to this builder, with a transform.
     * @param builder   the builder to copy from
     * @param relativePlacement if not null, the offset and/or rotation of the copied geometry
     */
    void    AppendCopyOfGeometry (GeometryBuilder^ builder, Placement3d^ relativePlacement)
        {
        BDGN::GeometryBuilderR  otherbuilder = *builder->m_native;
        BDGN::GeometryStream    otherStream;
        otherbuilder.GetGeometryStream (otherStream);
        BDGN::GeometryCollection otherGeomCollection (otherStream, m_native->GetDgnDb());

        Transform transform;
        if (nullptr != relativePlacement)
            transform = relativePlacement->GetNative()->GetTransform();
        else
            transform.InitIdentity();

        for (auto otherItem: otherGeomCollection)
            {
            BDGNR::GeometryParams sourceParams (otherItem.GetGeometryParams());
            sourceParams.SetCategoryId (m_native->GetGeometryParams().GetCategoryId());
            m_native->Append (sourceParams);

            auto geomprim = otherItem.GetGeometryPtr();
            if (geomprim.IsValid())
                {
                BDGN::GeometricPrimitivePtr cc = geomprim->Clone();
                cc->TransformInPlace (transform);
                m_native->Append(*cc);
                }
            else
                {
                /* *** TBD: embedded geompart instances
                DgnGeometryPartCPtr gp = otherItem.GetGeometryPartPtr();
                if (gp.IsValid())
                    {
                    Transform transform = otherItem.GetGeometryToSource();
                    }
                    */
                BeAssert(false && "AppendCopyOfBuilder - geompart instances not supported");
                }
            }
        }

    /**
     * Append RenderGeometryParams to the builder.
     * @param params The parameters to apply to subsequent geometry
     */
    void    AppendRenderGeometryParams (RenderGeometryParams^ rgParams)
        {
        m_native->Append (*rgParams->GetNative());
        }

    /** The RenderGeometryParams that will be applied to geometry appended to the builder. */
    property RenderGeometryParams^ GeometryParams
        {
        RenderGeometryParams^ get ()
            {
            BDGNR::GeometryParamsCR     nativeGeomParams = m_native->GetGeometryParams();
            RenderGeometryParams^       geometryParams = gcnew RenderGeometryParams (&nativeGeomParams, this);
            return geometryParams;
            }
        }

    /**
     * Append a SubCategoryId to the builder.
     * @param subcategoryId The SubCategoryId to apply to subsequent geometry.
     */
    void    AppendSubCategoryId (DgnSubCategoryId^ subcategoryId)
        {
        m_native->Append (BDGN::DgnSubCategoryId (subcategoryId->Value));
        }

    /**
     * Append a geometry of some kind
     * @param geometry  The geometry
     */
    void    AppendGeometry (GeometryNET::AnyGeometry^ geometry)
        {
        GeometryNET::CurveVector^ curveVector;
        if (nullptr != (curveVector = dynamic_cast <GeometryNET::CurveVector^>(geometry)))
            {
            System::IntPtr nativeIntPtr = GeometryNET::CurveVector::DereferenceToNative (curveVector, false);
            CurveVectorP nativeCV = (CurveVectorP) nativeIntPtr.ToPointer();
            m_native->Append (*nativeCV);
            return;
            }

        GeometryNET::SolidPrimitive^ solidPrimitive;
        if (nullptr != (solidPrimitive = dynamic_cast <GeometryNET::SolidPrimitive^>(geometry)))
            {
            System::IntPtr nativeIntPtr = GeometryNET::SolidPrimitive::DereferenceToNative (solidPrimitive, false);
            ISolidPrimitiveP nativeSP = (ISolidPrimitiveP) nativeIntPtr.ToPointer();
            m_native->Append (*nativeSP);
            return;
            }

        GeometryNET::CurvePrimitive^ curvePrimitive;
        if (nullptr != (curvePrimitive = dynamic_cast <GeometryNET::CurvePrimitive^>(geometry)))
            {
            System::IntPtr nativeIntPtr = GeometryNET::CurvePrimitive::DereferenceToNative (curvePrimitive, false);
            ICurvePrimitiveP nativeCP = (ICurvePrimitiveP) nativeIntPtr.ToPointer();
            m_native->Append (*nativeCP);
            return;
            }

        GeometryNET::PolyfaceHeader^ polyfaceHeader;
        if (nullptr != (polyfaceHeader = dynamic_cast <GeometryNET::PolyfaceHeader^>(geometry)))
            {
            System::IntPtr nativeIntPtr = GeometryNET::PolyfaceHeader::DereferenceToNative (polyfaceHeader, false);
            PolyfaceHeaderP nativePFH = (PolyfaceHeaderP) nativeIntPtr.ToPointer();
            m_native->Append (*nativePFH);
            return;
            }

        GeometryNET::MSBsplineSurface^ bsplineSurface;
        if (nullptr != (bsplineSurface = dynamic_cast <GeometryNET::MSBsplineSurface^>(geometry)))
            {
            System::IntPtr nativeIntPtr = GeometryNET::MSBsplineSurface::DereferenceToNative (bsplineSurface, false);
            MSBsplineSurfaceP nativeBSS = (MSBsplineSurfaceP) nativeIntPtr.ToPointer();
            m_native->Append (*nativeBSS);
            return;
            }

        // wasn't any of those geometry types.
        BeAssert (false);
        }

    void    AppendGeometry (GeometryNET::MSBsplineCurve^ bsplineCurve)
        {
        System::IntPtr nativeIntPtr = GeometryNET::MSBsplineCurve::DereferenceToNative (bsplineCurve, false);
        MSBsplineCurveP nativeBSC = (MSBsplineCurveP) nativeIntPtr.ToPointer();
        ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateBsplineCurve (*nativeBSC);
        m_native->Append (*curvePrimitive.get());
        }


#if defined (NEEDSWORK_DgnPlatformNET_Geometry)
    /**
     * Append the geometry from a GeometryNode.
     * @remark All leaf geometry is transformed to the node's root coordinates and saved as separate geometry items.
     * @param node the root of the geometry.
     */
    void    AppendGeometryNode(GeometryNode^ node)
#endif

    /**
     * Append an instance of a DgnGeometryPart
     * @param geometryPart  The DgnGeometryPart
     * @param relativePlacement if not null, the offset and/or rotation of the instance
     */
    void    AppendGeometryPart (DgnGeometryPart^ geometryPart, Placement3d^ relativePlacement)
        {
        BDGN::DgnGeometryPartP nativePart = geometryPart->GetNative();
        Transform transform;
        if (nullptr != relativePlacement)
            transform = relativePlacement->GetNative()->GetTransform();
        else
            transform.InitIdentity();

        m_native->Append (nativePart->GetId(), transform);
        }

    /**
     * Copy the geometry in this builder to an element.
     * @param element   The element
     * @return non-zero error status if \a element is invalid or if this geometry stream is invalid
     */
    int     Finish (GeometrySource^ source)
        {
        BDGN::GeometrySourceP nativeSource = Convert::GeometrySourceToNative (source);
        return m_native->Finish (*nativeSource);
        }

    int     Finish (GeometricElement3d^ element)
        {
        BDGN::DgnElementP           nativeElement       = element->GetNative();
        BDGN::GeometrySourceP       nativeSource        = nativeElement->ToGeometrySourceP();
        return m_native->Finish (*nativeSource);
        }

    /**
     * Copy the geometry in this builder to a DgnGeometryPart.
     * @param part  The DgnGeometryPart
     * @return non-zero error status if \a part is invalid or if this geometry stream is invalid
     */
    int     FinishPart (DgnGeometryPart^ part)
        {
        BDGN::DgnGeometryPartP nativePart = part->GetNative();
        return m_native->Finish (*nativePart);
        }


    ~GeometryBuilder ()
        {
        if (nullptr == m_native)
            return;

        m_native->Release();
        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !GeometryBuilder ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (m_native);
            m_native     = nullptr;
            m_marshaller = nullptr;
            }
        }

};


/* ------------------------------------------ ScriptBasedTool -----------------------------------------------*/

enum class HitDetailType
{
    Hit          = BDGN::HitDetailType::Hit,
    Snap         = BDGN::HitDetailType::Snap,
    Intersection = BDGN::HitDetailType::Intersection,
};

/*=================================================================================**//**
* HitDetail - managed version of Dgn::HitDetail
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct HitDetail
{
private:
    BDGN::HitDetailP        m_native;
    ReleaseMarshaller*      m_marshaller;


internal:

    HitDetail (BDGN::HitDetailP native)
        {
        m_native = native;
        }

    property GeometrySource3d^ Element
        {
        GeometrySource3d^ get ()
            {
            BDGN::DgnElementCPtr      nativeElement = m_native->GetElement();
            if (nativeElement.IsNull())
                return nullptr;

            DgnElement^ dgnElement = gcnew DgnElement (const_cast<BDGN::DgnElementP> (nativeElement.get()));

            BDGN::GeometrySource3dP     nativeSource = (const_cast<BDGN::DgnElementP> (nativeElement.get()))->ToGeometrySource3dP();
            if (nullptr == nativeSource)
                return nullptr;

            return gcnew GeometrySource3d (nativeSource, dgnElement);
            }
        }

    /** The point at which the element was picked */
    property GeometryNET::DPoint3d TestPoint
        {
        GeometryNET::DPoint3d get ()
            {
            DPoint3dCR nativePoint = m_native->GetTestPoint();
            return Convert::DPoint3dToManaged (nativePoint);
            }
        }

    /** The adjusted point on the element that was picked */
    property GeometryNET::DPoint3d HitPoint
        {
        GeometryNET::DPoint3d get ()
            {
            DPoint3dCR nativePoint = m_native->GetHitPoint();
            return Convert::DPoint3dToManaged (nativePoint);
            }
        }

    /** The type of hit: 'hit', 'snap', 'intersection' */
    property HitDetailType HitType
        {
        HitDetailType get ()
            {
            return (HitDetailType) m_native->GetHitType();
            }
        }

    ~HitDetail ()
        {
        if (nullptr == m_native)
            return;

        m_native->Release();
        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !HitDetail ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (m_native);
            m_native     = nullptr;
            m_marshaller = nullptr;
            }
        }
};

    /* ------------------------------------------ Briefcase Management ------------------------------------------*/

enum class LockableType : uint8_t
{
    Db          = BDGN::LockableType::Db,
    Model       = BDGN::LockableType::Model,
    Element     = BDGN::LockableType::Element,
};

enum class LockLevel : uint8_t
{
    None        = BDGN::LockLevel::None,
    Shared      = BDGN::LockLevel::Shared,
    Exclusive   = BDGN::LockLevel::Exclusive,
};

    /** Describes the ID of a lockable object */
/*=================================================================================**//**
*
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public value struct LockableId
{
private:
    uint64_t        m_id;
    LockableType    m_type;

public:

    //!< Constructs a LockableId for an element
    LockableId (DgnElementId^ id) : m_id (id->Value), m_type (LockableType::Element) { }

    //!< Constructs a LockableId for a model
    LockableId(DgnModelId^ id) : m_id (id->Value), m_type (LockableType::Model) { }

    //!< Constructs a LockableId of the specified type and ID
    LockableId (LockableType type, uint64_t id) : m_id (id), m_type (type) { }

    property BeInt64Id^ Id
        {
        BeInt64Id^  get ()
            {
            return gcnew BeInt64Id (m_id);
            }
        }

    property LockableType Type
        {
        LockableType get ()
            {
            return m_type;
            }
        }

    bool IsValid ()
        {
        return 0 != m_id;
        }

    void Invalidate()
        {
        m_id = 0;
        }

    /** Tests if the ID matches another ID @param other The other ID */
    bool Equals (LockableId other)
        {
        return (m_id == other.m_id) && (m_type == other.m_type);
        }

    /** Creates a LockableId for an element @param element The element */
    static LockableId FromElement (DgnElement^ element)
        {
        BDGN::DgnElementId nativeElementId = element->GetNative()->GetElementId();
        return LockableId (LockableType::Element, nativeElementId.GetValue());
        }

    /** Creates a LockableId for a model @param model The model */
    static LockableId FromModel (DgnModel^ model)
        {
        BDGN::DgnModelId nativeModelId = model->GetNative()->GetModelId();
        return LockableId (LockableType::Model, nativeModelId.GetValue());
        }

    /** Creates a LockableId for a DgnDb @param dgndb The DgnDb */
    static LockableId FromDgnDb (DgnDb^ dgnDb)
        {
        // the m_id for a DgnDb is always the same (1), but make sure we get it from the native code.
        BDGN::LockableId dgnDbLockable (*dgnDb->GetNative());
        return LockableId (LockableType::Db, dgnDbLockable.GetId().GetValue());
        }
};


/*=================================================================================**//**
* DgnLock - managed version of Dgn::DgnLock
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public value struct DgnLock
{
private:
    LockableId  m_id;
    LockLevel   m_level;


public:
    /**
    * Construct by ID and lock level
    * @param id     The ID of the lockable object
    @ @param level  The level at which the object is locked
    */
    DgnLock (LockableId id, LockLevel level)
        {
        m_id    = id;
        m_level = level;
        }

    /** The Id of the lockable object */
    property LockableId Id
        {
        LockableId get ()
            {
            return m_id;
            }
        }

    /** The level at which the object is locked */
    property LockLevel Level
        {
        LockLevel get ()
            {
            return m_level;
            }
        }


    /** Ensure the lock level is no lower than the specified level @param minLevel The minimum level */
    void EnsureLevel (LockLevel minLevel)
        {
        if ((int) minLevel < (int) m_level)
            m_level = minLevel;
        }

    /** Sets this to be an invalid lock */
    void Invalidate()
        {
        m_id.Invalidate();
        }

    /**
    * Create a DgnLock for the specified element
    * @param element    The locked element
    * @param level      The level at which the element is locked
    */
    static DgnLock FromElement (DgnElement^ element, LockLevel level)
        {
        LockableId lockableId = LockableId::FromElement (element);
        return DgnLock (lockableId, level);
        }

    /**
    * Create a DgnLock for the specified model
    * @param model  The locked model
    * @param level  The level at which the model is locked
    */
    static DgnLock FromModel (DgnModel^ model, LockLevel level)
        {
        LockableId lockableId = LockableId::FromModel (model);
        return DgnLock (lockableId, level);
        }

    /**
    * Create a DgnLock for the specified DgnDb
    * @param dgndb  The locked DgnDb
    * @param level  The level at which the DgnDb is locked
    */
    static DgnLock FromDgnDb (DgnDb^ dgnDb, LockLevel level)
        {
        LockableId lockableId = LockableId::FromDgnDb (dgnDb);
        return DgnLock (lockableId, level);
        }

};


/*=================================================================================**//**
* RepositoryRequest - managed version of Dgn::IBriefcaseManager::Request, a request to acquire locks and/or codes, or query the availability thereof
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct RepositoryRequest
{
private:
    BDGN::IBriefcaseManager::Request*   m_native;
    DgnDb^                              m_db;
    System::String^                     m_operation;

internal:
    RepositoryRequest (DgnDb^ db, System::String^ operation) : m_db (db), m_operation (operation)
        {
        m_native = new BDGN::IBriefcaseManager::Request ();
        }

    RepositoryStatus Query (bool fast)
        {
        m_native->SetOptions (BDGN::DgnPlatformLib::GetHost().GetRepositoryAdmin()._GetResponseOptions(true));
        BDGN::IBriefcaseManager::Response response (fast ? BDGN::IBriefcaseManager::RequestPurpose::FastQuery : BDGN::IBriefcaseManager::RequestPurpose::Query, m_native->Options());
        m_db->GetNative()->BriefcaseManager().AreResourcesAvailable (*m_native, &response, fast ? BDGN::IBriefcaseManager::FastQuery::Yes : BDGN::IBriefcaseManager::FastQuery::No);

        pin_ptr<wchar_t const>  operationPinned = PtrToStringChars (m_operation);
        Utf8String              utf8Operation (operationPinned);
        BDGN::DgnPlatformLib::GetHost().GetRepositoryAdmin()._OnResponse(response, utf8Operation.c_str());
        return (RepositoryStatus) response.Result();
        }

    BDGN::IBriefcaseManager::Request* GetNative () { return m_native; }


public:
    /**
    * Create a new, empty request object for the specified briefcase
    * @param dgndb      The requesting briefcase
    * @param operation  The name of the operation for which the request is being made (e.g., a tool name)
    */
    static RepositoryRequest^ Create (DgnDb^ db, System::String^ operation)
        {
        return gcnew RepositoryRequest (db, operation);
        }

    /** The briefcase associated with this request */
    property DgnDb^     Briefcase
        {
        DgnDb^ get ()
            {
            return m_db;
            }
        }

    /** The name of the operation for which the request is being made (e.g., a tool name) */
    property System::String^ Operation
        {
        System::String^ get ()
            {
            return m_operation;
            }
        }

    /** Attempts to acquire the locks and/or codes from the repository manager on this request's briefcase's behalf */
    RepositoryStatus    Acquire()
        {
        m_native->SetOptions(BDGN::DgnPlatformLib::GetHost().GetRepositoryAdmin()._GetResponseOptions(false));
        auto response = m_db->GetNative()->BriefcaseManager().Acquire (*m_native);
        pin_ptr<wchar_t const>  operationPinned = PtrToStringChars (m_operation);
        Utf8String              utf8Operation (operationPinned);
        BDGN::DgnPlatformLib::GetHost().GetRepositoryAdmin()._OnResponse(response, utf8Operation.c_str());
        return (RepositoryStatus) response.Result();
        }

    /** Queries the repository manager to determine if the locks and/or codes in this request are available to be acquired by this request's briefcase */
    RepositoryStatus    QueryAvailability()
        {
        return Query (false);
        }

    /** Queries a local cache to determine if the locks and/or codes in this request are available to be acquired by this request's briefcase. Does not contact server, therefore faster than QueryAvailability() but potentially less accurate */
    RepositoryStatus    FastQueryAvailability()
        {
        return Query (true);
        }

    /** Add a request to exclusively lock an element */
    void                AddElement (DgnElement^ element)
        {
        m_native->Locks().Insert (*element->GetNative(), BDGN::LockLevel::Exclusive);
        }

    /** Add a request to lock a model at the specified level */
    void AddModel (DgnModel^ model, LockLevel level)
        {
        m_native->Locks().Insert (*model->GetNative(), (BDGN::LockLevel) level);
        }

    /** Add a request to lock the briefcase at the specified level */
    void AddBriefcase (LockLevel level)
        {
        m_native->Locks().Insert (*m_db->GetNative(), (BDGN::LockLevel) level);
        }

    /** Add a request to reserve a code */
    void AddCode (AuthorityIssuedCode^ code)
        {
        m_native->Codes().insert (*code->GetNative());
        }

};

/* ------------------------------------------ EC -----------------------------------------------*/

/*=================================================================================**//**
* ECClass - managed version of ECN::ECClass
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECClass
{
private:
    ECN::ECClassCP      m_native;
    System::Object^     m_owner;

internal:
    ECClass (ECN::ECClassCP native, System::Object^ owner)
        {
        // the ECClass is integral to the DgnDb, so no need for dispose method.
        m_native = native;
        m_owner  = owner;
        }

    ECN::ECClassCP      GetNative () { return m_native; }

public:
    /** The ID of this class */
    property DgnNET::ECClassId^ ECClassId
        {
        DgnNET::ECClassId^ get ()
            {
            return gcnew DgnNET::ECClassId (m_native->GetId().GetValue());
            }
        }

    /** The name of this class */
    property System::String^ Name
        {
        System::String^ get ()
            {
            Utf8StringCR name = m_native->GetName();
            return gcnew System::String (name.c_str());
            }
        }

    /** The ECSql name of this class */
    property System::String^ ECSqlName
        {
        System::String^ get ()
            {
            Utf8StringCR name = m_native->GetECSqlName();
            return gcnew System::String (name.c_str());
            }
        }

    /** The schema to which this class belongs */
    property ECSchema^ Schema
        {
        ECSchema^ get ()
            {
            // NEEDSWORK_DgnPlaformNET_Schemas - should we attempt to return the same ECSchema^ every time?
            ECN::ECSchemaCR  nativeSchema = m_native->GetSchema();
            return Convert::ECSchemaToManaged (&nativeSchema, m_owner);
            }
        }

    /** The base classes of this class */
    property ECClassCollection^ BaseClasses
        {
        ECClassCollection^ get ()
            {
            ECN::ECBaseClassesList const& nativeBaseClasses = m_native->GetBaseClasses();
            return Convert::ECClassCollectionToManaged (&nativeBaseClasses, m_owner);
            }
        }

    /** The classes that derive from this class */
    property ECClassCollection^ DerivedClasses
        {
        ECClassCollection^ get ()
            {
            ECN::ECDerivedClassesList const& nativeDerivedClasses = m_native->GetDerivedClasses();
            return Convert::ECClassCollectionToManaged (&nativeDerivedClasses, m_owner);
            }
        }

    /** The properties defined by this class and all of its base classes. */
    property ECPropertyCollection^ Properties
        {
        ECPropertyCollection^ get ();
        }
    /**
     * Get the definition of the specified property of this class or any of its base classes
     * @param name The name of the property to look up
     * @return the property definition or null if no such property is found
     $$PUBLISH_INSERT_FILE$$ dgnJsApi_ECClass_GetProperty.sampleCode
     */
    ECProperty^ GetProperty (System::String^ name)
        {
        pin_ptr<wchar_t const>  namePinned = PtrToStringChars (name);
        Utf8String  utf8Name (namePinned);

        ECN::ECPropertyP nativeProperty = m_native->GetPropertyP (utf8Name.c_str());
        return Convert::ECPropertyToManaged (nativeProperty, m_owner);
        }

    /**
     * Query the specified custom attribute on this property definition
     * @param className The class of the custom attribute to look up
     * @return the custom attribute or null if no such custom attribute is defined for this property.
     */
    ECInstance^ GetCustomAttribute (System::String^ className)
        {
        pin_ptr<wchar_t const>  classNamePinned = PtrToStringChars (className);
        Utf8String  utf8ClassName (classNamePinned);

        ECN::IECInstancePtr nativeInstance = m_native->GetCustomAttribute (utf8ClassName.c_str());
        if (nativeInstance.IsNull())
            return nullptr;

        return Convert::ECInstanceToManaged (nativeInstance.get());
        }

    /** Create a non-persistent instance of this ECClass */
    ECInstance^ MakeInstance()
        {
        ECN::IECInstancePtr nativeInstance = m_native->GetDefaultStandaloneEnabler()->CreateInstance();

        if (nativeInstance.IsNull())
            return nullptr;
        return Convert::ECInstanceToManaged (nativeInstance.get());
        }

};



struct ECClassContainerEnumeratorImpl
    {
    // we need this class only because a managed class can have only a pointer to a struct as a member, not a struct
    ECN::ECClassContainerCR                 m_collection;
    ECN::ECClassContainer::const_iterator   m_current;

    ECClassContainerEnumeratorImpl (ECN::ECClassContainerCR collection) : m_collection (collection), m_current (m_collection.begin()) {}
    };

/*=================================================================================**//**
* ECClassContainerEnumerator - an enumerator over an ECClassCollection - never instantiated directly.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECClassContainerEnumerator : System::Collections::Generic::IEnumerator <ECClass^>
{
private:
    ECN::ECClassContainer const&        m_collection;
    System::Object^                     m_owner;

    ECClassContainerEnumeratorImpl*     m_impl;
    ECClass^                            m_currentMember;

    ECClass^     GetMember()
        {
        if (NULL == m_impl)
            return nullptr;

        ECN::ECClassContainer::const_iterator theCurrent = m_impl->m_current;
        ECN::ECClassContainer::const_iterator theEnd     = m_impl->m_collection.end();
        if (!(theCurrent != theEnd))
            return nullptr;

        ECN::ECClassCP native = (*m_impl->m_current);
        return gcnew ECClass (native, m_owner);
        }

    void            SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    bool            MoveNextInternal ()
        {
        if (nullptr == m_impl)
            {
            m_impl = new ECClassContainerEnumeratorImpl (m_collection);
            return m_impl->m_current != m_impl->m_collection.end();
            }

        ECN::ECClassContainer::const_iterator endIterator = m_impl->m_collection.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }


internal:
    ECClassContainerEnumerator (ECN::ECClassContainerCR const& collection, System::Object^ owner) : m_collection (collection)
        {
        m_owner         = owner;
        m_impl          = nullptr;
        m_currentMember = nullptr;
        }

public:

    /// <summary> Advances the enumerator to the next ECClass in the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECClass in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = nullptr;
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current ECClass in the collection. </summary>
    property ECClass^ Current
        {
        virtual ECClass^ get () = System::Collections::Generic::IEnumerator <ECClass^>::Current::get
            {
            return m_currentMember;
            }
        };

    ~ECClassContainerEnumerator ()
        {
        FreeImpl ();
        }

    !ECClassContainerEnumerator ()
        {
        FreeImpl();
        }

};

/*=================================================================================**//**
* ECSchema - managed version of ECN::ECSchema
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECSchema : System::Collections::Generic::IEnumerable <ECClass^>
{
private:
    ECN::ECSchemaCP     m_native;
    System::Object^     m_owner;

internal:

    ECSchema (ECN::ECSchemaCP native, System::Object^ owner)
        {
        // the ECSchema is owned by the SchemaManager, which is owned by the DgnDb.
        m_native = native;
        m_owner  = owner;
        }

public:
    property System::String^ Name
        {
        System::String^ get ()
            {
            Utf8StringCR name = m_native->GetName();
            return gcnew System::String (name.c_str());
            }
        }

    ECClass^ GetECClass (System::String^ className)
        {
        pin_ptr<wchar_t const>  classNamePinned = PtrToStringChars (className);
        Utf8String  utf8ClassName (classNamePinned);
        ECN::ECClassCP ecClass = m_native->GetClassCP (utf8ClassName.c_str());
        if (nullptr == ecClass)
            return nullptr;
        return gcnew ECClass (ecClass, m_owner);
        }

    virtual System::Collections::IEnumerator^ RawGetEnumerator () = System::Collections::IEnumerable::GetEnumerator
        {
        return GetEnumerator ();
        }

    virtual System::Collections::Generic::IEnumerator <ECClass^>^ GetEnumerator ()
        {
        return gcnew ECClassContainerEnumerator (m_native->GetClasses(), this);
        }


};


struct ECSchemaEnumeratorImpl
    {
    // we need this class only because a managed class can have only a pointer to a struct as a member, not a struct
    bvector<ECN::ECSchemaCP>                    m_collection;
    bvector<ECN::ECSchemaCP>::const_iterator    m_current;

    ECSchemaEnumeratorImpl (BeSQLite::EC::SchemaManager const* nativeSchemaManager) : m_collection (nativeSchemaManager->GetSchemas()), m_current (m_collection.begin()) {}
    };


/*=================================================================================**//**
* ECSchemaEnumerator - an enumerator over an ECSchemaCollection - never instantiated directly.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECSchemaEnumerator : System::Collections::Generic::IEnumerator <ECSchema^>
{
private:
    BeSQLite::EC::SchemaManager const*  m_nativeSchemaManager;
    System::Object^                     m_owner;

    ECSchemaEnumeratorImpl*             m_impl;
    ECSchema^                           m_currentMember;

    ECSchema^     GetMember()
        {
        if (NULL == m_impl)
            return nullptr;

        bvector<ECN::ECSchemaCP>::const_iterator theCurrent = m_impl->m_current;
        bvector<ECN::ECSchemaCP>::const_iterator theEnd     = m_impl->m_collection.end();
        if (!(theCurrent != theEnd))
            return nullptr;

        ECN::ECSchemaCP native = (*m_impl->m_current);
        return gcnew ECSchema (native, m_owner);
        }

    void            SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    bool            MoveNextInternal ()
        {
        if (nullptr == m_impl)
            {
            m_impl = new ECSchemaEnumeratorImpl (m_nativeSchemaManager);
            return m_impl->m_current != m_impl->m_collection.end();
            }

        bvector<ECN::ECSchemaCP>::const_iterator endIterator = m_impl->m_collection.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }


internal:
    ECSchemaEnumerator (BeSQLite::EC::SchemaManager const* nativeSchemaManager, System::Object^ owner)
        {
        m_nativeSchemaManager   = nativeSchemaManager;
        m_owner                 = owner;
        m_impl                  = nullptr;
        m_currentMember         = nullptr;
        }

public:

    /// <summary> Advances the enumerator to the next ECSchema in the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECSchema in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = nullptr;
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current ECSchema in the collection. </summary>
    property ECSchema^ Current
        {
        virtual ECSchema^ get () = System::Collections::Generic::IEnumerator <ECSchema^>::Current::get
            {
            return m_currentMember;
            }
        };

    ~ECSchemaEnumerator ()
        {
        FreeImpl ();
        }

    !ECSchemaEnumerator ()
        {
        FreeImpl();
        }

};


/*=================================================================================**//**
* SchemaManager - Provides access to ECSchemas and ECClasses with a DgnDb
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct SchemaManager : System::Collections::Generic::IEnumerable <ECSchema^>
{
private:
    BeSQLite::EC::SchemaManager const*  m_native;
    DgnDb^                              m_owner;

internal:
    SchemaManager (BeSQLite::EC::SchemaManager const* native, DgnDb^ owner)
        {
        // the SchemaManager is integral to the DgnDb, so no need for dispose method.
        m_native = native;
        m_owner  = owner;
        }

public:
    ECClass^    GetECClass (System::String^ schemaNameOrPrefix, System::String^ className)
        {
        pin_ptr<wchar_t const>  schemaNamePinned = PtrToStringChars (schemaNameOrPrefix);
        pin_ptr<wchar_t const>  classNamePinned = PtrToStringChars (className);
        Utf8String  utf8SchemaName (schemaNamePinned);
        Utf8String  utf8ClassName (classNamePinned);
        ECN::ECClassCP nativeClass = m_native->GetClass (utf8SchemaName.c_str(), utf8ClassName.c_str());

        return gcnew ECClass (nativeClass, m_owner);
        }

    /** Look up an ECClass by its ECClassId */
    ECClass^    GetECClassById (ECClassId^ classId)
        {
        ECN::ECClassCP nativeClass = m_native->GetClass (ECN::ECClassId (classId->Value));
        if (nullptr == nativeClass)
            return nullptr;

        return gcnew ECClass (nativeClass, m_owner);
        }

    virtual System::Collections::IEnumerator^ RawGetEnumerator () = System::Collections::IEnumerable::GetEnumerator
        {
        return GetEnumerator ();
        }

    virtual System::Collections::Generic::IEnumerator <ECSchema^>^ GetEnumerator ()
        {
        return gcnew ECSchemaEnumerator (m_native, this);
        }

};


/*=================================================================================**//**
* ECInstance - managed version of ECN::ECInstance
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECInstance
{
private:
    ECN::IECInstanceP   m_native;
    ReleaseMarshaller*  m_marshaller;

internal:
    ECInstance (ECN::IECInstanceP native)
        {
        m_native = native;
        m_native->AddRef();
        m_marshaller = ReleaseMarshaller::GetMarshaller();
        }

    ECN::IECInstanceP   GetNative () { return m_native; }

public:
    property ECClass^ Class
        {
        ECClass^ get ()
            {
            // NEEDSWORK_DgnPlatformNET_Schemas - does it need to return the same ECClass^ every time?
            ECN::ECClassCR ecClass = m_native->GetClass();
            return Convert::ECClassToManaged (&ecClass, this);
            }
        }

    ECValue^ GetValue (System::String^ propertyName)
        {
        pin_ptr<wchar_t const> propertyNamePinned = PtrToStringChars (propertyName);
        Utf8String utf8PropertyName (propertyNamePinned);
        ECN::ECValue    nativeValue;
        ECN::ECObjectsStatus status = m_native->GetValue (nativeValue, utf8PropertyName.c_str());
        if (ECN::ECObjectsStatus::Success != status)
            return nullptr;

        return Convert::ECValueToManaged (new ECN::ECValue (nativeValue));
        }

    void SetValue (System::String^ propertyName, ECValue^ value)
        {
        pin_ptr<wchar_t const> propertyNamePinned = PtrToStringChars (propertyName);
        Utf8String utf8PropertyName (propertyNamePinned);
        ECN::ECValue    nativeValue;
        Convert::ECValueToNative (value);
        ECN::ECObjectsStatus status = m_native->SetValue (utf8PropertyName.c_str(), nativeValue);

        // should do someting with status here.
        if (ECN::ECObjectsStatus::Success != status)
            return;
        }

    ~ECInstance ()
        {
        if (nullptr == m_native)
            return;
        m_native->Release();

        m_native     = nullptr;
        m_marshaller = nullptr;
        // System.GC.SuppressFinalize is called for us.
        }

    !ECInstance ()
        {
        if (nullptr != m_marshaller)
            {
            m_marshaller->QueueEntry (m_native);
            m_marshaller = nullptr;
            m_native = nullptr;
            }
        }

};

/*=================================================================================**//**
* AdHocPropertyQuery - managed version of ECN::AdHocPropertyQuery
* Provides read-only access to ad-hoc properties defined on an IECInstance.
* AdHoc properties are name-value pairs stored on an ECInstance.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct AdHocPropertyQuery
{
private:
    ECInstance^                 m_instance;
    ECN::AdHocPropertyQuery*    m_query;

public:
    AdHocPropertyQuery (ECInstance^ instance, System::String^ containerAccessString)
        {
        m_instance      = instance;
        pin_ptr<wchar_t const> containerAccessStringPinned = PtrToStringChars(containerAccessString);
        Utf8String utf8CAS (containerAccessStringPinned);
        m_query         = new ECN::AdhocPropertyQuery (*instance->GetNative(), utf8CAS.empty() ? "Parameters" : utf8CAS.c_str());
        }

    property ECInstance^ Host
        {
        ECInstance^ get ()
            {
            return m_instance;
            }
        }

    int GetPropertyIndex (System::String^ accessString)
        {
        uint32_t index;
        pin_ptr<wchar_t const> accessStringPinned = PtrToStringChars (accessString);
        Utf8String utf8AS (accessStringPinned);
        if (m_query->GetPropertyIndex (index, utf8AS.c_str()))
            return index;

        return UINT32_MAX;
        }

    property int Count
        {
        int get ()
            {
            return m_query->GetCount();
            }
        }

    System::String^ GetName (int index)
        {
        Utf8String name;
        m_query->GetName (name, index);
        return gcnew System::String (name.c_str());
        }

    System::String^ GetDisplayLabel (int index)
        {
        Utf8String displayLabel;
        m_query->GetDisplayLabel (displayLabel, index);
        return gcnew System::String (displayLabel.c_str());
        }

    ECValue^ GetValue (int index)
        {
        ECN::ECValue value;
        m_query->GetValue (value, index);
        return Convert::ECValueToManaged (new ECN::ECValue (value));
        }

    ECPropertyPrimitiveType GetPrimitiveType (int index)
        {
        ECN::PrimitiveType type = (ECN::PrimitiveType)0;
        m_query->GetPrimitiveType (type, index);
        return (ECPropertyPrimitiveType) type;
        }

    System::String^ GetUnitName (int index)
        {
        Utf8String unit;
        m_query->GetDisplayLabel (unit, index);
        return gcnew System::String (unit.c_str());
        }

    bool IsReadOnly (int index)
        {
        bool isReadOnly = false;
        m_query->IsReadOnly (isReadOnly, index);
        return isReadOnly;
        }

    bool IsHidden (int index)
        {
        bool isHidden = false;
        m_query->IsHidden (isHidden, index);
        return isHidden;
        }
};


/*=================================================================================**//**
* ECValue - managed version of ECN::ECValue
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECValue
{
private:
    ECN::ECValueCP  m_native;

internal:
    ECValue (ECN::ECValueCP native)
        {
        m_native = native;
        }

    ECValue (double value)
        {
        m_native = new ECN::ECValue (value);
        }

    ECValue (int value)
        {
        m_native = new ECN::ECValue (value);
        }

    ECValue (System::String^ value)
        {
        pin_ptr<wchar_t const> valuePinned = PtrToStringChars (value);
        m_native = new ECN::ECValue (valuePinned, true);
        }

    ECValue (System::DateTime dateTime)
        {
        int64_t commonEraTicks = dateTime.Ticks;
        uint64_t julianDay = BENTLEY_NAMESPACE_NAME::DateTime::CommonEraMillisecondsToJulianDay (commonEraTicks / 10000);
        BENTLEY_NAMESPACE_NAME::DateTime::Info  dateTimeInfo  = BENTLEY_NAMESPACE_NAME::DateTime::Info::CreateForDateTime (BENTLEY_NAMESPACE_NAME::DateTime::Kind::Utc);
        BENTLEY_NAMESPACE_NAME::DateTime        nativeDateTime;
        BENTLEY_NAMESPACE_NAME::DateTime::FromJulianDay (nativeDateTime, julianDay, dateTimeInfo);
        m_native = new ECN::ECValue (nativeDateTime);
        }

    ECValue (GeometryNET::DPoint3d value)
        {
        pin_ptr<GeometryNET::DPoint3d>  valuePinned = &value;
        m_native = new ECN::ECValue (*(DPoint3dP) valuePinned);
        }

    ECN::ECValueCP  GetNative () { return m_native; }

public:
    static ECValue^ FromDouble (double value)
        {
        return gcnew ECValue (value);
        }

    static ECValue^ FromInteger (int value)
        {
        return gcnew ECValue (value);
        }

    static ECValue^ FromString (System::String^ value)
        {
        return gcnew ECValue (value);
        }

    static ECValue^ FromDateTime (System::DateTime value)
        {
        return gcnew ECValue (value);
        }

    static ECValue^ FromPoint3d (GeometryNET::DPoint3d value)
        {
        return gcnew ECValue (value);
        }

    property bool IsNull
        {
        bool get ()
            {
            return m_native->IsNull();
            }
        }

    property bool IsPrimitive
        {
        bool get ()
            {
            return m_native->IsPrimitive();
            }
        }

    property ECPropertyPrimitiveType ECPrimitiveType
        {
        ECPropertyPrimitiveType get ()
            {
            return (ECPropertyPrimitiveType) m_native->GetPrimitiveType();
            }
        }

    System::String^ GetString()
        {
        return m_native->IsNull() ? System::String::Empty : gcnew System::String (m_native->ToString().c_str());
        }

    int GetInteger()
        {
        return m_native->IsNull() ? 0 : m_native->GetInteger();
        }

    double GetDouble()
        {
        return m_native->IsNull() ? 0.0 : m_native->GetDouble();
        }

    GeometryNET::DPoint3d GetPoint3d()
        {
        return m_native->IsNull() ? GeometryNET::DPoint3d() : Convert::DPoint3dToManaged (m_native->GetPoint3d());
        }

    System::DateTime GetDateTime()
        {
        return Convert::DateTimeToManaged (m_native->GetDateTime());
        }

    ~ECValue ()
        {
        // Dispose method

        // call finalizer to clean up native resources.
        this->!ECValue();

        // don't need finalize, when the Dispose method is called, the runtime calls System::GC::SuppressFinalize for us.
        }

    !ECValue ()
        {
        // finalize method
        delete m_native;
        m_native = nullptr;
        }

};


#if defined (NEEDSWORK_ECDB_AdhocJsonPropertyValue)
/*=================================================================================**//**
* AdHocJsonPropertyValue - managed version of ECN::AdHocJsonPropertyValue
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct AdHocJsonPropertyValue
{
private:
    ECN::AdHocJsonPropertyValueP    m_native;

internal:
    AdHocJsonPropertyValue (ECN::AdHocJsonPropertyValueP native)
        {
        m_native = native;
        }

    /** The value of this ad hoc property. */
    property ECValue^ ValueEC
        {
        ECValue^ get ()
            {
            ECN::ECValue value (m_native->GetValueEC());
            return gcnew ECValue (new ECN::ECValue (value));
            }
        void set (ECValue^ value)
            {
            m_native->SetValueEC (*Convert::ECValueToNative (value));
            }
        }

    /** The type of this ad hoc property's value. */
    property ECPropertyPrimitiveType Type
        {
        ECPropertyPrimitiveType get ()
            {
            ECN::PrimitiveType type;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetType (type);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? (ECPropertyPrimitiveType)type : ECPropertyPrimitiveType::Unknown;

            }
        }

    /** The ExtendedType of this ad hoc property value, used to show or edit the property value. */
    property System::String^ ExtendedType
        {
        System::String^ get ()
            {
            Utf8String  extendedType;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetExtendedType (extendedType);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? gcnew System::String (extendedType.c_str()) : System::String::Empty;
            }
        void set (System::String^ value)
            {
            pin_ptr<wchar_t const> valuePinned = PtrToStringChars (value);
            Utf8String utf8Value (valuePinned);
            m_native->SetExtendedType (utf8Value.c_str());
            }
        }

    /** The units of this ad hoc property value. */
    property System::String^ Units
        {
        System::String^ get ()
            {
            Utf8String  units;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetUnits (units);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? gcnew System::String (units.c_str()) : System::String::Empty;
            }
        void set (System::String^ value)
            {
            pin_ptr<wchar_t const> valuePinned = PtrToStringChars (value);
            Utf8String utf8Value (valuePinned);
            m_native->SetUnits (utf8Value.c_str());
            }
        }

    /** Controls if this ad hoc property should be hidden. */
    property bool Hidden
        {
        bool get ()
            {
            bool hidden;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetHidden (hidden);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? hidden : false;
            }
        void set (bool value)
            {
            m_native->SetHidden (value);
            }
        }

    /** Controls if this ad property's value should be read-only or not. */
    property bool ReadOnly
        {
        bool get ()
            {
            bool readOnly;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetReadOnly (readOnly);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? readOnly : false;
            }
        void set (bool value)
            {
            m_native->SetReadOnly (value);
            }
        }

    /** The Priority of this ad hoc property value. Typically used for presentation. */
    property int Priority
        {
        int get ()
            {
            int priority;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetPriority (priority);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? priority : 0;
            }
        void set (int value)
            {
            m_native->SetPriority (value);
            }
        }

    property System::String^ Category
        {
        System::String^ get ()
            {
            Utf8String  category;
            ECN::AdHocJsonPropertyValue::GetStatus status = m_native->GetCategory (category);
            return (ECN::AdHocJsonPropertyValue::GetStatus::Found == status) ? gcnew System::String (category.c_str()) : System::String::Empty;
            }
        void set (System::String^ value)
            {
            pin_ptr<wchar_t const> valuePinned = PtrToStringChars (value);
            Utf8String utf8Value (valuePinned);
            m_native->SetCategory (utf8Value.c_str());
            }
        }
};
#endif

struct ECClassEnumeratorImpl
    {
    // we need this class only because a managed class can have only a pointer to a struct as a member, not a struct
    bvector<ECN::ECClassP> const&           m_collection;
    bvector<ECN::ECClassP>::const_iterator  m_current;

    ECClassEnumeratorImpl (bvector<ECN::ECClassP> const& collection) : m_collection (collection), m_current (m_collection.begin()) {}
    };


/*=================================================================================**//**
* ECClassEnumerator - an enumerator over an ECClassCollection - never instantiated directly.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECClassEnumerator : System::Collections::Generic::IEnumerator <ECClass^>
{
private:
    bvector<ECN::ECClassP> const&   m_collection;
    System::Object^                 m_owner;

    ECClassEnumeratorImpl*          m_impl;
    ECClass^                        m_currentMember;

    ECClass^     GetMember()
        {
        if (NULL == m_impl)
            return nullptr;

        bvector<ECN::ECClassP>::const_iterator theCurrent = m_impl->m_current;
        bvector<ECN::ECClassP>::const_iterator theEnd     = m_impl->m_collection.end();
        if (!(theCurrent != theEnd))
            return nullptr;

        ECN::ECClassCP native = (*m_impl->m_current);
        return gcnew ECClass (native, m_owner);
        }

    void            SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    bool            MoveNextInternal ()
        {
        if (nullptr == m_impl)
            {
            m_impl = new ECClassEnumeratorImpl (m_collection);
            return m_impl->m_current != m_impl->m_collection.end();
            }

        bvector<ECN::ECClassP>::const_iterator endIterator = m_impl->m_collection.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }


internal:
    ECClassEnumerator (bvector<ECN::ECClassP> const& collection, System::Object^ owner) : m_collection (collection)
        {
        m_owner         = owner;
        m_impl          = nullptr;
        m_currentMember = nullptr;
        }

public:

    /// <summary> Advances the enumerator to the next ECClass in the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECClass in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = nullptr;
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current ECClass in the collection. </summary>
    property ECClass^ Current
        {
        virtual ECClass^ get () = System::Collections::Generic::IEnumerator <ECClass^>::Current::get
            {
            return m_currentMember;
            }
        };

    ~ECClassEnumerator ()
        {
        FreeImpl ();
        }

    !ECClassEnumerator ()
        {
        FreeImpl();
        }

};


/*=================================================================================**//**
* ECClassCollection - a collection of ECClass objects.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECClassCollection : System::Collections::Generic::IEnumerable <ECClass^>
{
private:
    bvector<ECN::ECClassP> const*   m_native;
    System::Object^                 m_owner;

internal:
    ECClassCollection (bvector<ECN::ECClassP> const* native, System::Object^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

public:
    virtual System::Collections::IEnumerator^ RawGetEnumerator () = System::Collections::IEnumerable::GetEnumerator
        {
        return GetEnumerator ();
        }

    virtual System::Collections::Generic::IEnumerator <ECClass^>^ GetEnumerator ()
        {
        return gcnew ECClassEnumerator (*m_native, this);
        }
};



/*=================================================================================**//**
* ECProperty - managed version of ECN::ECProperty class
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECProperty
{
private:
    ECN::ECPropertyCP   m_native;
    System::Object^     m_owner;

internal:
    ECProperty (ECN::ECPropertyCP native, System::Object^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

    ECN::ECPropertyCP   GetNative() {return m_native;}

public:
    property System::String^ Name
        {
        System::String^ get ()
            {
            Utf8StringCR name = m_native->GetName ();
            return gcnew System::String (name.c_str());
            }
        }

    /** If this property holds a primitive type, return a PrimitiveECProperty. If not, it holds a struct or an array. */
    PrimitiveECProperty^    GetAsPrimitiveProperty();

    /**
     * Query the specified custom attribute on this property definition
     * @param className The class of the custom attribute to look up
     * @return the custom attribute or null if no such custom attribute is defined for this property.
     */
    ECInstance^ GetCustomAttribute (System::String^  className)
        {
        pin_ptr<wchar_t const>  classNamePinned = PtrToStringChars (className);
        Utf8String  utf8ClassName (classNamePinned);

        ECN::IECInstancePtr nativeInstance = m_native->GetCustomAttribute (utf8ClassName.c_str());
        if (nativeInstance.IsNull())
            return nullptr;

        return Convert::ECInstanceToManaged (nativeInstance.get());
        }

};

/*=================================================================================**//**
* PrimitiveECProperty - managed version of ECN::PrimitiveEcProperty
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct PrimitiveECProperty : public ECProperty
{
internal:
    PrimitiveECProperty (ECN::PrimitiveECPropertyCP native, System::Object^ owner) : ECProperty (native, owner)
        {
        }

public:
    property ECPropertyPrimitiveType Type
        {
        ECPropertyPrimitiveType get ()
            {
            ECN::PrimitiveECPropertyCP native = dynamic_cast <ECN::PrimitiveECPropertyCP> (GetNative());
            return (ECPropertyPrimitiveType) native->GetType();
            }
        }
};


// can't be inlined because requires PrimitiveECProprty to be defined.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveECProperty^    ECProperty::GetAsPrimitiveProperty ()
    {
    // we might have already instantiated a PrimitiveECProperty.
    PrimitiveECProperty^ primitiveECProperty;
    if (nullptr != (primitiveECProperty = dynamic_cast<PrimitiveECProperty^> (this)))
        return primitiveECProperty;

    // see if the native property is primitive.
    ECN::PrimitiveECPropertyCP nativePrimitive;
    if (nullptr == (nativePrimitive = m_native->GetAsPrimitiveProperty()))
        return nullptr;

    return Convert::PrimitiveECPropertyToManaged (nativePrimitive, m_owner);
    }


struct ECPropertyEnumeratorImpl
    {
    // we need this class only because a managed class can have only a pointer to a struct as a member, not a struct
    ECN::ECPropertyIterable                         m_collection;
    ECN::ECPropertyIterable::const_iterator         m_current;

    ECPropertyEnumeratorImpl (ECN::ECClassCP ecClass) : m_collection (ecClass->GetProperties()), m_current (m_collection.begin())
        {
        }
    };


/*=================================================================================**//**
* ECPropertyEnumerator - an enumerator over an ECPropertyCollection - never instantiated directly.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECPropertyEnumerator : System::Collections::Generic::IEnumerator <ECProperty^>
{
private:
    ECClass^                    m_ecClass;
    System::Object^             m_owner;
    ECPropertyEnumeratorImpl*   m_impl;
    ECProperty^                 m_currentMember;

    ECProperty^     GetMember()
        {
        if (NULL == m_impl)
            return nullptr;

        ECN::ECPropertyIterable::const_iterator theCurrent = m_impl->m_current;
        ECN::ECPropertyIterable::const_iterator theEnd     = m_impl->m_collection.end();
        if (!(theCurrent != theEnd))
            return nullptr;

        ECN::ECPropertyCP native = (*m_impl->m_current);
        return gcnew ECProperty (native, m_owner);
        }

    void            SetCurrentMember ()
        {
        m_currentMember = GetMember();
        }

    bool            MoveNextInternal ()
        {
        if (nullptr == m_impl)
            {
            m_impl = new ECPropertyEnumeratorImpl (m_ecClass->GetNative());
            return m_impl->m_current != m_impl->m_collection.end();
            }

        ECN::ECPropertyIterable::const_iterator endIterator = m_impl->m_collection.end();
        if (!(m_impl->m_current != endIterator))
            return false;

        ++m_impl->m_current;
        return m_impl->m_current != endIterator;
        }

    void            FreeImpl()
        {
        if (nullptr == m_impl)
            return;

        delete m_impl;
        m_impl = NULL;
        }


internal:
    ECPropertyEnumerator (ECClass^ ecClass, System::Object^ owner)
        {
        m_ecClass       = ecClass;
        m_owner         = owner;
        m_impl          = nullptr;
        m_currentMember = nullptr;
        }

public:

    /// <summary> Advances the enumerator to the next ECProperty in the collection. </summary>
    virtual bool MoveNext ()
        {
        bool retval = MoveNextInternal ();
        SetCurrentMember ();
        return retval;
        }

    /// <summary> Sets the enumerator to its initial position, which is before the first ECClass in the collection. </summary>
    virtual void Reset ()
        {
        m_currentMember = nullptr;
        FreeImpl();
        }

    property System::Object^ RawCurrent
        {
        virtual Object^ get() = System::Collections::IEnumerator::Current::get
            {
            return m_currentMember;
            }
        };

    /// <summary> Gets the current ECProperty in the collection. </summary>
    property ECProperty ^ Current
        {
        virtual ECProperty^ get () = System::Collections::Generic::IEnumerator <ECProperty^>::Current::get
            {
            return m_currentMember;
            }
        };

    ~ECPropertyEnumerator ()
        {
        FreeImpl ();
        }

    !ECPropertyEnumerator ()
        {
        FreeImpl();
        }

};


/*=================================================================================**//**
* ECPropertyCollection - a collection of ECProperty objects.
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ECPropertyCollection : System::Collections::Generic::IEnumerable <ECProperty^>
{
private:
    ECClass^            m_ecClass;
    System::Object^     m_owner;

internal:
    ECPropertyCollection (ECClass^ ecClass, System::Object^ owner)
        {
        m_ecClass = ecClass;
        m_owner   = owner;
        }

public:
    virtual System::Collections::IEnumerator^ RawGetEnumerator () = System::Collections::IEnumerable::GetEnumerator
        {
        return GetEnumerator ();
        }

    virtual System::Collections::Generic::IEnumerator <ECProperty^>^ GetEnumerator ()
        {
        return gcnew ECPropertyEnumerator (m_ecClass, this);
        }
};

// can't be inlined
ECPropertyCollection^ ECClass::Properties::get ()
    {
    return gcnew ECPropertyCollection (this, m_owner);
    }


/*=================================================================================**//**
* DgnViewport - managed version of Dgn::DgnViewport
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct Viewport
{
private:
    BDGN::DgnViewportP m_native;

internal:
    Viewport (BDGN::DgnViewportP native)
        {
        m_native = native;
        }

public:
    /** The ViewController for this view */
    property DgnNET::ViewController^ ViewController
        {
        DgnNET::ViewController^ get ()
            {
            return Convert::ViewControllerToManaged (&m_native->GetViewControllerR(), this);
            }
        }
};


/*=================================================================================**//**
* ViewController - managed version of Dgn::ViewController
* @bsiclass                                                     Barry.Bentley   10/16
+===============+===============+===============+===============+===============+======*/
public ref struct ViewController
{
private:
    BDGN::ViewControllerP   m_native;
    Viewport^               m_owner;

internal:
    ViewController (BDGN::ViewControllerP native, Viewport^ owner)
        {
        m_native = native;
        m_owner  = owner;
        }

public:
    property DgnModel^ TargetModel
        {
        DgnModel^ get ()
            {
            // NEEDSWORK - where do we get the DgnDb we should be using for the owner?
            return gcnew DgnModel (m_native->GetTargetModel(), nullptr, false);
            }

        }
};


// Here are the methods that we could not inline.
BDGN::DgnDbP            Convert::DgnDbToNative (DgnNET::DgnDb^ managed)
    {
    return managed->GetNative();
    }

#if defined (NEEDSWORK_ECDB_ArrayValue)
ECSqlArrayValue^        Convert::ECSqlArrayValueToManaged (BeSQLite::EC::IECSqlArrayValue* native)
    {
    return gcnew ECSqlArrayValue (native);
    }
#endif

ECSqlValue^             Convert::ECSqlValueToManaged (BeSQLite::EC::IECSqlValue* native, ECSqlValue^ owner)
    {
    return gcnew ECSqlValue (native, owner);
    }

DgnDb^                  Convert::DgnDbToManaged (BDGN::DgnDbP dgnDb)
    {
    return gcnew DgnDb (dgnDb);
    }

DgnModels^              Convert::DgnModelsToManaged (BDGN::DgnModels* models, DgnDb^ owner)
    {
    return gcnew DgnModels (models, owner);
    }

SchemaManager^          Convert::SchemaManagerToManaged (BeSQLite::EC::SchemaManager const* schemaManager, DgnDb^ dgnDb)
    {
    return gcnew SchemaManager (schemaManager, dgnDb);
    }

DgnElementCollection^   Convert::ElementCollectionToManaged (BDGN::DgnElements* elements, DgnDb^ dgnDb)
    {
    return gcnew DgnElementCollection (elements, dgnDb);
    }

DgnModel^               Convert::DgnModelToManaged (BDGN::DgnModelP model, DgnDb^ dgnDb, bool needRelease)
    {
    return gcnew DgnModel (model, dgnDb, needRelease);
    }

BDGN::DgnModelP         Convert::DgnModelToNative (DgnModel^ model)
    {
    return model->GetNative();
    }

DgnElement^             Convert::DgnElementToManaged (BDGN::DgnElementP element)
    {
    // create the appropriate subclass depending on the characteristics of element.
    BDGN::GeometricElement3d*    geom3d;
    BDGN::GeometricElement*      geom;
    if (nullptr != (geom3d = dynamic_cast <BDGN::GeometricElement3d*>(element)))
        return gcnew GeometricElement3d (geom3d);
    else if (nullptr != (geom = dynamic_cast <BDGN::GeometricElement*>(element)))
        return gcnew GeometricElement (geom);
    return gcnew DgnElement (element);
    }

BDGN::DgnElementP       Convert::DgnElementToNative (DgnElement^ element)
    {
    return element->GetNative();
    }

GeometrySource^         Convert::GeometrySourceToManaged (BDGN::GeometrySourceP source, DgnElement^ element)
    {
    return gcnew GeometrySource (source, element);
    }

GeometrySource3d^       Convert::GeometrySource3dToManaged (BDGN::GeometrySource3dP source, DgnElement^ element)
    {
    return gcnew GeometrySource3d (source, element);
    }

GeometrySource2d^       Convert::GeometrySource2dToManaged (BDGN::GeometrySource2dP source, DgnElement^ element)
    {
    return gcnew GeometrySource2d (source, element);
    }

BDGN::GeometrySourceP   Convert::GeometrySourceToNative (GeometrySource^ source)
    {
    return source->GetNative();
    }

ECClass^                 Convert::ECClassToManaged (ECN::ECClassCP native, System::Object^ owner)
    {
    return gcnew ECClass (native, owner);
    }

ECValue^                 Convert::ECValueToManaged (ECN::ECValueP native)
    {
    return gcnew ECValue (native);
    }

ECN::ECValueCP           Convert::ECValueToNative (ECValue^ ecValue)
    {
    return ecValue->GetNative();
    }

#if defined (NEEDSWORK_ECDB_AdhocJsonPropertyValue)
AdHocJsonPropertyValue^  Convert::AdHocJsonPropertyValueToManaged (ECN::AdHocJsonPropertyValueP native)
    {
    return gcnew AdHocJsonPropertyValue (native);
    }
#endif

BDGN::IBriefcaseManager::Request* Convert::RepositoryRequestToNative (RepositoryRequest^ request)
    {
    return request->GetNative();
    }

GeometryCollection^      Convert::GeometrySourceToCollection (GeometrySource^ managed)
    {
    return gcnew GeometryCollection (managed);
    }

ECSchema^                Convert::ECSchemaToManaged (ECN::ECSchemaCP nativeSchema, System::Object^ owner)
    {
    return gcnew ECSchema (nativeSchema, owner);
    }

ECClassCollection^       Convert::ECClassCollectionToManaged (bvector<ECN::ECClassP> const* native, System::Object^ owner)
    {
    return gcnew ECClassCollection (native, owner);
    }

ECProperty^              Convert::ECPropertyToManaged (ECN::ECPropertyCP native, System::Object^ owner)
    {
    ECN::PrimitiveECPropertyCP nativePrimitiveProperty = native->GetAsPrimitiveProperty();
    if (nullptr != nativePrimitiveProperty)
        return gcnew PrimitiveECProperty (nativePrimitiveProperty, owner);
    else
        return gcnew ECProperty (native, owner);
    }

ECInstance^              Convert::ECInstanceToManaged (ECN::IECInstanceP native)
    {
    return gcnew ECInstance (native);
    }

PrimitiveECProperty^     Convert::PrimitiveECPropertyToManaged (ECN::PrimitiveECPropertyCP native, System::Object^ owner)
    {
    return gcnew PrimitiveECProperty (native, owner);
    }

ViewController^         Convert::ViewControllerToManaged (BDGN::ViewControllerP native, Viewport^ owner)
    {
    return gcnew ViewController (native, owner);
    }

DgnCategory^             Convert::DgnCategoryToManaged (BDGN::DgnCategoryP category, DgnDb^ dgnDb)
    {
    return gcnew DgnCategory (category, dgnDb);
    }

}
}

