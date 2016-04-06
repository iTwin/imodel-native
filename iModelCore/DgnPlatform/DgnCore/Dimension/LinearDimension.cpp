//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Dimension/LinearDimension.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatformInternal/DgnCore/Dimension.fb.h>
#include <DgnPlatform/Dimension/Dimension.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
using namespace flatbuffers;

#define PROP_StyleId    "StyleId"
#define PROP_Points     "Points"

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(LinearDimensionHandler2d)

void LinearDimensionHandler2d::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROP_StyleId);
    params.Add(PROP_Points);
    }
};

/* ctor */ LinearDimension2d::LinearDimension2d(CreateParams const& params) : T_Super(params) { }
LinearDimension2dPtr LinearDimension2d::Create(CreateParams const& params) { return new LinearDimension2d(params); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
LinearDimension2dPtr LinearDimension2d::Create(CreateParams const& params, DgnElementId dimStyleId, DPoint3dCR endPoint)
    {
    LinearDimension2dPtr  dim = Create (params);
    dim->SetDimensionStyleId (dimStyleId);
    dim->AppendPoint (endPoint);

    return dim;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
void LinearDimension2d::_CopyFrom(DgnElementCR rhsElement)
    {
    T_Super::_CopyFrom(rhsElement);

    LinearDimensionCP rhs = dynamic_cast<LinearDimensionCP>(&rhsElement);
    if (nullptr == rhs)
        return;

#if defined (NEEDSWORK)
    Clear();

    m_tableHeader = rhs->m_tableHeader;

    Initialize (false);

    ... copy the data
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension2d::_BindInsertParams(BeSQLite::EC::ECSqlStatement& insert)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(insert);
    if (DgnDbStatus::Success != status)
        return status;

    return LinearDimension::BindParams(insert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension2d::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& update)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(update);
    if (DgnDbStatus::Success != status)
        return status;

    return LinearDimension::BindParams(update);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension2d::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParamsCR selectParams)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(select, selectParams);
    if (DgnDbStatus::Success != status)
        return status;

    return LinearDimension::ReadSelectParams (select, selectParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension2d::_OnInsert()
    {
    DgnDbStatus status = T_Super::_OnInsert();
    if (DgnDbStatus::Success != status)
        return status;

    UpdateGeometryRepresentation();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension2d::_OnUpdate(DgnElementCR original)
    {
    DgnDbStatus status = T_Super::_OnUpdate(original);
    if (DgnDbStatus::Success != status)
        return status;

    UpdateGeometryRepresentation();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnElementId    LinearDimension::GetDimensionStyleId () const           { return m_dimStyleId; }
void            LinearDimension::SetDimensionStyleId (DgnElementId id)  { m_dimStyleId = id; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
uint32_t      LinearDimension::GetPointCount() const
    {
    return (uint32_t) m_points.size() + 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DPoint2d      LinearDimension::GetPointLocal(uint32_t index) const
    {
    if (0 == index)
        return DPoint2d::FromZero();

    return m_points[index - 1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
BentleyStatus   LinearDimension::AppendPointLocal(DPoint2dCR point)
    {
    m_points.push_back (point); return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DPoint3d      LinearDimension::GetPoint(uint32_t index) const
    {
    DPoint3d    world;
    DPoint2d    local = GetPointLocal (index);
    Transform   toWorld = ToElement().ToGeometrySource()->GetPlacementTransform();

    toWorld.Multiply (&world, &local, 1);

    return world;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinearDimension::AppendPoint(DPoint3dCR world)
    {
    DPoint3d    local;
    Transform   toWorld = ToElement().ToGeometrySource()->GetPlacementTransform();

    toWorld.MultiplyTranspose (&local, &world, 1);

    return AppendPointLocal (DPoint2d::From (local));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    // Bind DimStyleId
    if (ECSqlStatus::Success != stmt.BindInt64(stmt.GetParameterIndex (PROP_StyleId), m_dimStyleId.GetValue()))
        return DgnDbStatus::BadArg;

    // Serilize points to a flat buff
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint2d*) &m_points.front(), m_points.size());

    FB::DimensionPointsBuilder builder(fbb);

    builder.add_coords(coords);

    auto mloc = builder.Finish();
    fbb.Finish(mloc);

    // Bind points
    if (ECSqlStatus::Success != stmt.BindBinary(stmt.GetParameterIndex (PROP_Points), fbb.GetBufferPointer(), fbb.GetSize(), IECSqlBinder::MakeCopy::Yes))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus LinearDimension::ReadSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParamsCR selectParams)
    {
    m_dimStyleId = DgnElementId (select.GetValueUInt64(selectParams.GetSelectIndex(PROP_StyleId)));

    // Read points and deserialize from flatbuf
    int dataSize = 0;
    ByteCP data = static_cast<ByteCP>(select.GetValueBinary(selectParams.GetSelectIndex(PROP_Points), &dataSize));

    auto fbPoints = GetRoot<FB::DimensionPoints>(data);
    FB::DPoint2dVector const* coords   = fbPoints->coords();

    for (auto const& point : *coords)
        {
        AppendPointLocal (DPoint2d::From (point.x(), point.y()));
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
void LinearDimension::UpdateGeometryRepresentation()
    {
    GeometricElement&           elem = ToElementR();
    GeometrySourceP             geomSource = elem.ToGeometrySourceP();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomSource);
    LinearDimensionStroker stroker (*this, *builder);
    stroker.AppendDimensionGeometry();

    builder->SetGeometryStreamAndPlacement(*geomSource);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
