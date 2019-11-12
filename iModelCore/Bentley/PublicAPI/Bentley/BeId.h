/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include "BeStringUtilities.h"
#include "BeAssert.h"
#include <limits>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// Base class for 64 bit Ids.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct BeInt64Id
    {
protected:
    uint64_t m_id;

public:
    //! @see BeInt64Id::ToString(Utf8Char*)
    static const size_t ID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

    //! Construct an invalid BeInt64Id
    BeInt64Id() {Invalidate();}

    //! Construct a BeInt64Id from a 64 bit value.
    explicit BeInt64Id(uint64_t u) : m_id(u) {}

    //! Move constructor.
    BeInt64Id(BeInt64Id&& rhs) {m_id = rhs.m_id;}

    //! Copy constructor.
    BeInt64Id(BeInt64Id const& rhs) {m_id = rhs.m_id;}

    BeInt64Id& operator=(BeInt64Id const& rhs) {m_id = rhs.m_id; return *this;}

    //! determine whether this BeInt64Id is valid (non-zero)
    bool IsValid() const {return Validate();}

    //! Compare two BeInt64Id for equality
    bool operator==(BeInt64Id const& rhs) const {return rhs.m_id == m_id;}

    //! Compare two BeInt64Id for inequality
    bool operator!=(BeInt64Id const& rhs) const {return !(*this == rhs);}

    //! Compare two BeInt64Id
    bool operator<(BeInt64Id const& rhs) const {return m_id < rhs.m_id;}
    bool operator<=(BeInt64Id const& rhs) const {return m_id <= rhs.m_id;}
    bool operator>(BeInt64Id const& rhs) const {return m_id > rhs.m_id;}
    bool operator>=(BeInt64Id const& rhs) const {return m_id >= rhs.m_id;}

    //! Get the 64 bit value of this BeInt64Id
    uint64_t GetValue() const {BeAssert(IsValid()); return m_id;}

    //! Get the 64 bit value of this BeGuid. Does not check for valid value in debug builds.
    uint64_t GetValueUnchecked() const {return m_id;}

    //! Test to see whether this BeInt64Id is valid. 0 is not a valid id.
    bool Validate() const {return m_id != 0;}

    //! Set this BeInt64Id to an invalid value (0).
    void Invalidate() {m_id = 0;}

    enum class UseHex {No=0, Yes=1};
    //! Converts this BeInt64Id to its string representation.
    //! 
    //! Typical example:
    //!
    //!     Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    //!     myId.ToString(idStrBuffer, BeInt64Id::UseHex::Yes);
    //!
    //! @remarks The method does not have any checks that the buffer is large enough. Callers
    //! must ensure this to avoid unexpected behavior. 
    //!
    //! @param[in,out] stringBuffer The output buffer for the id string. Must be large enough
    //! to hold the maximal number of decimal digits of UInt64 plus the trailing 0 character.
    //! You can use BeInt64Id::ID_STRINGBUFFER_LENGTH to allocate the @p stringBuffer.
    //! @param[in] useHex if UseHex::Yes, output will be in hex with leading "0x".
    void ToString(Utf8P stringBuffer, UseHex useHex=UseHex::No) const 
        {
        if (!IsValid())
            {
            stringBuffer[0] = '0';
            stringBuffer[1] = 0;
            return;
            }

        if (useHex == UseHex::Yes)
            BeStringUtilities::FormatUInt64(stringBuffer, ID_STRINGBUFFER_LENGTH, m_id, (HexFormatOptions) ((int)HexFormatOptions::IncludePrefix));
        else
            BeStringUtilities::FormatUInt64(stringBuffer, m_id);
        }

    //! Converts this BeInt64Id to a string representation.
    //! @param[in] useHex set to Yes to create a hexadecimal string with leading "0x"
    //! @remarks Consider the overload BeInt64Id::ToString(Utf8Char*) if you want to avoid allocating Utf8Strings.
    Utf8String ToString(UseHex useHex=UseHex::No) const
        {
        Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
        ToString(idStrBuffer, useHex);
        return Utf8String(idStrBuffer); 
        }

    Utf8String ToHexStr() const {return ToString(UseHex::Yes);}

    //! Parse a string into a BeInt64Id
    //! @remarks To parse correctly, the id string must contain an unsigned number in decimal or hexadecimal format.
    //! @param[in] idString String to parse
    //! @param[in] status optional status to distinguish between illegal input and invalid value
    static BeInt64Id FromString(Utf8CP idString, BentleyStatus* status=nullptr) {return BeInt64Id(BeStringUtilities::ParseUInt64(idString, status));}

    static BentleyStatus FromString(BeInt64Id& id, Utf8CP idString) //!< @private for backwards compatibility only
        {
        id = FromString(idString);
        return id.IsValid() ? SUCCESS : ERROR;
        }

    // Returns true if the input string representation of a BeInt64Id is "well-formed" as described in bentleyjs-core typescript package.
    BENTLEYDLL_EXPORT static bool IsWellFormedString(Utf8StringCR idString);
    };

#define BEINT64_ID_DECLARE_MEMBERS(classname,superclass) \
    classname() {Invalidate();}\
    explicit classname(uint64_t v) : superclass(v) {} \
    classname(classname&& rhs) : superclass(std::move(rhs)) {} \
    classname(classname const& rhs) : superclass(rhs) {} \
    classname& operator=(classname const& rhs) {superclass::operator=(rhs); return *this;} \
    private: explicit classname(int32_t v) : superclass() {} /* private to catch int vs. Id issues */

//=======================================================================================
//! Base class for 32 bit Ids.
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
template <typename Derived, uint32_t s_invalidValue> struct BeUInt32Id
    {
private:
    uint32_t m_id;

public:
    //! Construct an invalid BeUInt32Id
    BeUInt32Id() {Invalidate();}

    //! Construct a BeUInt32Id from a 32 bit value.
    explicit BeUInt32Id(uint32_t u) : m_id(u) {}

    bool IsValid() const {return Validate();}

    //! Compare two BeUInt32Id's for equality
    bool operator==(BeUInt32Id const& rhs) const {return rhs.m_id == m_id;}

    //! Compare two BeUInt32Id's for inequality
    bool operator!=(BeUInt32Id const& rhs) const {return !(*this == rhs);}

    //! Compare two BeUInt32Id's
    bool operator<(BeUInt32Id const& rhs) const {return m_id < rhs.m_id;}
    bool operator<=(BeUInt32Id const& rhs) const {return m_id <= rhs.m_id;}
    bool operator>(BeUInt32Id const& rhs) const {return m_id > rhs.m_id;}
    bool operator>=(BeUInt32Id const& rhs) const {return m_id >= rhs.m_id;}

    //! Get the 32 bit value of this BeUInt32Id
    uint32_t GetValue() const {static_cast<Derived const*>(this)->CheckValue(); return m_id;}

    //! Test to see whether this BeUInt32Id is valid.
    bool Validate() const {return m_id != s_invalidValue;}

    //! Set this BeUInt32Id to the invalid value).
    void Invalidate() {m_id = s_invalidValue;}

    //! only for internal callers that understand the semantics of invalid IDs.
    uint32_t GetValueUnchecked() const {return m_id;}
    };

END_BENTLEY_NAMESPACE

