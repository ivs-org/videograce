/**
 * query.h - Contains database query interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <db/connection.h>
#include <db/i_query.h>

namespace db
{

class query : public i_query
{
public:
    query(connection &connection_);
    virtual ~query();

    virtual result prepare(const std::string &sql) final;
    virtual result set(size_t position, int32_t value) final;
    virtual result set(size_t position, int64_t value) final;
    virtual result set(size_t position, double value) final;
    virtual result set(size_t position, const std::string &value) final;
    virtual result set_null(size_t position) final;

    virtual bool step() final;
    virtual result reset() final;

    virtual int32_t get_int32(size_t position) final;
    virtual int64_t get_int64(size_t position) final;
    virtual double get_double(size_t position) final;
    virtual std::string get_string(size_t position) final;
    virtual bool get_null(size_t position) final;

    virtual void close() final;

private:
    std::unique_ptr<i_query> inst;
};

}
