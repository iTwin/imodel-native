//----------------------------------------------------------------------------
//
// leak-test.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
void  testMemoryLeak()
{
    int             vectorSize = 5000;
    std::vector<double>  pointVector;
    pointVector.resize(vectorSize*3);

    // retrieve the first and only scene
    int numScenes = ptNumScenes();		
    PThandle *handles = new PThandle[numScenes];
    ptGetSceneHandles( handles );
    PThandle podHandle = handles[0];
    delete [] handles;

    int nbOfLoops (1000000); // one million
    for (int i = 0; i < nbOfLoops; i++)
        {
        PThandle query = ptCreateVisPointsQuery ();
        ptSetQueryScope(query, podHandle);
        ptSetQueryDensity (query, 0x01, .1f);


        // will leak
        int pointsRead2 = ptGetDetailedQueryPointsd( query, vectorSize, &pointVector[0], NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL);

        // will not leak
//            int pointsRead2 = ptGetQueryPointsd( query, vectorSize, &pointVector[0], NULL, NULL, NULL, NULL);

        ptDeleteQuery(query);
        }
    }
