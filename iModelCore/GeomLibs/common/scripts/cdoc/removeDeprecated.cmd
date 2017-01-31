
@rem This script is called to delete unwanted files before building the index.
@rem (jmdlgeom.mke builds doc files en masse --- no concern for publication decisions.)
@rem Assume CDOCTARGET points at the parent directory.
del %CDOCTARGET%\geom\dellipse4d.html
del %CDOCTARGET%\geom\rename.html
del %CDOCTARGET%\geom\pencil.html
del %CDOCTARGET%\geom\bench.html
