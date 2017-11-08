/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/PublicAPI/CS06Bridge/CS06Bridge.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "CS06BridgeApi.h"

BEGIN_CS06BRIDGE_NAMESPACE

//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct CS06Bridge : Dgn::iModelBridgeWithSyncInfoBase
{
	DEFINE_T_SUPER(Dgn::iModelBridgeWithSyncInfoBase)

protected:
	void UpdateProjectExtents(Dgn::SpatialModelR);
	Utf8String ComputeJobSubjectName();

	Dgn::SubjectCPtr CreateAndInsertJobSubject(Dgn::DgnDbR db, Utf8CP jobName);
	Dgn::SubjectCPtr QueryJobSubject(Dgn::DgnDbR db, Utf8CP jobName);

public:
	virtual Dgn::iModelBridge::CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]) override;
    virtual WString _SupplySqlangRelPath() override;
	virtual BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
	virtual BentleyStatus _OpenSource() override;
    virtual void _CloseSource(BentleyStatus) override;
	virtual BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
	virtual Dgn::SubjectCPtr _InitializeJob() override;
	virtual Dgn::SubjectCPtr _FindJob() override;
	virtual void _OnDocumentDeleted(Utf8StringCR docId, Dgn::iModelBridgeSyncInfoFile::ROWID docSyncInfoid) override;

    // TODO: Do I need to register this key with someone?
    static WCharCP GetRegistrySubKey() { return L"OpenRoads ConceptStation Bridge"; }

    CS06Bridge() = default;
    virtual ~CS06Bridge() = default;
};

extern "C"
{
	CS06BRIDGE_EXPORT Dgn::iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName);
	CS06BRIDGE_EXPORT void iModelBridge_getAffinity(Dgn::iModelBridge::BridgeAffinity& bridgeAffinity,
		BeFileName const& affinityLibraryPath, BeFileName const& sourceFileName);
};

END_CS06BRIDGE_NAMESPACE
