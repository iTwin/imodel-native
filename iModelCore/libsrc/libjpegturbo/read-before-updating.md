# Notes Before Updating

As of updating to 3.1.2 the source code file structure changed causing some files to move around. Noteably all cpp files are now in the src dir.
This messes with some files, and the expected locations of some .h files.

To get a working build:
1. Follow 3rd party wiki as normal until merging with libsrc-Main
2. Resolve conflicts, and accept current, or combination of incoming and current when needed
  - it is important that mentions of .in files in the include statements stay, as we will keep the .in files
3. Open a copy libjpegturbo in another instance of VS code or Visual Studio (whichever has access to make)
4. On a windows machine, or with windows configs, run `make Configure`
  - this will configure the following .in files
    - jconfig.h
    - jversion.h
    - jconfigint.h
5. Go back to your imodel-native libsrc-Main branch
6. copy the newly configured jversion.h into `libjpegturbo/vendor/jversion.h`. Only do this for jversion.h
7. Find the three .h files mentioned above and in `libjpegturbo/vendor/` and copy them into `src/`
  - I suggest copying and not moving so we don't loose these files in the future
  - you should notice that the jconfig .h only points to other .in files. This is ok, and why we don't want to accidently ovewrite these 2 config files
8. Copy the contents of jconfig.h and jconfigint.h, from your configured copy, into your the .in versions of these files in `libjpegturbo/vendor/src/.h.in`
  - this basically turns the .in files into proper header files. I'm not sure why this is how we configure this code before, but I tried to keep it working the same way
9. After this I still ran into build errors, however, I used co pilot to help resolve them. All the remaining build errors came from issues in the .mke file usually regarding not fixing missing symbols. Once all missing symbols were resolved and all the correct cpp files were compiled I got a succesful build.
10. After a successful build you can go back to the original update wiki to finish.
