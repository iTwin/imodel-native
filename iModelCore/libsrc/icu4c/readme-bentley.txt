-----------------------------------------------------------------------------------------------------------------------------------------------------
ICU4C (International Components for Unicode for C)
http://site.icu-project.org/

-----------------------------------------------------------------------------------------------------------------------------------------------------
Version 61.1, Downloaded 2018-05-14

* Exporting from source is the recommend way to build the source; release bundles do not include the same sub-set of data files.
svn export http://source.icu-project.org/repos/icu/tags/release-61-1

-----------------------------------------------------------------------------------------------------------------------------------------------------
Configure Notes:
> Even though this seems nice and is recommended by the readme, the build will fail if you set them:
    -DUNISTR_FROM_CHAR_EXPLICIT=explicit -DUNISTR_FROM_STRING_EXPLICIT=explicit.

-----------------------------------------------------------------------------------------------------------------------------------------------------
ICU Data Notes:

When we first adopted ICU, we built its data into the library itself. This required anyone building the library to also build the data files.
While we produced and committed binaries for such purposes and it was functional, the processing time of data files is excessive with Microsoft's tools (i.e. the ml.exe/ml64.exe assembler to produce the .obj file).
For this reason, we are shifting to producing a platform-independent data file once and committing that.
While this will reduce routine build times and complexity, it has a run-time performance penalty (read from file vs. a pre-processed in-memory blob), and every user/host must manually deploy and resolve the data file at run-time.

-----------------------------------------------------------------------------------------------------------------------------------------------------
Build System Porting Notes:


-----------------------------------------------------------------------------------------------------------------------------------------------------
To Regenerate the Data File:


-----------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------
To use their build system on/for MacOS 10.9.3 + XCode 5.1.1 (useful for seeing what their official build does to port to our own):

* XCode command line tools were previously installed
* The following homebrew packages were previously installed: boost dos2unix gettext git libtool mercurial multimarkdown ninja ragel

> cd into .../source directory
> chmod u+x runConfigureICU configure mkinstalldirs
> dos2unix -f runConfigureICU config.guess config.sub configure configure.in mkinstalldirs
> grep -rl --include="Makefile.in" "cd \$\$subdir" . | xargs sed -i "" -e "s|cd \$\$subdir|cd \`pwd\`/\$\$subdir|g"

> cd into an output directory
> (Update flags and paths in the following command as desired)
  CFLAGS="-DU_USING_ICU_NAMESPACE=0 -DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1" CXXFLAGS="$CFLAGS --std=c++0x" ../source/runConfigureICU --enable-debug --disable-release MacOSX --enable-static --disable-shared --prefix=/Users/jeff/Graphite/graphite04-out/icu4c --enable-debug
> gnumake VERBOSE=1
