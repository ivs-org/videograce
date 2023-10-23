/**
 * Connection.cpp - Contains sqlite connection wrapper impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <DB/Connection.h>

namespace DB
{

Connection::Connection(const std::string &path)
	: handle(nullptr), result(rOK)
{
	result = static_cast<Result>(sqlite3_open(path.c_str(), &handle));
	sqlite3_busy_timeout(handle, 500);
}

bool Connection::IsOK() const
{
	return result == rOK;
}

Result Connection::GetResult() const
{
	return result;
}

const char* Connection::GetErrorMessage()
{
	return sqlite3_errmsg(handle);
}

Connection::~Connection()
{
	sqlite3_close(handle);
}

}
