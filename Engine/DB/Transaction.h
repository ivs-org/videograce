/**
 * Transaction.h - Contains sqlite transaction wrapper
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <DB/Connection.h>

namespace DB
{

class Transaction
{
public:
	Transaction(Connection &connection);
	~Transaction();

	Result Start();

	Result Commit();
	Result Rollback();
	
private:
	Connection &connection;

	bool started;

};

}
