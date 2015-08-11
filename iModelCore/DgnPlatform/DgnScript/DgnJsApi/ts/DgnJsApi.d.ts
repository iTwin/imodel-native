declare module BentleyApi.Dgn {

    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class JsDPoint3d { }
    class JsYawPitchRollAngles { }
    /*** END_FORWARD_DECLARATIONS ***/

    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;
    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    //! Utilities
    class JsUtils implements BeJsProjection_SuppressConstructor {
        //! Make sure that a script library is loaded
        static ImportLibrary(libName: Bentley_Utf8String): void;

        //! Report an error
        static ReportError(description: Bentley_Utf8String): void;
    }

    //! A wrapper for BentleyApi::Dgn::DgnElement.
    class JsDgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        GetElementId(): Bentley_Utf8String;
        Insert(): cxx_int32_t;
        Update(): cxx_int32_t;
        SetParent(parent: cxx_pointer<JsDgnElement>): void;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDgnElementP = cxx_pointer<JsDgnElement>;

    //! A wrapper for BentleyApi::Dgn::DgnModel. There is no constructor. The native caller must supply one.
    class JsDgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        GetModelId(): Bentley_Utf8String;
        CreateElement(elType: Bentley_Utf8String, categoryName: Bentley_Utf8String): JsDgnElementP;
        DeleteAllElements(): void;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    //! A wrapper for BentleyApi::Dgn::DgnElement. There is no constructor. The user must call the Create method to create a new one.
    class JsElementGeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        constructor(el: JsDgnElementP, o: JsDPoint3dP, angles: JsYawPitchRollAnglesP);
        AppendBox(x: cxx_double, y: cxx_double, z: cxx_double): void;
        SetGeomStreamAndPlacement(element: JsDgnElementP): cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }
}
