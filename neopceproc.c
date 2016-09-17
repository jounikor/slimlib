/*
 * neopceproc
 * Chilly Willy, based on neopcepad by mic
 *
 * Strips header (if present)
 * Pads 384KB HuCard ROMs for use with NeoFlash cards
 *
 * Usage: neopceproc
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

char rom[20*128*1024]; // big enough for SFIICE

int main(int argc, char **argv)
{
	int i, c, n, bank;
	char *data;
	FILE *inFile,*outFile;
    DIR *inDir;
    struct dirent *inDirEnt;

    inDir = opendir(".");
    if (inDir == NULL)
    {
		printf("Failed to open current directory\n");
		exit(1);
	}
    while ((inDirEnt = readdir(inDir)) != NULL)
    {
        char temp[1024];

        if (strcmp(&inDirEnt->d_name[strlen(inDirEnt->d_name)-4], ".pce"))
            continue;
        inFile = fopen(inDirEnt->d_name, "rb");
        if (inFile == NULL)
            continue;
        memset(rom, 0xFF, 20*128*1024);
        fseek(inFile, 0, SEEK_END);
        c = ftell(inFile);
        fseek(inFile, 0, SEEK_SET);
        if ((c & 0x3FF) >= 0x200)
            fseek(inFile, 0x200, SEEK_SET); // skip header
        printf("Reading %s\n", inDirEnt->d_name);
        fread(rom, 1, c & 0xFFFFFC00, inFile);
        fclose(inFile);
        // now pad rom if needed
        if ((c & 0xFFFFFC00) == 0x60000)
        {
            for (i = 48; i < 64; i++)
            {
                bank = i & 0x1F;
                memcpy(&rom[i * 0x2000], &rom[bank * 0x2000], 0x2000);
            }
            for (i = 64; i < 96; i++)
            {
                bank = (i & 0x0F) + 32;
                memcpy(&rom[i * 0x2000], &rom[bank * 0x2000], 0x2000);
            }
            c = 0xC0000;
        }
        sprintf(temp, "../%s", inDirEnt->d_name);
        printf("Writing %s\n", temp);
        outFile = fopen(temp, "wb");
        if (outFile == NULL)
        {
            printf("Failed to open %s for writing\n", temp);
        }
        else
        {
            fwrite(rom, 1, c & 0xFFFFFC00, outFile);
            fclose(outFile);
        }
    }

	printf("Done\n");

	return 0;
}
