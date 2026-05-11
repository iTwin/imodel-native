/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <cmath>
#include <ECDb/IECSqlValue.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Static helper functions shared between ECSqlField implementations and ChangesetValue
//! implementations that both implement IECSqlValue.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct IECSqlValueHelper final
    {
    //! Deserializes a geometry from a FlatBuffer blob.
    //! Returns nullptr if @p blob is null or the data is not in FlatBuffer format.
    static IGeometryPtr GeometryFromBlob(void const* blob, int blobSize);

    //! Returns the sub-value of a navigation property by member name
    //! (@c "Id" or @c "RelECClassId"). Returns NoopECSqlValue for null pointers or
    //! unknown member names (and logs an error in that case).
    static IECSqlValue const& GetNavMemberValue(Utf8CP memberName,
                                                IECSqlValue const* id,
                                                IECSqlValue const* relClassId);

    //! Returns the current navigation property sub-value for a given iterator state index.
    //! @p stateIndex: 1 = Id, 2 = RelECClassId; anything else returns NoopECSqlValue.
    static IECSqlValue const& GetNavIterCurrentByStateIndex(uint8_t stateIndex,
                                                            IECSqlValue const* id,
                                                            IECSqlValue const* relClassId);

    //! Returns true when a double coordinate value is null (NaN or infinite).
    static bool IsNullCoord(double d) { return std::isinf(d) || std::isnan(d); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
