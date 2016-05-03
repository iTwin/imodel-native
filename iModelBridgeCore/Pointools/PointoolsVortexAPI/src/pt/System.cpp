/** 
  @file System.cpp
 
  @maintainer Morgan McGuire, matrix@graphics3d.com

  Note: every routine must call init() first.

  There are two kinds of detection used in this file.  At compile
  time, the _MSC_VER #define is used to determine whether x86 assembly
  can be used at all.  At runtime, processor detection is used to
  determine if we can safely call the routines that use that assembly.

  @cite Rob Wyatt http://www.gamasutra.com/features/wyatts_world/19990709/processor_detection_01.htm
  @cite Benjamin Jurke http://www.flipcode.com/cgi-bin/msg.cgi?showThread=COTD-ProcessorDetectionClass&forum=cotd&id=-1
  @cite Michael Herf http://www.stereopsis.com/memcpy.html

  @created 2003-01-25
  @edited  2004-08-24

  @feb 07 - unicode support added by Faraz Ravi
 */

#include "PointoolsVortexAPIInternal.h"
#include <pt/System.h>
#include <pt/debug.h>
#include <pt/pterror.h>
#include <pt/units.h>

static bool isPow2(int num) { return ((num & -num) == num); }
static int iMin(int x, int y) { return x < y ? x : y; }
static int iMax(int x, int y) { return x > y ? x : y; }

#ifdef __INTEL_COMPILER
#include <mathimf.h> //not platform independant
#else
#include <math.h>
#endif

#define RealTime double

#ifdef _WIN32

    #include <conio.h>
    #include <sys/timeb.h>

#elif defined(__linux__) 

    #include <stdlib.h>
    #include <stdio.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <stropts.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <sys/time.h>

#elif defined(__APPLE__)

    #include <stdlib.h>
    #include <stdio.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
    #include <sys/time.h>

    #include <sstream>
    #include <CoreServices/CoreServices.h>
#endif

