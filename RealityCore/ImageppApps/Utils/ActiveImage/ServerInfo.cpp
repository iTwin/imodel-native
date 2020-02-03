/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ServerInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "ServerInfo.h"

CServerInfo::CServerInfo()
{
}

CServerInfo::CServerInfo(WString pi_Name, 
                         WString pi_PathExtention, 
                         WString pi_IPAddress, 
                         unsigned short pi_Port, 
                         uint32_t pi_Type,
                         uint32_t pi_Index)
{
    m_Name = pi_Name;
    m_Type = pi_Type;
    m_Port = pi_Port;
    m_Index   = pi_Index;
    m_Address = pi_IPAddress;
    m_PathExtention = pi_PathExtention;
}

CServerInfo::CServerInfo(const CServerInfo& pi_Src)
{
    CommonCopy(pi_Src);
}
CServerInfo::~CServerInfo()
{
}

CServerInfo& CServerInfo::operator=(const CServerInfo& pi_Src)
{
    if( &pi_Src != this )
        CommonCopy(pi_Src);

    return *this;
}

void CServerInfo::CommonCopy(const CServerInfo& pi_Src)
{
    m_Name = pi_Src.m_Name;
    m_Type = pi_Src.m_Type;
    m_Port = pi_Src.m_Port;
    m_Index   = pi_Src.m_Index;
    m_Address = pi_Src.m_Address;
    m_PathExtention = pi_Src.m_PathExtention;
}