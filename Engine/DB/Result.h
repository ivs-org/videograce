/**
 * Result.h - Contains sqlite results enum
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <DB/sqlite/sqlite3.h>

#pragma once

namespace DB
{

enum Result
{
	rOK = SQLITE_OK,
	
	///error codes
	rError = SQLITE_ERROR,
	rInternal = SQLITE_INTERNAL,
	rPerm = SQLITE_PERM,
	rAbort = SQLITE_ABORT,
	rBusy = SQLITE_BUSY,
	rLocked = SQLITE_LOCKED,
	rNomem = SQLITE_NOMEM,
	rReadOnly = SQLITE_READONLY,
	rInterrupt = SQLITE_INTERRUPT,
	rIOErr = SQLITE_IOERR,
	rCorrupt = SQLITE_CORRUPT,
	rNotFound = SQLITE_NOTFOUND,
	rFull = SQLITE_FULL,
	rCantOpen = SQLITE_CANTOPEN,
	rProtocol = SQLITE_PROTOCOL,
	rEmpty = SQLITE_EMPTY,
	rSchema = SQLITE_SCHEMA,
	rTooBig = SQLITE_TOOBIG,
	rConstraint = SQLITE_CONSTRAINT,
	rMisMath = SQLITE_MISMATCH,
	rMisUse = SQLITE_MISUSE,
	rNoLFS = SQLITE_NOLFS,
	rAuth = SQLITE_AUTH,
	rFormat = SQLITE_FORMAT,
	rRange = SQLITE_RANGE,
	rNotADB = SQLITE_NOTADB,
	rNotice = SQLITE_NOTICE,
	rWarning = SQLITE_WARNING,
	rRow = SQLITE_ROW,
	rDone = SQLITE_DONE
};

}
