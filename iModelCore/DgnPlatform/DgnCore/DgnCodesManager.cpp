/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCodesManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnChangeSummary.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnrestrictedCodesManager : IDgnCodesManager
{
private:
    UnrestrictedCodesManager(DgnDbR db) : IDgnCodesManager(db) { }

    virtual Response _ReserveCodes(Request const&) override { return Response(CodeStatus::Success); }
    virtual Response _ReleaseCodes(Request const&) override { return Response(CodeStatus::Success); }
    virtual CodeStatus _RelinquishCodes() override { return CodeStatus::Success; }
    virtual CodeStatus _ReserveCode(DgnCodeCR) override { return CodeStatus::Success; }
    virtual CodeStatus _QueryCodeStates(DgnCodeInfoSet& states, DgnCodeSet const& codes) override
        {
        states.clear();
        auto bcId = GetDgnDb().GetBriefcaseId();
        for (auto const& code : codes)
            states.insert(DgnCodeInfo(code)).first->SetReserved(bcId);

        return CodeStatus::Success;
        }
public:
    static IDgnCodesManagerPtr Create(DgnDbR db) { return new UnrestrictedCodesManager(db); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesManagerPtr DgnPlatformLib::Host::ServerAdmin::_CreateCodesManager(DgnDbR db) const
    {
    // ###TODO_CODES: Check IsMasterCopy()...when we have an actual server...
    return UnrestrictedCodesManager::Create(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
IDgnCodesServerP IDgnCodesManager::GetCodesServer() const
    {
    return T_HOST.GetServerAdmin()._GetCodesServer(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesManager::_ReserveCode(DgnCodeCR code)
    {
    Request req;
    req.insert(code);
    return _ReserveCodes(req).GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CodeStatus IDgnCodesManager::QueryCodeState(DgnCodeStateR state, DgnCodeCR code)
    {
    DgnCodeSet codes;
    codes.insert(code);
    DgnCodeInfoSet states;

    auto status = _QueryCodeStates(states, codes);
    if (CodeStatus::Success == status)
        {
        BeAssert(1 == states.size());
        state = *states.begin();
        }

    return status;
    }

