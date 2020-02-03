/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/Toolbox.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// TOOLBOX.h : header file
//

#ifndef __TOOLBOX_H__
#define __TOOLBOX_H__

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HRFRasterFile.h>

#include <iostream>
#include <TIME.H>

//-----------------------------------------------------------------------------

class Chronometer
{
    public:
        Chronometer();

        void            Start();
        Utf8String         Stop();
        void            Reset();

		Utf8String         GetDuration();
        Utf8String         GetCount();

		Utf8String         GetBestDuration();
		Utf8String         GetAverageDuration();
		Utf8String         GetWorstDuration();

        Utf8String         GetBestIteration();
		Utf8String         GetWorstIteration();

        Utf8String         GetStartTime();
		Utf8String         GetFinishTime();


    private:
        clock_t m_start;
        clock_t m_finish;

        time_t  m_startTime;
        time_t  m_finishTime;

        unsigned long   m_Count;
        unsigned long   m_bestIteration;
        unsigned long   m_worstIteration;
        double          m_duration;
        double          m_durationBest;
        double          m_durationWorst;
        bool            m_ToReset;
        bool            m_IsStarted;
};

//-----------------------------------------------------------------------------

void ConvertThePixels(uint32_t                        pi_Width,
					  uint32_t                        pi_Height,
					  const HFCPtr<HRPPixelType>&   pi_rpSrcPixelType,
					  const HFCPtr<HCDPacket>&	    pi_rpSrcPacket, 
					  const HFCPtr<HRPPixelType>&   pi_rpDstPixelType,
					  HFCPtr<HCDPacket>&            po_rpDstPacket);

//-----------------------------------------------------------------------------

Utf8String GetLongPathName(Utf8String pi_Path);

//-------------------------------------------------------------------------------------------------

typedef struct _NODE {
    BOOL bIsLeaf;               // true if node has no children
    UINT nPixelCount;           // Number of pixels represented by this leaf
    UINT nRedSum;               // Sum of red components
    UINT nGreenSum;             // Sum of green components
    UINT nBlueSum;              // Sum of blue components
    struct _NODE* pChild[8];    // Pointers to child nodes
    struct _NODE* pNext;        // Pointer to next reducible node
} NODE;

//-------------------------------------------------------------------------------------------------

class GCIPalette : public CPalette
{
   public:
      GCIPalette();                       
	   virtual ~GCIPalette();

      bool CreateIdenticalPalette(CPalette *p_palette);
      bool CreateOptimizedPaletteFromHandle(HANDLE hImage, int p_numberColorBits);

   private:

      bool CreateExactPalette (HANDLE hImage);
      bool CreateOctreePalette (HANDLE hImage, UINT nMaxColors, UINT nColorBits);

      void AddColor (NODE**, BYTE, BYTE, BYTE, UINT, UINT, UINT*, NODE**);
      NODE* CreateNode (UINT, UINT, UINT*, NODE**);
      void ReduceTree (UINT, UINT*, NODE**);
      void DeleteTree (NODE**);
      void GetPaletteColors (NODE*, PALETTEENTRY*, UINT*);
      int GetRightShiftCount (DWORD);
      int GetLeftShiftCount (DWORD);
      UINT BitsPerPixel (HANDLE);
};

//-------------------------------------------------------------------------------------------------

#endif // __TOOLBOX_H__
