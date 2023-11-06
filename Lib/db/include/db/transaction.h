/**
 * transaction.h - Contains database transaction interface
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
#include <db/i_transaction.h>

#include <memory>

namespace db
{

class transaction : public i_transaction
{
public:
    transaction(connection &connection_);
    virtual ~transaction() final;

    virtual result start() final;

    virtual result commit() final;
    virtual result rollback() final;

private:
    std::unique_ptr<i_transaction> inst;
};

}
