!! prepend copyright and insert namespace after generated comment
\B<T>\Npublic=@read{\.\\fixup\\copyright\.txt}\n$1\n@read{\.\\fixup\\topOfFileManaged\.txt}\npublic

!! close the namespace at end of file
\E=@read{\.\\fixup\\endOfFileManaged\.txt}
?=?