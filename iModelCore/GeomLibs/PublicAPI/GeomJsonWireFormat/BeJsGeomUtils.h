/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/GeomApi.h>
#include <json/BeJsValue.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
PUSH_MSVC_IGNORE(4701) // potentially uninitialized

//=======================================================================================
//! Utilities to help save and restore values as JSON
// @bsiclass
//=======================================================================================
struct BeJsGeomUtils {
    BE_JSON_NAME(low)
    BE_JSON_NAME(high)
    BE_JSON_NAME(yaw)
    BE_JSON_NAME(pitch)
    BE_JSON_NAME(roll)
    BE_JSON_NAME(degrees)
    BE_JSON_NAME(radians)
    BE_JSON_NAME(_degrees)
    BE_JSON_NAME(_radians)

    static void FromAngle(BeJsValue out, Angle angle) { out = angle.Degrees(); }
    static void AngleInDegreesToJson(BeJsValue out, AngleInDegrees angle) { out = angle.Degrees(); } // default for angles in json is degrees.
    static AngleInDegrees AngleInDegreesFromJson(BeJsConst val) { return AngleInDegrees(ToAngle(val)); }

    static Angle ToAngle(BeJsConst val) {
        if (val.isNull()) return Angle::FromDegrees(0.0);
        if (val.isObject()) {
            if (val.isMember(json_degrees()))
                return Angle::FromDegrees(val[json_degrees()].asDouble());
            if (val.isMember(json_radians()))
                return Angle::FromRadians(val[json_radians()].asDouble());
            if (val.isMember(json__degrees()))
                return Angle::FromDegrees(val[json__degrees()].asDouble());
            if (val.isMember(json__radians()))
                return Angle::FromRadians(val[json__radians()].asDouble());
        }
        return Angle::FromDegrees(val.asDouble());
    }

    static void YawPitchRollToJson(BeJsValue val, YawPitchRollAngles angles) {
        // omit any members that are zero
        if (angles.GetYaw().Degrees() != 0.0) AngleInDegreesToJson(val[json_yaw()], angles.GetYaw());
        if (angles.GetPitch().Degrees() != 0.0) AngleInDegreesToJson(val[json_pitch()], angles.GetPitch());
        if (angles.GetRoll().Degrees() != 0.0) AngleInDegreesToJson(val[json_roll()], angles.GetRoll());
    }

    static YawPitchRollAngles YawPitchRollFromJson(BeJsConst val) {
        auto yaw = AngleInDegreesFromJson(val[json_yaw()]);
        auto pitch = AngleInDegreesFromJson(val[json_pitch()]);
        auto roll = AngleInDegreesFromJson(val[json_roll()]);
        return YawPitchRollAngles::FromDegrees(yaw.Degrees(), pitch.Degrees(), roll.Degrees());
    }

    static DPoint3d ToDPoint3d(BeJsConst inValue) {
        DPoint3d point;
        if (inValue.isArray()) {
            point.x = inValue[0].asDouble();
            point.y = inValue[1].asDouble();
            point.z = inValue[2].asDouble();
        } else {
            point.x = inValue["x"].asDouble();
            point.y = inValue["y"].asDouble();
            point.z = inValue["z"].asDouble();
        }
        return point;
    }

    static void DPoint3dFromJson(DPoint3dR point, BeJsConst inValue) { point = ToDPoint3d(inValue); }

    static void DPoint3dToJson(BeJsValue val, DPoint3dCR point) {
        val[0] = point.x;
        val[1] = point.y;
        val[2] = point.z;
    }

    static void Point2dFromJson(Point2dR point, BeJsConst inValue) {
        if (inValue.isArray()) {
            point.x = inValue[0].asInt();
            point.y = inValue[1].asInt();
        } else {
            point.x = inValue["x"].asInt();
            point.y = inValue["y"].asInt();
        }
    }

    static void Point2dToJson(BeJsValue outValue, Point2dCR point) {
        outValue[0] = point.x;
        outValue[1] = point.y;
    }

    static void DPoint2dFromJson(DPoint2dR point, BeJsConst inValue) {
        if (inValue.isArray()) {
            point.x = inValue[0].asDouble();
            point.y = inValue[1].asDouble();
        } else {
            point.x = inValue["x"].asDouble();
            point.y = inValue["y"].asDouble();
        }
    }

    static void DPoint2dToJson(BeJsValue outValue, DPoint2dCR point) {
        outValue[0u] = point.x;
        outValue[1] = point.y;
    }

    static void DVec2dFromJson(DVec2dR vec, BeJsConst inValue) {
        DPoint2dFromJson((DPoint2dR)vec, inValue);
    }

    static void DVec2dToJson(BeJsValue outValue, DVec2dCR vec) {
        DPoint2dToJson(outValue, (DPoint2dCR)vec);
    }

    static void DVec3dFromJson(DVec3dR vec, BeJsConst inValue) { DPoint3dFromJson((DPoint3dR)vec, inValue); }
    static void DVec3dToJson(BeJsValue out, DVec3dCR vec) { DPoint3dToJson(out, (DPoint3dCR)vec); }
    static DVec3d ToDVec3d(BeJsConst inValue) {
        DVec3d vec;
        if (inValue.isArray()) {
            vec.x = inValue[0].asDouble();
            vec.y = inValue[1].asDouble();
            vec.z = inValue[2].asDouble();
        } else {
            vec.x = inValue["x"].asDouble();
            vec.y = inValue["y"].asDouble();
            vec.z = inValue["z"].asDouble();
        }
        return vec;
    }

