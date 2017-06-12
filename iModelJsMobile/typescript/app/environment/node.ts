/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/node.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Environment from "./Environment";

/**
 * An environment where Node.js is available.
 */
export interface INodeEnvironment {

}

/**
 * The environment delivered by Node.js on a server host.
 */
export class NodeEnvironment extends Environment implements INodeEnvironment {

private NodeEnvironmentTypeId;

}
