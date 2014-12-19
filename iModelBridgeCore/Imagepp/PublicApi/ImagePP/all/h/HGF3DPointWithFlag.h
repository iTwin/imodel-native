//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DPointWithFlag.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DPointWithFlag
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------
#pragma once

#include "HGF3DPoint.h"
#include "HGF2DCoordSys.h"
#include "HGF2DLiteExtent.h"
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the nature and behavior for raw coordinates DTM point. Such coordinates
    are expressed using 3 numbers indicating values for three dimensions arbitrarily
    named X, Y and Z.
    This class is used to describe position in three-dimensional coordinate
    systems for Data Terrain Model. In addition to the three coordinates, a user-define flag
    is added.

    Note the existence of the following definition in addition to the HGF3DPointWithFlag<T> class:
    typedef vector<HGF3DPointWithFlag<T>, allocator<HGF3DPointWithFlag<T>> > HGF3DPointWithFlagCollection<T>;

    This defines a collection of coordinates, and answers in all
    points to the vector<> Standard Template Library interface.

    -----------------------------------------------------------------------------
*/
class HGF3DPointWithFlag : public HGF3DCoord<double>
    {
public:

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor for this class.  They allow three
        ways to construct a coordinate object: as an empty position (all
        coordinates at zero), as a new coord with specified coordinates,
        or as a duplicate of another coordinate object.

        @param pi_X X-axis coordinate of the position.

        @param pi_Y Y-axis coordinate of the position.

        @param pi_Z Z-axis coordinate of the position.

        @param pi_UserFlag Optional paramters Integer value that requires
                           a user-defined value. The default is 0

        @param pi_rObject Constant reference to a HGF3DPointWithFlag to duplicate.

        Example:
        @code
        HGF3DPointWithFlag               ImageOrigin (10.0, 10.0, 0.0, 4);
            HGF3DPointWithFlag               ImageWorld(ImageOrigin);
            HGF3DPointWithFlag               APoint;
        @end

        -----------------------------------------------------------------------------
    */
    HGF3DPointWithFlag();
    HGF3DPointWithFlag(double pi_X,
                       double pi_Y,
                       double pi_Z = 0.0,
                       int    pi_UserFlag = 0);

    HGF3DPointWithFlag(HGF2DCoord<double> pi_2DPoint,
                       double pi_Z = 0.0,
                       int    pi_UserFlag = 0);

    HGF3DPointWithFlag(HGF3DCoord<double> pi_3DPoint,
                       int                pi_UserFlag = 0);





    HGF3DPointWithFlag(const HGF3DPointWithFlag& pi_rObj);

    ~HGF3DPointWithFlag();
    HGF3DPointWithFlag&     operator=(const HGF3DPointWithFlag& pi_rObj);

    // Compare operations
    bool                operator==(const HGF3DPointWithFlag& pi_rObj) const;
    bool                operator!=(const HGF3DPointWithFlag& pi_rObj) const;



    int                GetUserFlag() const;
    void               SetUserFlag(int pi_UserFlag);

    virtual void       PrintState(ostream& po_rOutput) const;

protected:


private:
public :
    int m_UserFlag;
    };



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the nature and behavior for a list of DTM points
    -----------------------------------------------------------------------------
*/
class HGF3DPointWithFlagCollection : public vector<HGF3DPointWithFlag, allocator<HGF3DPointWithFlag> >
    {
public:
    HGF3DPointWithFlagCollection(const HFCPtr<HGF2DCoordSys> pi_rpCoordSys) : m_pCoordSys(pi_rpCoordSys) {};
    virtual ~HGF3DPointWithFlagCollection() {};

    virtual void ChangeCoordSys(const HFCPtr<HGF2DCoordSys> pi_rpCoordSys);
    virtual HGF2DLiteExtent Get2DLiteExtent() const;

#if (0)
    HGF3DPointWithFlagCollection& operator=(const HGF3DPointWithFlagCollection& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            vector<HGF3DPointWithFlag, allocator<HGF3DPointWithFlag> >::operator=(pi_rObj);
            }
        return(*this);
        }
#endif

private:
    HFCPtr<HGF2DCoordSys> m_pCoordSys;
    };




// Stream interaction global operators.
istream& operator>>(istream& pi_InStream, HGF3DPointWithFlagCollection& pio_rPoints);
ostream& operator<<(ostream& pi_OutStream, const HGF3DPointWithFlagCollection& pi_rPoints);



#include "HGF3DPointWithFlag.hpp"

