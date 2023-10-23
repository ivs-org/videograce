/**
 * Transaction.cpp - Contains sqlite transaction wrapper impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <DB/Transaction.h>

namespace DB
{

Transaction::Transaction(Connection &connection_)
	: connection(connection_), started(false)
{
}

Transaction::~Transaction()
{
	if (started)
	{
		Rollback();
	}
}

Result Transaction::Start()
{
	started = true;
	return static_cast<Result>(sqlite3_exec(connection.handle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr));
}

Result Transaction::Commit()
{
	started = false;
	return static_cast<Result>(sqlite3_exec(connection.handle, "COMMIT;", nullptr, nullptr, nullptr));
}

Result Transaction::Rollback()
{
	started = false;
	return static_cast<Result>(sqlite3_exec(connection.handle, "ROLLBACK;", nullptr, nullptr, nullptr));
}

}
