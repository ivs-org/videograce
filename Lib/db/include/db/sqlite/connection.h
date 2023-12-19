/**
 * connection.h - Contains sqlite connection wrapper
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <string>

#include <db/i_connection.h>

#include <sqlite/sqlite3.h>

namespace db
{

namespace sqlite
{

class transaction;
class query;

class connection : public i_connection
{
public:
    connection(std::string_view path);
    virtual ~connection();

    virtual bool is_ok() const final;
    virtual result get_result() const final;
    virtual const char* get_error_message() final;
    virtual dbms get_dbms() final { return dbms::SQLite; }

private:
    sqlite3 *handle;

    result result_;

    friend transaction;
    friend query;
};

}

}
