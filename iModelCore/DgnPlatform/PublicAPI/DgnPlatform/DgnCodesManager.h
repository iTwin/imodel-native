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

    //! Returns the revision ID in which the code became used or discarded, or else an empty string
    Utf8StringCR GetRevisionId() const { return m_revisionId; }

    //! Returns the ID of the briefcase which has reserved the code, or an invalid ID.
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

    DgnCode const& GetCode() const { return m_code; } //!< The DgnCode whose state is represented by this DgnCodeInfo

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
    //! Options specifying what additional information should be included in responses to requests
    enum class ResponseOptions
    {
        None = 0, //!< No special options
        IncludeState = 1 << 0, //!< Include DgnCodeState for any codes for which the request was denied
    };

    //! Represents a request to operate on a set of codes.
    struct Request : DgnCodeSet
    {
    protected:
        ResponseOptions m_options;
    public:
        //! Constructor
        //! @param[in]      options Specifies what additional data should be included in the response.
        explicit Request(ResponseOptions options=ResponseOptions::None) : m_options(options) { }

        ResponseOptions GetOptions() const { return m_options; } //!< Set the options specifying additional data to be included in response
        void SetOptions(ResponseOptions options) { m_options = options; } //!< Get the options specifying additional data to be included in response
    };

    //! Represents a response to a request. A response always includes a CodeStatus indicating the result. Based on options supplied in
    //! the request, may also include additional information.
    struct Response
    {
    private:
        DgnCodeInfoSet  m_details;
        CodeStatus      m_result;
    public:
        //! Construct a response with the specified result
        explicit Response(CodeStatus result=CodeStatus::InvalidResponse) : m_result(result) { }

        //!< Returns the overall result of the operation as a CodeStatus
        CodeStatus GetResult() const { return m_result; }

        //! Provides the state of each code for which the operation did not succeed, if ResponseOptions::IncludeState specified in request
        DgnCodeInfoSet const& GetDetails() const { return m_details; }
        //! Provides the state of each code for which the operation did not succeed, if ResponseOptions::IncludeState specified in request
        DgnCodeInfoSet& GetDetails() { return m_details; }

        //! Reset the response
        void Invalidate() { m_result = CodeStatus::InvalidResponse; m_details.clear(); }
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
    virtual CodeStatus _OnFinishRevision(DgnRevision const& rev) = 0;

    DGNPLATFORM_EXPORT virtual CodeStatus _ReserveCode(DgnCodeCR code);

    DGNPLATFORM_EXPORT IDgnCodesServerP GetCodesServer() const;
public:
    DgnDbR GetDgnDb() const { return m_dgndb; } //!< The DgnDb for which this object manages DgnCodes

    //! Attempts to reserve a set of codes for this briefcase.
    //! Note: the request object may be modified by this function
    Response ReserveCodes(Request& request) { return _ReserveCodes(request); }
    Response ReleaseCodes(Request const& request) { return _ReleaseCodes(request); } //!< Attempts to release a set of codes reserved by this briefcase
    CodeStatus RelinquishCodes() { return _RelinquishCodes(); } //!< Attempts to release all codes reserved by this briefcase
    CodeStatus QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) { return _QueryCodeStates(states, codes); } //!< Queries the state of a set of codes
    DGNPLATFORM_EXPORT CodeStatus QueryCodeState(DgnCodeStateR state, DgnCodeCR code); //!< Queries the state of a code
    CodeStatus RefreshCode() { return _RefreshCodes(); } //!< Updates a local cache of codes reserved by this briefcase by querying the server

    CodeStatus ReserveCode(DgnCodeCR code) { return _ReserveCode(code); } //!< Attempts to reserve a code
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void BackDoor_SetEnabled(bool enable);
    CodeStatus OnFinishRevision(DgnRevision const& rev) { return _OnFinishRevision(rev); }
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

