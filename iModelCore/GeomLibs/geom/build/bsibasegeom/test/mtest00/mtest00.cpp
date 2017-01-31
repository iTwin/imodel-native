//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <Geom/GeomApi.h>
#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>



int main(int argc, char * argv[])
    {
    initErrorTracking (NULL, argc, argv);
    EmbeddedIntArray *pArray0, *pArray1;

    
    int numCached = 1;
    for (;;)
        {
        pArray0 = jmdlEmbeddedIntArray_new ();
        // See if it gets saved on drop ...
        jmdlEmbeddedIntArray_drop (pArray0);
        pArray1 = jmdlEmbeddedIntArray_grab ();
        if (pArray0 == pArray1)
            {
            // send it back ..
            jmdlEmbeddedIntArray_drop (pArray1);
            numCached++;
            }
        else
            {
            break;
            }
        }

    printf ("Apparent cache size %d\n", numCached);
    return getExitStatus();
    }
