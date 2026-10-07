# v8serial

Header-only writer vendored from https://github.com/khanaffan/v8serial at
`e5b65d4ace4a1b95a74a5eeabab798af43956b67`. The MIT license is in `vendor/LICENSE`.

Local changes: relative includes, UTF-8 object keys, and a dense-array overload
whose length is finalized by `endArray()`. Unknown-length arrays reserve a
five-byte uint32 varint so patching the count never shifts UTF-16 alignment.
Both the header and footer contain the final element count.

The ECSQL integration writes the existing rendered JSON value types, not Date,
BigInt, or binary views. Non-finite numbers remain null and blobs remain
base64-prefixed strings. The transport is opt-in through `useV8Serialization`;
the iTwin.js backend deserializes it before returning normal query responses.
