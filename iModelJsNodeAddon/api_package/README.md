# @bentley/imodeljs-native-platform-api

<p style="text-align: center;">
<i><mark>
<b>The native platform and its API are for the private use of imodeljs-backend only.</b>
</i></mark>
</p>

The iModel.js native library for node contains native code that is projected into JavaScript and is loaded by nodejs or electron. In fact, there are many native platform packages, one for each combination of node version and target platform that is supported. For example, @bentley/imodeljs-n_8-win32-x64 is the version of the native platform that can be used on a Windows desktop machine running 64-bit node v8.x.x. In addition, some apps may build custom versions of the native platform. This "api" package stands for them all. It has an install script to install the appropriate native platform for the platform.

The *declarations* for the native platform are defined in a d.ts file in @bentley/imodeljs-backend.

The implementation and the declaration are each written by hand and in different places. There is nothing to make this automatic or to check consistency between them.
