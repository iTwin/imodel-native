// PkgCmdID.cs
// MUST match PkgCmdID.h
using System;

namespace Bentley.ImageViewer
{
    //List of possible command linked to ImageDebugger application and their respective Id in visual studio
    static class PkgCmdIDList
    {
        public const uint beImageDbg =    0x101;
        public const uint beImageDbgWatch = 0x102;
        public const uint addWatch = 0x103;
        public const uint visualize = 0x104;
    };
}