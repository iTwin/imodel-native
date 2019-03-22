//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTExportProgressIndicator.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HUTExportProgressIndicator
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// SetActivate
// Activate or deactivate the progress indicator.
//-----------------------------------------------------------------------------
inline void HUTExportProgressIndicator::SetExportedFile(HRFRasterFile* pi_pExportedFile)
    {
    m_pExportedFile = pi_pExportedFile;

    if (pi_pExportedFile != 0)
        {
        m_FirstResBlockSentToPool   = 0;
        m_FirstResBlockAlreadyCount = 0;
        m_NbBlocksPerRes.clear();
        m_NbBlocksPerRes.resize(m_pExportedFile->GetPageDescriptor(0)->CountResolutions(), 0);
        }
    }

//-----------------------------------------------------------------------------
// public
// ContinueIteration
// Increment the progress indicator if an export is occurring.
//-----------------------------------------------------------------------------
inline bool HUTExportProgressIndicator::ContinueIteration(HRFRasterFile* pi_pWrittenRasterFile,
                                                           uint32_t      pi_ResIndex,
                                                           bool          pi_BlockSentToPool)
    {
    bool RetVal = false;

    if (pi_pWrittenRasterFile == m_pExportedFile)
        {
        //The block is sent to the pool (HRA)
        if (pi_BlockSentToPool)
            {
            HASSERT(pi_ResIndex == 0);
            m_FirstResBlockSentToPool++;

            double UnwrittenBlockInPoolPercentage = (m_FirstResBlockSentToPool - (uint32_t)m_IterationProcessed) / (double)m_CountIteration;

            //If there is a too great difference between the number of block in the pool
            //and the number of block written to file, update the progress bar based
            //on the last block sent too the pool (CopyFrom)
            if (UnwrittenBlockInPoolPercentage > 0.10)
                {
                m_FirstResBlockAlreadyCount++;
                m_NbBlocksPerRes[pi_ResIndex]++;
                RetVal = HFCProgressIndicator::ContinueIteration();
                }
            }
        else
            {
            //If a block from the principal resolution was written to the file and
            //the total number of blocks written to file isn't equal the number of block
            //already count (e.g. : use to increment the progress bar)
            if ((pi_ResIndex == 0) &&
                (m_FirstResBlockAlreadyCount > 0))
                {
                //Decrement the difference
                m_FirstResBlockAlreadyCount--;
                }
            else
                {
                //Increment the progress bar
                m_NbBlocksPerRes[pi_ResIndex]++;
                RetVal = HFCProgressIndicator::ContinueIteration();
                }
            }
        }

    return RetVal;
    }

//-----------------------------------------------------------------------------
// public
// GetNbBlocks
// Return the number of blocks currently exported for a given resolution.
//-----------------------------------------------------------------------------
inline uint32_t HUTExportProgressIndicator::GetNbExportedBlocks(uint32_t pi_ResIndex)
    {
    return m_NbBlocksPerRes[pi_ResIndex];
    }

END_IMAGEPP_NAMESPACE
