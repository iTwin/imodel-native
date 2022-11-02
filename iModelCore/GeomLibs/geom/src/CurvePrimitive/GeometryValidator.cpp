/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "bsibasegeomPCH.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*
Base class implementations for GeometryValidator.
<ul>
<li> The base class discerns on the must blunt IEEE error -- std::isnan.
<li> (But if similarly low level tests for near-nan become relevant, perhaps they should be added)
<li> Fine grain checks such as angle range or metric geo coordinate may be implemented by domain-specific derived classes.
<li> Point and Vector ask IsValidCoordinate for their components.
</ul>
*/
GeometryValidatorPtr GeometryValidator::Create ()
    {
    return new GeometryValidator ();
    }
GeometryValidator::GeometryValidator()
    {
    }

bool GeometryValidator::IsValidDouble(double x)
    {
    if (isnan(x))
        return false;
    return true;
    }

bool GeometryValidator::IsValidDouble(double x, double minValue, double maxValue)
    {
    return !isnan(x) && x >= minValue && x <= maxValue;
    }

bool GeometryValidator::IsValidRadians(double x) {return IsValidDouble (x); }
bool GeometryValidator::IsValidSweepRadians(double x) { return IsValidDouble(x); }
bool GeometryValidator::IsValidDegrees(double x) { return IsValidDouble(x); }
bool GeometryValidator::IsValidSweepDegrees(double x) { return IsValidDouble(x); }
bool GeometryValidator::IsValidMetricGeoCoordinate(double x) { return IsValidDouble(x); }
bool GeometryValidator::IsValidCoordinate(double x) { return IsValidDouble(x); }

bool GeometryValidator::IsValidGeometry(DSegment3dCR segment)
    {
    return IsValidGeometry(segment.point[0]) && IsValidGeometry(segment.point[1]);
    }
bool GeometryValidator::IsValidGeometry(DPoint3dCR point)
    {
    return IsValidCoordinate(point.x) && IsValidCoordinate(point.y) && IsValidCoordinate(point.z);
    }
bool GeometryValidator::IsValidGeometry(DPoint2dCR point)
    {
    return IsValidCoordinate(point.x) && IsValidCoordinate(point.y);
    }
bool GeometryValidator::IsValidGeometry(DVec3dCR vector)
    {
    return IsValidCoordinate(vector.x) && IsValidCoordinate(vector.y) && IsValidCoordinate(vector.z);
    }
bool GeometryValidator::IsValidGeometry(DEllipse3dCR arc)
    {
    return IsValidGeometry(arc.center) && IsValidGeometry(arc.vector0) && IsValidGeometry(arc.vector90) && IsValidRadians (arc.start) && IsValidSweepRadians (arc.sweep);
    }
bool GeometryValidator::IsValidGeometry(RotMatrixCR data)
    {
    return IsValidGeometry(&data.form3d[0][0], 9);
    }
bool GeometryValidator::IsValidGeometry(TransformCR data)
    {
    return IsValidGeometry(&data.form3d[0][0], 12);
    }
bool GeometryValidator::IsValidGeometry(double const *data, uint32_t count)
    {
    if (data == nullptr)
        return false;
    for (uint32_t i = 0; i < count; i++)
        if (!IsValidDouble (data[i]))
            return false;
    return true;
    }
bool GeometryValidator::IsValidGeometry(double const *data, uint32_t count, double minValue, double maxValue)
    {
    if (data == nullptr)
        return false;
    for (uint32_t i = 0; i < count; i++)
        if (!IsValidDouble(data[i], minValue, maxValue))
            return false;
    return true;
    }
bool GeometryValidator::IsValidGeometry(bvector<DPoint3d> const &data)
    {
    for (auto &xyz : data)
        if (!IsValidGeometry(xyz))
            return false;
    return true;
    }

bool GeometryValidator::IsValidGeometry(DPoint3d const *data, size_t count)
    {
    if (data == nullptr)
        return false;
    for (size_t i = 0; i < count; i++)
        if (!IsValidGeometry(data[i]))
            return false;
    return true;
    }

