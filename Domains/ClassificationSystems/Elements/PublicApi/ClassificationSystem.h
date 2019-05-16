/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>
#include <BuildingShared/interfaces.h>
#include <ClassificationSystems/Elements/ForwardDeclarations.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
//! A Classification System element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ClassificationSystem final : Dgn::DefinitionElement, BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::IBCSSerializable
    {
    DGNELEMENT_DECLARE_MEMBERS(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem, Dgn::DefinitionElement);
        
    protected:
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params) : T_Super(params) {}
        explicit CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationSystem(CreateParams const& params, Utf8StringCR name, Utf8StringCR edition);
        friend struct ClassificationSystemHandler;
        friend struct ClassificationSystemsDomain;

        virtual void _SerializeProperties(Json::Value& elementData) const override;
        virtual void _FormatSerializedProperties(Json::Value& elementData) const override;

        BE_PROP_NAME(Source);
        BE_PROP_NAME(Edition);
        BE_PROP_NAME(Location);
    public:
        DECLARE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem, CLASSIFICATIONSYSTEMSELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a ClassificationSystem
        //! @param[in]  db          db to insert class definition in
        //! @param[in]  name        name of the Classification System
        //! @param[in]  edition     edition of the Classification System
        //! @return     a ptr to created ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemPtr Create(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition);

        //! Tries to get a ClassificationSystem from the database using provided name
        //! @param[in]  db       db that containes ClassificationSystem
        //! @param[in]  model    model which should contain the classificationsystem
        //! @param[in]  name     Classification System's name
        //! @param[in]  edition  Classification System's edition
        //! @return     a ptr to ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemCPtr TryGet(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition);

        //! Tries to get a ClassificationSystem from the database using provided name, if it does not exist, creates a new one.
        //! @param[in]  db       db that containes ClassificationSystem
        //! @param[in]  name     Classification System's name
        //! @param[in]  edition  Classification System's edition
        //! @return     a ptr to the ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static ClassificationSystemCPtr GetOrCreateSystemByName(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition);

        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static Dgn::ElementIterator MakeIterator(Dgn::DgnDbR dgnDbR);

        //---------------------------------------------------------------------------------------
        // Getters and setters
        //---------------------------------------------------------------------------------------
        //! Gets the name of this ClassificationSystem
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8CP GetName() const;

        //! Gets ClassificationSystem code generated from given parameters
        //! @param[in]  db       db that contains code specs
        //! @param[in]  model    model which should contain the classificationsystem
        //! @param[in]  name     name of the ClassificationSystem that will be used for code generation
        //! @param[in]  edition  edition of the ClassificationSystem that will be used for code generation
        //! @return     generated code
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT static Dgn::DgnCode GetSystemCode(Dgn::DgnDbR db, Dgn::DgnModelCR model, Utf8StringCR name, Utf8StringCR edition);

        //! Gets source of this classification
        //! @return  source of this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8String GetSource() const;

        //! Sets source of this classification
        //! @param[in]  source   new source for this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void SetSource(Utf8StringCR source);

        //! Gets edition of this classification
        //! @return  edition of this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8String GetEdition() const;

        //! Sets edition of this classification
        //! @param[in]  edition   new edition for this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void SetEdition(Utf8StringCR edition);

        //! Gets location of this classification
        //! @return  location of this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT Utf8String GetLocation() const;

        //! Sets location of this classification
        //! @param[in]  location   new location for this classification
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void SetLocation(Utf8StringCR location);
    };
    
END_CLASSIFICATIONSYSTEMS_NAMESPACE
