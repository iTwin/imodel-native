/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/PickContext.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include  <DgnPlatform/SimplifyGraphic.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum class TestLStylePhase
{
    None        = 0,
    Component   = 1,
    BaseGeom    = 2
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class TestHitStatus
{
    NotOn       = 0,
    IsOn        = 1,
    Aborted     = 2
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/10
//=======================================================================================
struct StopLocateTest
{
    virtual ~StopLocateTest() {}
    virtual bool _CheckStopLocate() = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/14
+===============+===============+===============+===============+===============+======*/
struct LocateOptions
{
private:
    bool m_disableDgnDbFilter = false;
    bool m_allowTransients = false;
    uint32_t m_maxHits;
    HitSource m_hitSource;

public:
    LocateOptions()
        {
        m_maxHits = 20;
        m_hitSource = HitSource::DataPoint;
        }

    LocateOptions(HitSource hitSource, uint32_t maxHits)
        {
        m_hitSource = hitSource;
        m_maxHits = maxHits;
        }

    void SetDisableDgnDbFilter(bool disableDgnDbFilter) {m_disableDgnDbFilter = disableDgnDbFilter;}
    void SetAllowTransients(bool allowTransients) {m_allowTransients = allowTransients;}
    void SetMaxHits(uint32_t maxHits) {m_maxHits = maxHits;}
    void SetHitSource(HitSource hitSource) {m_hitSource = hitSource;}

    bool GetDisableDgnDbFilter() const {return m_disableDgnDbFilter;}
    bool GetAllowTransients() const {return m_allowTransients;}
    uint32_t GetMaxHits() const {return m_maxHits;}
    HitSource GetHitSource() const {return m_hitSource;}
};

END_BENTLEY_DGN_NAMESPACE
                                                                                                                                                                      
