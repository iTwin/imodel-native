/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

#define PHYSICAL_MATERIAL_DOMAIN_ECSCHEMA_PATH  L"ECSchemas/Domain/PhysicalMaterial.ecschema.xml"
#define PHYSICAL_MATERIAL_DOMAIN_NAME           "PhysicalMaterial"
#define PHYSICAL_MATERIAL_SCHEMA(className)     PHYSICAL_MATERIAL_DOMAIN_NAME "." className

#define PHYSICAL_MATERIAL_CLASS_Aluminum        "Aluminum"
#define PHYSICAL_MATERIAL_CLASS_Concrete        "Concrete"
#define PHYSICAL_MATERIAL_CLASS_Steel           "Steel"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    06/2017
//=======================================================================================
struct PhysicalMaterialDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(PhysicalMaterialDomain, DGNPLATFORM_EXPORT)

private:
    WCharCP _GetSchemaRelativePath() const override {return PHYSICAL_MATERIAL_DOMAIN_ECSCHEMA_PATH;}

public:
    PhysicalMaterialDomain() : DgnDomain(PHYSICAL_MATERIAL_DOMAIN_NAME, "Physical Material Domain", 1) {}
    ~PhysicalMaterialDomain() {}
};

END_BENTLEY_DGN_NAMESPACE
