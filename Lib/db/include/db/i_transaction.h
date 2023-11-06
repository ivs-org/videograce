/**
 * i_transaction.h - Contains database transaction interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

#include <db/result.h>

namespace db
{

class i_transaction
{
public:
    virtual result start() = 0;

    virtual result commit() = 0;
    virtual result rollback() = 0;
	
    virtual ~i_transaction() {}

};

}
