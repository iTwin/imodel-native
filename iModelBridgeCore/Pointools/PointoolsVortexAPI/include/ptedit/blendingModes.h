/* Photoshop Blending Modes */ 
/*
www.nathanm.com/photoshop-blending-math/

B is the base channel and L is the blend channel. For example you to blend using Glow you simply call:

	Target[i] = ChannelBlend_Glow(Base[i], Blend[i]);

The great thing about these macros is that you can apply the blending effect simply by passing in the channel value, without regard to which channel is red, green, or blue.

To use the blending along with opacity you can use the following.

	Target[i] = ChannelBlend_AlphaF(Base[i], Blend[i], Blend_Subtract, 0.5F)

*/
#define ChannelBlend_Normal(B,L)      ((uint8)(B))
#define ChannelBlend_Lighten(B,L)     ((uint8)((L > B) ? L:B))
#define ChannelBlend_Darken(B,L)      ((uint8)((L > B) ? B:L))
#define ChannelBlend_Multiply(B,L)    ((uint8)((B * L) / 255))
#define ChannelBlend_Average(B,L)     ((uint8)((B + L) / 2))
#define ChannelBlend_Add(B,L)         ((uint8)(min(255, (B + L))))
#define ChannelBlend_Subtract(B,L)    ((uint8)((B + L < 255) ? 0:(B + L - 255)))
#define ChannelBlend_Difference(B,L)  ((uint8)(abs(B - L)))
#define ChannelBlend_Negation(B,L)    ((uint8)(255 - abs(255 - B - L)))
#define ChannelBlend_Screen(B,L)      ((uint8)(255 - (((255 - B) * (255 - L)) >> 8)))
#define ChannelBlend_Exclusion(B,L)   ((uint8)(B + L - 2 * B * L / 255))
#define ChannelBlend_Overlay(B,L)     ((uint8)((L < 128) ? (2 * B * L / 255):(255 - 2 * (255 - B) * (255 - L) / 255)))
#define ChannelBlend_SoftLight(B,L)   ((uint8)((L < 128) ? (2 * ((B >> 1) + 64)) * (L / 255): \
                                      (255 - (2 * (255 - ((B >> 1) + 64)) * (255 - L) / 255))))
#define ChannelBlend_HardLight(B,L)   (ChannelBlend_Overlay(L,B))
#define ChannelBlend_ColorDodge(B,L)  ((uint8)((B == 255) ? B:min(255, ((L << 8 ) / (255 - B)))))
#define ChannelBlend_ColorBurn(B,L)   ((uint8)((B == 0) ? B:max(0, (255 - ((255 - L) << 8 ) / B))))
#define ChannelBlend_LinearDodge(B,L) (ChannelBlend_Add(B,L))
#define ChannelBlend_LinearBurn(B,L)  (ChannelBlend_Subtract(B,L))
#define ChannelBlend_LinearLight(B,L) ((uint8)(B < 128)? ChannelBlend_LinearBurn((2*B),L):ChannelBlend_LinearDodge((2*(B - 128)),L))
#define ChannelBlend_VividLight(B,L)  ((uint8)(B < 128)? ChannelBlend_ColorBurn((2*B),L):ChannelBlend_ColorDodge((2*(B - 128)),L))
#define ChannelBlend_PinLight(B,L)    ((uint8)(B < 128)? ChannelBlend_Darken((2 * B),L):ChannelBlend_Lighten((2 *(B - 128)),L))
#define ChannelBlend_HardMix(B,L)     ((uint8)((ChannelBlend_VividLight(B,L) < 128) ? 0:255))
#define ChannelBlend_Reflect(B,L)     ((uint8)((L == 255) ? L:min(255, (B * B / (255 - L)))))
#define ChannelBlend_Glow(B,L)        (ChannelBlend_Reflect(L,B))
#define ChannelBlend_Phoenix(B,L)     ((uint8)(min(B,L) - max(B,L) + 255))
#define ChannelBlend_Alpha(B,L,O)     ((uint8)(O * B + (1 - O) * L))
#define ChannelBlend_AlphaF(B,L,F,O)  (ChannelBlend_Alpha(F(B,L),B,O))

