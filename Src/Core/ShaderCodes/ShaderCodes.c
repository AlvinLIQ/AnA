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
    printf("#pragma once\n#include <vector>\n");

    char path[512] = "";
    size_t pathLen = strlen(argv[1]);
    memcpy(path, argv[1], pathLen);
    size_t fileNameLen;
    for (int i = 2; i < argc; i++)
    {
        fileNameLen = strlen(argv[i]);
        memcpy(&path[pathLen], argv[i], fileNameLen);
        path[pathLen + fileNameLen] = '\0';
        FILE* file = fopen(path, "rb+");
        if (!file)
        {
            printf("Failed to open \"%s\"!", path);
            return -2;
        }
        
        //printf ("static unsigned char[] %s = {", argv[1]);
        argv[i][fileNameLen - 4] = '\0';
        printf ("static const std::vector<unsigned char> %s = {", argv[i]);
        int byte;
        size_t fs = 0;
        byte = fgetc(file);
        if (byte != EOF)
        {
            printf ("%#x", byte);
            fs++;
            while ((byte = fgetc(file)) != EOF)
            {
                printf (", %#x", byte);
                fs++;
            }
        }
        printf("};\n");
        //printf("static size_t %s_size = %llu;\n", argv[1], fs);
        fclose(file);
    }
    return 0;
}