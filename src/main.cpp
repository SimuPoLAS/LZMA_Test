// yada yada

#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include "lzma.h"
using std::string;

static void xzCompress(const std::string& from, std::string& to);
static void xzDecompress(const char* data, int length, std::string& to);

int main()
{
	std::cout << "Hello World!" << std::endl;

	std::string s;
	xzCompress("Norbert is a communist and loves Karl Marx very much. Capitalism should die! This is worth of compression.", s);
	std::cout << s << "\n";

	std::string o;
	xzDecompress(s.data(), s.size(), o);
	std::cout << o << "\n";


	char c;
	std::cin >> c;

	// all good
	return 0;
}

static void xzCompress(const std::string& from, std::string& to) {
	// lzma_stream is inizialized
	lzma_stream strm = LZMA_STREAM_INIT;

	// Space is freed so lzma_stream can always write
	std::unique_ptr<lzma_stream, void(*)(lzma_stream*)> freeStream(&strm, lzma_end);

	// lzma stream encoder is inizialized
	// LZMA_CHECK_CRC32 can be used because we will not compress big files
	if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64) != LZMA_OK) throw std::runtime_error("!lzma_easy_encoder");

	// The next input byte is set
	strm.next_in = (const uint8_t*)from.data();
	// The number of input bytes availiable in next_in is set
	strm.avail_in = from.length();

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
	// LZMA_CONCATENATED can not be used unsless the file is saved as .xz
	// We should probably use LZMA_TELL_NO_CHECK
	lzma_ret rc = lzma_stream_decoder(&strm, 99 * 1024 * 1024, LZMA_CONCATENATED);

	// If the inizializitasion of lzma_stream_decoder wasa not successful an error is thrown
	if (rc != LZMA_OK) std::runtime_error("!lzma_stream_decoder: " + std::to_string(rc));

	uint8_t outbuf[4096];
	// The next output position is set
	strm.next_out = outbuf;
	// The amount of free space in next_out is set
	strm.avail_out = sizeof outbuf;

	// A variable to write the decompressed data to the output string is created
	auto&& flush = [&]() {
		// The decompressed data is written to the output string
		to.append((const char*)outbuf, sizeof(outbuf) - strm.avail_out);
		// The next output position is reset 
		strm.next_out = outbuf;
		// The amount of free space in next_out is reset
		strm.avail_out = sizeof outbuf;
	};

	// As long as there is something to decompress this loop is run
	while (strm.avail_in != 0) {
		// lzma_code is decompressing the input
		// rc is waiting for a return from lzma_code
		rc = lzma_code(&strm, LZMA_FINISH);

		// The decompressed data is written to the output string
		flush();

		// If the input stream is completely decompressed exit the loop
		if (rc == LZMA_STREAM_END) break;

		// If the decompression operation wasn't completed succefully an error is thrown
		if (rc != LZMA_OK) std::runtime_error("!lzma_code: " + std::to_string(rc));
	}
}