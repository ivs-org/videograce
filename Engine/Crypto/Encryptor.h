/**
 * Encryptor.h - Contains encryptor interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <Crypto/IEncryptor.h>
#include <Transport/ISocket.h>

#include <atomic>
#include <memory>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *);

namespace Crypto
{

class Encryptor : public IEncryptor, public Transport::ISocket
{
public:
	Encryptor();
	~Encryptor();

	void SetReceiver(Transport::ISocket *receiver);

	/// Derived from IEncryptor
	virtual void Start(const std::string &secureKey);
	virtual void Stop();

    virtual bool Started() const;

	/// Derived from Transport::ISocket (input method)
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *address = nullptr) final;

private:
	std::atomic<bool> runned;

	Transport::ISocket *receiver;

	std::string key;

	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx;

	std::unique_ptr<uint8_t[]> buffer;

	void HandleErrors();		
};

}
