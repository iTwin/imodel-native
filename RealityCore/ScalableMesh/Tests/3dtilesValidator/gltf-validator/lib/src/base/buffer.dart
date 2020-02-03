/*
 * # Copyright (c) 2016 The Khronos Group Inc.
 * # Copyright (c) 2016 Alexey Knyazev
 * #
 * # Licensed under the Apache License, Version 2.0 (the "License");
 * # you may not use this file except in compliance with the License.
 * # You may obtain a copy of the License at
 * #
 * #     http://www.apache.org/licenses/LICENSE-2.0
 * #
 * # Unless required by applicable law or agreed to in writing, software
 * # distributed under the License is distributed on an "AS IS" BASIS,
 * # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * # See the License for the specific language governing permissions and
 * # limitations under the License.
 */

library gltf.core.buffer;

import 'gltf_property.dart';

class Buffer extends GltfChildOfRootProperty {
  static const String ARRAYBUFFER = "arraybuffer";

  final Uri uri;
  final List<int> data;
  final int byteLength;
  final String type;

  Buffer._(this.uri, this.data, this.byteLength, this.type, String name,
      Map<String, Object> extensions, Object extras)
      : super(name, extensions, extras);

  String toString([_]) =>
      super.toString({URI: uri, BYTE_LENGTH: byteLength, TYPE: type});

  static Buffer fromMap(Map<String, Object> map, Context context) {
    if (context.validate) checkMembers(map, BUFFER_MEMBERS, context);

    const List<String> typesEnum = const <String>[ARRAYBUFFER];

    var byteLength = getInt(map, BYTE_LENGTH, context, min: 0, req: true);

    final uriString = getString(map, URI, context, req: false);

    Uri uri;
    List<int> data;

    if (uriString != null) {
      if (uriString.startsWith("data:")) {
        try {
          final uriData = UriData.parse(uriString);
          if (uriData.mimeType == "application/octet-stream") {
            data = uriData.contentAsBytes();
          } else if (uriData.contentText.isNotEmpty) {
            // Stub for an empty data URI
            context.addIssue(GltfError.INVALID_DATA_URI_MIME,
                name: URI, args: [uriData.mimeType]);
          }
        } on FormatException catch (e) {
          context.addIssue(GltfError.INVALID_DATA_URI, name: URI, args: [e]);
        }
      } else {
        uri = parseUri(uriString, context);
      }

      if (data != null && data.length > 0 && data.length != byteLength) {
        context.addIssue(GltfWarning.BUFFER_EMBEDDED_BYTELENGTH_MISMATCH,
            args: [byteLength, data.length], name: BYTE_LENGTH);
        byteLength = data.length;
      }
    }

    return new Buffer._(
        uri,
        data,
        byteLength,
        getString(map, TYPE, context, list: typesEnum, def: ARRAYBUFFER),
        getName(map, context),
        getExtensions(map, Buffer, context),
        getExtras(map));
  }
}
