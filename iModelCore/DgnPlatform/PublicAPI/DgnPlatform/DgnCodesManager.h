/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCodesManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnAuthority.h>

DGNPLATFORM_TYPEDEFS(DgnCodeState);
DGNPLATFORM_TYPEDEFS(DgnCodeInfo);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! The possible states in which an authority-issued code can exist.
//! The server is the ultimate authority on every code's state.
// @bsistruct                                                    Paul.Connelly   01/16
//=======================================================================================
struct DgnCodeState
{
    enum Type : uint8_t
    {
        Available,  //!< The Code can be reserved for use by any briefcase
        Reserved,   //!< The Code has been reserved for use by a briefcase
        Used,       //!< A revision has been committed to the server in which the Code was used by an object.
        Discarded,  //!< A revision has been committed to the server in which the Code became discarded by the object by which it was previously used.
    };
private:
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

    bool IsAvailable() const { return Available == GetState(); }
    bool IsReserved() const { return Reserved == GetState(); }
    bool IsUsed() const { return Used == GetState(); }
    bool IsDiscarded() const { return Discarded == GetState(); }

    void SetAvailable() { Init(Available); }
    void SetReserved(BeSQLite::BeBriefcaseId reservedBy) { Init(Reserved, "", reservedBy); }
    void SetUsed(Utf8StringCR revisionId) { Init(Used, revisionId); }
    void SetDiscarded(Utf8StringCR revisionId) { Init(Discarded, revisionId); }

    Utf8StringCR GetRevisionId() const { return m_revisionId; }
    BeSQLite::BeBriefcaseId GetReservedBy() const { return m_reservedBy; }
    Type GetState() const { return m_type; }
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
    explicit DgnCodeInfo(DgnCode const& code) : m_code(code) { }
    DgnCodeInfo() { }

    DgnCode const& GetCode() const { return m_code; }

    // For inclusion in DgnCodeInfoSet...only the DgnCode is significant for ordering.
    bool operator<(DgnCodeInfo const& rhs) const
        {
        return GetCode() < rhs.GetCode();
        }
};

typedef bset<DgnCodeInfo> DgnCodeInfoSet;

//=======================================================================================
//! Manages acquisition of authority-issued codes for a briefcase.
// @bsiclass                                                      Paul.Connelly   01/16
//=======================================================================================
struct IDgnCodesManager : RefCountedBase
{
    enum class ResponseOptions
    {
        None = 0, //!< No special options
        IncludeState = 1 << 0, //!< Include DgnCodeState for any codes for which the request was denied
    };

    struct Response
    {
    private:
        DgnCodeInfoSet  m_details;
        CodeStatus      m_result;
    public:
        explicit Response(CodeStatus result=CodeStatus::InvalidResponse) : m_result(result) { }

        CodeStatus GetResult() const { return m_result; }
        DgnCodeInfoSet const& GetDetails() const { return m_details; }
        DgnCodeInfoSet& GetDetails() { return m_details; }

        void Invalidate() { m_result = CodeStatus::InvalidResponse; m_details.clear(); }
    };

    struct Request : DgnCodeSet
    {
    protected:
        ResponseOptions m_options;

    public:
        explicit Request(ResponseOptions options=ResponseOptions::None) : m_options(options) { }

        ResponseOptions GetOptions() const { return m_options; }
        void SetOptions(ResponseOptions options) { m_options = options; }
    };
private:
    DgnDbR      m_dgndb;
protected:
    IDgnCodesManager(DgnDbR dgndb) : m_dgndb(dgndb) { }

    virtual Response _ReserveCodes(Request& request) = 0;
    virtual Response _ReleaseCodes(Request const& request) = 0;
    virtual CodeStatus _RelinquishCodes() = 0;
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) = 0;
    virtual CodeStatus _RefreshCodes() = 0;

    DGNPLATFORM_EXPORT virtual CodeStatus _ReserveCode(DgnCodeCR code);

    DGNPLATFORM_EXPORT IDgnCodesServerP GetCodesServer() const;
public:
    DgnDbR GetDgnDb() const { return m_dgndb; }
    Response ReserveCodes(Request& request) { return _ReserveCodes(request); }
    Response ReleaseCodes(Request const& request) { return _ReleaseCodes(request); }
    CodeStatus RelinquishCodes() { return _RelinquishCodes(); }
    CodeStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); }
    DGNPLATFORM_EXPORT CodeStatus QueryCodeState(DgnCodeStateR state, DgnCodeCR code);
    CodeStatus RefreshCode() { return _RefreshCodes(); }

    CodeStatus ReserveCode(DgnCodeCR code) { return _ReserveCode(code); }
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void BackDoor_SetEnabled(bool enable);
//__PUBLISH_SECTION_START__
};

ENUM_IS_FLAGS(IDgnCodesManager::ResponseOptions);

//=======================================================================================
//! Interface adopted by a server-like object which can coordinate authority-issued codes
//! between multiple briefcases.
//! In general, applications should interact with the IDgnCodesManager object associated
//! with a given briefcase via DgnDb::Codes(). The IDgnCodesManager will communicate with
//! the IDgnCodesServer as required.
// @bsiclass                                                      Paul.Connelly   01/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IDgnCodesServer
{
    typedef IDgnCodesManager::Request Request;
    typedef IDgnCodesManager::Response Response;
protected:
    virtual Response _ReserveCodes(Request const& request, DgnDbR db) = 0;
    virtual Response _ReleaseCodes(Request const& request, DgnDbR db) = 0;
    virtual CodeStatus _RelinquishCodes(DgnDbR db) = 0;
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) = 0;
    virtual CodeStatus _QueryCodes(DgnCodeSet& codes, DgnDbR db) = 0;
public:
    Response ReserveCodes(Request const& request, DgnDbR db) { return _ReserveCodes(request, db); }
    Response ReleaseCodes(Request const& request, DgnDbR db) { return _ReleaseCodes(request, db); }
    CodeStatus RelinquishCodes(DgnDbR db) { return _RelinquishCodes(db); }
    CodeStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); }
    DGNPLATFORM_EXPORT CodeStatus QueryCodeState(DgnCodeStateR state, DgnCodeCR code);
    CodeStatus QueryCodes(DgnCodeSet& codes, DgnDbR db) { return _QueryCodes(codes, db); }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

