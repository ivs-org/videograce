/**
 * connection.h - Contains database connection interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <db/i_connection.h>
#include <db/dbms.h>

#include <string>
#include <memory>

namespace db
{

class transaction;
class query;

class connection : public i_connection
{
public:
    connection(dbms dbms_, const std::string &conn_info);
    virtual ~connection();

    virtual bool is_ok() const final;
    virtual result get_result() const final;
    virtual const char* get_error_message() final;
    virtual dbms get_dbms() final;

private:
    std::unique_ptr<i_connection> inst;

    friend transaction;
    friend query;
};

}
