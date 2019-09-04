/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Client/iModelBaseInfo.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas             08/2019
//---------------------------------------------------------------------------------------
StatusResult iModelBaseInfo::Validate() const
    {
    if (m_name.empty())
        return StatusResult::Error(Error::Id::InvalidiModelName);

    StatusResult extentResult = ValidateExtent();
    if (!extentResult.IsSuccess())
        return extentResult;

    return StatusResult::Success();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas             08/2019
//---------------------------------------------------------------------------------------
StatusResult iModelBaseInfo::ValidateExtent() const
    {
    if (m_extent.empty())
        return StatusResult::Success();

    if (m_extent.size() != s_extentSize)
        return StatusResult::Error(Error::Id::InvalidiModelExtentCount);

    for (int i = 0; i < s_extentSize; i++)
        {
        int limit;
        if (i % 2 == 0)
            limit = s_latitudeLimit;            
        else
            limit = s_longitudeLimit;

        StatusResult coordinateResult = ValidateCoordinate(m_extent[i], limit);
        if (!coordinateResult.IsSuccess())
            return coordinateResult;
        }

    return StatusResult::Success();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Vilius.Kazlauskas             08/2019
//---------------------------------------------------------------------------------------
StatusResult iModelBaseInfo::ValidateCoordinate(double coordinate, int limit) const
    {
    if (coordinate < -limit || coordinate > limit)
        return StatusResult::Error(Error::Id::InvalidiModelExtentCoordinate);

    return StatusResult::Success();
    }
