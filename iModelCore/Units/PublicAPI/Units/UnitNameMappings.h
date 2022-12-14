/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! Comparison function that is used within various schema related data structures
//! for string comparison in STL collections.
// @bsistruct
//=======================================================================================
struct less_str
{
bool operator()(Utf8String s1, Utf8String s2) const
    {
    if (BeStringUtilities::Stricmp(s1.c_str(), s2.c_str()) < 0)
        return true;
    return false;
    }
};

//=======================================================================================
//! There are 3 different sets of names:
//! - oldNames
//!     - Came from EC2.0 and the old ECF Units implementation
//! - newNames
//!     - New unit names that were created for the first version of a new Units System in the iModel world
//! - ecName
//!     - Added when the new Unit Systems needed to allow custom extensibility through an ECSchema, all names
//!        had to become ECName compatible. Check the ECObjects documentation for more information on ECName.
// @bsistruct
//=======================================================================================
struct UnitNameMappings
{
private:
    static UnitNameMappings * s_mappings;

    bmap<Utf8String, Utf8String, less_str> m_oldNameNewNameMapping; // key: oldName value: newName
    bmap<Utf8String, Utf8String, less_str> m_newNameOldNameMapping; // key: newName value: oldName
    bmap<Utf8String, Utf8String, less_str> m_newNameECNameMapping; // key: newName value: ECName
    bmap<Utf8String, Utf8String, less_str> m_ecNameNewNameMapping; // key: ECName value: newName

    UnitNameMappings();
    void AddMapping(Utf8CP oldName, Utf8CP newName);
    void AddECMapping(Utf8CP newName, Utf8CP ecName);
    void AddNewNameToECNameMapping(Utf8CP newName, Utf8CP ecName);
    void AddNewNameToOldNameMapping(Utf8CP newName, Utf8CP oldName);
    void AddOldNameToNewNameMapping(Utf8CP oldName, Utf8CP newName);
    void AddMappings();

    static UnitNameMappings * GetMappings();

public:
    //! Returns an ECName corresponding to the given newName.
    UNITS_EXPORT static Utf8CP TryGetECNameFromOldName(Utf8CP name);
    //! Returns an ECName corresponding to the given newName.
    UNITS_EXPORT static Utf8CP TryGetECNameFromNewName(Utf8CP name);
    //! Returns the oldName corresponding to the given newName.
    UNITS_EXPORT static Utf8CP TryGetOldNameFromNewName(Utf8CP name);
    //! Returns the newName corresponding to the given oldName.
    UNITS_EXPORT static Utf8CP TryGetNewNameFromOldName(Utf8CP name);
    //! Returns the newName corresponding to the given ECName.
    UNITS_EXPORT static Utf8CP TryGetNewNameFromECName(Utf8CP name);
    //! Returns the oldName corresponding to the given ECName.
    UNITS_EXPORT static Utf8CP TryGetOldNameFromECName(Utf8CP name);
};

END_BENTLEY_UNITS_NAMESPACE
