//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFResolutionDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFResolutionDescriptor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFResolutionDescriptor.h>

//-----------------------------------------------------------------------------
// Public
// constructor.
// This constructor compute the 1:1 resolutions.
//-----------------------------------------------------------------------------
HGFResolutionDescriptor::HGFResolutionDescriptor(uint32_t pi_MaxWidth,
                                                 uint32_t pi_MaxHeight)
    {
    try
        {
        // Add the resolution 1:1 to the list.
        AddResolution(1.0, pi_MaxWidth, pi_MaxHeight);
        }
    catch(...)
        {
        RemoveResolutions();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Public
// constructor.
// This constructor compute the resolutions need to between the two limits if
// with the specified resolution factor.
// If pi_IsTiled == true
//      We create sub-resolution until (Height > pi_MinHeight) || (Width > pi_MinWidth)
// else
//      We create sub-resolution until (Height > pi_MinHeight)  // suppose to be strip
//-----------------------------------------------------------------------------
HGFResolutionDescriptor::HGFResolutionDescriptor(uint32_t pi_MaxWidth,
                                                 uint32_t pi_MaxHeight,
                                                 uint32_t pi_MinWidth,
                                                 uint32_t pi_MinHeight,
                                                 bool  pi_IsTiled,
                                                 uint32_t pi_ResolutionFactor)
    {
    try
        {
        // Add the resolution 1:1 to the list.
        AddResolution(1.0, pi_MaxWidth, pi_MaxHeight);

        // Add the resolution until we have reach the lower limits.
        double Resolution = 1;
        uint32_t Height     = pi_MaxHeight;
        uint32_t Width      = pi_MaxWidth;
        while ((Height > pi_MinHeight) || ((Width > pi_MinWidth) && pi_IsTiled))
            {
            // Compute the new resolution value.
            Width      = (uint32_t)ceil((double)Width / pi_ResolutionFactor);
            Height     = (uint32_t)ceil((double)Height / pi_ResolutionFactor);
            Resolution = Resolution / (double)pi_ResolutionFactor;

            // Add the resolution to the list.
            AddResolution(Resolution, Width, Height);
            }
        }
    catch(...)
        {
        RemoveResolutions();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Copy constructor.
//-----------------------------------------------------------------------------
HGFResolutionDescriptor::HGFResolutionDescriptor(const HGFResolutionDescriptor& pi_rObj)
    {
    // Copy all resolution description.
    double   Resolution;
    uint32_t  Width;
    uint32_t  Height;

    try
        {
        for (unsigned short SubImage = 0; SubImage < pi_rObj.CountResolutions(); SubImage++)
            {
            // Obtain the current resolution information.
            pi_rObj.GetDescription(SubImage, &Resolution, &Width, &Height);

            // Add the resolution to the list.
            AddResolution(Resolution, Width, Height);
            }
        }
    catch(...)
        {
        RemoveResolutions();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Default destructor.
//-----------------------------------------------------------------------------
HGFResolutionDescriptor::~HGFResolutionDescriptor()
    {
    RemoveResolutions();
    }

//-----------------------------------------------------------------------------
// Public
// Assignment operator.
//-----------------------------------------------------------------------------
HGFResolutionDescriptor& HGFResolutionDescriptor::operator=(
    const HGFResolutionDescriptor& pi_rObj)
    {
    double   Resolution;
    uint32_t  Width;
    uint32_t  Height;

    // Remove all exsiting resolution from the object.
    RemoveResolutions();

    // Copy all resolution description.
    for (unsigned short SubImage = 0; SubImage < pi_rObj.CountResolutions(); SubImage++)
        {
        // Obtain the current resolution information.
        pi_rObj.GetDescription(SubImage, &Resolution, &Width, &Height);

        // Add the resolution to the list.
        AddResolution(Resolution, Width, Height);
        }

    // Return our self.
    return *this;
    }

//-----------------------------------------------------------------------------
// Public
// Return the number of resolutions.
//-----------------------------------------------------------------------------
unsigned short HGFResolutionDescriptor::CountResolutions() const
    {
    return (unsigned short)m_ListOfResolution.size();
    }

//-----------------------------------------------------------------------------
// Public
// Return the resolution for the specified SubImage.
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::GetDescription (  unsigned short pi_SubImage,
                                                double*   po_pResolution,
                                                uint32_t*    po_pWidth,
                                                uint32_t*    po_pHeight)  const
    {
    HPRECONDITION(pi_SubImage < m_ListOfResolution.size());

    // Search the specified sub-image.
    HGFListOfResolution::const_iterator Itr = m_ListOfResolution.begin();
    for (unsigned short SubImage = (unsigned short)m_ListOfResolution.size()-1 ; SubImage != pi_SubImage ; SubImage--)
        {
        // Increment the iterator position;
        Itr++;
        }

    // Obtain the description for the specified resolution
    ResolutionDescriptor* ResDescriptor = (ResolutionDescriptor*)(*Itr).second;

    // Set the wanted parameters.
    if (po_pWidth)
        *po_pWidth  = ResDescriptor->Width;
    if (po_pHeight)
        *po_pHeight = ResDescriptor->Height;
    if (po_pResolution)
        *po_pResolution = (*Itr).first;
    }

//-----------------------------------------------------------------------------
// Public
// Return the resolution for the specified SubImage.
//-----------------------------------------------------------------------------
double HGFResolutionDescriptor::GetResolution(unsigned short pi_SubImage) const
    {
    HPRECONDITION(pi_SubImage < m_ListOfResolution.size());

    double Resolution = 0;

    GetDescription(pi_SubImage, &Resolution, 0 ,0);

    return Resolution;
    }

//-----------------------------------------------------------------------------
// Public
// Return the width for the specified SubImage.
//-----------------------------------------------------------------------------
uint32_t HGFResolutionDescriptor::GetWidth(unsigned short pi_SubImage) const
    {
    HPRECONDITION(pi_SubImage < m_ListOfResolution.size());

    uint32_t Width = 0;

    GetDescription(pi_SubImage, 0, &Width ,0);

    return Width;
    }

//-----------------------------------------------------------------------------
// Public
// Return the height for the specified SubImage.
//-----------------------------------------------------------------------------
uint32_t HGFResolutionDescriptor::GetHeight(unsigned short pi_SubImage) const
    {
    HPRECONDITION(pi_SubImage < m_ListOfResolution.size());

    uint32_t Height = 0;

    GetDescription(pi_SubImage, 0, 0, &Height);

    return Height;
    }

//-----------------------------------------------------------------------------
// Public
// This method adds the specified resolution and sorts the vector to have
// the resolutions in an ascending order.
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::AddResolution(double   pi_Resolution,
                                            uint32_t  pi_pWidth,
                                            uint32_t  pi_pHeight)
    {
    // Add the resolution to the list.
    ResolutionDescriptor* ResDescriptor = new ResolutionDescriptor;
    ResDescriptor->Width  = pi_pWidth;
    ResDescriptor->Height = pi_pHeight;
    m_ListOfResolution.insert(HGFListOfResolution::value_type(pi_Resolution, ResDescriptor));
    }


//-----------------------------------------------------------------------------
// Public
// This method adds the specified resolution relative to the 1.0 resolution.
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::AddResolutionRelativeToBase(double pi_Resolution,
                                                          uint32_t pi_MinWidth,
                                                          uint32_t pi_MinHeight,
                                                          uint32_t pi_ResolutionFactor)
    {
    HGFListOfResolution::const_iterator Itr(m_ListOfResolution.find(1.0));
    if (Itr != m_ListOfResolution.end())
        {
        // Create a new resolution based on dimensions of the 1.0 resolution
        ResolutionDescriptor* ResDescriptor = new ResolutionDescriptor;
        ResDescriptor->Width  = (uint32_t)ceil((double)(*Itr).second->Width * pi_Resolution);
        ResDescriptor->Height = (uint32_t)ceil((double)(*Itr).second->Height * pi_Resolution);

        // Add the resolution to the list.
        if (ResDescriptor->Width >= pi_MinWidth / pi_ResolutionFactor || ResDescriptor->Height >= pi_MinHeight / pi_ResolutionFactor)
            m_ListOfResolution.insert(HGFListOfResolution::value_type(pi_Resolution, ResDescriptor));
        }
    }


//-----------------------------------------------------------------------------
// Public
// This method adds the a resolution that is close to the specified dimensions
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::AddResolutionThatSupports(uint32_t pi_Width,
                                                        uint32_t pi_Height,
                                                        uint32_t pi_MinWidth,
                                                        uint32_t pi_MinHeight,
                                                        uint32_t pi_ResolutionFactor)
    {
    HGFListOfResolution::const_iterator Itr(m_ListOfResolution.find(1.0));
    if (Itr != m_ListOfResolution.end())
        {
        ResolutionDescriptor* ResDescriptor1 = (ResolutionDescriptor*)(*Itr).second;

        // Calculate the theoretical resolution
        double DesiredResolution = (((double)pi_Width / ResDescriptor1->Width) +
                                     ((double)pi_Height / ResDescriptor1->Height)) / 2.0;

        // Go down by increments until we fall below what's needed.
        double CurrentResolution = 1.0;
        while (CurrentResolution > DesiredResolution)
            CurrentResolution /= pi_ResolutionFactor;

        // Re-grow once so we get the first resolution that's bigger
        // or equal to the theoretical resolution
        CurrentResolution *= pi_ResolutionFactor;

        if (CurrentResolution < 1.0)
            {
            // Create a new resolution
            ResolutionDescriptor* NewResDescriptor = new ResolutionDescriptor;
            NewResDescriptor->Width  = (uint32_t)ceil((double)ResDescriptor1->Width * CurrentResolution);
            NewResDescriptor->Height = (uint32_t)ceil((double)ResDescriptor1->Height * CurrentResolution);

            // Add the resolution to the list.
            if (NewResDescriptor->Width >= pi_MinWidth / pi_ResolutionFactor || NewResDescriptor->Height >= pi_MinHeight / pi_ResolutionFactor)
                m_ListOfResolution.insert(HGFListOfResolution::value_type(CurrentResolution, NewResDescriptor));
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// This method remove the resolution information for the specified SubImage.
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::RemoveResolution(unsigned short pi_SubImage)
    {
    HPRECONDITION(pi_SubImage < m_ListOfResolution.size());

    // Search the sub-image to remove.
    HGFListOfResolution::iterator Itr = m_ListOfResolution.begin();
    for (unsigned short SubImage = (unsigned short)m_ListOfResolution.size()-1 ; SubImage != pi_SubImage ; SubImage--)
        {
        // Increment the iterator position.
        Itr++;
        }

    // Remove the sub-image information.
    delete (*Itr).second;
    m_ListOfResolution.erase(Itr);
    }

//-----------------------------------------------------------------------------
// Public
// This method remove all resolutions information.
//-----------------------------------------------------------------------------
void HGFResolutionDescriptor::RemoveResolutions()
    {
    // destroy all resolutions
    HGFListOfResolution::iterator itrFree;

    // Search the alloc block with the specified offset
    for (itrFree = m_ListOfResolution.begin(); itrFree != m_ListOfResolution.end(); itrFree++)
        delete (*itrFree).second;

    m_ListOfResolution.clear();
    }
