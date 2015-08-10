declare module BentleyApi.Dgn {

    //! Utilities
    class JsUtils implements BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted {
        //! Make sure that a script library is loaded
        static ImportLibrary(libName: Bentley_Utf8String): void;

        //! Report an error
        static ReportError(description: Bentley_Utf8String): void;
    }

    //! A wrapper for BentleyApi::DPoint3d
    class JsDPoint3d implements IDisposable, BeJsProjection_RefCounted {
        static Create(x: cxx_double, y: cxx_double, z: cxx_double): cxx_pointer<JsDPoint3d>;
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;

    //! A wrapper for BentleyApi::YawPitchRollAngles
    class JsYawPitchRollAngles implements IDisposable, BeJsProjection_RefCounted {
        static Create(yaw: cxx_double, pitch: cxx_double, roll: cxx_double): cxx_pointer<JsYawPitchRollAngles>;
        Yaw: cxx_double;
        Pitch: cxx_double;
        Roll: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    //! A wrapper for BentleyApi::Dgn::DgnElement. There is no constructor. The user must call DgnModel::Create to create a new one.
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

    //! A wrapper for BentleyApi::Dgn::DgnElement. There is no constructor. The user must call ?? to create a new one.
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
        static Create(el: JsDgnElementP, o: JsDPoint3dP, angles: JsYawPitchRollAnglesP): cxx_pointer<JsElementGeometryBuilder>;
        AppendBox(x: cxx_double, y: cxx_double, z: cxx_double): void;
        SetGeomStreamAndPlacement(element: JsDgnElementP): cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }
}
