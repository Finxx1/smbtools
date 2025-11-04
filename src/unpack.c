#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

/*
 * NOTES:
 * This source is not based off of any other pre-existing source. It is the
 * result of clean-room reverse-engineering.
 *
 * This source is released into the public domain, or is under the terms of
 * the ISC license if you are in a place that does not recognize the public
 * domain.
 *
 * This tool is mainly written to be fast, and therefore takes some shortcuts
 * in unpacking. In particular, directory IDs are ignored. This is not an issue
 * in dealing with all official .dat files known to exist.
 *
 * DO NOT USE THIS TOOL ON UNTRUSTED DATA. BUFFER OVERRUNS WILL OCCUR WHEN
 * READING INVALID VALUES.
 *
 * This tool was written by Finxx (finxx.xyz) for smbtools.
 */

#ifdef _WIN32
#include <windows.h>
#else
#error TODO: Implement CreateDirectoryA on POSIX
#endif

typedef struct {
	char* name;
	uint32_t first_file;
} directory;

typedef struct {
	char* name;
	uint32_t offset;
	uint32_t size;
	uint32_t dir;
} file;

uint32_t read_u32(FILE* fp) {
	uint32_t r = 0;
	if (fread(&r, 1, 4, fp) != 4) {
		printf("Unexpected end-of-file or file error.");
		exit(1);
	}
	return r;
}

void read_data(FILE* fp, void* ptr, size_t amnt) {
	if (fread(ptr, 1, amnt, fp) != amnt) {
		printf("Unexpected end-of-file or file error.");
		exit(1);
	}
}

void* xmalloc(size_t amnt) {
	void* r = malloc(amnt);
	if (r == NULL) {
		printf("Out of memory.");
		exit(1);
	}
	return r;
}

int parse_dat(FILE* fp) {
	uint32_t num_dirs = read_u32(fp);
	directory* dirs = xmalloc(num_dirs * sizeof(directory));
	for (size_t i = 0; i < num_dirs; i++) {
		(void)read_u32(fp);
		dirs[i].first_file = read_u32(fp);
	}
	
	uint32_t num_files = read_u32(fp);
	file* files = xmalloc(num_files * sizeof(file));
	for (size_t i = 0; i < num_files; i++) {
		files[i].offset = read_u32(fp);
		files[i].size = read_u32(fp);
		files[i].dir = read_u32(fp);
	}
	
	uint32_t dir_name_table_len = read_u32(fp);
	uint32_t file_name_table_len = read_u32(fp);
	
	char* dir_name_table = xmalloc(dir_name_table_len);
	char* file_name_table = xmalloc(file_name_table_len);
	read_data(fp, dir_name_table, dir_name_table_len);
	read_data(fp, file_name_table, file_name_table_len);
	
	for (size_t i = 0; i < num_dirs; i++) {
		dirs[i].name = dir_name_table;
		dir_name_table += strlen(dirs[i].name) + 1;
	}
	
	for (size_t i = 0; i < num_files; i++) {
		files[i].name = file_name_table;
		file_name_table += strlen(files[i].name) + 1;
	}
	
	// We do this separately from parsing so we don't extract half of the files
	// only for the .dat to later be corrupt or have an unexpected EOF.
	
	CreateDirectoryA("data", NULL);
	for (size_t i = 0; i < num_dirs; i++) {
		char path[256];
		strcpy(path, "data/");
		strcat(path, dirs[i].name);
		CreateDirectoryA(path, NULL);
	}
	
	for (size_t i = 0; i < num_files; i++) {
		char path[256];
		strcpy(path, "data/");
		strcat(path, files[i].name);
		FILE* outfp = fopen(path, "wb");
		if (outfp == NULL) {
			printf("File creation error.");
			return 1;
		}
		
		void* filedata = xmalloc(files[i].size);
		fseek(fp, files[i].offset, SEEK_SET);
		read_data(fp, filedata, files[i].size);
		fwrite(filedata, 1, files[i].size, outfp);
		free(filedata);
		
		fclose(outfp);
	}
	
	return 0;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Missing file.");
		return 1;
	}
	
	FILE* fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("Invalid file.");
		return 1;
	}
	
	return parse_dat(fp);
}