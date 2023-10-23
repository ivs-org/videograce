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
#include <Crypto/Encryptor.h>
#include <Transport/RTP/RTPPacket.h>

namespace Crypto
{

static const uint32_t ENCRYPTOR_BUFFER_SIZE = 1024 * 1024;

Encryptor::Encryptor()
	: runned(false),
	receiver(nullptr),
	key(),
	ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free),
	buffer(new uint8_t[ENCRYPTOR_BUFFER_SIZE])
{
	EVP_add_cipher(EVP_aes_256_ecb());
}

Encryptor::~Encryptor()
{
	Stop();
}

void Encryptor::SetReceiver(Transport::ISocket *receiver_)
{
	receiver = receiver_;
}

void Encryptor::Start(const std::string &secureKey)
{
	if (!runned)
	{
		runned = true;
	}

	key = secureKey;
}

void Encryptor::Stop()
{
	runned = false;
}

bool Encryptor::Started() const
{
    return runned;
}

void Encryptor::HandleErrors()
{
	unsigned long errCode;
	while (errCode = ERR_get_error())
	{
		std::cerr << "Encryptor error: " << ERR_error_string(errCode, NULL) << std::endl;
	}
}

void Encryptor::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	if (!runned)
	{
		return;
	}
	
	const Transport::RTPPacket &inputPacket = *static_cast<const Transport::RTPPacket*>(&packet_);
	
	if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_ecb(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), NULL) != 1)
	{
		return HandleErrors();
	}
	
	int encryptedSize = 0;
	if (EVP_EncryptUpdate(ctx.get(), static_cast<unsigned char*>(buffer.get()), &encryptedSize, inputPacket.payload, inputPacket.payloadSize) != 1)
	{
		return HandleErrors();
	}

	int finalSize = 0;
	if (EVP_EncryptFinal_ex(ctx.get(), buffer.get() + encryptedSize, &finalSize) != 1)
	{
		return HandleErrors();
	}
	encryptedSize += finalSize;
	
	Transport::RTPPacket outputPacket;
	outputPacket.rtpHeader = inputPacket.rtpHeader;
	outputPacket.payload = buffer.get();
	outputPacket.payloadSize = encryptedSize;

	receiver->Send(outputPacket);
}

}
