!! prepend copyright and insert namespace after first #include
\B<T>\N\#include<T>\N=@read{\.\\fixup\\copyright\.txt}\n$0\n\n@read{\.\\fixup\\topOfFileNative\.txt}

!! close the namespace before final #endif
\N\#endif<T>\E=@read{\.\\fixup\\endOfFileNative\.txt}\n$0
?=?