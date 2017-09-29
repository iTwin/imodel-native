#include <Geom\GeomApi.h>
#include <Bentley\BeFileName.h>
#include <Bentley\BeFile.h>
#include <GeomSerialization\GeomSerializationApi.h>

extern "C"
void main(int argc, char **argv) 
    {
    printf("%s\n", argv[1]);
    printf("%s\n", argv[2]);
    argv[2];
    Utf8String filestr0(argv[1]);
    Utf8String filestr1(argv[2]);
    BeFileName path(filestr0);
    BeFileName path2(filestr1);
    BeFile file, file2;
    file.Create(path.c_str(), false);
    file2.Create(path2.c_str(), false);
    if (!(BeFileName::DoesPathExist(path2.c_str()) && BeFileName::DoesPathExist(path.c_str()))) 
        {
        printf("DgnJs files donot exist");
        return;
        }
    ByteStream entireFile, entireFile2;
    if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
        {
        printf("file Open operation failed");
        return;
        }
    if(BeFileStatus::Success != file2.Open(path2.c_str(), BeFileAccess::Read))
        {
        printf("file Open operation failed");
        return;
        }
    if (BeFileStatus::Success != file.ReadEntireFile(entireFile)) 
        {
        printf("Read operation failed");
        return;
        }
    if (BeFileStatus::Success != file2.ReadEntireFile(entireFile2))
        {
        printf("Read operation failed");
        return;
        }
    Utf8String str((Utf8P)entireFile.GetDataP());
    Utf8String str2((Utf8P)entireFile2.GetDataP());
    bvector<IGeometryPtr> geometryA, geometryB;
    BentleyGeometryJson::TryJsonStringToGeometry(str, geometryA);
    BentleyGeometryJson::TryJsonStringToGeometry(str2, geometryB);
    printf("%s", str.c_str());
    printf("%s", str2.c_str());
    if (geometryA.size() != geometryB.size()) 
        {
        printf("failed\n");
        }
    else 
        {
        for (size_t i = 0; i < geometryA.size(); i++)
            {
            if (!geometryA[i]->IsSameStructureAndGeometry(*geometryB[i]))
                {
                printf("not same\n");
                }
            }
        }
    }