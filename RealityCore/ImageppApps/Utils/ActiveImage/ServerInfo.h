/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ServerInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef CServerInfo_H
#define CServerInfo_H


class CServerInfo
{
    public:
        
        CServerInfo();
        CServerInfo(WString pi_Name, 
                    WString pi_PathExtention, 
                    WString pi_IPAddress, 
                    unsigned short pi_Port, 
                    uint32_t pi_Type,
                    uint32_t pi_Index);

        CServerInfo(const CServerInfo& pi_Src);
        virtual ~CServerInfo();

        CServerInfo& operator=(const CServerInfo& pi_Src);

        WString GetName() const;
        WString GetPathExtention() const;
        WString GetAddress() const;
        uint32_t GetType() const;
        unsigned short GetPort() const;
        uint32_t GetIndex() const;

    protected:

    private:

        void CommonCopy(const CServerInfo& pi_Src);

        WString m_Name;
        WString m_PathExtention;
        WString m_Address;
        uint32_t m_Type;
        uint32_t m_Index;
        unsigned short m_Port;
};

#include "ServerInfo.hpp"

#endif