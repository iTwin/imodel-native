#include <Geom\GeomApi.h>
#include <Bentley\BeFileName.h>
#include <Bentley\BeFile.h>
#include <GeomSerialization\GeomSerializationApi.h>

extern "C"
void main(int argc, char **argv) 
    {
    Utf8String filestr0(argv[1]);
    Utf8String filestr1(argv[2]);
    BeFileName path(filestr0);
    BeFileName path2(filestr1);
    BeFile file, file2;
    file.Create(path.c_str(), false);
    file2.Create(path2.c_str(), false);
    if (!(BeFileName::DoesPathExist(path2.c_str()) && BeFileName::DoesPathExist(path.c_str()))) 
        {
        printf("Failure");
        return;
        }
    ByteStream entireFile, entireFile2;
    if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
        {
        printf("Failure");
        return;
        }
    if(BeFileStatus::Success != file2.Open(path2.c_str(), BeFileAccess::Read))
        {
        printf("Failure");
        return;
        }
    if (BeFileStatus::Success != file.ReadEntireFile(entireFile)) 
        {
        printf("Failure");
        return;
        }
    if (BeFileStatus::Success != file2.ReadEntireFile(entireFile2))
        {
        printf("Failure");
        return;
        }
    Utf8String str((Utf8P)entireFile.GetDataP());
    Utf8String str2((Utf8P)entireFile2.GetDataP());
    bvector<IGeometryPtr> geometryA, geometryB;
    BentleyGeometryJson::TryJsonStringToGeometry(str, geometryA);
    BentleyGeometryJson::TryJsonStringToGeometry(str2, geometryB);
    if (geometryA.size() != geometryB.size()) 
        {
        printf("Failure");
        return;
        }
    else 
        {
        for (size_t i = 0; i < geometryA.size(); i++)
            {
            if (!geometryA[i]->IsSameStructureAndGeometry(*geometryB[i]))
                {
                printf("Failure");
                return;
                }
            }
        }
    printf("Success");
    return;
    }