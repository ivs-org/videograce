/**
 * Query.h - Contains sqlite query wrapper
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <cstdint>
#include <string>

#include <DB/Connection.h>

namespace DB
{

class Query
{
public:
	Query(Connection &connection);
	~Query();

	Result Prepare(const std::string &sql);
	Result Prepare(const std::wstring &sql);
	Result Set(int position, int32_t value);
	Result Set(int position, int64_t value);
	Result Set(int position, double value);
	Result Set(int position, const std::string &value);
	Result Set(int position, const std::wstring &value);
	Result SetNull(int position);

	bool Step();
	Result Reset();

	int32_t GetInt32(int position);
	int64_t GetInt64(int position);
	double GetDouble(int position);
	std::string GetString(int position);
	std::wstring GetWString(int position);
	bool GetNull(int position);

	void Close();

private:
	Connection &connection;
	sqlite3_stmt *stmt;
};

}
