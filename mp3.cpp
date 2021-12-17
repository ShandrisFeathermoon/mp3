#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CLS_MP3.h"

void GetPic(int offset, FILE *file, const char *name);
void GetStream(FILE *file, const char *name);
void ParseMp3Header(FILE *file);

int main(int argc, char *argv[])
{
	if (argc < 4) {
		return 1;
	}
/*
	FILE *file = fopen(argv[1], "rb");
	if (!file) {
		printf("Failed to open %s\n", argv[1]);
		return 1;
	}

	int offset = 0x309;
	GetPic(offset, file, argv[1]);
	GetStream(file, argv[1]);

	fclose(file);*/

	CMP3 mp3(argv[1]);
	mp3.ReplaceImg(argv[2]);
	mp3.Save(argv[3]);
	return 0;
}

void GetPic(int offset, FILE *file, const char *name)
{
	char out[256] = {0};
	strcpy(out, name);
	char *ptr = strrchr(out, '.');
	if (ptr) {
		*ptr = 0;
	}
	strcat(out, ".jpeg");
	fseek(file, offset, SEEK_SET);
	int length = 0;
	fread(&length, sizeof(int), 1, file);
	unsigned char *buf = new unsigned char[length];
	int nRead = fread(buf, 1, length, file);
	FILE *fileOut = fopen(out, "wb");
	if (fileOut) {
		fwrite(buf, 1, nRead, fileOut);
		fclose(fileOut);
	}
	delete []buf;
}

void GetStream(FILE *file, const char *name)
{
	IDxHeader hd;
	memset(&hd, 0, sizeof(IDxHeader));
	hd.idx[0] = 'I';
	hd.idx[1] = 'D';
	hd.idx[2] = '3';
	hd.version = 3;

	char out[256] = {0};
	strcpy(out, name);
	char *ptr = strrchr(out, '.');
	if (ptr) {
		*ptr = 0;
	}
	strcat(out, ".mp3");
	FILE *fileOut = fopen(out, "wb");
	if (fileOut) {
		fwrite(&hd, sizeof(IDxHeader), 1, fileOut);
		char buf[4096];
		int nRead = 0;
		while (!feof(file)) {
			nRead = fread(buf, 1, 4096, file);
			if (nRead == 0) {
				break;
			}
			fwrite(buf, 1, nRead, fileOut);
		}
		fclose(fileOut);
	}
}

