// yada yada

#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include "lzma.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
using std::string;

static void xzCompress(const char* from, std::string& to, const char* fout);
static void xzDecompress(const char* data, int length, std::string& to);
static char* fileread(const char* in);
static void filewrite(const char* input, const int size, const char* fout);

int main()
{
	std::cout << "LZMA" << std::endl;
	//nobyStringTest();

	char* read = fileread("D:/lzma_test.txt");
	std::cout << read << std::endl;

	std::string compressed;
	xzCompress(read, compressed, "D:/compressed.txt");
	std::cout << compressed << std::endl;

	std::string decompressed;
	xzDecompress(compressed.data(), compressed.length(), decompressed);
	std::cout << decompressed << std::endl;
	filewrite(decompressed.data(), decompressed.size(), "D:/decompressed.txt");

	char c;
	std::cin >> c;

	// all good
	return 0;
}

typedef struct {
	lzma_stream strm;
	FILE* fp;
	char in[4096];
	size_t in_size;
	char out[4096];
	size_t out_size;
} LZMAFILE;

LZMAFILE* lzmaopen(const char* filepath, const char* mode) {
	LZMAFILE* file = (LZMAFILE*)malloc(sizeof(LZMAFILE));
	file->strm = LZMA_STREAM_INIT;
	file->fp = fopen(filepath, mode);
	file->in_size = 0;
	file->out_size = 0;
	return file;
}

void lzmaclose(LZMAFILE* file) {
	lzma_end(&file->strm);
	fclose(file->fp);
	free(file);
}

size_t lzmaread(void* buffer, size_t size, size_t count, LZMAFILE* file) {
	if (file->in_size < 4096) {
		fread(file->in, sizeof(char), 4096 - file->in_size, file->fp);
		file->in_size = 4096;
	}

	file->strm.next_in = (uint8_t*)file->in;
	file->strm.avail_in = file->in_size;
	file->strm.next_out = (uint8_t*)buffer;
	file->strm.avail_out = size*count;
	lzma_ret rc = lzma_code(&file->strm, LZMA_FINISH);
	if (rc != LZMA_OK)
		return -1;
	memcpy(buffer, file->in + (file->in_size - size*count), size*count);
	file->in_size = file->in_size - size*count;

}

static void filewrite(const char* input, const int size, const char* fout) {
	FILE* file = fopen(fout, "w");

	fwrite(input, 1, size, file);

	fclose(file);
}

static char* fileread(const char* fin) {
	// File which is read from
	FILE* sFile = fopen(fin, "r");

	// Ff unable to open file
	if (sFile == NULL)
	{
		return false;
	}

	// Seek to end of file
	fseek(sFile, 0, SEEK_END);

	// Get current file position which is end from seek
	size_t size = ftell(sFile);

	// Allocate string space and set length
	char* ss = (char*)malloc(size + 1);

	if (ss == NULL) {
		return false;
	}

	// Go back to beginning of file for read
	rewind(sFile);

	// Read 1*size bytes from sFile into ss
	fread(ss, 1, size, sFile);
	ss[size] = '\0';

	// Close the file
	fclose(sFile);

	return ss;
}

static void xzCompress(const char* from, std::string& to, const char* fout) {
	FILE* file = fopen(fout, "w");

	// lzma_stream is inizialized
	lzma_stream strm = LZMA_STREAM_INIT;

	// Space is freed so lzma_stream can always write
	std::unique_ptr<lzma_stream, void(*)(lzma_stream*)> freeStream(&strm, lzma_end);

	// lzma stream encoder is inizialized
	if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC32) != LZMA_OK) throw std::runtime_error("!lzma_easy_encoder");

	// The next input byte is set
	strm.next_in = (const uint8_t*)from;
	// The number of input bytes availiable in next_in is set
	strm.avail_in = strlen(from);

	uint8_t outbuf[4096];
	// The next output position is set
	strm.next_out = outbuf;
	// The amount of free space in next_out is set
	strm.avail_out = sizeof(outbuf);

	// In this loop the actual compression is happening
	for (;;) {
		// lzma_code is compressing the input
		// ret is waiting for a return from lzma_code
		lzma_ret ret = lzma_code(&strm, LZMA_FINISH);

		// If the output is full or
		// the input stream is completely compressed this is entered
		if (strm.avail_out == 0 || ret == LZMA_STREAM_END) {
			// The size that will be written to the output string is defined
			size_t write_size = sizeof(outbuf) - strm.avail_out;
			// The compressed data is written to the output string
			to.append((const char*)outbuf, write_size);
			fwrite((const char*)outbuf, sizeof(char), write_size, file);
			// The next output position is reset 
			strm.next_out = outbuf;
			// The amount of free space in next_out is reset
			strm.avail_out = sizeof(outbuf);
		}

		// If the input stream is completely compressed exit the loop
		if (ret == LZMA_STREAM_END) break;

		// If the compression worked fine the loop is continued
		if (ret == LZMA_OK) continue;

		// If any other result was produced an error is thrown
		throw std::runtime_error("lzma_code == " + std::to_string(ret));
	}

	// The memory alloced for the lzma_stream is freed
	lzma_end(&strm);
	fclose(file);
}

static void xzDecompress(const char* data, int length, std::string& to) {
	// lzma_stream is inizialized
	lzma_stream strm = LZMA_STREAM_INIT;

	// The next input byte is set
	strm.next_in = (const uint8_t*)data;
	// The number of input bytes availiable in next_in is set
	strm.avail_in = length;

	// Space is freed so lzma_stream can always write
	std::unique_ptr<lzma_stream, void(*)(lzma_stream*)> freeStream(&strm, lzma_end);

	// lzma_stream_decoder is inizialized
	//		The memory usage limit in bytes is set
	//		A decoder flag is set
	// If the inizializitasion of lzma_stream_decoder wasa not successful an error is thrown
	if (lzma_stream_decoder(&strm, 99 * 1024 * 1024, LZMA_TELL_NO_CHECK) != LZMA_OK) std::runtime_error("!lzma_stream_decoder!");

	uint8_t outbuf[4096];
	// The next output position is set
	strm.next_out = outbuf;
	// The amount of free space in next_out is set
	strm.avail_out = sizeof outbuf;

	// As long as there is something to decompress this loop is run
	while (strm.avail_in != 0) {
		// lzma_code is decompressing the input
		// rc is waiting for a return from lzma_code
		lzma_ret rc = lzma_code(&strm, LZMA_FINISH);

		// The decompressed data is written to the output string
		to.append((const char*)outbuf, sizeof(outbuf) - strm.avail_out);
		// The next output position is reset 
		strm.next_out = outbuf;
		// The amount of free space in next_out is reset
		strm.avail_out = sizeof outbuf;

		// If the input stream is completely decompressed exit the loop
		if (rc == LZMA_STREAM_END) break;

		// If the decompression operation wasn't completed succefully an error is thrown
		if (rc != LZMA_OK) std::runtime_error("!lzma_code: " + std::to_string(rc));
	}
}