/*
Color blending

To add certain blend modes that utilize hue, luminosity, and saturation we have to construct a per-color interface 
instead of per-channel interface. For these macros we assume that A and B are buffer pointers and they point to bytes 
with channels red, green, and blue in that order.

*/

#define ColorBlend_Buffer(T,A,B,M)      (T)[0] = ChannelBlend_##M((A)[0], (B)[0]), \
                                        (T)[1] = ChannelBlend_##M((A)[1], (B)[1]), \
                                        (T)[2] = ChannelBlend_##M((A)[2], (B)[2])
#define ColorBlend_Normal(T,A,B)        (ColorBlend_Buffer(T,A,B,Normal))
#define ColorBlend_Lighten(T,A,B)       (ColorBlend_Buffer(T,A,B,Lighten))
#define ColorBlend_Darken(T,A,B)        (ColorBlend_Buffer(T,A,B,Darken))
#define ColorBlend_Multiply(T,A,B)      (ColorBlend_Buffer(T,A,B,Multiply))
#define ColorBlend_Average(T,A,B)       (ColorBlend_Buffer(T,A,B,Average))
#define ColorBlend_Add(T,A,B)           (ColorBlend_Buffer(T,A,B,Add))
#define ColorBlend_Subtract(T,A,B)      (ColorBlend_Buffer(T,A,B,Subtract))
#define ColorBlend_Difference(T,A,B)    (ColorBlend_Buffer(T,A,B,Difference))
#define ColorBlend_Negation(T,A,B)      (ColorBlend_Buffer(T,A,B,Negation))
#define ColorBlend_Screen(T,A,B)        (ColorBlend_Buffer(T,A,B,Screen))
#define ColorBlend_Exclusion(T,A,B)     (ColorBlend_Buffer(T,A,B,Exclusion))
#define ColorBlend_Overlay(T,A,B)       (ColorBlend_Buffer(T,A,B,Overlay))
#define ColorBlend_SoftLight(T,A,B)     (ColorBlend_Buffer(T,A,B,SoftLight))
#define ColorBlend_HardLight(T,A,B)     (ColorBlend_Buffer(T,A,B,HardLight))
#define ColorBlend_ColorDodge(T,A,B)    (ColorBlend_Buffer(T,A,B,ColorDodge))
#define ColorBlend_ColorBurn(T,A,B)     (ColorBlend_Buffer(T,A,B,ColorBurn))
#define ColorBlend_LinearDodge(T,A,B)   (ColorBlend_Buffer(T,A,B,LinearDodge))
#define ColorBlend_LinearBurn(T,A,B)    (ColorBlend_Buffer(T,A,B,LinearBurn))
#define ColorBlend_LinearLight(T,A,B)   (ColorBlend_Buffer(T,A,B,LinearLight))
#define ColorBlend_VividLight(T,A,B)    (ColorBlend_Buffer(T,A,B,VividLight))
#define ColorBlend_PinLight(T,A,B)      (ColorBlend_Buffer(T,A,B,PinLight))
#define ColorBlend_HardMix(T,A,B)       (ColorBlend_Buffer(T,A,B,HardMix))
#define ColorBlend_Reflect(T,A,B)       (ColorBlend_Buffer(T,A,B,Reflect))
#define ColorBlend_Glow(T,A,B)          (ColorBlend_Buffer(T,A,B,Glow))
#define ColorBlend_Phoenix(T,A,B)       (ColorBlend_Buffer(T,A,B,Phoenix))
#define ColorBlend_Hue(T,A,B)            ColorBlend_Hls(T,A,B,HueB,LuminationA,SaturationA)
#define ColorBlend_Saturation(T,A,B)     ColorBlend_Hls(T,A,B,HueA,LuminationA,SaturationB)
#define ColorBlend_Color(T,A,B)          ColorBlend_Hls(T,A,B,HueB,LuminationA,SaturationB)
#define ColorBlend_Luminosity(T,A,B)     ColorBlend_Hls(T,A,B,HueA,LuminationB,SaturationA)

