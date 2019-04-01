/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/ClassificationGroup.h $
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
//! A ClassificationSystem Class Definition Group element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationGroup final : Dgn::GroupInformationElement
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationGroup, Dgn::GroupInformationElement);

    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationGroup(CreateParams const& params, Utf8CP name);
        friend struct ClassificationGroupHandler;
        friend struct ClassificationSystemsDomain;

    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationGroup, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)
        
        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationGroup
        //! @param[in]  db          db to insert class definition group in
        //! @param[in]  name        name of the ClassificationSystem class definition group
        //! @return     a ptr to created ClassificationGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationGroupPtr Create(ClassificationSystemCR system, Utf8CP name);

        //!Returns Id property of the parent ClassificationSystem
        //! @return Id property of the parent ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Dgn::DgnElementId GetClassificationSystemId() const;

        //! Gets the name of this ClassificationGroup
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const { return GetUserLabel(); }
    };
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
