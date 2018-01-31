# hg2git
This directory contains the files neccesary for converting Bis mercurial
repositories to directories under a git monorepo.

## Conversion Process
Prerequisites:
+ Install mercurial `hg-git` extension
    + ```sh
        $ sudo apt-get install python-setuptools
        $ easy_install hg-git
        ```
+ Enable `hg-git` and other neccesary extensions in your `.hgrc`
    + ```sh
        # other .hgrc stuff...
        [extensions]
        hgext.bookmarks =
        hgext.convert =
        hggit =
        ```
+ (optional) Install the `tree` package for pretty printing of directory
structure. This is only needed if running the `basic_example` make target.
    + ```sh
        $ sudo apt-get install tree
        ```

`hg2git.py` is the main work horse for the conversion process.
It may be invoked with
```sh
$ ./hg2git.py convert <hgrepo> <gitrepo> <filemap>
```
to convert mercurial repository `<hgrepo>` to a branch on `<gitrepo>` with
branch name `$(basename <hgrepo>)`. The conversion process uses filemaps from
the [mercurial convert extension](https://www.mercurial-scm.org/wiki/ConvertExtension).
```sh
# example
echo "include ECSchemas/BisCore.ecschema.xml" > DgnPlatform.filemap
./hg2git.py convert /mnt/d/bim0200/src/DgnPlatform/ /mnt/d/bis-schemas/ ./DgnPlatform.filemap
```

---

The script may also be invoked with
```sh
$ ./hg2git.py merge <hgrepo> <gitrepo> <gitbranch>
```
to merge a branch previously created with `convert` into branch `<gitbranch>`
in the git repository.
```sh
# example
./hg2git.py merge /mnt/d/bim0200/src/DgnPlatform/ /mnt/d/bis-schemas/ "master"
```

## Example conversion
To see an example of the conversion process in action, a basic example
conversion has been set up in the `basic_conversion` directory. To see the
example in action invoke make with
```sh
$ make basic_conversion
```
from the hg2git directory. To go though the entire process step by step, invoke
the `./basic_conversion/basic.test.sh` directory directly with one of the
options specified by `./basic_conversion/basic.test.sh --help`.