    static DRange3d ToDRange3d(BeJsConst inValue) {
        DRange3d range = DRange3d::NullRange();
        if (inValue.isArray()) { // if it's an array, just extend range by all points.
            for (Json::ArrayIndex i = 0; i < inValue.size(); ++i)
                range.Extend(ToDPoint3d(inValue[i]));
            return range;
        }

        range.Extend(ToDPoint3d(inValue[json_low()]));
        range.Extend(ToDPoint3d(inValue[json_high()]));
        return range;
    }
    static void DRange3dFromJson(DRange3dR range, BeJsConst inValue) { range = ToDRange3d(inValue); }

    static void DRange3dToJson(BeJsValue outValue, DRange3dCR range) {
        DPoint3dToJson(outValue[json_low()], range.low);
        DPoint3dToJson(outValue[json_high()], range.high);
    }

    static void DRange2dFromJson(DRange2dR range, BeJsConst inValue) {
        DPoint2dFromJson(range.low, inValue[json_low()]);
        DPoint2dFromJson(range.high, inValue[json_high()]);
    }

    static void DRange2dToJson(BeJsValue outValue, DRange2dCR range) {
        DPoint2dToJson(outValue[json_low()], range.low);
        DPoint2dToJson(outValue[json_high()], range.high);
    }

    static void MatrixRowFromJson(double* row, BeJsConst inValue) {
        if (!inValue.isArray()) {
            BeAssert(false && "must be array");
            return;
        }
        for (int y = 0; y < 3; ++y)
            row[y] = inValue[y].asDouble();
    }

    static void MatrixRowToJson(BeJsValue outValue, double const* row) {
        for (int y = 0; y < 3; ++y)
            outValue[y] = row[y];
    }

    static RotMatrix ToRotMatrix(BeJsConst inValue) {
        if (!inValue.isArray()) {
            BeAssert(false && "must be array");
            return RotMatrix::FromIdentity();
        }
        RotMatrix rotation;
        for (int x = 0; x < 3; ++x)
            MatrixRowFromJson(rotation.form3d[x], inValue[x]);
        return rotation;
    }

    static void RotMatrixFromJson(RotMatrixR rotation, BeJsConst inValue) { rotation = ToRotMatrix(inValue); }

    static void RotMatrixToJson(BeJsValue outValue, RotMatrixCR rotation) {
        for (int x = 0; x < 3; ++x)
            MatrixRowToJson(outValue[x], rotation.form3d[x]);
    }

    static void TransformRowFromJson(double* row, BeJsConst inValue) {
        if (!inValue.isArray()) {
            BeAssert(false && "must be array");
            return;
        }

        for (int y = 0; y < 4; ++y)
            row[y] = inValue[y].asDouble();
    }

    static void TransformRowToJson(BeJsValue outValue, double const* row) {
        for (int y = 0; y < 4; ++y)
            outValue[y] = row[y];
    }
    static void TransformFromJson(TransformR trans, BeJsConst inValue) {
        if (inValue.isArray()) {
            if (inValue.size() == 3) {
                for (int x = 0; x < 3; ++x)
                    TransformRowFromJson(trans.form3d[x], inValue[x]);
                return;
            }
            if (inValue.size() == 12) {
                trans.InitFromRowValues(
                    inValue[0].asDouble(), inValue[1].asDouble(), inValue[2].asDouble(), inValue[3].asDouble(),
                    inValue[4].asDouble(), inValue[5].asDouble(), inValue[6].asDouble(), inValue[7].asDouble(),
                    inValue[8].asDouble(), inValue[9].asDouble(), inValue[10].asDouble(), inValue[11].asDouble());
                return;
            }
        } else {
            auto matJson = inValue["matrix"];
            auto orgJson = inValue["origin"];
            if (matJson.isArray() && (orgJson.isArray() || orgJson.isObject())) {
                trans.InitFrom(ToRotMatrix(matJson), ToDPoint3d(orgJson));
                return;
            }
        }
        BeAssert(false && "invalid transform properties");
    }

    static DMatrix4d ToDMatrix4d(BeJsConst inValue) {
        DMatrix4d matrix;
        for (int x = 0; x < 4; ++x)
            TransformRowFromJson(matrix.coff[x], inValue[x]);
        return matrix;
    }

    static void FromTransform(BeJsValue out, TransformCR trans) {
        for (int x = 0; x < 3; ++x)
            TransformRowToJson(out[x], trans.form3d[x]);
    }
    static void TransformToJson(BeJsValue outValue, TransformCR trans) {
        FromTransform(outValue, trans);
    }

    static void DPoint3dVectorToJson(BeJsValue outValue, bvector<DPoint3d> const& points) {
        auto size = points.size();
        for (Json::ArrayIndex i = 0; i < size; ++i) {
            DPoint3dToJson(outValue[i], points[i]);
        }
    }
    static void DPoint3dVectorFromJson(bvector<DPoint3d>& points, BeJsConst inValue) {
        if (!inValue.isArray()) {
            BeAssert(false && "must be array");
            return;
        }

        for (uint32_t i = 0; i < inValue.size(); ++i) {
            DPoint3d pt;
            DPoint3dFromJson(pt, inValue[i]);
            points.push_back(pt);
        }
    }
};
POP_MSVC_IGNORE

END_BENTLEY_GEOMETRY_NAMESPACE
