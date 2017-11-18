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
	lzma_stream strm = LZMA_STREAM_INIT;

	std::unique_ptr<lzma_stream, void(*)(lzma_stream*)> freeStream(&strm, lzma_end);

	if (lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64) != LZMA_OK) throw std::runtime_error("!lzma_easy_encoder");

	strm.next_in = (const uint8_t*)from.data();
	strm.avail_in = from.length();

	uint8_t outbuf[4096];
	strm.next_out = outbuf;
	strm.avail_out = sizeof(outbuf);

	for (;;) {
		lzma_ret ret = lzma_code(&strm, LZMA_FINISH);

		if (strm.avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t write_size = sizeof(outbuf) - strm.avail_out;
			to.append((const char*)outbuf, write_size);
			strm.next_out = outbuf;
			strm.avail_out = sizeof(outbuf);
		}

		if (ret == LZMA_STREAM_END) break;

		if (ret == LZMA_OK) continue;

		throw std::runtime_error("lzma_code == " + std::to_string(ret));
	}

	lzma_end(&strm);
}

static void xzDecompress(const char* data, int length, std::string& to) {

	lzma_stream strm = LZMA_STREAM_INIT;

	strm.next_in = (const uint8_t*)data;
	strm.avail_in = length;

	std::unique_ptr<lzma_stream, void(*)(lzma_stream*)> freeStream(&strm, lzma_end);

	lzma_ret rc = lzma_stream_decoder(&strm, 99 * 1024 * 1024, LZMA_CONCATENATED);

	if (rc != LZMA_OK) std::runtime_error("!lzma_stream_decoder: " + std::to_string(rc));

	uint8_t outbuf[4096]; strm.next_out = outbuf; strm.avail_out = sizeof outbuf;

	auto&& flush = [&]() {
		to.append((const char*)outbuf, sizeof(outbuf) - strm.avail_out);
		strm.next_out = outbuf; strm.avail_out = sizeof outbuf;
	};

	while (strm.avail_in != 0) {
		rc = lzma_code(&strm, LZMA_FINISH);
		flush(); if (rc == LZMA_STREAM_END) break;
		if (rc != LZMA_OK) std::runtime_error("!lzma_code: " + std::to_string(rc));
	}
}