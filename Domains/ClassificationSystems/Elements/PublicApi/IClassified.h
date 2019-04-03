/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/IClassified.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BuildingShared/BuildingSharedApi.h>
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>
#include <ClassificationSystems/Elements/ForwardDeclarations.h>

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               06/2018
//=======================================================================================
struct IClassified
    {
    private:
        static ECN::ECRelationshipClassCR GetRelClassIClassifiedIsClassifiedAs(Dgn::DgnDbR);

    protected:
        IClassified() {}
        virtual ~IClassified() {}

        virtual Dgn::DgnElementCR _GetAsDgnElement() const = 0;

    public:
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::ElementIdIterator MakeClassificationsIterator() const;
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT ClassificationCPtr GetClassification(ClassificationSystemCR system) const;
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT bvector<ClassificationCPtr> GetClassifications(ClassificationSystemCR system) const;
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void AddClassification(ClassificationCR classification);
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT void RemoveClassification(ClassificationCR classification);
        CLASSIFICATIONSYSTEMSELEMENTS_EXPORT bool IsClassifiedAs(ClassificationCR classification) const;
    };

END_CLASSIFICATIONSYSTEMS_NAMESPACE