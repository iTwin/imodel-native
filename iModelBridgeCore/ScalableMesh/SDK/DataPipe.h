#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

/*

#include <Geom\GeomApi.r.h>
*/

#include <Bentley\Bentley.h>
#include <Bentley\Bentley.r.h>
#include <Geom\msgeomstructs_typedefs.h>
#include <Bentley\stdcxx\bvector.h>

using namespace Bentley::Bstdcxx;

#include <Geom\dpoint3d.h>

#include <TerrainModel\TerrainModel.h>

 class DataPipe
        {
        private : 

            const DPoint3d*         m_points;
            size_t                  m_nbOfPoints;
            DTMFeatureType          m_featureType;
            std::condition_variable m_consume;
            std::condition_variable m_produce;
            std::mutex              m_availableDataMutex; 
            std::atomic<bool>       m_consumeDone;                        
                                    
        public : 

            DataPipe()
                {
                m_consumeDone = true;                
                }

            void WritePoints(const DPoint3d* points,
                             size_t          nbOfPoints)
                {                                                                    
                std::unique_lock<std::mutex> lck(m_availableDataMutex);                   
                m_points = points;
                m_nbOfPoints = nbOfPoints; 
                m_consumeDone = false;                
                m_consume.notify_one();              
                while (!m_consumeDone) m_produce.wait(lck);                
                }

            void ReadPoints(bvector<DPoint3d>& points)
                { 
                std::unique_lock<std::mutex> lck(m_availableDataMutex); 
                while (m_consumeDone) m_consume.wait(lck);                                
                points.resize(m_nbOfPoints);
                if (points.size() > 0)
                    {
                    memcpy(&points[0], m_points, sizeof(DPoint3d) * points.size());                                           
                    }                           
                }

            void FinishProcessingPoints()
                {
                m_consumeDone = true;
                m_produce.notify_one();              
                }
        };