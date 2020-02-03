// Guids.cs
// MUST match guids.h
using System;

namespace Bentley.ImageViewer
{
    static class GuidList
    {
        public const string guidImageViewerPkgString = "17d5d5e0-4d45-4df2-b9bf-19fb036ba99d";
        public const string guidImageViewerCmdSetString = "35d4bc9c-e700-4862-9dd5-0e71e573e812";
        public const string guidToolWindowPersistanceString = "fde8f819-d231-4ff1-b343-d83e313cb6ef";
        public const string guidWatchWindowToolWindowPersistanceString = "fde8f819-d231-4ff1-b343-d83e313cb6ee";
        public const string guidContextMenuAddWatchAndVisualizeCmdSet = "35d4bc9c-e700-4862-9dd5-0e71e573e811";
        public static readonly Guid guidImageViewerCmdSet = new Guid(guidImageViewerCmdSetString);
        public static readonly Guid guidMenuAddWatchAndVisualizeCmdSet = new Guid(guidContextMenuAddWatchAndVisualizeCmdSet);
    };
}