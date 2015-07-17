/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMTime.h $
|    $RCSfile: IMrDTMTime.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/03/28 16:39:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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