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
#include <ClassificationSystems/Elements/ForwardDeclarations.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification Table element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationTable : Dgn::DefinitionElement
    {
        DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationTable, Dgn::DefinitionElement);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationTable(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationTable(CreateParams const& params, Dgn::DgnElementId systemId, Utf8CP name);

        friend struct ClassificationTableHandler;

        //! Sets Classification System Id
        //! @param[in] systemId to set
        void SetSystemId(Dgn::DgnElementId systemId);

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        ////! Creates a ClassificationTable
        ////! @param[in]  db  db to insert class definition in
        ////! @return  a ptr to created ClassificationTable
        //CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationTablePtr Create(Dgn::DgnDbR db);

        //! Creates and inserts a ClassificationTable
        //! @param[in]  system      db to insert ClassificationTable in
        //! @return     a ptr to created ClassificationTable
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationTablePtr Create(ClassificationSystemCR system, Utf8CP name);

        //!Returns this Classification Table Name property
        //! @return Name property of the Classification Table
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }

        //!Returns id of the System that has this Table
        //! @return Id of Classification System that has this Table
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetSystemId() const;
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE