/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ServerInfo.hpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
inline WString CServerInfo::GetName() const
{
    return m_Name;
}

inline WString CServerInfo::GetPathExtention() const
{
    return m_PathExtention;
}

inline WString CServerInfo::GetAddress() const
{
    return m_Address;
}

inline uint32_t CServerInfo::GetType() const
{
    return m_Type;
}

inline uint32_t CServerInfo::GetIndex() const
{
    return m_Index;
}

inline unsigned short CServerInfo::GetPort() const
{
    return m_Port;
}
