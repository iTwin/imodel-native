/*--------------------------------------------------------------------------------------+
|
|     $Source: ILinearElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <LinearReferencingInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocated::ILinearlyLocated(ILinearElementCR linearElement):
    m_linearElementId(linearElement.ToElement().GetElementId())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearElementCP ILinearlyLocated::GetLinearElement() const
    {
    return dynamic_cast<ILinearElementCP>(ToElement().GetDgnDb().Elements().GetElement(GetLinearElementId()).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearlyReferencedLocationId> ILinearlyLocated::QueryLinearlyReferencedLocationIds() const
    {
    bvector<LinearlyReferencedLocationId> retVal;

    auto stmtPtr = ToElement().GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedLocation) " WHERE ElementId = ? ORDER BY ECInstanceId;");

    stmtPtr->BindId(1, ToElement().GetElementId());

    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<LinearlyReferencedLocationId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationCP ILinearlyLocated::GetLinearlyReferencedLocation(LinearlyReferencedLocationId id) const
    {
    return const_cast<ILinearlyLocatedP>(this)->GetLinearlyReferencedLocationP(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationP ILinearlyLocated::GetLinearlyReferencedLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedLocation>(*ToElementP(), *QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::AddLinearlyReferencedLocation(LinearlyReferencedLocationR location)
    {
    DgnElement::MultiAspect::AddAspect(*ToElementP(), location);
    }
