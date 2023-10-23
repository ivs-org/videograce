/**
 * Connection.h - Contains sqlite connection wrapper
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>

#include <DB/sqlite/sqlite3.h>
#include <DB/Result.h>

namespace DB
{

class Transaction;
class Query;

class Connection
{
public:
	Connection(const std::string &path);
	~Connection();

	bool IsOK() const;
	Result GetResult() const;
	const char* GetErrorMessage();

private:
	sqlite3 *handle;

	Result result;

	friend Transaction;
	friend Query;
};

}
