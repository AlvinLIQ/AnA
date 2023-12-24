#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("wrong args count\n");
        return -1;
    }
    FILE* file = fopen(argv[1], "rb+");
    if (!file)
    {
        printf("Failed to open \"%s\"!", argv[1]);
        return -2;
    }
    
    int pos = 0;
    while (argv[1][pos])
    {
        if (argv[1][pos] == '/' || argv[1][pos] == '\\' || argv[1][pos] == '.')
        {
            argv[1][pos] = '_';
        }
        ++pos;
    }
    printf ("const std::vector<unsigned char> %s = {", argv[1]);
    int byte;
    size_t fs = 0;
    byte = fgetc(file);
    if (byte != EOF)
    {
        printf ("%#x", byte);
        while ((byte = fgetc(file)) != EOF)
        {
            printf (", %#x", byte);
        }
    }
    printf("};\n");
    fclose(file);
    return 0;
}