#define ColorBlend_Hls(T,A,B,O1,O2,O3) {                                        \
    float64 HueA, LuminationA, SaturationA;                                     \
    float64 HueB, LuminationB, SaturationB;                                     \
    Color_RgbToHls((A)[0],(A)[1],(A)[2], &HueA, &LuminationA, &SaturationA);    \
    Color_RgbToHls((B)[0],(B)[1],(B)[2], &HueB, &LuminationB, &SaturationB);    \
    Color_HlsToRgb(O1,O2,O3,&(T)[0],&(T)[1],&(T)[2]);                           \
    }

int32 Color_HueToRgb(float64 Hue, float64 M1, float64 M2, float64 *Channel)
{
    if (Hue < 0.0)
        Hue += 1.0;
    if (Hue > 1.0)
        Hue -= 1.0;

    if ((6.0 * Hue) < 1.0)
        *Channel = (M1 + (M2 - M1) * Hue * 6.0);
    else if ((2.0 * Hue) < 1.0)
        *Channel = (M2);
    else if ((3.0 * Hue) < 2.0)
        *Channel = (M1 + (M2 - M1) * ((2.0 / 3.0) - Hue) * 6.0);
    else
        *Channel = (M1);

    return TRUE;
}

int32 Color_RgbToHls(uint8 Red, uint8 Green, uint8 Blue, float64 *Hue, float64 *Lumination, float64 *Saturation)
{
    float64 Delta;
    float64 Max, Min;
    float64 Redf, Greenf, Bluef;

    Redf    = (float64)Red   / 255.0F;
    Greenf  = (float64)Green / 255.0F;
    Bluef   = (float64)Blue  / 255.0F; 

    Max     = max(max(Redf, Greenf), Bluef);
    Min     = min(min(Redf, Greenf), Bluef);

    *Hue        = 0;
    *Lumination = (Max + Min) / 2.0;
    *Saturation = 0;

    if (Max == Min)
        return TRUE;

    Delta = (Max - Min);

    if (*Lumination < 0.5)
        *Saturation = Delta / (Max + Min);
    else
        *Saturation = Delta / (2.0 - Max - Min);

    if (Redf == Max)
        *Hue = (Greenf - Bluef) / Delta;
    else if (Greenf == Max)
        *Hue = (2.0 + (Bluef - Redf) / Delta);
    else
        *Hue = (4.0 + (Redf - Greenf) / Delta);

    *Hue /= 6.0;

    if (*Hue < 0.0)
        *Hue += 1.0;

    return TRUE;
}

int32 Color_HlsToRgb(float64 Hue, float64 Lumination, float64 Saturation, uint8 *Red, uint8 *Green, uint8 *Blue)
{
    float64 M1, M2;
    float64 Redf, Greenf, Bluef;

    if (Saturation == 0)
        {
        *Red    = (uint8)(Lumination * 255.0F);
        *Green  = *Red;
        *Blue   = *Red;
        }
    else
        {
        if (Lumination <= 0.5)
            M2 = (Lumination * (1.0 + Saturation));
        else
            M2 = (Lumination + Saturation) - (Lumination * Saturation);

        M1 = (2.0 * Lumination - M2);

        Color_HueToRgb(M1, M2, Hue + (1.0 / 3.0), &Redf);
        Color_HueToRgb(M1, M2, Hue, &Greenf);
        Color_HueToRgb(M1, M2, Hue - (1.0 / 3.0), &Bluef);

        *Red    = (uint8)(Redf * 255.0F);
        *Blue   = (uint8)(Bluef * 255.0F);
        *Green  = (uint8)(Greenf * 255.0F);
        }

    return TRUE;
}
