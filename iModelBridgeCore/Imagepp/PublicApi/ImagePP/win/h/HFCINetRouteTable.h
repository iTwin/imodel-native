//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCINetRouteTable.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFCInetRouteTable
//-----------------------------------------------------------------------------

#pragma once


#include "HFCInetRoute.h"



class HFCInetRouteTable
    {
public:

    // Constructor - Destructor
    HFCInetRouteTable();
    HFCInetRouteTable(const HFCInetRouteTable& pi_rObj);
    virtual ~HFCInetRouteTable();

    // Assignment operator
    HFCInetRouteTable& operator=(const HFCInetRouteTable& pi_rObj);

    // Management
    void            Refresh();

    // Information
    const HFCInetRoute&
    GetRouteFor(const WString& pi_rDestination);

    static uint32_t GetValueOf(const WString& pi_rAddress);

protected:

private:

    void            SkipHeader(wistringstream& pi_rInput);
    void            ReadAnEntry(wistringstream& pi_rInput);
    BOOL            StartCommand(HANDLE pi_StdoutWr);
    void            ReadContents(WString& pi_rContents);
    void            ReadLine(wistringstream& pi_rInput, WString& pi_rString);

    static WString   s_SectionDelimiter;
    static WString   s_HeadingDelimiter;

    typedef list<HFCInetRoute, allocator<HFCInetRoute> > RouteList;

    // The list of routes
    RouteList       m_RouteTable;

    // A default route that should never be used...
    static HFCInetRoute
    s_DefaultRoute;
    };

