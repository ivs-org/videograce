/**
 * i_query.h - Contains database query interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <cstdint>
#include <string>

#include <db/result.h>

namespace db
{

class i_query
{
public:
    virtual result prepare(const std::string &sql) = 0;
    virtual result set(size_t position, int32_t value) = 0;
    virtual result set(size_t position, int64_t value) = 0;
    virtual result set(size_t position, double value) = 0;
    virtual result set(size_t position, const std::string &value) = 0;
    virtual result set_null(size_t position) = 0;

    virtual bool step() = 0;
    virtual result reset() = 0;

    virtual int32_t get_int32(size_t position) = 0;
    virtual int64_t get_int64(size_t position) = 0;
    virtual double get_double(size_t position) = 0;
    virtual std::string get_string(size_t position) = 0;
    virtual bool get_null(size_t position) = 0;

    virtual void close() = 0;

    virtual ~i_query() {}
};

}
