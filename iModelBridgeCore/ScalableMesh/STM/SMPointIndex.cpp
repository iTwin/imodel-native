#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMesh.h"
#include "ScalableMeshQuery.h"
#include "Stores\SMStreamingDataStore.h"
#include "SMPointIndex.h"
#include "SMPointIndex.hpp"
#include "HGF3DFilterCoord.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

template <> class SpatialOp<HGF3DFilterCoord<double, double>, HGF3DFilterCoord<double, double>, HGF3DExtent<double> >
    {

    public:
        static  HGF3DExtent<double> GetExtent(const HGF3DFilterCoord<double, double> spatialObject)
            {
            return  HGF3DExtent<double>(spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ(), spatialObject.GetX(), spatialObject.GetY(), spatialObject.GetZ());
            }

        static bool IsPointIn2D(const HGF3DFilterCoord<double, double> spatialObject, HGF3DFilterCoord<double, double> pi_rCoord)
            {
            return spatialObject.IsEqualTo2D(pi_rCoord);
            }

        static bool IsSpatialInExtent2D(const HGF3DFilterCoord<double, double>& spatial, const  HGF3DExtent<double>& extent)
            {
            return extent.IsPointIn2D(spatial);
            }

        static bool IsSpatialInExtent3D(const HGF3DFilterCoord<double, double>& spatial, const  HGF3DExtent<double>& extent)
            {
            return extent.IsPointIn(spatial);
            }

    };


template class ISMPointIndexQuery<DPoint3d, Extent3dType>;

template class SMPointIndexNode<DPoint3d, Extent3dType>;

template class SMPointIndex<DPoint3d, Extent3dType>;

//template class SMPointIndex<HGF3DFilterCoord<double, double>, HGF3DExtent<double>>;

//template class SMPointIndexNode<HGF3DFilterCoord<double, double>, HGF3DExtent<double>>;