# SQLite sources

This directory holds the SQLite amalgamation (`sqlite3.c`, `sqlite3.h`, `shell.c`) and the CloudSqlite sources (`bcv*`, `blockcachevfs*`, `simplexml*`), copied from upstream by the two update scripts below. The next update overwrites any local change to them. `bentley-sqlite.c` sets our compile options and includes the amalgamation.

SQLite and CloudSqlite are updated separately and come from different sources: SQLite from our git fork of SQLite, CloudSqlite from the sqlite.org fossil repository (see [Updating CloudSqlite](#updating-cloudsqlite)).

## Updating SQLite

SQLite comes from our fork [iTwin/sqlite](https://github.com/iTwin/sqlite). Each SQLite minor version gets a branch `itwin-sqlite/v{MAJOR}.{MINOR}.x`, and each release we consume gets a tag `itwin-sqlite-v{MAJOR}.{MINOR}.{PATCH}-r{N}`. `UpdateSqlite.bat` builds the amalgamation from such a tag, so steps 1-5 must be done and the tag pushed before running it. Steps 1-5 are plain git and make and run on Linux or macOS; step 6 needs Windows.

### 1. Sync the fork with upstream

```sh
git clone https://github.com/iTwin/sqlite.git itwin-sqlite
cd itwin-sqlite
git remote add upstream https://github.com/sqlite/sqlite.git   # if `git remote` does not list it yet
git fetch upstream
git merge --ff-only upstream/master
git push
git push --tags
```

### 2. Find the release tag

```sh
git tag --sort=-version:refname -l 'version-*' | head -5
# version-3.53.4
```

### 3. Create or update the release branch

Check whether the branch for that minor version exists:

```sh
git ls-remote --exit-code --heads origin itwin-sqlite/v3.53.x
```

If it does not exist (exit code 2), create it from the release tag:

```sh
git switch -c itwin-sqlite/v3.53.x version-3.53.4
git push --set-upstream origin itwin-sqlite/v3.53.x
```

If it exists, merge the release tag into it, resolve conflicts and push:

```sh
git checkout itwin-sqlite/v3.53.x
git merge version-3.53.4
git push
```

Fixes from SQLite trunk that are not in a release yet can be cherry-picked onto the branch (`git cherry-pick <sha>`).

### 4. Check that it builds

```sh
./configure
make sqlite3.c
```

### 5. Tag the branch

The first three numbers are the SQLite release. `r0` is the first tag for that release; increment `r` for each tag on top of the same release, for example after cherry-picking a fix. Check the latest existing tag first:

```sh
git tag --sort=-committerdate -l 'itwin-sqlite-v3.53*'
git tag -a itwin-sqlite-v3.53.4-r1 -m "itwin-sqlite v3.53.4-r1"
git push --tags
```

### 6. Update imodel-native

1. Set `sqlite_tag` in `UpdateSqlite.bat` to the new tag.
2. Run `UpdateSqlite.bat` from a Visual Studio developer prompt with `SrcRoot` set. It clones or updates iTwin/sqlite in `%APPDATA%\itwin-sqlite`, checks out the tag, runs `nmake /f Makefile.msc clean sqlite3.c` and copies `sqlite3.c`, `sqlite3.h` and `shell.c` here.
3. Check that the version in `sqlite3.h` matches the tag.
4. Commit `UpdateSqlite.bat` together with the three generated files, and name the tag in the commit message.

## Updating CloudSqlite

`UpdateCloudSqlite.bat` uses [fossil](https://fossil-scm.org/) to clone or update `https://sqlite.org/cloudsqlite` in `%APPDATA%\sqlite-cloud-src` and copies the CloudSqlite sources here. It checks out the branch named in `SqliteCloudBranch`, or `trunk` if that variable is not set.

1. Make sure `fossil` is on `PATH` and `SrcRoot` is set.
2. Optionally set `SqliteCloudBranch`, then run `UpdateCloudSqlite.bat`.
3. Commit the copied `bcv*`, `blockcachevfs*` and `simplexml*` files.

The script only runs on Windows.
