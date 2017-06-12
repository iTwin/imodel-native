/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/web/view.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import * as web from "../web";

/**
 * The environment delivered by a web content control on a mobile app host.
 */
export class WebViewEnvironment extends web.WebEnvironment {

private WebViewEnvironmentTypeId;

}

/**
 * An Android android.webkit.WebView.
 */
export class AndroidWebViewEnvironment extends WebViewEnvironment {

private AndroidWebViewEnvironmentTypeId;

}

/**
 * An iOS WKWebView.
 */
export class iOSWebViewEnvironment extends WebViewEnvironment {

private iOSWebViewEnvironmentTypeId;

}

/**
 * A Universal Windows Platform Windows.UI.Xaml.Controls.Web​View.
 */
export class UwpWebViewEnvironment extends WebViewEnvironment {

private UwpWebViewEnvironmentTypeId;

}
