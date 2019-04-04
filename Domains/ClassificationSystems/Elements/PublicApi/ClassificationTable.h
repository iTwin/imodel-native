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
        void SetClassificationSystemId(Dgn::DgnElementId systemId);

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationTable, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //! Creates and inserts a ClassificationTable
        //! @param[in]  system   Classification System that will be set as Table's parent
        //! @param[in]  name     Classification Table's name
        //! @return     a ptr to created ClassificationTable
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationTablePtr Create(ClassificationSystemCR system, Utf8CP name);

        //!Returns this Classification Table Name property
        //! @return Name property of the Classification Table
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }

        //!Returns id of the Classification System that has this Classification Table
        //! @return Id of Classification System that has this Classification Table
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetClassificationSystemId() const;

        //!Returns an iterator that will go through every Classification assigned to this Table
        //! @return Iterator of Classifications
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::ElementIterator MakeClassificationIterator() const;

        //!Returns an iterator that will go through every Classification Group assigned to this Table
        //! @return Iterator of Classification Groups
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::ElementIterator MakeClassificationGroupIterator() const;

        //! Assigns classification data to the provided elementData parameter
        //! @param[out]  elementData   container for the classification data
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void GetClassificationDataVerbose(Json::Value& elementData) const;
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE