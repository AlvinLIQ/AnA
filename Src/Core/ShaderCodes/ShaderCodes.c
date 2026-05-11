#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("wrong args count\n");
        return -1;
    }
    /*char path[512] = "";
    size_t pathLen = strlen(argv[1]);
    memcpy(path, argv[1], pathLen);*/
    size_t fileNameLen;
    char* filename;
    FILE* outputFile = fopen(argv[1], "wb");
    if (!outputFile)
    {
        printf("Failed to open or create \"%s\"!", argv[1]);
        return -2;
    }
    fprintf(outputFile, "#pragma once\n#include <vector>\n");
    for (int i = 2, j; i < argc; i++)
    {
        fileNameLen = strlen(argv[i]);
        //memcpy(&path[pathLen], argv[i], fileNameLen);
        //path[pathLen + fileNameLen] = '\0';

        FILE* file = fopen(argv[i], "rb+");
        if (!file)
        {
            printf("Failed to open \"%s\"!", argv[i]);
            fclose(outputFile);
            return -2;
        }

        filename = argv[i];
        for (j = (int)fileNameLen; j >= 0; --j)
        {
            if (argv[i][j] == '.')
                argv[i][j] = '_';
            else if (argv[i][j] == '/')
            {
                filename = &argv[i][j + 1];
                break;
            }
        }
        
        //printf ("static unsigned char[] %s = {", argv[1]);
        argv[i][fileNameLen - 4] = '\0';
        fprintf (outputFile, "static const std::vector<unsigned char> %s = {", filename);
        int byte;
//        size_t fs = 0;
        byte = fgetc(file);
        if (byte != EOF)
        {
            fprintf (outputFile, "%#x", byte);
//            fs++;
            while ((byte = fgetc(file)) != EOF)
            {
                fprintf (outputFile, ", %#x", byte);
//                fs++;
            }
        }
        fprintf(outputFile, "};\n");
        //printf("static size_t %s_size = %llu;\n", argv[1], fs);
        fclose(file);
    }
    fclose(outputFile);

    return 0;
}