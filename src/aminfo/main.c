#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#define STRINGIFY(s) STRINGIFY2(s)
#define STRINGIFY2(s) #s
#define SafeMalloc(bytes) SafeMalloc2(bytes, __FILE__ ":" STRINGIFY(__LINE__))

typedef struct {
	bool Signed;
	uint16_t StringTableSize;
	uint32_t SymbolCount;
	uint32_t FtexCount;
} AmHeader;

typedef struct {
	uint32_t TextureId;
	float Xpos;
	float Ypos;
	float Xscale;
	float Yscale;
	float Xskew;
	float Yskew;
	float Xoff;
	float Yoff;
	int16_t RedValue;
	int16_t RedIntensity;
	int16_t GreenValue;
	int16_t GreenIntensity;
	int16_t BlueValue;
	int16_t BlueIntensity;
	int16_t AlphaValue;
	int16_t AlphaIntensity;
} AmKeyframe;

typedef struct {
	uint32_t KeyframeCount;
	float TimelineDuration;
	AmKeyframe* Keyframes;
} AmLayer;

typedef struct {
	bool Loop;
	uint32_t LayerCount;
	AmLayer* Layers;
} AmSymbol;

void* SafeMalloc2(size_t bytes, char* debug_info) {
	if (bytes == 0) {
		printf("Attempted to allocate zero bytes at %s.", debug_info);
		exit(3);
	}
	
	void* r = malloc(bytes);
	if (r == NULL) {
		printf("Malloc error at %s.", debug_info);
		exit(3);
	}
	
	return r;
}

void ReadFileData(FILE* fp, void* buf, size_t len) {
	if (fp == NULL || buf == NULL || len == 0) {
		printf("ReadFileData given invalid data.");
		exit(2);
	}
	
	if (fread(buf, 1, len, fp) != len) {
		printf("Unexpected file error.");
		exit(2);
	}
}

void ParseAmHeader(FILE* fp, AmHeader* header) {
	uint32_t signature1;
	ReadFileData(fp, &signature1, 4);
	
	if (signature1 == 0x46313031 || signature1 == 0x46313030) {
		/* Signed AM */
		
		uint16_t string_table_size;
		ReadFileData(fp, &string_table_size, 2);
		
		uint32_t symbol_count;
		ReadFileData(fp, &symbol_count, 4);
		
		uint16_t signature2;
		ReadFileData(fp, &signature2, 2);
		
		assert(signature2 == 0x41F0);
		
		uint32_t ftex_count;
		ReadFileData(fp, &ftex_count, 4);
		
		header->Signed = true;
		header->StringTableSize = string_table_size;
		header->SymbolCount = symbol_count;
		header->FtexCount = ftex_count;
	} else {
		/* Unsigned AM */
		
		uint16_t string_table_size = signature1 & 0xFFFF;
		
		uint32_t symbol_count = signature1 >> 16;
		ReadFileData(fp, (char*)(&symbol_count) + 2, 2);
		
		uint16_t signature2;
		ReadFileData(fp, &signature2, 2);
		
		assert(signature2 == 0x41F0);
		
		header->Signed = false;
		header->StringTableSize = string_table_size;
		header->SymbolCount = symbol_count;
		header->FtexCount = 0;
	}
}

void ParseStringTable(FILE* fp, AmHeader* header, char*** string_table_out) {
	char* raw_table = SafeMalloc(header->StringTableSize);
	char** string_table = SafeMalloc((header->SymbolCount + header->FtexCount) * sizeof(char**));
	
	ReadFileData(fp, raw_table, header->StringTableSize);
	
	char* current = raw_table;
	for (size_t i = 0; i < header->SymbolCount + header->FtexCount; i++) {
		assert(current < raw_table + header->StringTableSize);
		
		string_table[i] = current;
		current += strlen(current) + 1;
	}
	
	*string_table_out = string_table;
}

