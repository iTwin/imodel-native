/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbSystemSchemaHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Static helper that centralises the EC-property-name → JS-name mapping used by
//! row renderers (ECSqlRowAdaptor) and changeset readers (ChangesetValueFactory).
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyNamer final {
private:
    //! Maps a system property to its canonical JS name.
    //!
    //! For the well-known system extended types (Id, ClassId, SourceId, SourceClassId,
    //! TargetId, TargetClassId, NavId, NavRelClassId) the corresponding JSON system name
    //! (e.g. "id", "className", "sourceId" …) is returned.
    //! For anything else the name is returned with its first character lowered.
    //!
    //! The ClassId mapping respects @p useClassFullName: when true it emits "classFullName"
    //! instead of "className" (SourceClassId and TargetClassId always use the short-name
    //! form irrespective of the flag, matching legacy behaviour).
    //!
    //! @param extType          Extended type resolved from the property's extended-type name.
    //! @param ecName           The EC property name (e.g. "ECClassId", "ECInstanceId", …).
    //! @param useClassFullName When true, ECClassId maps to "classFullName" instead of "className".
    //! @return                 JS name string.
    static Utf8String GetJsNameForProp(ExtendedTypeHelper::ExtendedType extType, Utf8StringCR ecName, bool useClassFullName);
public:
    ECSqlPropertyNamer() = delete;

    //! Maps a property name to its canonical JS name.
    //!
    //! For the well-known system extended types (Id, ClassId, SourceId, SourceClassId,
    //! TargetId, TargetClassId, NavId, NavRelClassId) the corresponding JSON system name
    //! (e.g. "id", "className", "sourceId" …) is returned.
    //! For anything else the name is returned with its first character lowered.
    //!
    //! The ClassId mapping respects @p useClassFullName: when true it emits "classFullName"
    //! instead of "className" (SourceClassId and TargetClassId always use the short-name
    //! form irrespective of the flag, matching legacy behaviour).
    //!
    //! @param prop          Property whose name is to be mapped; must not be null.
    //! @param useClassFullName When true, ECClassId maps to "classFullName" instead of "className".
    //! @return                 JS name string.
    static Utf8String GetJsNameForProp(ECN::ECPropertyCP prop, bool useClassFullName);

    //! Maps a struct member property name to its canonical JS name.
    //! @param prop          Property whose name is to be mapped; must not be null and its class must be a struct class.
    //! @return                 JS name string.
    static Utf8String GetJsNameForStructMemberProp(ECN::ECPropertyCP prop);

    //! Maps a sub-property leaf name to its JS equivalent.
    //!
    //! Covers the standard sub-properties that appear in path-length > 1 system
    //! property paths:
    //!   - NavPropId         → "id"
    //!   - NavPropRelECClassId → "relClassName"
    //!   - PointX / PointY / PointZ → "x" / "y" / "z"
    //!
    //! Returns the original name unchanged for anything else.
    static Utf8CP GetSubPropertyLeafJsName(Utf8StringCR leafName);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
