/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ctime>

#include <TerrainModel/TerrainModel.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  Class representing a moment in time.
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Time
    {
    static Time                 CreateActual                   ();
    static Time                 CreateSmallestPossible         ();
    static Time                 CreateGreathestPossible        ();


      

    bool                        IsSmallestPossible             () const;
    bool                        IsGreathestPossible            () const;

    bool                        operator<                      (const Time&                 time) const;

    explicit                    Time                           ();

                                ~Time                          ();

                                Time                           (const Time&                 time); 
    Time&                       operator=                      (const Time&                 time); 

/*__PUBLISH_SECTION_END__*/
private:
    typedef time_t              TimeType;                    

    friend TimeType             GetCTimeFor                    (const Time&                 time);
    friend Time                 CreateTimeFrom                 (TimeType                    time);
        
    explicit                    Time                           (TimeType                    time); 
    TimeType                    m_cTime;

/*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_MRDTM_NAMESPACE