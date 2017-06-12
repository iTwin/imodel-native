/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/host/mobile.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import InteractiveHost from "./InteractiveHost";

/**
 * A mobile application.
 */
export class MobileAppHost extends InteractiveHost {

private MobileAppHostTypeId;

}

/**
 * An iOS application.
 */
export class iOSAppHost extends MobileAppHost {

private iOSAppHostTypeId;

}

/**
 * An Android application.
 */
export class AndroidAppHost extends MobileAppHost {

private AndroidAppHostTypeId;

}

/**
 * A Universal Windows Platform application.
 */
export class UwpAppHost extends MobileAppHost {

private UwpAppHostTypeId;

}
