/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               11/2018
//=======================================================================================
struct IArcElementKeyPointContainer : IArcKeyPointContainer
    {
    private:
        template<typename T> bool TryGetKeyPoint(T fn) const;

    protected:
        virtual ~IArcElementKeyPointContainer() {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _TryGetStartKeyPoint(DPoint3dR) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _TryGetCenterKeyPoint(DPoint3dR) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _TryGetMidKeyPoint(DPoint3dR) const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _TryGetEndKeyPoint(DPoint3dR) const override;

        //! This cannot return a reference to itself.
        virtual IArcKeyPointContainer const& _GetIArcKeyPointContainer() const = 0;
    };

END_BUILDING_SHARED_NAMESPACE