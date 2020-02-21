
# Update SQLite to latest version with SEE

1. Login to private repository https://sqlite.org/see-base/info/see
2. Download one of the compress archive "Tarball".
3. Extract it into a folder.
4. Build source
   - Linux run `./configure && ./buildsrc.sh`
   - Window install cygwin and run `sh ./configure && sh buildsrc.sh`
5. Rename/override `sqlite-see.c` to `sqlite3.c`
6. Copy `sqlite3.c` and `sqlite3.h` to `%SrcRoot%imodel02\iModelCore\BeSQLite\SQLite\`
