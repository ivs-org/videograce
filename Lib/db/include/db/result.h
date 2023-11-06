/**
 * result.h - Contains database results enum
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/db
 */

#pragma once

namespace db
{

enum class result
{
    OK,
	
    ///error codes
    Error,
    Internal,
    Perm,
    Abort,
    Busy,
    Locked,
    Nomem,
    ReadOnly,
    Interrupt,
    IOErr,
    Corrupt,
    NotFound,
    Full,
    CantOpen,
    Protocol,
    Empty,
    Schema,
    TooBig,
    Constraint,
    MisMath,
    MisUse,
    NoLFS,
    Auth,
    Format,
    Range,
    NotADB,
    Notice,
    Warning,
    Row,

    Done
};

}
