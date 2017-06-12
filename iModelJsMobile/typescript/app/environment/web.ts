/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/web.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Environment from "./Environment";

/**
 * An environment where web technologies are available.
 */
export interface IWebEnvironment {

}

/**
 * An environment where Web Workers technology is available.
 */
export class WebWorkersEnvironment extends Environment implements IWebEnvironment {

private WebWorkersEnvironmentTypeId;

}

/**
 * An environment where web rendering technologies like HTML, CSS, and WebGL are available.
 */
export class WebEnvironment extends Environment implements IWebEnvironment {

private WebEnvironmentTypeId;

}
