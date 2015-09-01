//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFJpegLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFJpegLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFJpegLineEditor.h>
#include <Imagepp/all/h/HRFJpegFile.h>

#include <libjpeg-turbo/jpeglib.h>

#include <Imagepp/all/h/HGFCMYKColorSpace.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFJpegLineEditor::HRFJpegLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    // get the JPEG member to simplify thing in the method
        HRFJpegFile::JPEG*  pJpeg = static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->GetFilePtr();

    m_IsCMYK = false;

    if (m_AccessMode.m_HasReadAccess && pJpeg->m_pDecompress)
        m_IsCMYK = (pJpeg->m_pDecompress->out_color_space == JCS_CMYK);
    else if (m_AccessMode.m_HasWriteAccess && pJpeg->m_pCompress)
        m_IsCMYK = (pJpeg->m_pCompress->in_color_space == JCS_CMYK);
    else if (m_AccessMode.m_HasCreateAccess && pJpeg->m_pCompress)
        m_IsCMYK = (pJpeg->m_pCompress->in_color_space == JCS_CMYK);

    if (m_IsCMYK)
        m_pCMYKData = new Byte[pJpeg->m_pDecompress->output_width * 4];
    else
        m_pCMYKData = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFJpegLineEditor::~HRFJpegLineEditor()
    {
    delete m_pCMYKData;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFJpegLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*  po_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    HSTATUS Status = H_ERROR;
    HRFJpegFile::JPEG*  pJpeg;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }
    if (m_IsCMYK)
        {
        Status = ReadCMYKBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
        }
    else
        {
        // get the JPEG member to simplify thing in the method
        pJpeg = static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->GetFilePtr();

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // Restart the file at the begining when the ask for a before block
        // or if we need to synchronize the file with the memory.
        if (pJpeg->m_pDecompress->output_scanline > pi_PosBlockY)
            {
            static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->SaveJpegFile(true);
            static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->Open();
            }

        // verify that the line number specified is the same
        // as the next line to be read in the jpeg.
        // Remember: No random access!
        if (pi_PosBlockY == pJpeg->m_pDecompress->output_scanline)
            {
            // read one scanline into the specified data buffer
            // verify that the correct line number was read
            if (jpeg_read_scanlines(pJpeg->m_pDecompress, (JSAMPARRAY) &po_pData, 1) != 1)
                goto WRAPUP;    // H_ERROR

            } // Incorrect line number
        else
            {
            // Try to simulate a little random access
            if (pi_PosBlockY > pJpeg->m_pDecompress->output_scanline)
                {
                // If we ask a line > of the current line, read until this line.
                while (pi_PosBlockY >= pJpeg->m_pDecompress->output_scanline)
                    {
                    if (jpeg_read_scanlines(pJpeg->m_pDecompress,(JSAMPARRAY) &po_pData, 1) != 1)
                        goto WRAPUP;    // H_ERROR
                    }
                }
            else
                goto WRAPUP;    // H_ERROR
            }

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HSTATUS HRFJpegLineEditor::ReadCMYKBlock(uint64_t pi_PosBlockX,
                                         uint64_t pi_PosBlockY,
                                         Byte*  po_pData,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HSTATUS Status = H_SUCCESS;
    HRFJpegFile::JPEG*  pJpeg;

    // get the JPEG member to simplify thing in the method
    pJpeg = static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->GetFilePtr();

    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Restart the file at the begining when the ask for a before block
    // or if we need to synchronize the file with the memory.
    if (pJpeg->m_pDecompress->output_scanline > pi_PosBlockY)
        {
        static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->SaveJpegFile(true);
        static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->Open();
        }

    // verify that the line number specified is the same
    // as the next line to be read in the jpeg.
    // Remember: No random access!
    if (pi_PosBlockY == pJpeg->m_pDecompress->output_scanline)
        {
        // read one scanline into the specified data buffer
        // verify that the correct line number was read
        if (jpeg_read_scanlines(pJpeg->m_pDecompress, (JSAMPARRAY) &m_pCMYKData, 1) != 1)
            Status = H_ERROR;

        } // Incorrect line number
    else
        {
        // Try to simulate a little random access
        if (pi_PosBlockY > pJpeg->m_pDecompress->output_scanline)
            {
            // If we ask a line > of the current line, read until this line.
            while ((Status == H_SUCCESS) && (pi_PosBlockY >= pJpeg->m_pDecompress->output_scanline))
                {
                if (jpeg_read_scanlines(pJpeg->m_pDecompress,(JSAMPARRAY) &m_pCMYKData, 1) != 1)
                    Status = H_ERROR;
                }
            }
        else
            Status = H_ERROR;
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    if (Status == H_SUCCESS && m_pCMYKData)
        {
        // Convert data from CMYK buffer to RGB buffer.
        // Conversion de CMYK vers RGB.  CMYKBuffer -> po_pData
        HGFCMYKColorSpace CMYKColorSpace(true);

        CMYKColorSpace.ConvertArrayToRGB (m_pCMYKData, po_pData, pJpeg->m_pDecompress->output_width);
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFJpegLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                      uint64_t     pi_PosBlockY,
                                      const Byte*  pi_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_ERROR;
    HRFJpegFile::JPEG*  pJpeg;

    if (m_IsCMYK)
        {
        Status = WriteCMYKBlock(pi_PosBlockX, pi_PosBlockY, pi_pData, pi_pSisterFileLock);
        }
    else
        {
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // get the JPEG member to simplify thing in the method
        pJpeg = static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->GetFilePtr();

        if ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0))
        	{
            // Backup table to restore the same compression quality
            jpeg_compress_struct Saved_quant_tbl;
            memset(&Saved_quant_tbl, 0, sizeof(struct jpeg_compress_struct));
            jpeg_create_compress(&Saved_quant_tbl);
            JQUANT_TBL** qtblptr;
            int tblno;
            for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) 
            	{
                if (pJpeg->m_pDecompress->quant_tbl_ptrs[tblno] != 0) 
                	{
                	qtblptr = &Saved_quant_tbl.quant_tbl_ptrs[tblno];
                	*qtblptr = jpeg_alloc_quant_table((j_common_ptr)&Saved_quant_tbl);
                	 BeStringUtilities::Memcpy ((*qtblptr)->quantval, sizeof((*qtblptr)->quantval), 
                          pJpeg->m_pDecompress->quant_tbl_ptrs[tblno]->quantval, sizeof((*qtblptr)->quantval));
                	(*qtblptr)->sent_table = FALSE;
              		}
            	}

            static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->SaveJpegFile(TRUE);
            static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->Create();
            static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->AssignPageToStruct2(&Saved_quant_tbl);

            jpeg_destroy_compress(&Saved_quant_tbl);
        	}

        // verify that the line number specified is the same
        // as the next line to be written in the jpeg.
        // Remember: No random access!
        if (pi_PosBlockY == pJpeg->m_pCompress->next_scanline)
            {
            // read one scanline into the specified data buffer
            // verify that the correct line number was read
            if (jpeg_write_scanlines(pJpeg->m_pCompress,(JSAMPARRAY) &pi_pData,1) == 1)
                {
                // Increment the modification counter;
                GetRasterFile()->SharingControlIncrementCount();

                Status = H_SUCCESS;
                }
            }

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFJpegLineEditor::WriteCMYKBlock(uint64_t     pi_PosBlockX,
                                          uint64_t     pi_PosBlockY,
                                          const Byte*  pi_pData,
                                          HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pCMYKData != 0);
    HPRECONDITION (pi_pData    != 0);

    HSTATUS Status = H_ERROR;
    HRFJpegFile::JPEG* pJpeg;

    // Lock the sister file for the ReadBlock operation
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // get the JPEG member to simplify thing in the method
    pJpeg = static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->GetFilePtr();

    if ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0))
    	{
        // Backup table to restore the same compression quality
        jpeg_compress_struct Saved_quant_tbl;
        memset(&Saved_quant_tbl, 0, sizeof(struct jpeg_compress_struct));
        jpeg_create_compress(&Saved_quant_tbl);
        JQUANT_TBL** qtblptr;
        int tblno;
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) 
        	{
            if (pJpeg->m_pDecompress->quant_tbl_ptrs[tblno] != 0) 
            	{
                qtblptr = &Saved_quant_tbl.quant_tbl_ptrs[tblno];
                *qtblptr = jpeg_alloc_quant_table((j_common_ptr)&Saved_quant_tbl);
                 BeStringUtilities::Memcpy ((*qtblptr)->quantval, sizeof((*qtblptr)->quantval), 
                          pJpeg->m_pDecompress->quant_tbl_ptrs[tblno]->quantval, sizeof((*qtblptr)->quantval));
                (*qtblptr)->sent_table = FALSE;
            	}
        	}

        static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->SaveJpegFile(true);
        static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->Create();
        static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->AssignPageToStruct2(&Saved_quant_tbl);

        jpeg_destroy_compress(&Saved_quant_tbl);
   	 	}

    // verify that the line number specified is the same
    // as the next line to be written in the jpeg.
    // Remember: No random access!
    if (pi_PosBlockY == pJpeg->m_pCompress->next_scanline)
        {
        // Convert data from CMYK buffer to RGB buffer.
        // Conversion de CMYK vers RGB.  CMYKBuffer -> po_pData
        HGFCMYKColorSpace CMYKColorSpace(true);
        CMYKColorSpace.ConvertArrayFromRGB ((unsigned char*)pi_pData, m_pCMYKData, pJpeg->m_pCompress->image_width);

        // read one scanline into the specified data buffer
        // verify that the correct line number was read
        if (jpeg_write_scanlines(pJpeg->m_pCompress,(JSAMPARRAY) &m_pCMYKData,1) == 1)
            {
            // Increment the modification counter;
            GetRasterFile()->SharingControlIncrementCount();
            Status = H_SUCCESS;
            }
        }
    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFJpegLineEditor::OnSynchronizedSharingControl()
    {
    static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->SaveJpegFile(true);
    static_cast<HRFJpegFile*>(GetRasterFile().GetPtr())->Open();
    }