/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "FormsDomainDefinitions.h"

BEGIN_BENTLEY_FORMS_NAMESPACE

struct FormsDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(FormsDomain, FORMS_DOMAIN_EXPORT)

protected:
    WCharCP _GetSchemaRelativePath() const override { return BENTLEY_FORMS_SCHEMA_PATH; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

public:
    FormsDomain();
    FORMS_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb);
    FORMS_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

END_BENTLEY_FORMS_NAMESPACE