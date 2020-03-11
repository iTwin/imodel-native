/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/CommonDefinition.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
//=======================================================================================
//! Information about changeSet.
//@bsiclass                                      Algirdas.Mikoliunas            01/2017
//=======================================================================================
enum struct ChangeSetKind
    {
        NotSpecified      = -1,
        Regular           = 0,
        Schema            = 1 << 0, // ChangeSet contains minor schema changes
        Definition        = 1 << 1,
        SpatialData       = 1 << 2,
        SheetsAndDrawings = 1 << 3,
        ViewsAndModels    = 1 << 4,
        GlobalProperties  = 1 << 5
    };


inline ChangeSetKind operator| (ChangeSetKind a, ChangeSetKind b)
    { return static_cast<ChangeSetKind>(static_cast<int>(a) | static_cast<int>(b)); }
inline ChangeSetKind& operator|= (ChangeSetKind& a, ChangeSetKind b)
    { return a = a | b; }
inline ChangeSetKind operator& (ChangeSetKind a, ChangeSetKind b)
    { return static_cast<ChangeSetKind>(static_cast<int>(a) & static_cast<int>(b)); }
inline ChangeSetKind& operator&= (ChangeSetKind& a, ChangeSetKind b)
    { return a = a & b; }

END_BENTLEY_IMODELHUB_NAMESPACE