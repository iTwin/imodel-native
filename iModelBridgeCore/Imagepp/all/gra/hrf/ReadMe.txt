The following files should be modified appropriatly when the support of a new file format is added:
   - imagepp\all\h\HRFFileFormats.h
   - msj\mstn\library\rasterlib\rastercore\HIEFileTypeContainer.cpp
   - mstn\pubinc\image.h
   - msj\mstn\mscore\imagelib\imagutil.c
   - msj\mstn\mscore\imagelib\english\imagemsg.r
   - msj\mstn\library\rasterext\rlUtilRaster.c

Only those formats supported in batch file mode:
   - msj\mstn\mdlapps\rastbatch\rcnvpars.r

If the new file type contain DEM(Data Elevation Model) data, you will also need to update:
   - rasterapps\applications\dcartes\mdlapps\dcartes\dcserver\main\dcmain.c "cmd_IMGMANAGER_SURFACE_ATTACH" function


Then, to ensure that modifications are taken into account:
   - Rebuild rasterlib
   - Rebuild rasterext
   - Rebuild imageLib (You should call bmake msj.mke or copy manually \mstn\pubinc\image.h into
                       \out\Debug\PowerPlatform\SDK\Delivery\PseudoStation\mdl\include before
                       rebuilding imagelib)
   - Rebuild dcartes (only if dcmain.c was modified)


The following documentation files and directories should also be updated/created:
   - IppDocs\docs\Technical\Raster Files\FileFormatsSpecSheets.doc
   - IppDocs\docs\Technical\Raster Files\[File Format Name]\


For the file format label, take it from one of the following source (i.e. : if the file
format cannot be found using the first source, use the second source) :
   1) Adobe Photoshop
   2) CorelDraw PaintShop
   3) http://filext.com/
   4) GlobalMapper
   5) ...