bool GeometryValidator::IsValidGeometry(GeometryValidatorPtr &validator, IGeometryCR geometry)
    {
    if (!validator.IsValid ())
        return true;
    switch (geometry.GetGeometryType())
        {
        case IGeometry::GeometryType::CurvePrimitive:
        {
        return validator->IsValidGeometry(geometry.GetAsICurvePrimitive());
        }

        case IGeometry::GeometryType::CurveVector:
        {
        return validator->IsValidGeometry(geometry.GetAsCurveVector());
        }

        case IGeometry::GeometryType::SolidPrimitive:
        {
        return validator->IsValidGeometry(geometry.GetAsISolidPrimitive());
        }

        case IGeometry::GeometryType::BsplineSurface:
        {
        return validator->IsValidGeometry(geometry.GetAsMSBsplineSurface());
        }

        case IGeometry::GeometryType::Polyface:
        {
        return validator->IsValidGeometry(geometry.GetAsPolyfaceHeader());
        }

        default:
            return false;
        }
    }
bool GeometryValidator::IsValidGeometry (IGeometryPtr const &geometry)
    {
    GeometryValidatorPtr validator (this);
    return geometry.IsValid () && GeometryValidator::IsValidGeometry (validator, *geometry);
    }
bool GeometryValidator::IsValidGeometry(ICurvePrimitivePtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator (this);
    return geometry->IsValidGeometry(validator);
    }
bool GeometryValidator::IsValidGeometry(CurveVectorPtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator(this);
    return geometry->IsValidGeometry(validator);
    }
bool GeometryValidator::IsValidGeometry(ISolidPrimitivePtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator(this);
    return geometry->IsValidGeometry(validator);
    }
bool GeometryValidator::IsValidGeometry(MSBsplineSurfacePtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator(this);
    return geometry->IsValidGeometry(validator);
    }
bool GeometryValidator::IsValidGeometry(MSBsplineCurvePtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator(this);
    return geometry->IsValidGeometry(validator);
    }
bool GeometryValidator::IsValidGeometry(PolyfaceHeaderPtr const &geometry)
    {
    if (!geometry.IsValid())
        return false;
    GeometryValidatorPtr validator(this);
    return geometry->IsValidGeometry(validator);
    }
// 

void GeometryValidator::RemoveInvalidGeometry(
    GeometryValidatorPtr &validator,
    bvector< IGeometryPtr> &inout,
    bvector<IGeometryPtr> *invalidGeometry)
    {
    size_t numAccepted = 0;
    if (invalidGeometry)
        invalidGeometry->clear();
    if (!validator.IsValid ())
        return;

    for (size_t i = 0; i < inout.size(); i++)
        {
        if (validator->IsValidGeometry(inout[i]))
            {
            inout[numAccepted++] = inout[i];
            }
        else
            {
            if (invalidGeometry)
                invalidGeometry->push_back(inout[i]);
            }
        }
    if (numAccepted < inout.size())
        inout.resize(numAccepted);
    }

bool ICurvePrimitive::IsValidGeometry(GeometryValidatorPtr &validator) const
    {
    return _IsValidGeometry(validator);
    }
bool IGeometry::IsValidGeometry(GeometryValidatorPtr &validator)
    {
    return GeometryValidator::IsValidGeometry (validator, *this);
    }
bool CurveVector::IsValidGeometry(GeometryValidatorPtr &validator) const
    {
    if (!validator.IsValid())
        return true;
    for (size_t i = 0; i < this->size (); i++)
        {
        if (!validator->IsValidGeometry (this->at(i)))
            return false;
        }
    return true;
    }
bool ISolidPrimitive::IsValidGeometry(GeometryValidatorPtr &validator) const { return _IsValidGeometry (validator);}
bool GeometryValidator::ValidateAndAppend
(
GeometryValidatorPtr &validator,
IGeometryPtr &g,
bvector<IGeometryPtr> *validGeometry,
bvector<IGeometryPtr> *invalidGeometry
)
    {
    bool isValid = validator.IsValid () ? validator->IsValidGeometry(g) : true;
    if (isValid)
        {
        if (validGeometry)
            validGeometry->push_back(g);
        return true;
        }
    else
        {
        if (invalidGeometry)
            invalidGeometry->push_back (g);
        return false;
        }
    }
bool GeometryValidator::ValidateAndAppend
(
GeometryValidatorPtr &validator,
bvector<IGeometryPtr> &g,
bvector<IGeometryPtr> *validGeometry,
bvector<IGeometryPtr> *invalidGeometry
)
    {
    bool allOK = true;
    for (auto &g1 : g)
        allOK &= ValidateAndAppend (validator, g1, validGeometry, invalidGeometry);
    return allOK;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
