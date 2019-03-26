/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationTable.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>

CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationTable)

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification Table element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationTable : Dgn::DefinitionElement
    {
        DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable, Dgn::DefinitionElement);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationTable(CreateParams const& params) : T_Super(params) {}
        friend struct ClassificationTableHandler;

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Creates a ClassificationTable
        //! @param[in]  db  db to insert class definition in
        //! @return  a ptr to created ClassificationTable
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationTablePtr Create(Dgn::DgnDbR db);
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE