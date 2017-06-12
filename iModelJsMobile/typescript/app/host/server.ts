/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/host/server.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Host from "./Host";

/**
 * An HTTP server.
 */
export class ServerHost extends Host {

private ServerHostTypeId;

}

/**
 * A Windows server.
 */
export class WindowsServerHost extends ServerHost {

private WindowsServerHostTypeId;

}

/**
 * A Linux server.
 */
export class LinuxServerHost extends ServerHost {

private LinuxServerHostTypeId;

}
