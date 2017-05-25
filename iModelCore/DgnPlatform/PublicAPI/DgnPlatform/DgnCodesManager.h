/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCodesManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/CodeSpec.h>

DGNPLATFORM_TYPEDEFS(DgnCodeState);
DGNPLATFORM_TYPEDEFS(DgnCodeInfo);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! The possible states in which a code can exist.
//! The server is the ultimate authority on every code's state.
//!
//! A code can be in exactly one of four possible states:
//!     - Available: Any briefcase can reserve the code.
//!     - Reserved: a briefcase has reserved the code for use. No other briefcase may reserve or use it.
//!     - Used: A revision has been committed in which the code was assigned to an object. No briefcase may reserve or use it.
//!     - Discarded: A revision has been committed in which a previously-used code became disused.
//!       Any briefcase can reserve it, provided they have pulled the revision in which it became discarded.
//!
//! Possible transitions of code state:
//!     Available => Reserved: A code which has never previously been used becomes reserved by a briefcase.
//!     Reserved => Available: A reserved code which was never used is relinquished by the briefcase which reserved it.
//!     Reserved => Used: A briefcase committed a revision in which it assigned a reserved code to an obejct.
//!     Used => Discarded: A briefcase committed a revision in which the code became disused.
//!     Discarded => Reserved: A previously-used code was reserved for use by a briefcase.
//! Once a code is used, it remains tracked for the lifetime of a repository; therefore its state never returns to Available.
//!
//! In order to assign a code to an object, a briefcase must first reserve the code. This is verified when inserting and updating coded objects into the DgnDb.
//! If this verification fails, the operation will return DgnDbStatus::CodeNotReserved.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct DgnCodeState
{
private:
    enum Type : uint8_t
    {
        Available,  //!< The Code can be reserved for use by any briefcase
        Reserved,   //!< The Code has been reserved for use by a briefcase
        Used,       //!< A revision has been committed to the server in which the Code was used by an object.
        Discarded,  //!< A revision has been committed to the server in which the Code became discarded by the object by which it was previously used.
    };

    Type                    m_type;
    Utf8String              m_revisionId;
    BeSQLite::BeBriefcaseId m_reservedBy;
protected:
    void Init(Type type, Utf8StringCR revisionId="", BeSQLite::BeBriefcaseId reservedBy=BeSQLite::BeBriefcaseId())
        {
        m_type = type;
        m_revisionId = revisionId;
        m_reservedBy = reservedBy;
        }
public:
    DgnCodeState() { SetAvailable(); } //!< Constructs a DgnCodeState for an available code

    bool IsAvailable() const { return Available == m_type; }
    bool IsReserved() const { return Reserved == m_type; }
    bool IsUsed() const { return Used == m_type; }
    bool IsDiscarded() const { return Discarded == m_type; }

    void SetAvailable() { Init(Available); }
    void SetReserved(BeSQLite::BeBriefcaseId reservedBy) { Init(Reserved, "", reservedBy); }
    void SetUsed(Utf8StringCR revisionId) { Init(Used, revisionId); }
    void SetDiscarded(Utf8StringCR revisionId) { Init(Discarded, revisionId); }

    //! Returns the revision ID in which the code became used or discarded, or else an empty string
    Utf8StringCR GetRevisionId() const { return m_revisionId; }

    //! Returns the ID of the briefcase which has reserved the code, or an invalid ID.
    BeSQLite::BeBriefcaseId GetReservedBy() const { return m_reservedBy; }

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

//=======================================================================================
//! Pairs a DgnCode with its state.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct DgnCodeInfo : DgnCodeState
{
private:
    DgnCode m_code;
public:
    explicit DgnCodeInfo(DgnCodeCR code) : m_code(code) { }
    DgnCodeInfo() { }

    DgnCodeCR GetCode() const { return m_code; } //!< The DgnCode whose state is represented by this DgnCodeInfo

    // For inclusion in DgnCodeInfoSet...only the DgnCode is significant for ordering.
    bool operator<(DgnCodeInfo const& rhs) const
        {
        return GetCode() < rhs.GetCode();
        }

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

typedef bset<DgnCodeInfo> DgnCodeInfoSet;

END_BENTLEY_DGNPLATFORM_NAMESPACE
