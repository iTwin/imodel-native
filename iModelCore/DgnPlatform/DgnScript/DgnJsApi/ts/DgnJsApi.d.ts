declare module BentleyApi.Dgn {

    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class JsDPoint3d { }
    class JsYawPitchRollAngles { }
    /*** END_FORWARD_DECLARATIONS ***/

    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;
    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    //! Logging serverity level.
    enum LoggingSeverity { }

    //! Access to the message log
    class Logging implements BeJsProjection_SuppressConstructor {

        //! Set the severity level for the specified category
        //! @param catagory     The logging category
        //! @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
        static SetSeverity(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): void;

        //! Test if the specified severity level is enabled for the specified category
        //! @param category     The logging category
        //! @param severity     The severity to test
        static IsSeverityEnabled(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): cxx_bool;

        //! Send a message to the log
        //! @param catagory     The logging category
        //! @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
        //! @param message      The message to log
        static Message(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>, message: Bentley_Utf8String): void;
    }

    //! Script Management Utilities
    class Script implements BeJsProjection_SuppressConstructor {

        //! Make sure the that specified library is loaded
        //! @param libName  The name of the library that is to be loaded
        static ImportLibrary(libName: Bentley_Utf8String): void;

        //! Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
        //! @param description  A description of the error
        static ReportError(description: Bentley_Utf8String): void;
    }

    //! A wrapper for BentleyApi::Dgn::DgnElement.
    class JsDgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        GetElementId(): Bentley_Utf8String;
        Insert(): cxx_int32_t;
        Update(): cxx_int32_t;
        SetParent(parent: cxx_pointer<JsDgnElement>): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDgnElementP = cxx_pointer<JsDgnElement>;

    //! A wrapper for BentleyApi::Dgn::DgnModel. There is no constructor. The native caller must supply one.
    class JsDgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        GetModelId(): Bentley_Utf8String;
        CreateElement(elType: Bentley_Utf8String, categoryName: Bentley_Utf8String): JsDgnElementP;
        DeleteAllElements(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    //! A wrapper for BentleyApi::Dgn::ElementGeometryBuilder. There is no constructor. The user must call the Create method to create a new one.
    class JsElementGeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        constructor(el: JsDgnElementP, o: JsDPoint3dP, angles: JsYawPitchRollAnglesP);
        AppendBox(x: cxx_double, y: cxx_double, z: cxx_double): void;
        AppendSphere(radius: cxx_double): void;
        SetGeomStreamAndPlacement(element: JsDgnElementP): cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }
}