void ParseSymbol(FILE* fp, AmHeader* header, AmSymbol* symbol_out) {
	AmSymbol symbol;
	
	uint32_t data;
	ReadFileData(fp, &data, 4);
	
	symbol.Loop = data & 1;
	symbol.LayerCount = data >> 1;
	
	ReadFileData(fp, &data, 4);
	ReadFileData(fp, &data, 4);
	
	for (size_t i = 0; i < symbol.LayerCount; i++) {
		/* The purpose of these uint32s is currently unknown. */
		ReadFileData(fp, &data, 4);
	}
	
	AmLayer* layers = SafeMalloc(symbol.LayerCount * sizeof(AmLayer));
	
	for (size_t i = 0; i < symbol.LayerCount; i++) {
		float fdata;
		
		ReadFileData(fp, &data, 4);
		assert(data == 0);
		
		ReadFileData(fp, &data, 4);
		layers[i].KeyframeCount = data;
		
		ReadFileData(fp, &fdata, 4);
		layers[i].TimelineDuration = fdata;
		
		if (header->Signed) {
			ReadFileData(fp, &data, 4);
			assert(data == 0xFFFFFFFF);
			
			ReadFileData(fp, &data, 4);
			assert(data == 0xFFFFFFFF);
			
			ReadFileData(fp, &data, 4);
			assert(data == 0xFFFFFFFF);
		}
		
		layers[i].Keyframes = SafeMalloc(layers[i].KeyframeCount * sizeof(AmKeyframe));
		
		for (size_t j = 0; j < layers[i].KeyframeCount; j++) {
			AmKeyframe keyframe;
			
			ReadFileData(fp, &keyframe.TextureId, 4);
			ReadFileData(fp, &keyframe.Xpos, 4);
			ReadFileData(fp, &keyframe.Ypos, 4);
			ReadFileData(fp, &keyframe.Xscale, 4);
			ReadFileData(fp, &keyframe.Yscale, 4);
			ReadFileData(fp, &keyframe.Xskew, 4);
			ReadFileData(fp, &keyframe.Yskew, 4);
			ReadFileData(fp, &keyframe.Xoff, 4);
			ReadFileData(fp, &keyframe.Yoff, 4);
			ReadFileData(fp, &keyframe.RedValue, 2);
			ReadFileData(fp, &keyframe.RedIntensity, 2);
			ReadFileData(fp, &keyframe.GreenValue, 2);
			ReadFileData(fp, &keyframe.GreenIntensity, 2);
			ReadFileData(fp, &keyframe.BlueValue, 2);
			ReadFileData(fp, &keyframe.BlueIntensity, 2);
			ReadFileData(fp, &keyframe.AlphaValue, 2);
			ReadFileData(fp, &keyframe.AlphaIntensity, 2);
			
			ReadFileData(fp, &data, 4);
			assert(data == 0);
			
			layers[i].Keyframes[j] = keyframe;
		}
	}
	
	symbol.Layers = layers;
	
	*symbol_out = symbol;
}

void ParseAm(FILE* fp) {
	AmHeader header;
	ParseAmHeader(fp, &header);
	
	char** string_table;
	ParseStringTable(fp, &header, &string_table);
	
	printf("AmHeader:\n");
	printf("  StringTableSize: %u\n", header.StringTableSize);
	printf("  SymbolCount: %u\n", header.SymbolCount);
	printf("  FtexCount: %u\n", header.FtexCount);
	printf("\n");
	
	printf("String table:\n");
	for (uint32_t i = 0; i < header.SymbolCount + header.FtexCount; i++) {
		printf("  %u: %s\n", i, string_table[i]);
	}
	printf("\n");
	
	AmSymbol symbol;
	for (size_t i = 0; i < header.SymbolCount; i++) {
		ParseSymbol(fp, &header, &symbol);
		
		printf("AmSymbol:\n");
		printf("  Loop: %s\n", symbol.Loop != 0 ? "true" : "false");
		printf("  LayerCount: %u\n", symbol.LayerCount);
		printf("  Layers:\n");
		for (uint32_t j = 0; j < symbol.LayerCount; j++) {
			printf("    %u:\n", j);
			printf("      KeyframeCount: %u\n", symbol.Layers[j].KeyframeCount);
			printf("      TimelineDuration: %f\n", symbol.Layers[j].TimelineDuration);
		}
		printf("\n");
	}
}

int main(int argc, char** argv) {
	fflush(stdout);
	if (argc < 2) {
		printf("No file specified.");
		return 1;
	} else if (argc > 2) {
		printf("Invalid parameters.");
		return 1;
	}
	
	FILE* fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("Failed to open file.");
		return 1;
	}
	
	ParseAm(fp);
	
	return 0;
}
