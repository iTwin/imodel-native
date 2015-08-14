declare module BentleyApi.Dgn {

    //! A wrapper for BentleyApi::DPoint3d
    class JsDPoint3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDPoint3dP = cxx_pointer<JsDPoint3d>;

    //! A wrapper for BentleyApi::DVector3d
    class JsDVector3d implements IDisposable {
        constructor(x: cxx_double, y: cxx_double, z: cxx_double);
        X: cxx_double;
        Y: cxx_double;
        Z: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDVector3dP = cxx_pointer<JsDVector3d>;

    //! A wrapper for BentleyApi::YawPitchRollAngles
    class JsYawPitchRollAngles implements IDisposable {
        constructor(yaw: cxx_double, pitch: cxx_double, roll: cxx_double);
        YawDegrees: cxx_double;
        PitchDegrees: cxx_double;
        RollDegrees: cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsYawPitchRollAnglesP = cxx_pointer<JsYawPitchRollAngles>;

    
    //! A strongly typed angle, with explicitly named access to degrees and radians
    class JsAngle implements IDisposable {
        
        static CreateDegrees (value : cxx_double) : JsAngleP;
        static CreateRadians (value : cxx_double) : JsAngleP;
        Radians : cxx_double;
        Degrees : cxx_double;
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }
    type JsAngleP = cxx_pointer<JsAngle>;



 


    //! A wrapper for BentleyApi::DEllipse3d
    class JsDEllipse3d implements IDisposable {
        constructor (
                center      : JsDPoint3dP,
                vector0     : JsDVector3dP,
                vector90    : JsDVector3dP,
                startAngle  : JsAngleP,
                sweepAngle  : JsAngleP
                );
        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDEllipse3dP = cxx_pointer<JsDEllipse3d>;

    //! A wrapper for BentleyApi::DSegment3d
    class JsDSegment3d implements IDisposable {
        constructor (
                startPoint : JsDPoint3dP,
                endPoint   : JsDPoint3dP
                );
        PointAtFraction (f : cxx_double) : JsDPoint3dP;                
        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsDSegment3dP = cxx_pointer<JsDSegment3d>;

    type JsDPoint3dArrayP = cxx_pointer<JsDPoint3dArray>;

    //! A wrapper for BentleyApi::DSegment3d
    class JsDPoint3dArray implements IDisposable {
        constructor();

        Add(value: JsDPoint3dP): void;

        Size(): cxx_double;
        Clear (): void;
        Append(other: JsDPoint3dArrayP): void;

        At(index: cxx_double): JsDPoint3dP;

        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }





    //! A wrapper for BentleyApi::DSegment3d
    class JsCurvePrimitive implements IDisposable {
        constructor ();
        static CreateLineSegment(segment: JsDSegment3dP): JsCurvePrimitiveP;
        static CreateEllipticArc(arc: JsDEllipse3dP): JsCurvePrimitiveP;
        static CreateLineString(points: JsDPoint3dArrayP): JsCurvePrimitiveP;
        CurvePrimitiveType(): cxx_double;
        PointAtFraction(f: cxx_double): JsDPoint3dP; 

        _OnDispose(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type JsCurvePrimitiveP = cxx_pointer<JsCurvePrimitive>;



}
