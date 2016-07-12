//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Dimension/DimensionStyle.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatformInternal/DgnCore/Dimension.fb.h>
#include <DgnPlatform/Dimension/Dimension.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define PROP_TextStyleId "TextStyleId"

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(DimensionStyleHandler)

void DimensionStyleHandler::_TEMPORARY_GetHandlingCustomAttributes(ECSqlClassParams::HandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetHandlingCustomAttributes(params);
    params.Add(PROP_TextStyleId);
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DimensionStylePtr DimensionStyle::Create(DgnElementId textStyleId, DgnDbR dgnDb)
    {
    DimensionStylePtr  style = new DimensionStyle (dgnDb);
    style->SetTextStyleId (textStyleId);

    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
void DimensionStyle::_CopyFrom(DgnElementCR rhsElement)
    {
    T_Super::_CopyFrom(rhsElement);

    DimensionStyleCP rhs = dynamic_cast<DimensionStyleCP>(&rhsElement);
    if (nullptr == rhs)
        return;

#if defined (NEEDSWORK)
    Clear();

    m_tableHeader = rhs->m_tableHeader;

    Initialize (false);

    ... copy the data
#else
    m_textStyleId = rhs->m_textStyleId;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
void DimensionStyle::_RemapIds(DgnImportContext& context)
    {
    DgnElementId srcTextStyleId = GetTextStyleId();
#if defined (NEEDSWORK)
    DgnElementId dstTextStyleId = context.RemapTextStyleId(srcTextStyleId);
#else
    DgnElementId dstTextStyleId = srcTextStyleId;
#endif
    SetTextStyleId(dstTextStyleId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DimensionStyle::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    // Bind
    if (ECSqlStatus::Success != stmt.BindInt64(stmt.GetParameterIndex (PROP_TextStyleId), m_textStyleId.GetValue()))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DimensionStyle::_BindInsertParams(BeSQLite::EC::ECSqlStatement& insert)
    {
    DgnDbStatus status = T_Super::_BindInsertParams(insert);
    if (DgnDbStatus::Success != status)
        return status;

    return BindParams(insert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DimensionStyle::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& update)
    {
    DgnDbStatus status = T_Super::_BindUpdateParams(update);
    if (DgnDbStatus::Success != status)
        return status;

    return BindParams(update);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DimensionStyle::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& select, ECSqlClassParamsCR selectParams)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(select, selectParams);
    if (DgnDbStatus::Success != status)
        return status;

    m_textStyleId = DgnElementId (select.GetValueUInt64(selectParams.GetSelectIndex(PROP_TextStyleId)));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   03/2016
//---------------------------------------------------------------------------------------
DgnElementId DimensionStyle::GetTextStyleId () const                { return m_textStyleId;}
void DimensionStyle::SetTextStyleId (DgnElementId textStyleId)      { m_textStyleId = textStyleId;}

END_BENTLEY_DGNPLATFORM_NAMESPACE
