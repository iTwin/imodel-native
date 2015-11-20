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
            std::condition_variable m_consumePoints;
            std::condition_variable m_producePoints;
            std::condition_variable m_consumeFeatures;
            std::condition_variable m_produceFeatures;
            std::mutex              m_availableDataMutex; 
            std::atomic<bool>       m_consumePointsDone;   
            std::atomic<bool>       m_consumeFeaturesDone;   
            std::atomic<bool>       m_finishWritingPoints;   
            std::atomic<bool>       m_finishWritingFeature;   

                                    
        public : 

            DataPipe()
                {                
                m_consumePointsDone = true;   
                m_consumeFeaturesDone = true;   
                m_finishWritingPoints = false;
                m_finishWritingFeature = false;
                }

            bool WriteFeature(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType)
                {
                std::unique_lock<std::mutex> lck(m_availableDataMutex);                   
                m_points = featurePoints;
                m_nbOfPoints = nbOfFeaturesPoints;
                m_featureType = featureType;
                m_consumeFeaturesDone = false;                
                m_consumeFeatures.notify_one();              
                while (!m_consumeFeaturesDone) m_produceFeatures.wait(lck);   

                return true;
                }

             void ReadFeature(bvector<DPoint3d>& points, DTMFeatureType& featureType)
                {                 
                std::unique_lock<std::mutex> lck(m_availableDataMutex); 
                while (m_consumeFeaturesDone && !m_finishWritingFeature) m_consumeFeatures.wait(lck);                                
                points.resize(m_nbOfPoints);
                if (points.size() > 0)
                    {
                    memcpy(&points[0], m_points, sizeof(DPoint3d) * points.size());                                           
                    featureType = m_featureType;
                    }                           
                }

            void WritePoints(const DPoint3d* points,
                             size_t          nbOfPoints)
                {                                                                    
                std::unique_lock<std::mutex> lck(m_availableDataMutex);                   
                m_points = points;
                m_nbOfPoints = nbOfPoints; 
                m_consumePointsDone = false;                
                m_consumePoints.notify_one();              
                while (!m_consumePointsDone) m_producePoints.wait(lck);                
                }

            void ReadPoints(bvector<DPoint3d>& points)
                {               
                std::unique_lock<std::mutex> lck(m_availableDataMutex); 
                while (m_consumePointsDone && !m_finishWritingPoints) m_consumePoints.wait(lck);                                
                points.resize(m_nbOfPoints);
                if (points.size() > 0)
                    {
                    memcpy(&points[0], m_points, sizeof(DPoint3d) * points.size());                                           
                    }                           
                }

            void FinishProcessingPoints()
                {
                m_consumePointsDone = true;
                m_producePoints.notify_one();              
                }

            void FinishProcessingFeatures()
                {
                m_consumeFeaturesDone = true;
                m_produceFeatures.notify_one();              
                }
                        
            void FinishWritingPoints()
                {
                WritePoints(0, 0);
                m_finishWritingPoints = true;                
                }

            void FinishWritingFeatures()
                {
                WriteFeature(0, 0, DTMFeatureType::None);
                m_finishWritingFeature = true;                
                }
                
        };