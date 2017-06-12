/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/host/desktop.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import InteractiveHost from "./InteractiveHost";

/**
 * A desktop application.
 */
export class DesktopAppHost extends InteractiveHost {

private DesktopAppHostTypeId;

}

/**
 * A Windows application.
 */
export class WindowsDesktopAppHost extends DesktopAppHost {

private WindowsDesktopAppHostTypeId;

}

/**
 * A Mac application.
 */
export class MacDesktopAppHost extends DesktopAppHost {

private MacDesktopAppHostTypeId;

}
