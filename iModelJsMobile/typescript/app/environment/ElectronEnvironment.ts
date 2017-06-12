/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/ElectronEnvironment.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import * as web from "./web";
import * as node from "./node";

/**
 * The environment delivered by the Electron framework on a desktop app host.
 */
export default class ElectronEnvironment extends web.WebEnvironment implements node.INodeEnvironment {

private ElectronEnvironmentTypeId;

}
