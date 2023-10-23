/**
 * Checker.cpp - Contains ssl checker impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <Crypto/Checker.h>

#include <boost/asio/ssl.hpp>

namespace Crypto
{

bool CheckSSLCertificate(const std::string &certificate_, const std::string &privateKey_)
{
    try
    {
        boost::asio::ssl::context ctx{ boost::asio::ssl::context::sslv23 };

        ctx.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::single_dh_use);

        ctx.use_certificate_chain(
            boost::asio::buffer(certificate_.data(), certificate_.size()));

        ctx.use_private_key(
            boost::asio::buffer(privateKey_.data(), privateKey_.size()),
            boost::asio::ssl::context::file_format::pem);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

}
