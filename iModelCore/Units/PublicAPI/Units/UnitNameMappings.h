/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitNameMappings.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! There are 3 different sets of names:
//! - oldNames
//!     - Came from EC2.0 and the old ECF Units implementation
//! - newNames
//!     - New unit names that were created for the first version of a new Units System in the iModel world
//! - ecName
//!     - Added when the new Unit Systems needed to allow custom extensibility through an ECSchema, all names
//!        had to become ECName compatible. Check the ECObjects documentation for more information on ECName.
// @bsistruct                                                    Caleb.Shafer      01/18
//=======================================================================================
struct UnitNameMappings
{
private:
    static UnitNameMappings * s_mappings;

    bmap<Utf8String, Utf8String> m_oldNameNewNameMapping; // key: oldName value: newName
    bmap<Utf8String, Utf8String> m_newNameOldNameMapping; // key: newName value: oldName
    bmap<Utf8String, Utf8String> m_newNameECNameMapping; // key: newName value: ECName
    bmap<Utf8String, Utf8String> m_ecNamenewNameMapping; // key: ECName value: newName

    UnitNameMappings();
    void AddMapping(Utf8CP oldName, Utf8CP newName);
    void AddECMapping(Utf8CP newName, Utf8CP ecName);
    void AddMappings();

    static UnitNameMappings * GetMappings();

public:
    //! Returns an ECName corresponding to the given newName.
    UNITS_EXPORT static Utf8CP TryGetECNameFromNewName(Utf8CP name);
    //! Returns the oldName corresponding to the given newName.
    UNITS_EXPORT static Utf8CP TryGetOldNameFromNewName(Utf8CP name);
    //! Returns the newName corresponding to the given oldName.
    UNITS_EXPORT static Utf8CP TryGetNewNameFromOldName(Utf8CP name);
    //! Returns the newName corresponding to the given ECName.
    UNITS_EXPORT static Utf8CP TryGetNewNameFromECName(Utf8CP name);
};

END_BENTLEY_UNITS_NAMESPACE