namespace pt {

static bool                                     _rdtsc              = false;
static bool                                     _mmx                = false;
static bool                                     _sse                = false;
static bool                                     _sse2                   = false;
static bool                                     _3dnow              = false;
static std::string                      _cpuVendor          = "Unknown";
//static bool                                   initialized         = false;
bool System::initialized = false;
static bool                                     _cpuID              = false;
static PTEndian            _machineEndian      = PT_LITTLE_ENDIAN;
static std::string          _cpuArch            = "Unknown";
static std::string          _operatingSystem    = "Unknown";

#ifdef _WIN32
/** Used by getTick() for timing */
static LARGE_INTEGER        _start;
static LARGE_INTEGER        _counterFrequency;
#else
static struct timeval       _start;
#endif

#ifdef __APPLE__
    long System::m_OSXCPUSpeed;
    double System:: m_secondsPerNS;
#endif

/** The Real-World time of System::getTick() time 0.  Set by initTime */
static RealTime             realWorldGetTickTime0;


static int               maxSupportedCPUIDLevel = 0;
static int            maxSupportedExtendedLevel = 0;

#define checkBit(var, bit)   ((var & (1 << bit)) ? true : false)

/** Checks if the CPUID command is available on the processor (called from init) */
static void checkForCPUID();

/** ReadRead the standard processor extensions.  Called from init(). */
static void getStandardProcessorExtensions();

/** Perform processor identification and initialize the library (if not
    already initialized). */
static void init();

/** Called from init */
static void initTime();


bool System::hasRDTSC() {
    init();
    return _rdtsc;
}


bool System::hasSSE() {
    init();
    return _sse;
}


bool System::hasSSE2() {
    init();
    return _sse2;
}


bool System::hasMMX() {
    init();
    return _mmx;
}


bool System::has3DNow() {
    init();
    return _3dnow;
}


const std::string& System::cpuVendor() {
    init();
    return _cpuVendor;
}


PTEndian System::machineEndian() {
    init();
    return _machineEndian;
}

const std::string& System::operatingSystem() {
    init();
    return _operatingSystem;
}
        

const std::string& System::cpuArchitecture() {
    init();
    return _cpuArch;
}


void System::init() {

    if (System::initialized) {
        return;
    }

    System::initialized = true;

    //unsigned long eaxreg, ebxreg, ecxreg, edxreg;

    char cpuVendorTmp[13];
    (void)cpuVendorTmp;
 
        // First of all we check if the CPUID command is available
        checkForCPUID();

    // Figure out if this machine is little or big endian.
    {
        int32 a = 1;
        if (*(uint8*)&a == 1) {
            _machineEndian = PT_LITTLE_ENDIAN;
        } else {
            _machineEndian = PT_BIG_ENDIAN;
        }
    }

    if (_cpuID) {
#ifndef _WIN64
    // Process the CPUID information

        // We read the standard CPUID level 0x00000000 which should
        // be available on every x86 processor.  This fills out
    // a tstring with the processor vendor tag.
        #ifdef _MSC_VER
            __asm {
                mov eax, 0
                cpuid
                mov eaxreg, eax
                mov ebxreg, ebx
                mov edxreg, edx
                mov ecxreg, ecx
            }
        #elif defined(__GNUC__) && defined(i386)
            // TODO: linux
            ebxreg = 0;
            edxreg = 0;
            ecxreg = 0;
        #else
            ebxreg = 0;
            edxreg = 0;
            ecxreg = 0;
        #endif

        // Then we connect the single register values to the vendor tstring
        *((unsigned long *) cpuVendorTmp)       = ebxreg;
        *((unsigned long *) (cpuVendorTmp + 4)) = edxreg;
        *((unsigned long *) (cpuVendorTmp + 8)) = ecxreg;
        cpuVendorTmp[12] = '\0';
        _cpuVendor = cpuVendorTmp;

        // We can also read the max. supported standard CPUID level
        maxSupportedCPUIDLevel = eaxreg & 0xFFFF;

        // Then we read the ext. CPUID level 0x80000000
        #ifdef _MSC_VER
            __asm {
                mov eax, 0x80000000
                cpuid
                mov eaxreg, eax
            }
        #elif defined(__GNUC__) && defined(i386)
            // TODO: Linux
            eaxreg = 0;
        #else
            eaxreg = 0;
        #endif

        // ...to check the max. supported extended CPUID level
        maxSupportedExtendedLevel = eaxreg;

        // Then we switch to the specific processor vendors.
        // Fill out _cpuArch based on this information.  It will
        // be overwritten by the next block of code on Windows,
        // but on Linux will stand.
        switch (ebxreg) {
        case 0x756E6547:        // GenuineIntel
            _cpuArch = "Intel Processor";
            break;
            
        case 0x68747541:        // AuthenticAMD
            _cpuArch = "AMD Processor";
            break;

        case 0x69727943:        // CyrixInstead
            _cpuArch = "Cyrix Processor";
            break;

        default:
            _cpuArch = "Unknown Processor Vendor";
            break;
        }
#endif
	}
    #ifdef _WIN32
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        char* arch;
        switch (systemInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            arch = "Intel";
            break;
    
        case PROCESSOR_ARCHITECTURE_MIPS:
            arch = "MIPS";
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            arch = "Alpha";
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            arch = "Power PC";
            break;

        default:
            arch = "Unknown";
        }

        uint64 maxAddr = (uint64)systemInfo.lpMaximumApplicationAddress;
        _cpuArch = format(
                    "%d x %d-bit %s processor",
                    systemInfo.dwNumberOfProcessors,
                    (int)(log((double)maxAddr) / log(2.0) + 2.0),
                    arch);

        OSVERSIONINFO osVersionInfo;
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        bool success = GetVersionEx(&osVersionInfo) != 0;

        if (success) {
            _operatingSystem = format("Windows %d.%d build %d Platform %d %s",
                osVersionInfo.dwMajorVersion, 
                osVersionInfo.dwMinorVersion,
                osVersionInfo.dwBuildNumber,
                osVersionInfo.dwPlatformId,
                osVersionInfo.szCSDVersion);
        } else {
            _operatingSystem = "Windows";
        }
    
    #elif defined(__linux__)

        {
            // Shell out to the 'uname' command

            FILE* f = popen("uname -a", "r");

            int len = 100;
            char* r = (char*)malloc(len * sizeof(char));
            fgets(r, len, f);
            // Remove trailing newline
            if (r[strlen(r) - 1] == '\n') {
                r[strlen(r) - 1] = '\0';
            }
            fclose(f);

            _operatingSystem = r;
            free(r);
        }

    #elif defined(__APPLE__)

        //Operating System:
        SInt32 macVersion;
        Gestalt(gestaltSystemVersion, &macVersion);
        
        int major = 10;
        int minor = (macVersion >> 4) & 0xF;
        int revision = macVersion & 0xF;
        
        std::ostringstream ss;
        ss << "OS X " << major << "." << minor << "." << revision;
        _operatingSystem = ss.str();
        
        //Clock Cycle Timing Information:
        Gestalt('pclk', &System::m_OSXCPUSpeed);
        m_secondsPerNS = 1.0 / 1.0e9;
        
        //System Architecture:
        SInt32 CPUtype;
        Gestalt('cpuf', &CPUtype);
        switch (CPUtype){
        case 0x0108:
            _cpuArch = "PPC G3";
            _cpuVendor = "Motorola";
            break;
        case 0x010C:
            _cpuArch = "PPC G4";
            _cpuVendor = "Motorola";
            break;
        case 0x0139:
            _cpuArch = "PPC G5";
            _cpuVendor = "IBM";
            break;
        }
            
    #endif

    initTime();

    getStandardProcessorExtensions();
}


void checkForCPUID() {
        //unsigned long bitChanged;

        // We've to check if we can toggle the flag register bit 21.
        // If we can't the processor does not support the CPUID command.
#ifndef _WIN64
        #ifdef _MSC_VER
                __asm {
                        pushfd
                        pop   eax
                        mov   ebx, eax
                        xor   eax, 0x00200000 
                        push  eax
                        popfd
                        pushfd
                        pop   eax
                        xor   eax, ebx 
                        mov   bitChanged, eax
                }

        #elif defined(__GNUC__) && defined(i386)
        // Linux
        int has_CPUID = 0;
        __asm__ (
"push %%ecx\n"
"        pushfl                      # Get original EFLAGS             \n"
"        popl    %%eax                                                 \n"
"        movl    %%eax,%%ecx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
"        popfl                       # Replace current EFLAGS value    \n"
"        pushfl                      # Get new EFLAGS                  \n"
"        popl    %%eax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
"pop %%ecx\n"
        : "=r" (has_CPUID)
        :
        : "%eax", "%ecx"
        );
        _cpuID = (has_CPUID != 0);

    #else               
                // Unknown architecture
                _cpuID = false;
        
        #endif

        _cpuID = ((bitChanged) ? true : false);
#endif
}


void getStandardProcessorExtensions() {
    if (! _cpuID) {
        return;
    }

        unsigned long features;

    // Invoking CPUID with '1' in EAX fills out edx with a bit tstring.
    // The bits of this value indicate the presence or absence of 
    // useful processor features.
    #ifdef _MSC_VER
        // Windows
#ifndef _WIN64
            __asm {
            push eax
            push ebx
            push ecx
            push edx
                    mov eax, 1
                    cpuid
                    mov features, edx
            pop edx
            pop ecx
            pop ebx
            pop eax
            }

    #elif defined(__GNUC__) && defined(i386)
        // Linux
        __asm__ (
"push %%eax\n"
"push %%ebx\n"
"push %%ecx\n"
"push %%edx\n"
"        xorl    %%eax,%%eax                                           \n"
"        incl    %%eax                                                 \n"
"        cpuid                       # Get family/model/stepping/features\n"
"        movl    %%edx,%0                                              \n"
"pop %%edx\n"
"pop %%ecx\n"
"pop %%ebx\n"
"pop %%eax\n"
        : "=r" (features)
        :
        : "%eax", "%ebx", "%ecx", "%edx"
        );

    #else
        // Other
        features = 0;
    #endif
    
        // FPU_FloatingPointUnit                                                        = checkBit(features, 0);
        // VME_Virtual8086ModeEnhancements                                      = checkBit(features, 1);
        // DE_DebuggingExtensions                                                       = checkBit(features, 2);
        // PSE_PageSizeExtensions                                                       = checkBit(features, 3);
        // TSC_TimeStampCounter                                                         = checkBit(features, 4);
        // MSR_ModelSpecificRegisters                                           = checkBit(features, 5);
        // PAE_PhysicalAddressExtension                                         = checkBit(features, 6);
        // MCE_MachineCheckException                                            = checkBit(features, 7);
        // CX8_COMPXCHG8B_Instruction                                           = checkBit(features, 8);
        // APIC_AdvancedProgrammableInterruptController         = checkBit(features, 9);
        // APIC_ID                                                                                      = (ebxreg >> 24) & 0xFF;
        // SEP_FastSystemCall                                                           = checkBit(features, 11);
        // MTRR_MemoryTypeRangeRegisters                                        = checkBit(features, 12);
        // PGE_PTE_GlobalFlag                                                           = checkBit(features, 13);
        // MCA_MachineCheckArchitecture                                         = checkBit(features, 14);
        // CMOV_ConditionalMoveAndCompareInstructions           = checkBit(features, 15);

    // (According to SDL)
        _rdtsc                                                                  = checkBit(features, 16);

        // PSE36_36bitPageSizeExtension                                         = checkBit(features, 17);
        // PN_ProcessorSerialNumber                                                     = checkBit(features, 18);
        // CLFSH_CFLUSH_Instruction                                                     = checkBit(features, 19);
        // CLFLUSH_InstructionCacheLineSize                                     = (ebxreg >> 8) & 0xFF;
        // DS_DebugStore                                                                        = checkBit(features, 21);
        // ACPI_ThermalMonitorAndClockControl                           = checkBit(features, 22);
        _mmx                                                                                            = checkBit(features, 23);
        // FXSR_FastStreamingSIMD_ExtensionsSaveRestore         = checkBit(features, 24);
        _sse                                                                                            = checkBit(features, 25);
        _sse2                                                                                           = checkBit(features, 26);
        // SS_SelfSnoop                                                                         = checkBit(features, 27);
        // HT_HyperThreading                                                            = checkBit(features, 28);
        // HT_HyterThreadingSiblings = (ebxreg >> 16) & 0xFF;
        // TM_ThermalMonitor                                                            = checkBit(features, 29);
        // IA64_Intel64BitArchitecture                                          = checkBit(features, 30);
        _3dnow                                              = checkBit(features, 31);
#endif
}


#undef checkBit



/** Michael Herf's fast memcpy */
#if defined(_WIN32) && defined(SSE)

// On x86 processors, use MMX
void memcpy2(void *dst, const void *src, int nbytes) {
        int remainingBytes = nbytes;

        if (nbytes > 64) {
                _asm { 
                        mov esi, src 
                        mov edi, dst 
                        mov ecx, nbytes 
                        shr ecx, 6 // 64 bytes per iteration 

        loop1: 
                        movq mm1,  0[ESI] // Read in source data 
                        movq mm2,  8[ESI]
                        movq mm3, 16[ESI]
                        movq mm4, 24[ESI] 
                        movq mm5, 32[ESI]
                        movq mm6, 40[ESI]
                        movq mm7, 48[ESI]
                        movq mm0, 56[ESI]

                        movntq  0[EDI], mm1 // Non-temporal stores 
                        movntq  8[EDI], mm2 
                        movntq 16[EDI], mm3 
                        movntq 24[EDI], mm4 
                        movntq 32[EDI], mm5 
                        movntq 40[EDI], mm6 
                        movntq 48[EDI], mm7 
                        movntq 56[EDI], mm0 

                        add esi, 64 
                        add edi, 64 
                        dec ecx 
                        jnz loop1 

                        emms
                }
                remainingBytes -= ((nbytes >> 6) << 6); 
        }

        if (remainingBytes > 0) {
                // Memcpy the rest
                memcpy((uint8*)dst + (nbytes - remainingBytes), (const uint8*)src + (nbytes - remainingBytes), remainingBytes); 
        }
}

#else
    // Fall back to memcpy
    void memcpy2(void *dst, const void *src, int nbytes) {
            memcpy(dst, src, nbytes);
    }
#endif


void System::memcpy(void* dst, const void* src, size_t numBytes) {
        if (System::hasSSE() && System::hasMMX()) {
                pt::memcpy2(dst, src, numBytes);
        } else {
                ::memcpy(dst, src, numBytes);
        }
}


/** Michael Herf's fastest memset. n32 must be filled with the same
    character repeated. */
#if defined(_WIN32) && defined(SSE)

// On x86 processors, use MMX
void memfill(void *dst, int n32, unsigned long i) {

    int originalSize = i;
    int bytesRemaining = i;

        if (i > 16) {
        
        bytesRemaining = i % 16;
        i -= bytesRemaining;
                __asm {
                        movq mm0, n32
                        punpckldq mm0, mm0
                        mov edi, dst

                loopwrite:

                        movntq 0[edi], mm0
                        movntq 8[edi], mm0

                        add edi, 16
                        sub i, 16
                        jg loopwrite

                        emms
                }
        }

        if (bytesRemaining > 0) {
                memset((uint8*)dst + (originalSize - bytesRemaining), n32, bytesRemaining); 
        }
}

#else

// For non x86 processors, we fall back to the standard memset
void memfill(void *dst, int n32, unsigned long i) {
        memset(dst, n32, i);
}

#endif


void System::memset(void* dst, uint8 value, size_t numBytes) {
        if (System::hasSSE() && System::hasMMX()) {
                uint32 v = value;
                v = v + (v << 8) + (v << 16) + (v << 24); 
                pt::memfill(dst, v, numBytes);
        } else {
                ::memset(dst, value, numBytes);
        }
}


std::wstring System::currentProgramFilename() {
    wchar_t filename[2048];

    #ifdef _WIN32
    {
        GetModuleFileNameW(NULL, filename, sizeof(filename));
    } 
    #else
    {
            int ret = readlink("/proc/self/exe", filename, sizeof(filename));
            
            // In case of an error, leave the handling up to the caller
        if (ret == -1) {
                    return "";
        }
            
        debugAssert(sizeof(filename) > ret);
            
            // Ensure proper NULL termination
            filename[ret] = 0;      
    }
    #endif

    return filename;
}


void System::sleep(RealTime t) {
    RealTime now = time();
    RealTime wakeupTime = now + t;

    RealTime remainingTime = wakeupTime - now;

    while (remainingTime > 0) {
        
        // Default of 0 sleep time causes the program to yield only
        // the current time slice, then return.
        RealTime sleepTime = 0;

        if (remainingTime > 0.002) {
            // Safe to use sleep
            sleepTime = max(remainingTime * .5, 0.002);
        }

        #ifdef _WIN32
            Sleep((int)(sleepTime * 1e3));
        #else
            usleep((int)(sleepTime * 1e6));
        #endif

        now = time();
        remainingTime = wakeupTime - now;
    }
}


void System::consoleClearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}


