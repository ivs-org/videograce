/**
 * i_connection.h - Contains database connection interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <db/dbms.h>
#include <db/result.h>

namespace db
{

class i_connection
{
public:
    virtual bool is_ok() const = 0;
    virtual result get_result() const = 0;
    virtual const char* get_error_message() = 0;
    virtual dbms get_dbms() = 0;

    virtual ~i_connection() {}
};

}
