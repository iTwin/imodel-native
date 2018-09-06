#ifdef BUILDTMFORDGNDB
#include <BeSQLite/BeSQLite.h>
typedef BENTLEY_NAMESPACE_NAME::BeSQLite::BeGuid InroadsGuid;
#else

#include <Bentley/stg/guid.h>
struct InroadsGuid
{
    union BeGuid { struct _GUID g; UInt64 u[2]; UInt32 i[4]; char b[16]; };
    BeGuid m_guid;

    void Create();
    InroadsGuid& operator=(const InroadsGuid& guid)
        {
        m_guid.g = guid.m_guid.g;
        return *this;
        }
    bool operator==(const InroadsGuid& guid) const
        {
        return m_guid.g == guid.m_guid.g;
        }
    };
#endif
