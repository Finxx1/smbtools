#include <errno.h>
#include <stdio.h>
#include <string.h>
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
 * Improvements and some posix by AllMeatball.
 */


#define UNPACK_MAX_PATH 4096

#ifdef _WIN32
#include <windows.h>
#define PATH_DELIM '\\'
#define PATH_DELIM_STR "\\"
#else
#include <sys/stat.h>
#define PATH_DELIM '/'
#define PATH_DELIM_STR "/"

/*
 * NOTE: 0 is needed before the number to make it an octal
 * which is the docmented way to represent file modes.
 *
 * in this case I am using this mode (0644)
 * which set the bitflags as the following:
 *
 * Read (4)
 * Write (2)
 * Execute (1)
 *
 * Owner Rights  (u)
 * Group Rights  (g)
 * Others Rights (o)
 *
 *   u g o
 * r ! ! !
 * w ! . .
 * e . . .
 *
 *
 */
#define CreateDirectoryA(path, sec_desc) mkdir(path, 0644)
#endif

// NOTE: this code is seemingly compatible with Windows and Linux, according to
// the windows documentation seen at:
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen
const char *get_error_str() {
	return strerror(errno);
}

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

const char *get_basename(const char *path) {
	const char *filename = strrchr(path, PATH_DELIM);
	if (filename) {
		// remove prepending delimeter, if string is not empty
		if (strlen(filename) > 0)
			filename++;

	} else {
		filename = "[null path]";
	}

	return filename;
}

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

int parse_dat(FILE* fp, const char *output_path) {
	if (output_path == NULL)
		output_path = "data";

	printf("Selected output dir: %s\n", output_path);

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

	
	CreateDirectoryA(output_path, NULL);
	for (size_t i = 0; i < num_dirs; i++) {
		char path[UNPACK_MAX_PATH];

		strcpy(path, output_path);
		strcat(path, PATH_DELIM_STR);
		strcat(path, dirs[i].name);
		CreateDirectoryA(path, NULL);
	}
	
	for (size_t i = 0; i < num_files; i++) {
		char path[UNPACK_MAX_PATH];

		strcpy(path, output_path);
		strcat(path, PATH_DELIM_STR);
		strcat(path, files[i].name);
		FILE* outfp = fopen(path, "wb");
		if (outfp == NULL) {
			printf("File creation error: %s\n", get_error_str());
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
	const char *archive_path = NULL;
	const char *output_path = NULL;

	if (argc < 2) {
		printf("unpack [.dat file] (output path)\n");
		return 1;
	}

	if (argc >= 3)
		output_path = argv[2];
	
	archive_path = argv[1];
	FILE* fp = fopen(archive_path, "rb");
	if (fp == NULL) {
		printf("Failed to open archive \"%s\": %s\n", get_basename(archive_path), get_error_str());
		return 1;
	}
	
	return parse_dat(fp, output_path);
}
