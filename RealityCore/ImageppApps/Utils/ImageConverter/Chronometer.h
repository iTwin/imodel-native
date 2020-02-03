/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/Chronometer.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/Chronometer.h,v 1.2 2004/02/09 16:44:12 DonaldMorissette Exp $
//-----------------------------------------------------------------------------
// Class Chronometer
//-----------------------------------------------------------------------------

#ifndef __Chronometer__H__
#define __Chronometer__H__


class Chronometer
{
    public:
        Chronometer();

        void    Start();
        WString  Stop();
		WString  GetTime();

    private:
        clock_t m_start;
        clock_t m_finish;
};

#include "Chronometer.hpp"

#endif // __Chronometer__H__

