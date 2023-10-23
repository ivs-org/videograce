/**
 * Encryptor.cpp - Contains encryptor impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#include <iostream>

#include <Common/Common.h>
#include <Crypto/Decryptor.h>
#include <Transport/RTP/RTPPacket.h>

namespace Crypto
{

static const uint32_t DECRYPTOR_BUFFER_SIZE = 1024 * 1024;

Decryptor::Decryptor()
	: runned(false),
	receiver(nullptr),
	key(),
	ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free),
	buffer(new uint8_t[DECRYPTOR_BUFFER_SIZE])
{
	EVP_add_cipher(EVP_aes_256_ecb());
}

Decryptor::~Decryptor()
{
	Stop();
}

void Decryptor::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void Decryptor::Start(const std::string &secureKey)
{
	if (!runned)
	{
		runned = true;
	}

	key = secureKey;
}

void Decryptor::Stop()
{
	runned = false;
}

bool Decryptor::Started() const
{
    return runned;
}

void Decryptor::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}

	const Transport::RTPPacket &inputPacket = *static_cast<const Transport::RTPPacket*>(&packet_);

	if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_ecb(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), NULL) != 1)
	{
		return HandleErrors();
	}

	int decryptedSize = 0;
	if (EVP_DecryptUpdate(ctx.get(), static_cast<unsigned char*>(buffer.get()), &decryptedSize, inputPacket.payload, inputPacket.payloadSize) != 1)
	{
		return HandleErrors();
	}

	int finalSize = 0;
	if (EVP_DecryptFinal_ex(ctx.get(), buffer.get() + decryptedSize, &finalSize) != 1)
	{
		HandleErrors();
	}
	decryptedSize += finalSize;

	Transport::RTPPacket outputPacket;
	outputPacket.rtpHeader = inputPacket.rtpHeader;
	outputPacket.payload = buffer.get();
	outputPacket.payloadSize = decryptedSize;
	
	receiver->Send(outputPacket);
}

void Decryptor::HandleErrors()
{
	unsigned long errCode;
	while (errCode = ERR_get_error())
	{
		std::cerr << "Decryptor error: " << ERR_error_string(errCode, NULL) << std::endl;
	}
}

}