bool System::consoleKeyPressed() {
    #ifdef _WIN32
    
        return _kbhit() != 0;

    #else
    
        static const int STDIN = 0;
        static bool initialized = false;

        if (! initialized) {
            // Use termios to turn off line buffering
            termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, NULL);
            initialized = true;
        }

        #ifdef __linux__

            int bytesWaiting;
            ioctl(STDIN, FIONREAD, &bytesWaiting);
            return bytesWaiting;

        #else

            timeval timeout;
            fd_set rdset;

            FD_ZERO(&rdset);
            FD_SET(STDIN, &rdset);
            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;

            return select(STDIN + 1, &rdset, NULL, NULL, &timeout);
        #endif
    #endif
}


int System::consoleReadKey() {
    #ifdef _WIN32
        return _getch();
    #else
        char c;
        read(0, &c, 1);
        return c;
    #endif
}


void initTime() {
    #ifdef _WIN32
        if (QueryPerformanceFrequency(&_counterFrequency)) {
            QueryPerformanceCounter(&_start);
        }

        struct _timeb t;
        _ftime(&t);

        realWorldGetTickTime0 = (RealTime)t.time - t.timezone * MINUTE + (t.dstflag ? HOUR : 0);

    #else
        gettimeofday(&_start, NULL);
        // "sse" = "seconds since epoch".  The time
        // function returns the seconds since the epoch
        // GMT (perhaps more correctly called UTC). 
        time_t gmt = time(NULL);
        
        // No call to free or delete is needed, but subsequent
        // calls to asctime, ctime, mktime, etc. might overwrite
        // local_time_vals. 
        tm* localTimeVals = localtime(&gmt);
    
        time_t local = gmt;
        
        if (localTimeVals) {
            // tm_gmtoff is already corrected for daylight savings.
            local = local + localTimeVals->tm_gmtoff;
        }
        
        realWorldGetTickTime0 = local;
    #endif
}


