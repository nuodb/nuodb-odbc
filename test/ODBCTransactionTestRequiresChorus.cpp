/**
 * (C) Copyright 2016-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "ODBCTestBase.h"

//
//
class ODBCTransactionTestRequiresChorus : public ODBCTestBase
{
public:
    void doTwoConnectionsTest(int existingConnectionRowsRead, int transactionIsolation);
};

TEST_F(ODBCTransactionTestRequiresChorus, Rollback)
{
    EXPECT_EQ(SQL_SUCCESS, SQLSetConnectAttr(hdbc1,
                                        SQL_ATTR_AUTOCOMMIT,
                                        (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                        SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create table person (first_name string)", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, hdbc1, SQL_COMMIT));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"select count(*) from person", SQL_NTS));
    RETCODE ret = SQLFetch(stmt);
    ASSERT_TRUE(SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret);
    SQLINTEGER count;
    SQLLEN indicator;
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_SLONG, &count, sizeof(SQLINTEGER), &indicator));
    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_EQ(0, count);

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"insert into person (first_name) values ('oleg')", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, hdbc1, SQL_ROLLBACK));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"select count(*) from person", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_SLONG, &count, sizeof(SQLINTEGER), &indicator));
    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_EQ(0, count);

    EXPECT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, hdbc1, SQL_COMMIT));
}

TEST_F(ODBCTransactionTestRequiresChorus, Level)
{
    EXPECT_EQ(SQL_SUCCESS, SQLSetConnectAttr(hdbc1,
                                             SQL_ATTR_TXN_ISOLATION,
                                             (SQLPOINTER)SQL_TXN_SERIALIZABLE,
                                             SQL_NTS));
    SQLINTEGER value;
    EXPECT_EQ(SQL_SUCCESS, SQLGetInfo(hdbc1, SQL_DEFAULT_TXN_ISOLATION, &value, 0, 0));
    EXPECT_EQ(SQL_TXN_SERIALIZABLE, value);
}

const char *SELECT_SQL = "SELECT OID, BackupPath, BackupRollAlg, SmtpHost, SmtpPort, JdoVersion, "
"BackupInMessageHandler FROM DefaultExchangePointSettings WHERE OID = 1000";

void ODBCTransactionTestRequiresChorus::doTwoConnectionsTest(int existingConnectionRowsRead, int transactionIsolation)
{
    execDirect("drop table DefaultExchangePointSettings if exists");
    execDirect("CREATE TABLE DefaultExchangePointSettings("
        "OID NUMERIC(19, 0) NOT NULL,"
        "BackupPath VARCHAR(255) NOT NULL,"
        "BackupRollAlg SMALLINT NULL,"
        "SmtpHost VARCHAR(512) NOT NULL,"
        "SmtpPort INTEGER NOT NULL,"
        "JdoVersion NUMERIC(19, 0) DEFAULT (0) NOT NULL,"
        "BackupInMessageHandler SMALLINT DEFAULT (1) NOT NULL,"
        "PRIMARY KEY (OID))");

    // connection 1
    HDBC conn1 = NULL;
    newConnection(conn1);
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn1,
                                             SQL_ATTR_TXN_ISOLATION,
                                             (SQLPOINTER)(intptr_t)transactionIsolation,
                                             0));
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn1,
                                             SQL_ATTR_AUTOCOMMIT,
                                             (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                             0));
    // connection 2
    HDBC conn2 = NULL;
    newConnection(conn2);
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn2,
                                             SQL_ATTR_AUTOCOMMIT,
                                             (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                             SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn2,
                                             SQL_ATTR_TXN_ISOLATION,
                                             (SQLPOINTER)(intptr_t)transactionIsolation,
                                             0));

    // statement on connection1
    HSTMT s1 = NULL;
    ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, conn1, &s1));
    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(s1, (SQLCHAR*)"use " SCHEMANAME, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLPrepare(s1, (SQLCHAR*)SELECT_SQL, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLExecute(s1));
    int numRows = 0;
    while (SQLFetch(s1) == SQL_SUCCESS) {
        numRows++;
    }
    ASSERT_EQ(0, numRows);

    // close down statement on connection 1
    ASSERT_EQ(SQL_SUCCESS, SQLFreeStmt(s1, SQL_CLOSE));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, s1));

    // statement on connection 2
    HSTMT s2 = NULL;
    ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, conn2, &s2));
    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(s2, (SQLCHAR*)"use " SCHEMANAME, SQL_NTS));

    ASSERT_EQ(SQL_SUCCESS, SQLPrepare(s2,
                                      (SQLCHAR*)"INSERT INTO DefaultExchangePointSettings (OID, BackupPath, BackupRollAlg, "
                                      "SmtpHost, SmtpPort, JdoVersion, BackupInMessageHandler) "
                                      "VALUES (1000, 'C:\\backup', 1, 'mail.smtp.com', 25, 1, 1)",
                                      SQL_NTS));

    ASSERT_EQ(SQL_SUCCESS, SQLExecute(s2));

    SQLLEN rowsInserted;
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(s2, &rowsInserted));
    ASSERT_EQ(1, rowsInserted);

    // close down connection 2
    SQLFreeStmt(s2, SQL_CLOSE);

    ASSERT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, conn2, SQL_COMMIT));

    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, s2));
    ASSERT_EQ(SQL_SUCCESS, SQLDisconnect(conn2));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_DBC, conn2));

    // new statement on connection 1
    ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, conn1, &s1));
    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(s1, (SQLCHAR*)"use " SCHEMANAME, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLPrepare(s1, (SQLCHAR*)SELECT_SQL, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLExecute(s1));
    numRows = 0;
    while (SQLFetch(s1) == SQL_SUCCESS) {
        numRows++;
    }

    // this is an existing connection, so this is where we will
    // get difference in # of rows read depending upon the
    // TransactionIsolation
    ASSERT_EQ(existingConnectionRowsRead, numRows);

    // close down connection 1
    ASSERT_EQ(SQL_SUCCESS, SQLFreeStmt(s1, SQL_CLOSE));
    ASSERT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, conn1, SQL_COMMIT));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, s1));
    ASSERT_EQ(SQL_SUCCESS, SQLDisconnect(conn1));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_DBC, conn1));

    // connection 3
    HDBC conn3 = NULL;
    newConnection(conn3);
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn3,
                                             SQL_ATTR_AUTOCOMMIT,
                                             (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                             SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(conn3,
                                             SQL_ATTR_TXN_ISOLATION,
                                             (SQLPOINTER)(intptr_t)transactionIsolation,
                                             0));
    // statement on connection 3
    HSTMT s3 = NULL;
    ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, conn3, &s3));
    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(s3, (SQLCHAR*)"use " SCHEMANAME, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLPrepare(s3, (SQLCHAR*)SELECT_SQL, SQL_NTS));
    ASSERT_EQ(SQL_SUCCESS, SQLExecute(s3));
    numRows = 0;
    while (SQLFetch(s3) == SQL_SUCCESS) {
        numRows++;
    }
    // this is a brand new connection so we should always read 1 row
    ASSERT_EQ(1, numRows);

    // close down connection 3
    ASSERT_EQ(SQL_SUCCESS, SQLFreeStmt(s3, SQL_CLOSE));
    ASSERT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, conn3, SQL_COMMIT));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, s3));
    ASSERT_EQ(SQL_SUCCESS, SQLDisconnect(conn3));
    ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_DBC, conn3));
}

TEST_F(ODBCTransactionTestRequiresChorus, TwoConnectionReadCommitted)
{
    doTwoConnectionsTest(1, SQL_TXN_READ_COMMITTED);
}

TEST_F(ODBCTransactionTestRequiresChorus, TwoConnectionSerializable)
{
    doTwoConnectionsTest(0, SQL_TXN_SERIALIZABLE);
}
