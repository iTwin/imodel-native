/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationSystem.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>
#include <BuildingShared/BuildingSharedApi.h>
#include <ClassificationSystems/Elements/ForwardDeclarations.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification System element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystem final : Dgn::DefinitionElement, BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IBCSSerializable
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, Dgn::DefinitionElement);
    private:
        static Dgn::DgnCode GetSystemCode(Dgn::DgnDbR db, Utf8CP name);
    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params, Utf8CP name);
        friend struct ClassificationSystemHandler;
        friend struct ClassificationSystemsDomain;

        virtual void _SerializeProperties(Json::Value& elementData) const override;
        virtual void _FormatSerializedProperties(Json::Value& elementData) const override;

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationSystem
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the Classification System
        //! @return     a ptr to created ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemPtr Create(Dgn::DgnDbR db, Utf8CP name);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemCPtr TryGet(Dgn::DgnDbR db, Utf8CP name);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static Dgn::ElementIterator MakeIterator(Dgn::DgnDbR dgnDbR);

        //! Gets the name of this ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const;
    };
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
