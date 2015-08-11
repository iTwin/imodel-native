declare module BentleyApi.Dgn {

    //! A wrapper for BentleyApi::DPoint3d
    class JsDPoint3d implements IDisposable {
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
    class JsYawPitchRollAngles implements IDisposable {
        static Create(yaw: cxx_double, pitch: cxx_double, roll: cxx_double): cxx_pointer<JsYawPitchRollAngles>;
        Yaw: cxx_double;
        Pitch: cxx_double;
        Roll: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

}
