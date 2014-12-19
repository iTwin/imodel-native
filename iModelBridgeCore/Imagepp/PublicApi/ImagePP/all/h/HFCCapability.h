//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCCapability.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @end

    @author Ghislain Tardif (${mailto:Ghislain.Tardif@HMRInc.com})
    @end

    This class is used to hold description of any capabilities. These
    capabilities describe the functionalities supported by a object.
    -----------------------------------------------------------------------------
 */
class HFCCapability : public HFCShareableObject<HFCCapability>
    {
public:
    HDECLARE_BASECLASS_ID(1382)

    // Constructor/ Destructor
    HFCCapability();
    HFCCapability(const HFCCapability& pi_rObj);
    virtual ~HFCCapability();

    // operator
    HFCCapability&  operator=(const HFCCapability& pi_rObj);

    virtual HFCCapability*
    Clone() const = 0;

    // Utility
    /** -----------------------------------------------------------------------------
        Equality compare operator. This method indicates if self is similar to the
        given capability. This is if the data containt by the two capabilities are
        the same at the first level.
        @end

        @h3{Inheritance notes:}
            The method must be define.
        @end


        @param pi_rpCapability  Constant reference to a smart pointer making
                                reference to the capability to compare with.
        @end

        @return true if the capabilities are the same and false otherwise.
        @end

        @see HFCCapability::Support(const HFCCapability&) const
        @end
        -----------------------------------------------------------------------------
     */
    virtual bool           IsSameAs(const HFCCapability& pi_rCapability) const = 0;

    /** -----------------------------------------------------------------------------
        Compatibility compare operator. This method indicates if self is alike given
        capability. This method will return true if the given capability is supported
        or include in self.

        @h3{Inheritance notes:}
            The method must be define.
        @end

        @param pi_rpCapability Constant reference to a smart pointer making reference
                               to the capability to compare with.

        @return This method will return true if seft is supported or include in the
                given capability.

        @see HFCCapability::IsSameAs(const HFCCapability&)
        -----------------------------------------------------------------------------
     */
    virtual bool           Supports(const HFCCapability& pi_rCapability) const = 0;

private:

    // disabled methods
    bool             operator==(const HFCCapability& pi_rObj) const;
    bool             operator!=(const HFCCapability& pi_rObj) const;

    };

#include "HFCCapability.hpp"