RealTime System::getTick() { 
    init();
    #ifdef _WIN32
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        return (RealTime)(now.QuadPart - _start.QuadPart) /
                _counterFrequency.QuadPart;
    #else
        // Linux resolution defaults to 100Hz.
        // There is no need to do a separate RDTSC call as gettimeofday
        // actually uses RDTSC when on systems that support it, otherwise
        // it uses the system clock.
        struct timeval now;
        gettimeofday(&now, NULL);

        return (now.tv_sec  - _start.tv_sec) +
               (now.tv_usec - _start.tv_usec) / 1e6;
    #endif
}


RealTime System::getLocalTime() {
    return getTick() + realWorldGetTickTime0;
}


void* System::alignedMalloc(size_t bytes, size_t alignment) {
    alwaysAssertM(isPow2(alignment), _T("alignment must be a power of 2"));

    // We must align to at least a word boundary.
    alignment = iMax(alignment, sizeof(void *));

    // Pad the allocation size with the alignment size and the
    // size of the redirect pointer.
    size_t totalBytes = bytes + alignment + sizeof(void*);

    size_t truePtr = (size_t)malloc(totalBytes);

    if (truePtr == 0) {
        // malloc returned NULL
        return NULL;
    }

    debugAssert(isValidHeapPointer((void*)truePtr));
    #ifdef _WIN32
        debugAssert( _CrtIsValidPointer((void*)truePtr, totalBytes, TRUE) );
    #endif

    // The return pointer will be the next aligned location (we must at least
    // leave space for the redirect pointer, however).
    size_t  alignedPtr = truePtr + sizeof(void*);

    // 2^n - 1 has the form 1111... in binary.
    uint32 bitMask = (alignment - 1);

    // Advance forward until we reach an aligned location.
    while ((alignedPtr & bitMask) != 0) {
        alignedPtr += sizeof(void*);
    }

    debugAssert(alignedPtr - truePtr + bytes <= totalBytes);

    // Immediately before the aligned location, write the true array location
    // so that we can free it correctly.
    size_t* redirectPtr = (size_t *)(alignedPtr - sizeof(void *));
    redirectPtr[0] = truePtr;

    debugAssert(isValidHeapPointer((void*)truePtr));

    #ifdef _WIN32
        debugAssert( _CrtIsValidPointer((void*)alignedPtr, bytes, TRUE) );
    #endif
    return (void *)alignedPtr;
}


void System::alignedFree(void* _ptr) {
    if (_ptr == NULL) {
        return;
    }

    size_t alignedPtr = (size_t)_ptr;

    // Back up one word from the pointer the user passed in.
    // We now have a pointer to a pointer to the true start
    // of the memory block.
    size_t* redirectPtr = (size_t*)(alignedPtr - sizeof(void *));

    // Dereference that pointer so that ptr = true start
    void* truePtr = (void*)redirectPtr[0];

    debugAssert(isValidHeapPointer((void*)truePtr));
    free(truePtr);
}


void System::setEnv(const std::wstring& name, const std::wstring& value) {
    #ifdef _WIN32
        std::wstring cmd = name + L"=" + value;
        _wputenv(name.c_str());
    #else
        setenv(name.c_str(), value.c_str(), 1);
    #endif
}


}  // namespace
