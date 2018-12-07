//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/STM/HGF3DFilterCoord.h $
//:>    $RCSfile: HGF3DFilterCoord.h,v $
//:>   $Revision: 1.3 $
//:>       $Date: 2010/08/19 13:21:39 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DCoord
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HGF3DCoord.h>

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Mathieu St-Pierre
    
    This class decorates a 3D coord by adding information about the relevance of 
    a point.
    ----------------------------------------------------------------------------- 
*/

//template <class DataType> class GenericCompressedTileStore : public GenericTileStore<DataType>

template<class DataType = double, class RelevanceDataType = double> class HGF3DFilterCoord : public HGF3DCoord<DataType>
{
    public:

      
                           HGF3DFilterCoord()
                           : HGF3DCoord(), 
                           m_Relevance(0)                                
                           {
                           }

                           HGF3DFilterCoord(DataType pi_X,
                                            DataType pi_Y,
                                            DataType pi_Z,
                                            RelevanceDataType pi_Relevance = 0)
                           : HGF3DCoord(pi_X, pi_Y, pi_Z),
                           m_Relevance(pi_Relevance)                                
                           {

                           }

                           HGF3DFilterCoord(const HGF3DFilterCoord<DataType, RelevanceDataType>& pi_rObj)                           
                           : HGF3DCoord(pi_rObj), 
                           m_Relevance(pi_rObj.GetRelevance())               
                           {
                           }       

        virtual ~HGF3DFilterCoord()
        {

        }

        HGF3DFilterCoord<DataType>& operator=(const HGF3DFilterCoord<DataType>& pi_rObj)
        {   
            m_Relevance = pi_rObj.GetRelevance();
            HGF3DCoord::operator=(pi_rObj);
            return (*this);
        }

        //HGF3DFilterCoord<DataType, RelevanceDataType>& operator=(const HGF3DFilterCoord<DataType, RelevanceDataType>& pi_rObj);
                        
        // Location management
        RelevanceDataType  GetRelevance() const
        { 
            return m_Relevance;
        }

        void               SetRelevance(RelevanceDataType pi_Relevance)
        {
            m_Relevance = pi_Relevance;
        }
                        

    protected:

    private:
    public :
        // How the point is important in describing the terrain.       
        RelevanceDataType m_Relevance;
};
          