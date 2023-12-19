/**
 * Base64.cpp - contains the base64 helpers impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */
#include <string>
#include <algorithm>
#include <cstdint>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace Common
{

std::string toBase64(std::string_view source)
{
	BIO *b64 = BIO_new(BIO_f_base64());
	BIO *bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	BIO_set_close(bio, BIO_CLOSE);
	BIO_write(bio, source.data(), (int)source.size());
	BIO_flush(bio);

	BUF_MEM *bufferPtr;
	BIO_get_mem_ptr(bio, &bufferPtr);

	std::string result(bufferPtr->data, bufferPtr->length);

	BIO_free_all(bio);

	std::replace(result.begin(), result.end(), '+', '-');
	std::replace(result.begin(), result.end(), '/', '_');
	result.erase(result.find_last_not_of(" ") + 1);
	
	return result;
}

std::string fromBase64(std::string_view source_)
{
	std::string source(source_);
	std::replace(source.begin(), source.end(), '-', '+');
	std::replace(source.begin(), source.end(), '_', '/');

	BIO *b64 = BIO_new(BIO_f_base64());
	BIO *bio = BIO_new_mem_buf((void*)source.c_str(), (int)source.size());
	bio = BIO_push(b64, bio);

	uint8_t *out = (uint8_t*) malloc(source.size());

	(void)BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	(void)BIO_set_close(bio, BIO_CLOSE);
	int size = BIO_read(bio, out, (int)source.size());

	std::string result((const char*)out, size);

	free(out);

	BIO_free_all(bio);

	return result;
}

}
