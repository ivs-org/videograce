/**
 * Query.cpp - Contains sqlite query wrapper impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <DB/Query.h>

#include <thread>

namespace DB
{

Query::Query(Connection &connection_)
	: connection(connection_), stmt(nullptr)
{
}

Query::~Query()
{
	Close();
}

Result Query::Prepare(const std::string &sql)
{
	return static_cast<Result>(sqlite3_prepare_v3(connection.handle, sql.c_str(), static_cast<int>(sql.size()), 0, &stmt, nullptr));
}

Result Query::Prepare(const std::wstring &sql)
{
	return static_cast<Result>(sqlite3_prepare16_v3(connection.handle, sql.c_str(), static_cast<int>(sql.size()), 0, &stmt, nullptr));
}

Result Query::Set(int position, int32_t value)
{
	return static_cast<Result>(sqlite3_bind_int(stmt, position + 1, value));
}

Result Query::Set(int position, int64_t value)
{
	return static_cast<Result>(sqlite3_bind_int64(stmt, position + 1, value));
}

Result Query::Set(int position, double value)
{
	return static_cast<Result>(sqlite3_bind_double(stmt, position + 1, value));
}

Result Query::Set(int position, const std::string &value)
{
	return static_cast<Result>(sqlite3_bind_text(stmt, position + 1, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
}

Result Query::Set(int position, const std::wstring &value)
{
	return static_cast<Result>(sqlite3_bind_text16(stmt, position + 1, value.c_str(), static_cast<int>(value.size() * 2), SQLITE_TRANSIENT));
}

Result Query::SetNull(int position)
{
	return static_cast<Result>(sqlite3_bind_null(stmt, position + 1));
}

bool Query::Step()
{
	return sqlite3_step(stmt) == SQLITE_ROW;
}

Result Query::Reset()
{
	return static_cast<Result>(sqlite3_reset(stmt));
}

int32_t Query::GetInt32(int position)
{
	return sqlite3_column_int(stmt, position);
}

int64_t Query::GetInt64(int position)
{
	return sqlite3_column_int64(stmt, position);
}

double Query::GetDouble(int position)
{
	return sqlite3_column_double(stmt, position);
}

std::string Query::GetString(int position)
{
	if (!GetNull(position))
	{
		return reinterpret_cast<const char*>(sqlite3_column_text(stmt, position));
	}
	return std::string();
}

std::wstring Query::GetWString(int position)
{
	if (!GetNull(position))
	{
		return static_cast<const wchar_t*>(sqlite3_column_text16(stmt, position));
	}
	return std::wstring();
}

bool Query::GetNull(int position)
{
	return sqlite3_column_type(stmt, position) == SQLITE_NULL;
}

void Query::Close()
{
	if (stmt != nullptr)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

}
