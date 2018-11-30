/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/ECInstanceChangeHandlers.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Content.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
//! Takes care of changing ECInstances.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct IECInstanceChangeHandler : RefCountedBase
{
    static const int PRIORITY_DEFAULT = 1000;

protected:
    virtual int _GetPriority() const {return PRIORITY_DEFAULT;}
    virtual bool _CanHandle(ECDbCR, ECClassCR) const = 0;
    virtual ECInstanceChangeResult _Change(ECDbR, ChangedECInstanceInfo const&, Utf8CP, ECValueCR) = 0;

public:
    int GetPriority() const {return _GetPriority();}
    bool CanHandle(ECDbCR connection, ECClassCR ecClass) const {return _CanHandle(connection, ecClass);}
    ECInstanceChangeResult Change(ECDbR connection, ChangedECInstanceInfo const& changeInfo, Utf8CP propertyAccessor, ECValueCR value)
        {
        return _Change(connection, changeInfo, propertyAccessor, value);
        }
};
typedef RefCountedPtr<IECInstanceChangeHandler> IECInstanceChangeHandlerPtr;

//=======================================================================================
//! Compares IECInstanceChangeHandler refcounted pointers using their priority.
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct IECInstanceChangeHandlerPtrComparer
    {
    bool operator()(IECInstanceChangeHandlerPtr const& lhs, IECInstanceChangeHandlerPtr const& rhs) const
        {
        return lhs->GetPriority() < rhs->GetPriority()
            || lhs->GetPriority() == rhs->GetPriority() && lhs.get() < rhs.get();
        }
    };

//=======================================================================================
//! Default handler for changing ECInstances. Can be used when ECDb doesn't require a
//! write token.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct DefaultECInstanceChangeHandler : IECInstanceChangeHandler
{
protected:
    int _GetPriority() const override {return 0;}
    ECPRESENTATION_EXPORT bool _CanHandle(ECDbCR, ECClassCR) const override;
    ECPRESENTATION_EXPORT ECInstanceChangeResult _Change(ECDbR, ChangedECInstanceInfo const&, Utf8CP, ECValueCR) override;
public:
    static RefCountedPtr<DefaultECInstanceChangeHandler> Create() {return new DefaultECInstanceChangeHandler();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
