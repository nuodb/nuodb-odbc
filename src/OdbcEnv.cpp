/**
 * (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "OdbcEnv.h"

#include <stdlib.h>

#include "OdbcBase.h"
#include "OdbcConnection.h"
#include "OdbcObject.h"

#include "NuoRemote/CallableStatement.h"
#include "NuoRemote/Connection.h"
#include "NuoRemote/DatabaseMetaData.h"
#include "NuoRemote/PreparedStatement.h"


OdbcEnv::OdbcEnv()
{
#ifndef _WIN32
    if (!(odbcIniFileName = getenv("ODBCINI")))
#endif
    odbcIniFileName = "ODBC.INI";
}

RETCODE OdbcEnv::allocHandle(int handleType, SQLHANDLE* outputHandle)
{
    clearErrors();
    *outputHandle = SQL_NULL_HDBC;

    if (handleType != SQL_HANDLE_DBC) {
        return sqlReturn(SQL_ERROR, "HY000", "General Error");
    }

    OdbcConnection* connection = new OdbcConnection(this);
    connection->next = connections;
    connections = connection;
    *outputHandle = connection;

    return sqlSuccess();
}

RETCODE OdbcEnv::sqlEndTran(int operation)
{
    clearErrors();
    RETCODE ret = SQL_SUCCESS;

    for (OdbcConnection* connection = connections; connection;
         connection = (OdbcConnection*)connection->next) {
        RETCODE retcode = connection->sqlEndTran(operation);
        if (retcode != SQL_SUCCESS) {
            ret = retcode;
        }
    }

    return ret;
}

void OdbcEnv::connectionClosed(OdbcConnection* connection)
{
    for (OdbcObject** ptr = (OdbcObject**)&connections; *ptr; ptr = &((*ptr)->next)) {
        if (*ptr == connection) {
            *ptr = connection->next;
            break;
        }
    }
}

RETCODE OdbcEnv::sqlSetEnvAttr(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER length)
{
    clearErrors();

    return sqlSuccess();
}

RETCODE OdbcEnv::sqlGetEnvAttr(SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER length, SQLINTEGER* stringLength)
{
    clearErrors();

    switch (attribute) {
        case SQL_ATTR_CONNECTION_POOLING: {
            int* p = (int*)value;
            *p = SQL_CP_OFF;
            break;
        }
        case SQL_ATTR_CP_MATCH: {
            int* p = (int*)value;
            *p = SQL_CP_STRICT_MATCH;
            break;
        }
        case SQL_ATTR_ODBC_VERSION: {
            int* p = (int*)value;
            *p = SQL_OV_ODBC3;
            break;
        }
        case SQL_ATTR_OUTPUT_NTS: {
            int* p = (int*)value;
            *p = SQL_TRUE;
            break;
        }
        default:
            notYetImplemented("Unimplemented SQLGetEnvAttr attribute");
            break;
    }

    return sqlSuccess();
}
