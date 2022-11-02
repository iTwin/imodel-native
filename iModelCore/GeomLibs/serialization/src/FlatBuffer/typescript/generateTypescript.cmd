@REM Copyright (c) Bentley Systems, Incorporated. All rights reserved.
@REM See LICENSE.md in the repository root for full copyright notice.
@rem file roles:
@rem generateTypescript.cmd = this command file to run the whole sequence
@rem fixupTypescript.g = gema stream edit patterns to turn the javascript into typescript.
@rem topOfFile.ts = copyright notice, import directive, and cspell, lint, and doc directives.
@rem When this script is run:
@rem   ... copy ..\allcg.flatbuf to local allcg.fbs
@rem   ... flatbuffer compiler converts that to allcg_generated.ts
@rem   ... gema converts that to allcg_generated.ts.1
@rem   ... copy and type concatenate topOfFile.ts with allcg_generated.ts.1 to create allcgFlatbufferInterface.ts
@echo .
@echo .
@rem ..\allcg.fbs is the schema.  Copy it here so this and native generator directories are complete for the instant of compiler call
copy ..\allcg.fbs allcg.fbs
@rem The flatc compiler bits are at https://github.com/google/flatbuffers/releases/tag/v1.12.0
@rem the --ts and --js options both generate .ts files which are really javascript -- e.g. do not apply "public" declarations as expected.
rem  %srcRoot%imodel02\iModelCore\libsrc\flatbuffers\bin\flatcv1.12.0.exe --ts allcg.fbs
c:\bin\flatcv1.12.0.exe --ts allcg.fbs
@echo .
@echo .
@rem apply various fixups that make it compile quietly in typescript ...
set finalFileName=BGFBAccessors.ts
gema -f fixupTypescript.g allcg_generated.ts > allcg_generated.ts.1
copy topOfFile.ts %finalFileName%
type allcg_generated.ts.1 >>%finalFileName%
type endOfFile.ts >>%finalFileName%

@rem TODO:  (by you!!) manually copy %finalFileName% into the imodeljs world.




