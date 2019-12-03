/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../C3dImporter.h"

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct C3dBridge : public DwgBridge
{
    DEFINE_T_SUPER (DwgBridge)

private:
    std::unique_ptr<C3dImporter>    m_importer;

public:
    C3dBridge () : T_Super() {}
    EXPORT_ATTRIBUTE DwgImporterP _CreateDwgImporter () override;
    EXPORT_ATTRIBUTE BentleyStatus _Initialize (int argc, WCharCP argv[]) override;
    EXPORT_ATTRIBUTE Dgn::SubjectCPtr _InitializeJob () override;
    EXPORT_ATTRIBUTE Dgn::SubjectCPtr _FindJob () override;
    EXPORT_ATTRIBUTE void _SetClientInfo () override;
};  // C3dBridge

END_C3D_NAMESPACE
