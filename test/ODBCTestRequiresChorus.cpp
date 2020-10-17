/**
 * (C) Copyright 2012-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "ODBCTestBase.h"

#include <vector>
#include <string>

#include "gtest/gtest.h"


#ifdef ODBC_WINDOWS
#define EXPSTREQ(_e,_v) EXPECT_STREQ((_e), convertToUTF8FromUnicode(_v).c_str())
#else
#define EXPSTREQ(_e,_v) EXPECT_STREQ((_e), (const char*)(_v))
#endif

class ODBCTestRequiresChorus : public ODBCTestBase
{
public:
    void setupTable()
    {
        RETCODE     retcode;
        const char* createStmt =
            "CREATE TABLE mxLattice ("
            "mxName nvarchar(128),"
            "mxInt integer,"
            "mxShort smallint,"
            "mxBigInt bigint,"
            "mxDouble double,"
            "mxChar smallint,"
            "mxFloat float,"
            "mxDecimal decimal(10,2),"
            "mxTableSpc nvarchar(128),"
            "mxDate date,"
            "mxTime time,"
            "mxTimeStamp timestamp,"
            "mxBinary binary(4099),"    // has to be big enough for DB915 test
            "mxClob clob)";

        ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)createStmt, SQL_NTS));

        ColumnBinding<TEST_CHAR_TYPE> params;
        memset(&params, 0, sizeof(params));

        const char* insertSQL = "insert into mxLattice (mxName, mxInt, mxShort, mxBigInt, mxDouble, mxChar, mxFloat, mxDecimal, mxTableSpc, mxDate, mxTime, mxTimeStamp, mxBinary, mxClob) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        // Allocate statement handle.
        SQLHSTMT insertStmt;
        ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &insertStmt));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                1,
                                                SQL_PARAM_INPUT,
                                                SQL_C_CHAR,
                                                TEST_SQL_C_CHAR,
                                                VALSIZE,
                                                0,
                                                &params.mxname,
                                                VALSIZE*sizeof(TEST_CHAR_TYPE),
                                                &params.mxnameInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                2,
                                                SQL_PARAM_INPUT,
                                                SQL_C_SLONG,
                                                SQL_INTEGER,
                                                0,
                                                0,
                                                &params.mxInt,
                                                0,
                                                &params.mxIntInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                3,
                                                SQL_PARAM_INPUT,
                                                SQL_C_SSHORT,
                                                SQL_SMALLINT,
                                                0,
                                                0,
                                                &params.mxShort,
                                                0,
                                                &params.mxShortInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                4,
                                                SQL_PARAM_INPUT,
                                                SQL_C_SBIGINT,
                                                SQL_BIGINT,
                                                0,
                                                0,
                                                &params.mxBigInt,
                                                0,
                                                &params.mxBigIntInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                5,
                                                SQL_PARAM_INPUT,
                                                SQL_C_DOUBLE,
                                                SQL_DOUBLE,
                                                0,
                                                0,
                                                &params.mxDouble,
                                                0,
                                                &params.mxDoubleInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                6,
                                                SQL_PARAM_INPUT,
                                                SQL_C_STINYINT,
                                                SQL_TINYINT,
                                                0,
                                                0,
                                                &params.mxTinyInt,
                                                0,
                                                &params.mxTinyIntInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                7,
                                                SQL_PARAM_INPUT,
                                                SQL_C_FLOAT,
                                                SQL_FLOAT,
                                                0,
                                                0,
                                                &params.mxFloat,
                                                0,
                                                &params.mxFloatInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                8,
                                                SQL_PARAM_INPUT,
                                                SQL_C_CHAR,
                                                SQL_DECIMAL,
                                                10,
                                                2,
                                                &params.mxDecimal,
                                                VALSIZE*sizeof(TEST_CHAR_TYPE),
                                                &params.mxDecimalInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                9,
                                                SQL_PARAM_INPUT,
                                                SQL_C_CHAR,
                                                TEST_SQL_C_CHAR,
                                                VALSIZE,
                                                0,
                                                &params.mxtablespc,
                                                VALSIZE*sizeof(TEST_CHAR_TYPE),
                                                &params.mxtablespcInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                10,
                                                SQL_PARAM_INPUT,
                                                SQL_C_TYPE_DATE,
                                                SQL_TYPE_DATE,
                                                sizeof(SQL_DATE_STRUCT),
                                                0,
                                                &params.mxDate,
                                                0,
                                                &params.mxDateInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                11,
                                                SQL_PARAM_INPUT,
                                                SQL_C_TYPE_TIME,
                                                SQL_TYPE_TIME,
                                                sizeof(SQL_TIME_STRUCT),
                                                0,
                                                &params.mxTime,
                                                0,
                                                &params.mxTimeInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                12,
                                                SQL_PARAM_INPUT,
                                                SQL_C_TYPE_TIMESTAMP,
                                                SQL_TYPE_TIMESTAMP,
                                                sizeof(SQL_TIMESTAMP_STRUCT),
                                                0,
                                                &params.mxTimeStamp,
                                                0,
                                                &params.mxTimeStampInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                13,
                                                SQL_PARAM_INPUT,
                                                SQL_C_BINARY,
                                                SQL_C_BINARY,
                                                VALSIZE,
                                                0,
                                                &params.mxBinary,
                                                VALSIZE,
                                                &params.mxBinaryInd));

        ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(insertStmt,
                                                14,
                                                SQL_PARAM_INPUT,
                                                SQL_C_CHAR,
                                                TEST_SQL_C_CHAR,
                                                VALSIZE,
                                                0,
                                                &params.mxClob,
                                                VALSIZE*sizeof(TEST_CHAR_TYPE),
                                                &params.mxClobInd));

        ASSERT_EQ(SQL_SUCCESS, SQLPrepare(insertStmt,
                                          (SQLCHAR*)insertSQL, SQL_NTS));
        ASSERT_EQ(SQL_SUCCESS, SQLPrepare(insertStmt,
                                          (SQLCHAR*)insertSQL, SQL_NTS));

        for (int i = 0; i < NUMROWS; i++) {
            if (NULLROW(i)) {
                params.mxnameInd = SQL_NULL_DATA;
                params.mxIntInd = SQL_NULL_DATA;
                params.mxShortInd = SQL_NULL_DATA;
                params.mxBigIntInd = SQL_NULL_DATA;
                params.mxDoubleInd = SQL_NULL_DATA;
                params.mxTinyIntInd = SQL_NULL_DATA;
                params.mxFloatInd = SQL_NULL_DATA;
                params.mxDecimalInd = SQL_NULL_DATA;
                params.mxtablespcInd = SQL_NULL_DATA;
                params.mxDateInd = SQL_NULL_DATA;
                params.mxTimeInd = SQL_NULL_DATA;
                params.mxTimeStampInd = SQL_NULL_DATA;
                params.mxBinaryInd = SQL_NULL_DATA;
                params.mxClobInd = SQL_NULL_DATA;
            } else {
                sprintf((char*)params.mxname, "%s%d", FIRSTSTRING, i);
                params.mxnameInd = SQL_NTS;
                params.mxInt = i;
                params.mxIntInd = 0;
                params.mxShort = i;
                params.mxShortInd = 0;
                params.mxBigInt = i;
                params.mxBigIntInd = 0;
                params.mxDouble = i;
                params.mxDoubleInd = 0;
                params.mxTinyInt = i;
                params.mxTinyIntInd = 0;
                params.mxFloat = (float)i;
                params.mxFloatInd = 0;
                sprintf((char *)params.mxDecimal, "10.1");
                params.mxDecimalInd = SQL_NTS;
                sprintf((char*)params.mxtablespc, "%s%d", SECONDSTRING, i);
                params.mxtablespcInd = SQL_NTS;
                params.mxDate.month = 10;
                params.mxDate.day = 18;
                params.mxDate.year = YEARBASE(i);
                params.mxDateInd = 0;
                params.mxTime.hour = 10;
                params.mxTime.minute = i % 60;
                params.mxTime.second = 33;
                params.mxTimeInd = 0;
                params.mxTimeStamp.month = 10;
                params.mxTimeStamp.day = 18;
                params.mxTimeStamp.year = YEARBASE(i);
                params.mxTimeStamp.hour = 10;
                params.mxTimeStamp.minute = i % 60;
                params.mxTimeStamp.second = 33;
                params.mxTimeStampInd = 0;
                sprintf(params.mxBinary, "%s%d", BINARYSTRING, i);
                params.mxBinaryInd = SQL_LEN_DATA_AT_EXEC((int)strlen(params.mxBinary));
                sprintf((char*)params.mxClob, "%s%d", CLOBSTRING, i);
                params.mxClobInd = SQL_LEN_DATA_AT_EXEC((int)strlen((const char*)params.mxClob));
            }

            retcode = SQLExecute(insertStmt);

            if (retcode == SQL_NEED_DATA) {
                PTR         paramId;
                int         clobOffset = 0;
                const int   CHUNK = 4;

                while (SQLParamData(insertStmt, &paramId) == SQL_NEED_DATA) {

                    if (paramId == &params.mxClob) {
                        ASSERT_EQ(SQL_SUCCESS, SQLPutData(insertStmt, params.mxClob + clobOffset, clobOffset == 0 ? CHUNK*sizeof(TEST_CHAR_TYPE) : SQL_NTS));
                        clobOffset = clobOffset == 0 ? CHUNK : clobOffset;

                    } else if (paramId == &params.mxBinary) {
                        ASSERT_EQ(SQL_SUCCESS, SQLPutData(insertStmt, params.mxBinary, strlen(params.mxBinary)));

                    } else {
                        FAIL() << "Unexpected paramId: " << paramId;
                    }
                }
            } else {
                ASSERT_EQ(SQL_SUCCESS, retcode);
            }
        }

        ASSERT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, insertStmt));
    }

    void extractError(SQLHANDLE handle, SQLSMALLINT handleType, std::string& stateRet, std::string& messageRet)
    {
        SQLSMALLINT i = 0;
        SQLINTEGER  native;
        SQLCHAR     state[7];
        SQLCHAR     text[256];
        SQLSMALLINT len;
        SQLRETURN   ret;

        do {
            ret = SQLGetDiagRec(handleType, handle, ++i, state, &native,
                                text, sizeof(text), &len);
            if (SQL_SUCCEEDED(ret)) {
                stateRet.assign((const char*)state);
                messageRet.assign((const char*)text);
            }
        } while (ret == SQL_SUCCESS);
    }

    void rowWiseBinding(int numRowsPerFetch);
    void columnWiseBinding(int numRowsPerFetch);
    void bindRows(int numRowsPerFetch, RowWiseBinding<TEST_CHAR_TYPE>* rows);
    void bindColumns(int numRowsPerFetch, ColumnWiseBinding<TEST_CHAR_TYPE>* rows);
};

void ODBCTestRequiresChorus::bindRows(int numRowsPerFetch, RowWiseBinding<TEST_CHAR_TYPE>* rows)
{
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)SQL_BIND_BY_COLUMN, 0));

    SQLULEN ip = numRowsPerFetch;
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ip, 0));

    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, rows->rowStatus.data(), 0));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, TEST_SQL_C_CHAR, rows->mxname.data(), VALSIZE*sizeof(TEST_CHAR_TYPE), rows->mxnameInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 2, SQL_C_LONG, rows->mxInt.data(), 0, rows->mxIntInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 3, SQL_C_SHORT, rows->mxShort.data(), 0, rows->mxShortInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 4, SQL_C_SBIGINT, rows->mxBigInt.data(), 0, rows->mxBigIntInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 5, SQL_C_DOUBLE, rows->mxDouble.data(), 0, rows->mxDoubleInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 6, SQL_C_TINYINT, rows->mxTinyInt.data(), 0, rows->mxTinyIntInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 7, SQL_C_FLOAT, rows->mxFloat.data(), 0, rows->mxFloatInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 8, TEST_SQL_C_CHAR, rows->mxDecimal.data(), 10*sizeof(TEST_CHAR_TYPE), rows->mxDecimalInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 9, TEST_SQL_C_CHAR, rows->mxtablespc.data(), VALSIZE*sizeof(TEST_CHAR_TYPE), rows->mxtablespcInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 10, SQL_C_DATE, rows->mxDate.data(), 0, rows->mxDateInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 11, SQL_C_TIME, rows->mxTime.data(), 0, rows->mxTimeInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 12, SQL_C_TIMESTAMP, rows->mxTimeStamp.data(), 0, rows->mxTimeStampInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 13, SQL_C_BINARY, rows->mxBinary.data(), VALSIZE, rows->mxBinaryInd.data()));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 14, TEST_SQL_C_CHAR, rows->mxClob.data(), VALSIZE*sizeof(TEST_CHAR_TYPE), rows->mxClobInd.data()));
}

void ODBCTestRequiresChorus::rowWiseBinding(int numRowsPerFetch)
{
    RETCODE retcode;

    setupTable();

    SQLULEN rowsFetched;
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER)&rowsFetched, 0));

    // directly execute the statement

    const char* stmtSQL = "SELECT * from mxLattice";
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)stmtSQL, SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)stmtSQL, SQL_NTS));

    auto rows = std::make_unique<RowWiseBinding<TEST_CHAR_TYPE>>(numRowsPerFetch);
    bindRows(numRowsPerFetch, rows.get());

    char buffer[1024];

    int index = 0;
    int loopcount = 0;

    while ((retcode = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0)) != SQL_NO_DATA) {

        for (SQLULEN i = 0; i < rowsFetched; i++) {

            SCOPED_TRACE(testing::Message() << "Row: " << (loopcount * rowsFetched) + i);

            if (rows->rowStatus[i] == SQL_ROW_SUCCESS || rows->rowStatus[i] == SQL_ROW_SUCCESS_WITH_INFO) {
                if (NULLROW(index)) {
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxnameInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxIntInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxShortInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxBigIntInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxDoubleInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxTinyIntInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxFloatInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxDecimalInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxtablespcInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxDateInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxTimeInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxTimeStampInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxBinaryInd[i]);
                    EXPECT_EQ(SQL_NULL_DATA, rows->mxClobInd[i]);
                } else {

                    sprintf(buffer, "%s%d", FIRSTSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->mxnameInd[i]);
                    EXPSTREQ(buffer, &rows->mxname[VALSIZE*i]);

                    EXPECT_EQ(4, rows->mxIntInd[i]);
                    EXPECT_EQ(index, rows->mxInt[i]);

                    EXPECT_EQ(2, rows->mxShortInd[i]);
                    EXPECT_EQ(index, rows->mxShort[i]);

                    EXPECT_EQ(8, rows->mxBigIntInd[i]);
                    EXPECT_EQ(index, rows->mxBigInt[i]);

                    EXPECT_EQ(8, rows->mxDoubleInd[i]);
                    EXPECT_EQ(index, rows->mxDouble[i]);

                    EXPECT_EQ(1, rows->mxTinyIntInd[i]);
                    EXPECT_EQ((char)index, (int)rows->mxTinyInt[i]);

                    EXPECT_EQ(4, rows->mxFloatInd[i]);
                    EXPECT_EQ(index, rows->mxFloat[i]);

                    sprintf(buffer, "10.10");
// Linux/Mac ODBC disagrees with Windows about the answer here.
// Linux returns the trimmed length, Windows pads to 10.
// DB-14560
#ifdef _WIN32
#define DECLEN 10
#else
#define DECLEN 5
#endif
                    EXPECT_EQ(DECLEN, rows->mxDecimalInd[i]);
                    EXPSTREQ(buffer, &rows->mxDecimal[10*i]);

                    sprintf(buffer, "%s%d", SECONDSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->mxtablespcInd[i]);
                    EXPSTREQ(buffer, &rows->mxtablespc[VALSIZE*i]);

                    EXPECT_EQ(sizeof(SQL_DATE_STRUCT), (size_t)rows->mxDateInd[i]);
                    EXPECT_EQ(YEARBASE(index), rows->mxDate[i].year);
                    EXPECT_EQ(10, rows->mxDate[i].month);
                    EXPECT_EQ(18, rows->mxDate[i].day);

                    EXPECT_EQ(sizeof(SQL_TIME_STRUCT), (size_t)rows->mxTimeInd[i]);
                    EXPECT_EQ(10, rows->mxTime[i].hour);
                    EXPECT_EQ(index % 60, rows->mxTime[i].minute);
                    EXPECT_EQ(33, rows->mxTime[i].second);

                    EXPECT_EQ(sizeof(SQL_TIMESTAMP_STRUCT), (size_t)rows->mxTimeStampInd[i]);
                    EXPECT_EQ(YEARBASE(index), rows->mxTimeStamp[i].year);
                    EXPECT_EQ(10, rows->mxTimeStamp[i].month);
                    EXPECT_EQ(18, rows->mxTimeStamp[i].day);
                    EXPECT_EQ(10, rows->mxTimeStamp[i].hour);
                    EXPECT_EQ(index % 60, rows->mxTimeStamp[i].minute);
                    EXPECT_EQ(33, rows->mxTimeStamp[i].second);

                    sprintf(buffer, "%s%d", BINARYSTRING, index);
                    EXPECT_EQ(strlen(buffer), (size_t)rows->mxBinaryInd[i]);
                    for (int el = 0; el < rows->mxBinaryInd[i]; el++) {
                        EXPECT_EQ(buffer[el], rows->mxBinary[(VALSIZE*i) + el]) << "mxBinary offset " << el;
                    }

                    sprintf(buffer, "%s%d", CLOBSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->mxClobInd[i]);
                    EXPSTREQ(buffer, &rows->mxClob[VALSIZE*i]);
                }

                index++;
            }
        }

        if (loopcount++ % 10 == 0) { // rebind every so often
            rows = std::make_unique<RowWiseBinding<TEST_CHAR_TYPE>>(numRowsPerFetch);
            bindRows(numRowsPerFetch, rows.get());
        }
    }

    EXPECT_EQ(NUMROWS, index);
    EXPECT_EQ(SQL_NO_DATA_FOUND, SQLFetch(stmt));
}

void ODBCTestRequiresChorus::bindColumns(int numRowsPerFetch, ColumnWiseBinding<TEST_CHAR_TYPE>* rows)
{
    SQLULEN ul = numRowsPerFetch;

    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ul, 0));

    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, rows->rowStatus, 0));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, TEST_SQL_C_CHAR, rows->rows[0].mxname, VALSIZE, &rows->rows[0].mxnameInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 2, SQL_C_LONG, &rows->rows[0].mxInt, 0, &rows->rows[0].mxIntInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 3, SQL_C_SHORT, &rows->rows[0].mxShort, 0, &rows->rows[0].mxShortInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 4, SQL_C_SBIGINT, &rows->rows[0].mxBigInt, 0, &rows->rows[0].mxBigIntInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 5, SQL_C_DOUBLE, &rows->rows[0].mxDouble, 0, &rows->rows[0].mxDoubleInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 6, SQL_C_TINYINT, &rows->rows[0].mxTinyInt, 0, &rows->rows[0].mxTinyIntInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 7, SQL_C_FLOAT, &rows->rows[0].mxFloat, 0, &rows->rows[0].mxFloatInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 8, TEST_SQL_C_CHAR, &rows->rows[0].mxDecimal, VALSIZE, &rows->rows[0].mxDecimalInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 9, TEST_SQL_C_CHAR, rows->rows[0].mxtablespc, VALSIZE, &rows->rows[0].mxtablespcInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 10, SQL_C_DATE, &rows->rows[0].mxDate, 0, &rows->rows[0].mxDateInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 11, SQL_C_TIME, &rows->rows[0].mxTime, 0, &rows->rows[0].mxTimeInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 12, SQL_C_TIMESTAMP, &rows->rows[0].mxTimeStamp, 0, &rows->rows[0].mxTimeStampInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 13, SQL_C_BINARY, rows->rows[0].mxBinary, VALSIZE, &rows->rows[0].mxBinaryInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 14, TEST_SQL_C_CHAR, rows->rows[0].mxClob, VALSIZE, &rows->rows[0].mxClobInd));
}

void ODBCTestRequiresChorus::columnWiseBinding(int numRowsPerFetch)
{
    RETCODE retcode;

    setupTable();

    SQLULEN rowsFetched;

    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(ColumnBinding<TEST_CHAR_TYPE>), 0));

    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER)&rowsFetched, 0));

    // directly execute the statement
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT * from mxLattice", SQL_NTS));

    auto rows = std::make_unique<ColumnWiseBinding<TEST_CHAR_TYPE>>(numRowsPerFetch);
    bindColumns(numRowsPerFetch, rows.get());

    char buffer[1024];

    int index = 0;
    int loopcount = 0;

    while ((retcode = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0)) != SQL_NO_DATA) {
        for (SQLULEN i = 0; i < rowsFetched; i++) {
            if (rows->rowStatus[i] == SQL_ROW_SUCCESS || rows->rowStatus[i] == SQL_ROW_SUCCESS_WITH_INFO) {

                if (NULLROW(index)) {
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxnameInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxIntInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxShortInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxBigIntInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxDoubleInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxTinyIntInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxFloatInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxDecimalInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxtablespcInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxDateInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxTimeInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxTimeStampInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxBinaryInd);
                    EXPECT_EQ(SQL_NULL_DATA, rows->rows[i].mxClobInd);
                } else {
                    sprintf(buffer, "%s%d", FIRSTSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->rows[i].mxnameInd);
                    EXPSTREQ(buffer, rows->rows[i].mxname);

                    EXPECT_EQ(4, rows->rows[i].mxIntInd);
                    EXPECT_EQ(index, rows->rows[i].mxInt);

                    EXPECT_EQ(2, rows->rows[i].mxShortInd);
                    EXPECT_EQ(index, rows->rows[i].mxShort);

                    EXPECT_EQ(8, rows->rows[i].mxBigIntInd);
                    EXPECT_EQ(index, rows->rows[i].mxBigInt);

                    EXPECT_EQ(8, rows->rows[i].mxDoubleInd);
                    EXPECT_EQ(index, rows->rows[i].mxDouble);

                    EXPECT_EQ(1, rows->rows[i].mxTinyIntInd);
                    EXPECT_EQ((char)index, (char)rows->rows[i].mxTinyInt);

                    EXPECT_EQ(4, rows->rows[i].mxFloatInd);
                    EXPECT_EQ(index, rows->rows[i].mxFloat);

                    sprintf(buffer, "10.10");
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->rows[i].mxDecimalInd);
                    EXPSTREQ(buffer, rows->rows[i].mxDecimal);

                    sprintf(buffer, "%s%d", SECONDSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->rows[i].mxtablespcInd);
                    EXPSTREQ(buffer, rows->rows[i].mxtablespc);

                    EXPECT_EQ(sizeof(SQL_DATE_STRUCT), (size_t)rows->rows[i].mxDateInd);
                    EXPECT_EQ(YEARBASE(index), rows->rows[i].mxDate.year);
                    EXPECT_EQ(10, rows->rows[i].mxDate.month);
                    EXPECT_EQ(18, rows->rows[i].mxDate.day);

                    EXPECT_EQ(sizeof(SQL_TIME_STRUCT), (size_t)rows->rows[i].mxTimeInd);
                    EXPECT_EQ(10, rows->rows[i].mxTime.hour);
                    EXPECT_EQ(index % 60, rows->rows[i].mxTime.minute);
                    EXPECT_EQ(33, rows->rows[i].mxTime.second);

                    EXPECT_EQ(sizeof(SQL_TIMESTAMP_STRUCT), (size_t)rows->rows[i].mxTimeStampInd);
                    EXPECT_EQ(YEARBASE(index), rows->rows[i].mxTimeStamp.year);
                    EXPECT_EQ(10, rows->rows[i].mxTimeStamp.month);
                    EXPECT_EQ(18, rows->rows[i].mxTimeStamp.day);
                    EXPECT_EQ(10, rows->rows[i].mxTimeStamp.hour);
                    EXPECT_EQ(index % 60, rows->rows[i].mxTimeStamp.minute);
                    EXPECT_EQ(33, rows->rows[i].mxTimeStamp.second);

                    sprintf(buffer, "%s%d", BINARYSTRING, index);
                    EXPECT_EQ(strlen(buffer), (size_t)rows->rows[i].mxBinaryInd);
                    for (int el = 0; el < rows->rows[i].mxBinaryInd; el++) {
                        EXPECT_EQ(buffer[el], rows->rows[i].mxBinary[el]);
                    }

                    sprintf(buffer, "%s%d", CLOBSTRING, index);
                    EXPECT_EQ(strlen(buffer)*sizeof(TEST_CHAR_TYPE), (size_t)rows->rows[i].mxClobInd);
                    EXPSTREQ(buffer, rows->rows[i].mxClob);
                }
                index++;
            }
        }

        if (loopcount++ % 10 == 0) { // rebind every so often
            rows = std::make_unique<ColumnWiseBinding<TEST_CHAR_TYPE>>(numRowsPerFetch);
            bindColumns(numRowsPerFetch, rows.get());
        }
    }

    EXPECT_EQ(NUMROWS, index);
    EXPECT_EQ(SQL_NO_DATA_FOUND, SQLFetch(stmt));
}


// uncomment the following 2 lines to disable the tests
// #define DISABLED(TESTNAME) DISABLED_ ## TESTNAME
// swap to enable tests

#define DISABLED(TESTNAME) TESTNAME

TEST_F(ODBCTestRequiresChorus, DISABLED(DB885))
{
    rowWiseBinding(1);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(MultiRowBinding))
{
    rowWiseBinding(7);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB873))
{
    columnWiseBinding(7);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(OneColumnBinding))
{
    columnWiseBinding(1);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DisconnectConnectionFirst))
{
    columnWiseBinding(1);
    // disconnect connection first, to test a customer memory issue
    EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(hdbc1));
    hdbc1 = NULL;
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB903))
{
    RETCODE retcode;

    setupTable();

    // directly execute the statement
    char param[sizeof(BINARYSTRING)+1];
    sprintf(param, "%s1", BINARYSTRING);

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                               1,
                               SQL_PARAM_INPUT,
                               SQL_C_BINARY,
                               SQL_C_BINARY,
                               strlen(param),
                               0,
                               param,
                               strlen(param),
                                            0));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"SELECT mxBinary from mxLattice where mxBinary = ?", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    char    mxBinary[VALSIZE];
    SQLLEN  mxBinaryInd;

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, SQL_C_BINARY, mxBinary, VALSIZE, &mxBinaryInd));

    int count = 0;
    while ((retcode = SQLFetch(stmt)) != SQL_NO_DATA) {

        ASSERT_EQ(strlen(param), (size_t)mxBinaryInd);

        for (int el = 0; el < mxBinaryInd; el++) {
            ASSERT_EQ(param[el], mxBinary[el]);
        }

        count++;
    }

    ASSERT_GT(count, 0);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB915))
{
    RETCODE retcode;

    setupTable();

    SQLWCHAR    mxClob[VALSIZE];
    SQLLEN      mxClobInd;
    char        mxBlob[VALSIZE];
    SQLLEN      mxBlobInd;
    SQLINTEGER  mxInt;
    SQLLEN      mxIntInd;

    const int   CHUNK_SIZE = 129;
    const int   CLOB_SIZE = 4099;
    std::string clobString;
    clobString.resize(CLOB_SIZE);

    for (int i = 0; i < CLOB_SIZE; i++) {
        clobString[i] = i % 26 + 'A';
    }

    clobString[CLOB_SIZE] = 0;

    mxClobInd = SQL_LEN_DATA_AT_EXEC(CLOB_SIZE);
    mxBlobInd = SQL_LEN_DATA_AT_EXEC(CLOB_SIZE);
    mxInt = 1;
    mxIntInd = 0;

    SQLHSTMT updateStmt;
    EXPECT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &updateStmt));

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(updateStmt,
                                            1,
                                            SQL_PARAM_INPUT,
                                            SQL_C_CHAR,
                                            TEST_SQL_C_CHAR,
                                            VALSIZE,
                                            0,
                                            &mxClob,
                                            VALSIZE*sizeof(TEST_CHAR_TYPE),
                                            &mxClobInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(updateStmt,
                                            2,
                                            SQL_PARAM_INPUT,
                                            SQL_C_BINARY,
                                            SQL_LONGVARBINARY,
                                            VALSIZE,
                                            0,
                                            &mxBlob,
                                            VALSIZE,
                                            &mxBlobInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(updateStmt,
                                            3,
                                            SQL_PARAM_INPUT,
                                            SQL_C_SLONG,
                                            SQL_INTEGER,
                                            0,
                                            0,
                                            &mxInt,
                                            0,
                                            &mxIntInd));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(updateStmt, (SQLCHAR*)"update mxLattice set mxClob = ?, mxBinary = ? where mxInt = ?", SQL_NTS));
    EXPECT_EQ(SQL_NEED_DATA, SQLExecute(updateStmt));

    PTR paramId;
    SQLINTEGER  clobOffset = 0;
    SQLINTEGER  blobOffset = 0;
    SQLINTEGER  len;

    while (SQLParamData(updateStmt, &paramId) == SQL_NEED_DATA) {
        if (paramId == &mxClob) {
            len = MIN(CHUNK_SIZE, CLOB_SIZE-clobOffset);
            EXPECT_EQ(SQL_SUCCESS, SQLPutData(updateStmt, &clobString[clobOffset], len));
            clobOffset += len;
        } else if (paramId == &mxBlob) {
            len = MIN(CHUNK_SIZE, CLOB_SIZE-blobOffset);
            EXPECT_EQ(SQL_SUCCESS, SQLPutData(updateStmt, &clobString[blobOffset], len));
            blobOffset += len;
        } else {
            FAIL() << "Unexpected param id: " << paramId;
        }
    }

    SQLLEN rowCount;
    EXPECT_EQ(SQL_SUCCESS, SQLRowCount(updateStmt, &rowCount));
    ASSERT_EQ(1, rowCount);

    EXPECT_EQ(SQL_SUCCESS, SQLFreeHandle(SQL_HANDLE_STMT, updateStmt));

    // now select the stuff back out
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT mxInt, mxClob, mxBinary from mxLattice where mxInt = 1", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    SQLINTEGER  mxIntSelect;
    SQLLEN      mxIntSelectInd;

    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_ULONG, &mxIntSelect, 0, &mxIntSelectInd));

    ASSERT_EQ(mxInt, mxIntSelect);
    ASSERT_EQ(4, mxIntSelectInd);

    std::vector<char> mxClobSelect(CLOB_SIZE*2);
    SQLLEN  mxClobSelectInd = 0;
    int     offset = 0;

    // 1st call to see total size of the CLOB, size 1 for NULL
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO, SQLGetData(stmt, 2, SQL_C_CHAR, &mxClobSelect[offset], 1, &mxClobSelectInd));
    ASSERT_EQ(CLOB_SIZE, (size_t)mxClobSelectInd);

    while (SQLGetData(stmt, 2, SQL_C_CHAR, &mxClobSelect[offset], CHUNK_SIZE, &mxClobSelectInd) != SQL_NO_DATA) {
        ASSERT_EQ(clobString.size()-offset, (size_t)mxClobSelectInd);
        offset += CHUNK_SIZE-1; // -1 to deal with NULL terminator
    }

    ASSERT_EQ(0, mxClobSelectInd);

    ASSERT_STREQ(clobString.c_str(), mxClobSelect.data());

    std::vector<char> mxBinarySelect(CLOB_SIZE*2);
    SQLLEN  mxBinarySelectInd;
    offset = 0;

    // 1st call to see total size of the BLOB
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO, SQLGetData(stmt, 3, SQL_C_BINARY, &mxBinarySelect[offset], 0, &mxBinarySelectInd));
    ASSERT_EQ(CLOB_SIZE, mxBinarySelectInd);

    while (SQLGetData(stmt, 3, SQL_C_BINARY, &mxBinarySelect[offset], CHUNK_SIZE, &mxBinarySelectInd) != SQL_NO_DATA) {
        ASSERT_EQ(CLOB_SIZE-offset, mxBinarySelectInd);
        offset += CHUNK_SIZE;
    }

    for (int i = 0; i < CLOB_SIZE; i++) {
        ASSERT_EQ(clobString[i], mxBinarySelect[i]);
    }

    ASSERT_EQ(0, mxBinarySelectInd);

    retcode = SQLFetch(stmt);
    ASSERT_EQ(SQL_NO_DATA, retcode);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(RowWiseSlightlyDifferent))
{
    const int STRLEN = 128;
    const char* createStmt =
        "create table mxServer (mxName nvarchar(128), mxOID integer, mxFlags integer, mxUser nvarchar(128), mxPass nvarchar(128), mxConnect nvarchar(128), mxTZ nvarchar(128))";

    int ROWS = 1;
    int rowSize =
        sizeof(SQLLEN) + STRLEN*sizeof(TEST_CHAR_TYPE)  + // mxName
        sizeof(SQLLEN) + sizeof(SQLINTEGER) + // mxOID
        sizeof(SQLLEN) + sizeof(SQLINTEGER) + // mxFlags
        sizeof(SQLLEN) + STRLEN*sizeof(TEST_CHAR_TYPE) + // mxUser
        sizeof(SQLLEN) + STRLEN*sizeof(TEST_CHAR_TYPE) + // mxPass
        sizeof(SQLLEN) + STRLEN*sizeof(TEST_CHAR_TYPE) + // mxConnect
        sizeof(SQLLEN) + STRLEN*sizeof(TEST_CHAR_TYPE); // mxTZ

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)createStmt, SQL_NTS));

    const char* insertStmt =
        "insert into mxServer (mxName, mxOID, mxFlags, mxUser, mxPass, mxConnect, mxTZ) values('ADMINISTRATOR', 42, 45, '','',null,'')";

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)insertStmt, SQL_NTS));
    int     BUF_OFFSET = 1000;
    int     rowBufferSize = ROWS * rowSize;

    std::vector<char> rowBufferVector(rowBufferSize+BUF_OFFSET+BUF_OFFSET);
    char* rowBuffer = rowBufferVector.data();

    memset(rowBuffer, 42, rowBufferSize+BUF_OFFSET+BUF_OFFSET); // set everything to a value

    // we layout the memory with indicator column followed by the data value
    char* mxNameIndOffset = rowBuffer + BUF_OFFSET;
    char* mxNameOffset = mxNameIndOffset + sizeof(SQLLEN);

    char* mxOIDIndOffset = mxNameOffset + STRLEN*sizeof(TEST_CHAR_TYPE);
    char* mxOIDOffset = mxOIDIndOffset + sizeof(SQLLEN);

    char* mxFlagsIndOffset = mxOIDOffset + sizeof(SQLINTEGER);
    char* mxFlagsOffset = mxFlagsIndOffset + sizeof(SQLLEN);

    char* mxUserIndOffset = mxFlagsOffset + sizeof(SQLINTEGER);
    char* mxUserOffset = mxUserIndOffset + sizeof(SQLLEN);

    char* mxPassIndOffset = mxUserOffset + STRLEN*sizeof(TEST_CHAR_TYPE);
    char* mxPassOffset = mxPassIndOffset + sizeof(SQLLEN);

    char* mxConnectIndOffset = mxPassOffset + STRLEN*sizeof(TEST_CHAR_TYPE);
    char* mxConnectOffset = mxConnectIndOffset + sizeof(SQLLEN);

    char* mxTZIndOffset = mxConnectOffset + STRLEN*sizeof(TEST_CHAR_TYPE);
    char* mxTZOffset = mxTZIndOffset + sizeof(SQLLEN);

    char* bufferEnd = mxTZOffset + STRLEN*sizeof(TEST_CHAR_TYPE);
    ASSERT_EQ(bufferEnd - rowBuffer - BUF_OFFSET, rowSize);

    SQLULEN ul = rowSize;
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)ul, 0));

    ul = ROWS;
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ul, 0));

    SQLULEN rowsFetched;
    EXPECT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER)&rowsFetched, 0));

    // directly execute the statement
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT * from mxServer", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, TEST_SQL_C_CHAR, mxNameOffset, STRLEN*sizeof(TEST_CHAR_TYPE), (SQLLEN*)mxNameIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 2, SQL_C_LONG, mxOIDOffset, sizeof(SQLINTEGER), (SQLLEN*)mxOIDIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 3, SQL_C_LONG, mxFlagsOffset, sizeof(SQLINTEGER), (SQLLEN*)mxFlagsIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 4, TEST_SQL_C_CHAR, mxUserOffset, STRLEN*sizeof(TEST_CHAR_TYPE), (SQLLEN*)mxUserIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 5, TEST_SQL_C_CHAR, mxPassOffset, STRLEN*sizeof(TEST_CHAR_TYPE), (SQLLEN*)mxPassIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 6, TEST_SQL_C_CHAR, mxConnectOffset, STRLEN*sizeof(TEST_CHAR_TYPE), (SQLLEN*)mxConnectIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 7, TEST_SQL_C_CHAR, mxTZOffset, STRLEN*sizeof(TEST_CHAR_TYPE), (SQLLEN*)mxTZIndOffset));

    EXPECT_EQ(SQL_SUCCESS, SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0));

    ASSERT_EQ((SQLULEN)1, rowsFetched);

    ASSERT_EQ(13*sizeof(TEST_CHAR_TYPE), (size_t)*((SQLLEN*)mxNameIndOffset));
    EXPSTREQ("ADMINISTRATOR", (TEST_CHAR_TYPE*)mxNameOffset);

    ASSERT_EQ(sizeof(SQLINTEGER), (size_t)*((SQLLEN*)mxOIDIndOffset));
    ASSERT_EQ(42, *((SQLINTEGER*)mxOIDOffset));

    ASSERT_EQ(sizeof(SQLINTEGER), (size_t)*((SQLLEN*)mxFlagsIndOffset));
    ASSERT_EQ(45, *((SQLINTEGER*)mxFlagsOffset));

    ASSERT_EQ(0*sizeof(TEST_CHAR_TYPE), (size_t)*((SQLLEN*)mxUserIndOffset));
    EXPSTREQ("", (TEST_CHAR_TYPE*)mxUserOffset);

    ASSERT_EQ(0*sizeof(TEST_CHAR_TYPE), (size_t)*((SQLLEN*)mxPassIndOffset));
    EXPSTREQ("", (TEST_CHAR_TYPE*)mxPassOffset);

    ASSERT_EQ(SQL_NULL_DATA, *((SQLLEN*)mxConnectIndOffset));

    EXPSTREQ("", (TEST_CHAR_TYPE*)mxTZOffset);

    // make sure we haven't trashed memory around the buffers
    for (int i = 0; i < BUF_OFFSET; i++) {
        ASSERT_EQ(42, rowBuffer[i]);
        ASSERT_EQ(42, bufferEnd[i]);
    }
    EXPECT_EQ(SQL_NO_DATA, SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(SQLDATAATEXEC))
{
    RETCODE retcode;

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create table lxdesc (lxoid integer, lxkind integer, lxdesc nclob)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"insert into lxdesc values (1,1,'a')", SQL_NTS));

    SQLHSTMT hstmt = SQL_NULL_HSTMT;

    // Allocate statement handle.
    EXPECT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(hstmt, (SQLCHAR*)"update lxDesc set lxDesc=? where lxOid=? and lxKind=?", SQL_NTS));

    SQLLEN  i1 = 0;
    void*   datap = (void*)"";

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_WVARCHAR, 0, 0, datap, 0, &i1));

    std::vector<SQLLEN> cbValue(1);
    cbValue[0] = 0;
    SQLLEN in = 1;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 10, 0, &in, 0, &cbValue[0]));

    std::vector<SQLLEN> cbValue2(1);
    cbValue2[0] = 0;
    SQLLEN in2 = 1;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 10, 0, &in2, 0, &cbValue2[0]));

    i1 = SQL_DATA_AT_EXEC; // set right before we exec

    retcode = SQLExecute(hstmt);
    EXPECT_TRUE(SQL_SUCCESS == retcode || SQL_NEED_DATA == retcode);

    const int   CLOBSIZE = 2048;
    const int   CHUNK_SIZE = 200;

    char inputString[CLOBSIZE+1];
    for (int i = 0; i < CLOBSIZE; i++) {
        inputString[i] = 'A' + (i % 26);
    }
    inputString[CLOBSIZE] = 0;

    while (retcode == SQL_NEED_DATA) {
        SQLPOINTER rgbValue = NULL;
        retcode = SQLParamData(hstmt, &rgbValue);
        if (retcode == SQL_NEED_DATA) {
            EXPECT_EQ(SQL_SUCCESS, SQLPutData(hstmt, (void*)inputString, CLOBSIZE+1));
        }

        retcode = SQLParamData(hstmt, &rgbValue);
    }

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"commit", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"select lxdesc from lxdesc", SQL_NTS));

    char    lxDesc[CLOBSIZE+1];
    SQLLEN  lxDescLen = CHUNK_SIZE;
    int     offset = 0;

    retcode = SQLFetch(stmt);
    ASSERT_TRUE(SQL_SUCCESS == retcode || SQL_SUCCESS_WITH_INFO == retcode);

    while (SQLGetData(stmt, 1, SQL_C_CHAR, lxDesc + offset, CHUNK_SIZE, &lxDescLen) != SQL_NO_DATA) {
        offset += lxDescLen > 0 ? CHUNK_SIZE-1 : 0; // -1 to deal with null terminator
    }

    ASSERT_EQ(0, lxDescLen);

    ASSERT_EQ(strlen(inputString), strlen(lxDesc));
    ASSERT_STREQ(inputString, lxDesc);

    retcode = SQLFetch(stmt);
    ASSERT_EQ(SQL_NO_DATA, retcode);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(returnGeneratedKeys))
{
    RETCODE retcode;

    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create table odbcgenkeys (id number generated by DEFAULT as identity, name string)", SQL_NTS));

    ASSERT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"insert into odbcgenkeys (name) values ('one'), ('two'), ('three')", SQL_NTS));

    SQLSMALLINT columns;
    int         row = 0;
    SQLINTEGER  rowIds[3];

    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &columns));
    EXPECT_EQ(0, columns);

    SQLLEN rowCount;
    EXPECT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &rowCount));
    EXPECT_EQ(3, rowCount);

    EXPECT_EQ(SQL_SUCCESS, SQLMoreResults(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &columns));
    EXPECT_EQ(1, columns);

    while (SQL_SUCCEEDED(retcode = SQLFetch(stmt))) {
        SQLUSMALLINT i;

        for (i = 1; i <= columns; i++) {
            SQLLEN indicator;

            EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, i, SQL_C_LONG, &rowIds[row], sizeof(SQLINTEGER), &indicator));
        }
        row++;
    }

    EXPECT_EQ(3, row);

    EXPECT_EQ(SQL_NO_DATA, SQLFetch(stmt));
    EXPECT_EQ(SQL_NO_DATA, SQLMoreResults(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"select id from odbcgenkeys", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &columns));
    EXPECT_EQ(1, columns);

    row = 0;
    while (SQL_SUCCEEDED(retcode = SQLFetch(stmt))) {
        SQLUSMALLINT i;

        for (i = 1; i <= columns; i++) {
            SQLLEN      indicator;
            SQLINTEGER  rowId;

            EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, i, SQL_C_LONG, &rowId, sizeof(SQLINTEGER), &indicator));
            EXPECT_EQ(rowIds[row], rowId);
        }
        row++;
    }

    EXPECT_EQ(3, row);

    EXPECT_EQ(SQL_NO_DATA, SQLFetch(stmt));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(StoredProcedure))
{
    RETCODE retcode;

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create procedure test(IN p1 integer, OUT p2 integer, INOUT p3 integer) RETURNS output(field integer) AS p2 = p1; p3 = p3 +1; insert into output values(p3); end_procedure;", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"call test(?, ?, ?)", SQL_NTS));

    int datap1 = 100;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &datap1, 0, NULL));

    int datap2;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt, 2, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &datap2, 0, NULL));

    int datap3 = 1000;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt, 3, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &datap3, 0, NULL));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    ASSERT_EQ(datap1, datap2);
    ASSERT_EQ(1001, datap3);

    SQLSMALLINT columns;
    SQLNumResultCols(stmt, &columns);
    ASSERT_EQ(1, columns);

    int row = 0;
    while (SQL_SUCCEEDED(retcode = SQLFetch(stmt))) {
        SQLUSMALLINT i;

        for (i = 1; i <= columns; i++) {
            SQLLEN      indicator;
            SQLINTEGER  value;

            EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, i, SQL_C_LONG, &value, sizeof(SQLINTEGER), &indicator));
            ASSERT_EQ(datap3, value);
        }
        row++;
    }

    ASSERT_EQ(1, row);

    retcode = SQLFetch(stmt);
    ASSERT_EQ(SQL_NO_DATA, retcode);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(StoredProcedureMultipleReturns))
{
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt,
                            (SQLCHAR*)"create procedure testm() RETURNS output(id int), output2(id int)"
                            "AS INSERT INTO output VALUES (-9);"
                            "INSERT INTO output2 VALUES (-10), (-11);"
                            "end_procedure;", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"call testm()", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    SQLSMALLINT columns;
    SQLNumResultCols(stmt, &columns);
    ASSERT_EQ(1, columns);

    SQLINTEGER value;
    SQLLEN indicator;

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_SLONG, &value, sizeof(SQLINTEGER), &indicator));
    ASSERT_EQ(-9, value);

    EXPECT_EQ(SQL_NO_DATA, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLMoreResults(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_SLONG, &value, sizeof(SQLINTEGER), &indicator));
    ASSERT_EQ(-10, value);

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_SLONG, &value, sizeof(SQLINTEGER), &indicator));
    ASSERT_EQ(-11, value);

    EXPECT_EQ(SQL_NO_DATA, SQLFetch(stmt));
    EXPECT_EQ(SQL_NO_DATA, SQLMoreResults(stmt));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB3898))
{
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "create table bindprob(a string, b string)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "insert into bindprob values('one',1), ('two',2), ('three',3)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"SELECT * from bindprob where a = ?", SQL_NTS));

    const char* paramOne = "one";

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(paramOne), 0, (void *)paramOne, 0, NULL));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB1384))
{
    const int BUFSIZE = 64200;

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "drop table db1384 if exists", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "create table db1384  (f1 integer, f2 string)", SQL_NTS));

    std::string largeBuf;
    largeBuf.resize(BUFSIZE-100);

    for (size_t i = 0; i < largeBuf.size(); ++i) {
        largeBuf[i] = 'A';
    }

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"insert into db1384 values(?,?)", SQL_NTS));

    int f1 = 1;

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                                            1,
                                            SQL_PARAM_INPUT,
                                            SQL_C_SLONG,
                                            SQL_INTEGER,
                                            0,
                                            0,
                                            &f1,
                                            0,
                                            0));

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                                            2,
                                            SQL_PARAM_INPUT,
                                            SQL_C_CHAR,
                                            SQL_CHAR,
                                            largeBuf.size(),
                                            0,
                                            (void *)largeBuf.data(),
                                            largeBuf.size(),
                                            NULL));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "select f2 from db1384 where f1 = 1", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    char    outbuf[BUFSIZE];
    memset(outbuf, 0, BUFSIZE);
    const int   CHUNK_SIZE = 129;
    SQLLEN  outbufLen = CHUNK_SIZE;
    int     offset = 0;

    while (SQLGetData(stmt, 1, SQL_C_CHAR, outbuf + offset, CHUNK_SIZE, &outbufLen) != SQL_NO_DATA) {
        offset += outbufLen > 0 ? CHUNK_SIZE-1 : 0; // -1 to deal with null terminator
    }

    EXPECT_EQ(largeBuf.size(), strlen(outbuf));
    EXPECT_STREQ(largeBuf.data(), outbuf);
}

static const int BUFSIZE=80;

void testRow(SQLHSTMT        otherHandle,
             SQLHSTMT        insertHandle,
             SQLULEN         columnSize,
             SQLPOINTER      parameterValuePtr,
             SQLLEN          bufferLength,
             RETCODE         bindRetCode,
             RETCODE         executeRetCode,
             SQLLEN*         resultSize)
{
    RETCODE retcode;

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(otherHandle, (SQLCHAR*) "truncate table nonts", SQL_NTS));

    retcode = SQLBindParameter(insertHandle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, columnSize, 0, parameterValuePtr, bufferLength, NULL);
    ASSERT_EQ(bindRetCode, retcode);

    retcode = SQLExecute(insertHandle);
    ASSERT_EQ(executeRetCode, retcode);

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(otherHandle, (SQLCHAR*) "select a from nonts", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(otherHandle));

    EXPECT_EQ(SQL_SUCCESS, SQLGetData(otherHandle, 1, SQL_C_CHAR, parameterValuePtr, BUFSIZE+1, resultSize));

    // for debugging:
    // printf("(%s)\n", (const char*) parameterValuePtr);

    retcode = SQLFetch(otherHandle);
    ASSERT_EQ(SQL_NO_DATA_FOUND, retcode);

    retcode = SQLMoreResults(otherHandle);
    ASSERT_EQ(SQL_NO_DATA, retcode);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(NONTS))
{
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "create table nonts(a string)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*) "insert into nonts values(?)", SQL_NTS));

    char buf[BUFSIZE+1]; // +1 for null

    SQLHSTMT otherHandle;
    SQLLEN resultSize;

    EXPECT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &otherHandle));

    const char* phoneNum = "+16175551212";
    size_t phoneNumLen = strlen(phoneNum);

    // test 1, buffer filled with a + null terminated phone number
    // since null terminator is god we ignore BUFSIZE and just report
    // the length of the phonen number
    // behaves like mysql & postgres
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strcpy(buf, phoneNum);
    testRow(otherHandle, stmt, BUFSIZE, buf, 0, SQL_SUCCESS, SQL_SUCCESS, &resultSize);
    EXPECT_EQ(phoneNumLen, (size_t)resultSize);
    EXPECT_STREQ(phoneNum, buf);

    // test 2, buffer filled with a + non-null terminated phone number
    // we don't have null termination until the end of the buffer so
    // we include everything in the buffer...so beginning will have
    // phone number but the rest will be A's
    // behaves like mysql & postgres
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strncpy(buf, phoneNum, phoneNumLen);
    testRow(otherHandle, stmt, BUFSIZE, buf, 0, SQL_SUCCESS, SQL_SUCCESS, &resultSize);
    EXPECT_TRUE(strncmp(phoneNum, buf, phoneNumLen) == 0);
    EXPECT_EQ(BUFSIZE, resultSize);

    // test 3, buffer has phone number + null at pos 5.  Null
    // terminiator is god so the phone number gets truncated.
    // behaves like mysql & postgres
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strcpy(buf, phoneNum);
    buf[5] = 0;
    testRow(otherHandle, stmt, BUFSIZE, buf, 0, SQL_SUCCESS, SQL_SUCCESS, &resultSize);

    EXPECT_TRUE(strcmp(buf, "+1617") == 0);
    EXPECT_EQ(5, resultSize);

    // test 4, buffer has phone number, we tell that the bind column
    // is 5.  We return 5 chars because that is what we told ODBC the
    // max size is.
    // This behaves differently from mysql & postgres as we limit to
    // what we say.  They still believe that null terminator is god
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strcpy(buf, phoneNum);
    testRow(otherHandle, stmt, 5, buf, 0, SQL_SUCCESS, SQL_SUCCESS, &resultSize);
    EXPECT_TRUE(strcmp(buf, "+1617") == 0);
    EXPECT_EQ(5, resultSize);

    // test 5, use SQL_NTS as buffer length... Nobody likes that
    //
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strcpy(buf, phoneNum);
    testRow(otherHandle, stmt, BUFSIZE, buf, SQL_NTS, SQL_ERROR, SQL_SUCCESS, &resultSize);

    // test 6, buffer length is ignored
    // Null terminator rules
    // behaves like mysql & postgres
    memset(buf, 'A', BUFSIZE);
    buf[BUFSIZE] = 0;
    strcpy(buf, phoneNum);
    testRow(otherHandle, stmt, BUFSIZE, buf, 5, SQL_SUCCESS, SQL_SUCCESS, &resultSize);
    EXPECT_EQ(phoneNumLen, (size_t)resultSize);
    EXPECT_STREQ(phoneNum, buf);
}

void thisismyfunction()
{
}


TEST_F(ODBCTestRequiresChorus, DISABLED_INVALIDUTF8)
{
    thisismyfunction();
    RETCODE retcode;
    char invalidutf8[] = "this is \xfe\xfe\xff\xff";
    char retbuf[100];

    // test as a string, will fail on insert
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "create table invalidutf8a(a string)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*) "insert into invalidutf8a values(?)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                                            1,
                                            SQL_PARAM_INPUT,
                                            SQL_C_CHAR,
                                            TEST_SQL_C_CHAR,
                                            strlen(invalidutf8)+1,
                                            0,
                                            &invalidutf8,
                                            strlen(invalidutf8)+1,
                                            NULL));

    retcode = SQLExecute(stmt);
    // may not have the correct stuff in the insert stmt for an
    // invalid utf8 string on windows
#ifndef ODBC_WINDOWS
    std::string sqlState, message;
    ASSERT_EQ(SQL_ERROR, retcode);
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    EXPECT_EQ("HY000", sqlState);
    EXPECT_EQ("invalid UTF-8 code sequence", message);
#else
    ASSERT_EQ(SQL_SUCCESS, retcode);
#endif

    // test as a binary, should work
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "create table invalidutf8b(a string)", SQL_NTS));

    // someday we may disallow inserting invalid utf8 via direct statement
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*) "insert into invalidutf8b values('this is \xfe\xfe\xff\xff')", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT a from invalidutf8b", SQL_NTS));

    retcode = SQLFetch(stmt);

    // may not have the correct stuff in the insert stmt for an
    // invalid utf8 string on windows
#ifndef ODBC_WINDOWS
    ASSERT_EQ(SQL_ERROR, retcode);
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    EXPECT_EQ("HY000", sqlState);
    EXPECT_EQ("invalid UTF-8 code sequence", message);
#else
    ASSERT_EQ(SQL_SUCCESS, retcode);
#endif

    SQLLEN ind = 0;
    // first shot will fail because of a < 0 len buffer
    EXPECT_EQ(SQL_ERROR, SQLGetData(stmt, 1, SQL_C_CHAR, retbuf, -1, &ind));
    EXPECT_EQ(SQL_ERROR, SQLGetData(stmt, 1, SQL_C_BINARY, retbuf, -1, &ind));

    // may not have the correct stuff in the insert stmt for an
    // invalid utf8 string on windows
    retcode = SQLGetData(stmt, 1, SQL_C_CHAR, retbuf, sizeof(retbuf), &ind);
#ifndef ODBC_WINDOWS
    ASSERT_EQ(SQL_ERROR, retcode);
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    EXPECT_EQ("HY000", sqlState);
    EXPECT_EQ("invalid UTF-8 code sequence", message);
#else
    ASSERT_EQ(SQL_SUCCESS, retcode);
#endif
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB3963))
{
    UCHAR FAR szDesc[33];
    SWORD  cbDesc;
    SQLLEN fDesc;

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"select * from system.tables where tablename=?", SQL_NTS));

    // call before we have executed the statement, should get an error
    // eventually will want to support this
    EXPECT_EQ(SQL_SUCCESS, SQLColAttributes(stmt, 1, SQL_COLUMN_COUNT, szDesc, sizeof(szDesc), &cbDesc, &fDesc));
}


TEST_F(ODBCTestRequiresChorus, DISABLED(DB3967))
{
    SQLSMALLINT numCols;

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"select tablename,schema,type from system.tables where tablename='fields'", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numCols));
    ASSERT_EQ(3, numCols);

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numCols));
    ASSERT_EQ(3, numCols);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB3966))
{
    RETCODE retcode;
    char driverver[100];

    EXPECT_EQ(SQL_SUCCESS, SQLGetInfo(hdbc1, SQL_DRIVER_VER, (SQLPOINTER) driverver, sizeof(driverver), NULL));

    char tooshort[4];

    retcode = SQLGetInfo(hdbc1, SQL_DRIVER_VER, (SQLPOINTER) tooshort, sizeof(tooshort), NULL);
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO, retcode);
    ASSERT_EQ(0, strncmp(driverver, tooshort, sizeof(tooshort)-1));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB4028))
{
    EXPECT_EQ(SQL_SUCCESS, SQLGetTypeInfo(stmt, SQL_ALL_TYPES));
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB5042))
{
    RETCODE retcode;
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create table foo(a string)", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*) "insert into foo values(?)", SQL_NTS));

    SQLLEN cbNumeric = 0;
    EXPECT_EQ(SQL_SUCCESS, SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 6,NULL,0,&cbNumeric));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT a from foo", SQL_NTS));

    char    aVal[VALSIZE];
    SQLLEN  aInd;

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, SQL_C_CHAR, aVal, VALSIZE, &aInd));

    int count = 0;
    while ((retcode = SQLFetch(stmt)) != SQL_NO_DATA) {
        ASSERT_EQ(SQL_NULL_DATA, aInd);
        count++;
    }

    ASSERT_EQ(1, count);
}

TEST_F(ODBCTestRequiresChorus, DISABLED(StoredProcedureMultipleResults))
{
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"CREATE PROCEDURE dummy() RETURNS TABLE t1(line string), TABLE t2(value int) AS \n"
                            "INSERT INTO t1 VALUES ('line1'),('line2');\n"
                            "INSERT INTO t2 VALUES (-10),(-11);\n"
                            "END_PROCEDURE;\n", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"call dummy()", SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    SQLSMALLINT columns;
    SQLNumResultCols(stmt, &columns);
    ASSERT_EQ(1, columns);

    char    outbuf[1024];
    memset(outbuf, 0, 1024);
    const int   CHUNK_SIZE = 129;
    SQLLEN  outbufLen;
    int     offset;

    ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    offset = 0;
    outbufLen = CHUNK_SIZE;
    while (SQLGetData(stmt, 1, SQL_C_CHAR, outbuf + offset, CHUNK_SIZE, &outbufLen) != SQL_NO_DATA) {
        offset += outbufLen > 0 ? CHUNK_SIZE-1 : 0; // -1 to deal with null terminator
    }
    EXPECT_EQ((size_t)5, strlen(outbuf));
    EXPECT_STREQ("line1", outbuf);

    ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    offset = 0;
    outbufLen = CHUNK_SIZE;
    while (SQLGetData(stmt, 1, SQL_C_CHAR, outbuf + offset, CHUNK_SIZE, &outbufLen) != SQL_NO_DATA) {
        offset += outbufLen > 0 ? CHUNK_SIZE-1 : 0; // -1 to deal with null terminator
    }
    EXPECT_EQ((size_t)5, strlen(outbuf));
    EXPECT_STREQ("line2", outbuf);

    ASSERT_EQ(SQL_NO_DATA, SQLFetch(stmt));

    ASSERT_EQ(SQL_SUCCESS, SQLMoreResults(stmt));

    SQLINTEGER  mxIntSelect;
    SQLLEN      mxIntSelectInd;

    ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_ULONG, &mxIntSelect, 0, &mxIntSelectInd));
    EXPECT_EQ(-10, mxIntSelect);

    ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLGetData(stmt, 1, SQL_C_ULONG, &mxIntSelect, 0, &mxIntSelectInd));
    EXPECT_EQ(-11, mxIntSelect);

    ASSERT_EQ(SQL_NO_DATA, SQLFetch(stmt));
    ASSERT_EQ(SQL_NO_DATA, SQLMoreResults(stmt));
}

TEST_F(ODBCTestRequiresChorus, DISABLED(DB11296))
{
    RETCODE retcode = SQLPrepare(stmt, (SQLCHAR*) "*** bad SQL ***", SQL_NTS);
    ASSERT_EQ(SQL_ERROR, retcode);
    std::string sqlState, message;
    ASSERT_EQ(SQL_ERROR, retcode);
    SQLINTEGER  native;
    SQLCHAR     state[7];
    SQLSMALLINT len = 0;
    retcode = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &native, NULL, 0, &len);
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO, retcode);
    ASSERT_NE(0, len);
    std::string errorMsg;
    errorMsg.resize(len);
    char* buffer = errorMsg.data();
    EXPECT_EQ(SQL_SUCCESS, SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &native, (SQLCHAR*)buffer, (SQLSMALLINT)errorMsg.length()+1, &len));
}

TEST_F(ODBCTestRequiresChorus, DisconnectWithTransOpen)
{
    EXPECT_EQ(SQL_SUCCESS, SQLSetConnectAttr(hdbc1,
                                        SQL_ATTR_AUTOCOMMIT,
                                        (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                        SQL_NTS));

    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"SELECT count(*) from system.tables", SQL_NTS));

    // can't disconnect with open transaction
    EXPECT_EQ(SQL_ERROR, SQLDisconnect(hdbc1));

    EXPECT_EQ(SQL_SUCCESS, SQLEndTran(SQL_HANDLE_DBC, hdbc1, SQL_COMMIT));
    EXPECT_EQ(SQL_SUCCESS, SQLDisconnect(hdbc1));

    hdbc1 = nullptr;
    stmt = nullptr;
}

TEST_F(ODBCTestRequiresChorus, DB14395)
{
    const char* selectSQL = "select char(128) from dual";
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)selectSQL, SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLExecute(stmt));

    char result[16];
    SQLLEN rlen;
    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, SQL_C_CHAR, result, sizeof(result), &rlen));

    // the bug was throwing an SQLError here.
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    EXPECT_EQ(SQL_SUCCESS, SQLFreeStmt(stmt, SQL_CLOSE));
}

TEST_F(ODBCTestRequiresChorus, DescribeCol)
{
    setupTable();
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)"select mxName, mxInt, mxShort, mxBigInt, mxDouble, mxChar, mxFloat, mxDecimal, mxTableSpc, mxDate, mxTime, mxTimeStamp, mxBinary, mxClob from mxLattice", SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);

    SQLCHAR colNameBuffer[1024];
    SQLSMALLINT colNameLength;
    SQLSMALLINT dataType;
    SQLULEN columnSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;

    ret = SQLDescribeCol(stmt, 1, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXNAME", (char *)colNameBuffer);
    EXPECT_EQ(SQL_VARCHAR, dataType);
    EXPECT_EQ((SQLULEN)128, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 2, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXINT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_INTEGER, dataType);
    //DB-14428
    //EXPECT_EQ(10, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 3, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXSHORT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_SMALLINT, dataType);
    //DB-14428
    //EXPECT_EQ(5, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 4, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXBIGINT", (char *)colNameBuffer);
    //DB-14428 -- we get -5, which might be on purpose but not very valid.
    //EXPECT_EQ(0, dataType);
    // we get 19.
    //EXPECT_EQ(0, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 5, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXDOUBLE", (char *)colNameBuffer);
    EXPECT_EQ(SQL_DOUBLE, dataType);
    EXPECT_EQ((size_t)15, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 6, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXCHAR", (char *)colNameBuffer);
    EXPECT_EQ(SQL_SMALLINT, dataType);
    //DB-14428
    //EXPECT_EQ(5, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 7, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXFLOAT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_DOUBLE, dataType);
    EXPECT_EQ((size_t)15, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 8, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXDECIMAL", (char *)colNameBuffer);
    EXPECT_EQ(SQL_NUMERIC, dataType);
    EXPECT_EQ((size_t)10, columnSize);
    EXPECT_EQ(2, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 9, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXTABLESPC", (char *)colNameBuffer);
    EXPECT_EQ(SQL_VARCHAR, dataType);
    EXPECT_EQ((size_t)128, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 10, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXDATE", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_DATE, dataType);
    EXPECT_EQ((size_t)10, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 11, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXTIME", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_TIME, dataType);
    //should be 8, or have decimal digits. we get 16. DB-14428
    //EXPECT_EQ(8, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 12, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXTIMESTAMP", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_TIMESTAMP, dataType);
    EXPECT_EQ((size_t)16, columnSize);
    EXPECT_EQ(6, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 13, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXBINARY", (char *)colNameBuffer);
    // DB-14428, shows up as -4, maybe correct, maybe not too valid
    //EXPECT_EQ(0, dataType);
    EXPECT_EQ((size_t)4099, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    ret = SQLDescribeCol(stmt, 14, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXCLOB", (char *)colNameBuffer);
    //DB-14428, shows up as -1, maybe correct, not too valid.
    //EXPECT_EQ(0, dataType);
    EXPECT_EQ((size_t)100, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    SQLFreeStmt(stmt, SQL_CLOSE);

    ret = SQLExecDirect(stmt, (SQLCHAR*)"create table nulless(henry int not null)", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLExecDirect(stmt, (SQLCHAR*)"select * from nulless", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLDescribeCol(stmt, 1, colNameBuffer, 1023, &colNameLength,
        &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("HENRY", (char *)colNameBuffer);
    EXPECT_EQ(SQL_INTEGER, dataType);
    EXPECT_EQ(SQL_NO_NULLS, nullable);
}

TEST_F(ODBCTestRequiresChorus, DescribeColAfterSQLPrepare)
{
    setupTable();
    RETCODE ret = SQLPrepare(stmt, (SQLCHAR*)"select mxName, mxInt, mxShort, mxBigInt, mxDouble, mxChar, mxFloat, mxDecimal, mxTableSpc, mxDate, mxTime, mxTimeStamp, mxBinary, mxClob from mxLattice", SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);

    SQLCHAR colNameBuffer[1024];
    SQLSMALLINT colNameLength;
    SQLSMALLINT dataType;
    SQLULEN columnSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 1, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(6, colNameLength);
    EXPECT_STREQ("MXNAME", (char *)colNameBuffer);
    EXPECT_EQ(SQL_VARCHAR, dataType);
    EXPECT_EQ((size_t)128, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 2, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(5, colNameLength);
    EXPECT_STREQ("MXINT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_INTEGER, dataType);
    //DB-14428
    //EXPECT_EQ(10, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 3, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(7, colNameLength);
    EXPECT_STREQ("MXSHORT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_SMALLINT, dataType);
    //DB-14428
    //EXPECT_EQ(5, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 4, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(8, colNameLength);
    EXPECT_STREQ("MXBIGINT", (char *)colNameBuffer);
    //DB-14428 -- we get -5, which might be on purpose but not very valid.
    //EXPECT_EQ(0, dataType);
    // we get 19.
    //EXPECT_EQ(0, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 5, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(8, colNameLength);
    EXPECT_STREQ("MXDOUBLE", (char *)colNameBuffer);
    EXPECT_EQ(SQL_DOUBLE, dataType);
    EXPECT_EQ((size_t)15, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 6, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(6, colNameLength);
    EXPECT_STREQ("MXCHAR", (char *)colNameBuffer);
    EXPECT_EQ(SQL_SMALLINT, dataType);
    //DB-14428
    //EXPECT_EQ(5, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 7, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(7, colNameLength);
    EXPECT_STREQ("MXFLOAT", (char *)colNameBuffer);
    EXPECT_EQ(SQL_DOUBLE, dataType);
    EXPECT_EQ((size_t)15, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 8, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(9, colNameLength);
    EXPECT_STREQ("MXDECIMAL", (char *)colNameBuffer);
    EXPECT_EQ(SQL_NUMERIC, dataType);
    EXPECT_EQ((size_t)10, columnSize);
    EXPECT_EQ(2, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 9, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(10, colNameLength);
    EXPECT_STREQ("MXTABLESPC", (char *)colNameBuffer);
    EXPECT_EQ(SQL_VARCHAR, dataType);
    EXPECT_EQ((size_t)128, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 10, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(6, colNameLength);
    EXPECT_STREQ("MXDATE", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_DATE, dataType);
    EXPECT_EQ((size_t)10, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 11, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(6, colNameLength);
    EXPECT_STREQ("MXTIME", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_TIME, dataType);
    //should be 8, or have decimal digits. we get 16. DB-14428
    //EXPECT_EQ(8, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 12, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(11, colNameLength);
    EXPECT_STREQ("MXTIMESTAMP", (char *)colNameBuffer);
    EXPECT_EQ(SQL_TYPE_TIMESTAMP, dataType);
    EXPECT_EQ((size_t)16, columnSize);
    EXPECT_EQ(6, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 13, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(8, colNameLength);
    EXPECT_STREQ("MXBINARY", (char *)colNameBuffer);
    // DB-14428, shows up as -4, maybe correct, maybe not too valid
    //EXPECT_EQ(0, dataType);
    EXPECT_EQ((size_t)4099, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);

    colNameLength= -1;
    dataType = -1;
    columnSize = 5000;
    decimalDigits = -1;
    nullable = -1;
    ret = SQLDescribeCol(stmt, 14, colNameBuffer, 1023, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    colNameBuffer[colNameLength] = 0;
    EXPECT_EQ(6, colNameLength);
    EXPECT_STREQ("MXCLOB", (char *)colNameBuffer);
    //DB-14428, shows up as -1, maybe correct, not too valid.
    //EXPECT_EQ(0, dataType);
    EXPECT_EQ((size_t)100, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
    SQLFreeStmt(stmt, SQL_CLOSE);
}

/*
SQLRETURN SQLRowCount(
      SQLHSTMT   StatementHandle,
      SQLLEN *   RowCountPtr);
*/

TEST_F(ODBCTestRequiresChorus, UpdateCount)
{
    execDirect("drop table t1 if exists");
    execDirect("create table t1(a int primary key)");
    execDirect("insert into t1 values (1), (2), (3)");
    SQLLEN count;
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    int updateCountFunc = getIntData(1);
    ASSERT_EQ(count, updateCountFunc);
    freeStmt();

    execDirect("update t1 set a = 20 where a = 2");
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    updateCountFunc = getIntData(1);
    ASSERT_EQ(count, updateCountFunc);
    freeStmt();

    execDirect("delete from t1 where a = 3");
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    updateCountFunc = getIntData(1);

    ASSERT_EQ(count, updateCountFunc);
    freeStmt();
}

TEST_F(ODBCTestRequiresChorus, UpdateCountPrepared)
{
    execDirect("drop table t1 if exists");
    execDirect("create table t1(a int primary key)");
    RETCODE ret = SQLPrepare(stmt, (SQLCHAR*)"insert into t1 values (1), (2), (3)", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLExecute(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    SQLLEN count;
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    int updateCountFunc = getIntData(1);
    ASSERT_EQ(count, updateCountFunc);
    freeStmt();

    ret = SQLPrepare(stmt, (SQLCHAR*)"update t1 set a = 20 where a = 2", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLExecute(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    updateCountFunc = getIntData(1);
    ASSERT_EQ(count, updateCountFunc);
    freeStmt();

    ret = SQLPrepare(stmt, (SQLCHAR*)"delete from t1 where a = 3", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLExecute(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ(SQL_SUCCESS, SQLRowCount(stmt, &count));
    execDirectAndFetch("select getUpdateCount() from dual");
    updateCountFunc = getIntData(1);
    ASSERT_EQ(count, updateCountFunc);
    freeStmt();
}

TEST_F(ODBCTestRequiresChorus, CheckResultSetsLeftOpenByInsert)
{
    freeStmt();
    execDirect("create table t1 (i int)");
    execDirectAndFetch("select value from system.properties where property = 'MAX_CONNECTION_OPEN_RESULTSETS'");
    bool isNull;
    std::string value = getCharData(1, isNull);
    freeStmt();
    // run the statement more than the number of result sets that can be kept open
    TCHAR cmd[256];
    for (int i = 0; i < atoi(value.c_str()) + 10; i++) {
        sprintf(cmd, "INSERT INTO t1 VALUES(%d)", i);
        execDirect(cmd);
    }
}

TEST_F(ODBCTestRequiresChorus, CheckSQLSTATEErrorCodeNoTable)
{
    RETCODE retcode;

    //No such table
    retcode = SQLExecDirect(stmt, (SQLCHAR*)"insert into user.t values ('kk')", SQL_NTS);
    ASSERT_EQ(SQL_ERROR, retcode);
    std::string sqlState, message;
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    ASSERT_STREQ("42000", sqlState.c_str());
    ASSERT_STREQ("can't find table \"USER.T\"\nSQL: insert into user.t values ('kk')", message.c_str());
}

TEST_F(ODBCTestRequiresChorus, CheckSQLSTATEErrorCodeUNIQUE_DUPLICATE)
{
    RETCODE retcode;
    std::string sqlState, message;
    //Duplicate values on unique index
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"create table tuniq(a integer, primary key (a))", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLExecDirect(stmt, (SQLCHAR*)"insert into tuniq values (1)", SQL_NTS));
    retcode = SQLExecDirect(stmt, (SQLCHAR*)"insert into tuniq values (1)", SQL_NTS);
    ASSERT_EQ(SQL_ERROR, retcode);
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    ASSERT_STREQ("23000", sqlState.c_str());
    ASSERT_STREQ("duplicate value in unique index TUNIQ..PRIMARY_KEY, key = '1'", message.c_str());
}

TEST_F(ODBCTestRequiresChorus, CheckSQLSTATEErrorCodeINVALID_OPERATION)
{
    RETCODE retcode;
    std::string sqlState, message;
    //DBA does not have autority to alter table
    retcode = SQLExecDirect(stmt, (SQLCHAR*)"drop table system.tables", SQL_NTS);
    ASSERT_EQ(SQL_ERROR, retcode);
    extractError(stmt, SQL_HANDLE_STMT, sqlState, message);
    ASSERT_STREQ("58000", sqlState.c_str());
    ASSERT_STREQ("user DBA does not have alter authority to Table SYSTEM.TABLES", message.c_str());
}

TEST_F(ODBCTestRequiresChorus, testSQLGetInfo)
{
    SQLUINTEGER limit = 0;
    RETCODE ret = SQLGetInfo(hdbc1, SQL_MAX_COLUMN_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_IDENTIFIER_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_CATALOG_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)255, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_PROCEDURE_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_SCHEMA_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_TABLE_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_CURSOR_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    limit = 0;
    ret = SQLGetInfo(hdbc1, SQL_MAX_USER_NAME_LEN, &limit, sizeof(limit), NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ASSERT_EQ((size_t)128, limit);

    setupTable();
    ret = SQLPrepare(stmt, (SQLCHAR*)"select mxName, mxInt, mxShort, mxBigInt, mxDouble, mxChar, mxFloat, mxDecimal, mxTableSpc, mxDate, mxTime, mxTimeStamp, mxBinary, mxClob from mxLattice", SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);

    SQLCHAR colNameBuffer[256];
    SQLSMALLINT colNameLength = -1;
    SQLSMALLINT dataType = -1;
    SQLULEN columnSize = 5000;
    SQLSMALLINT decimalDigits = -1;
    SQLSMALLINT nullable = -1;

    ret = SQLDescribeCol(stmt, 1, colNameBuffer, 256, &colNameLength,
                         &dataType, &columnSize, &decimalDigits, &nullable);
    EXPECT_EQ(ret, SQL_SUCCESS);
    EXPECT_EQ(6, colNameLength);
    colNameBuffer[colNameLength] = 0;
    EXPECT_STREQ("MXNAME", (char *)colNameBuffer);
    EXPECT_EQ(SQL_VARCHAR, dataType);
    EXPECT_EQ((size_t)128, columnSize);
    EXPECT_EQ(0, decimalDigits);
    EXPECT_EQ(SQL_NULLABLE, nullable);
}

TEST_F(ODBCTestRequiresChorus, checkSQLBindColAfterSQLPrepare)
{
    setupTable();
    RETCODE ret = SQLPrepare(stmt, (SQLCHAR*)"select mxName, mxInt, mxShort, mxBigInt, mxDouble, mxChar, mxFloat, mxDecimal, mxTableSpc, mxDate, mxTime, mxTimeStamp, mxBinary, mxClob from mxLattice", SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);

    SQLLEN LenOrInd = 5000;
    SQLCHAR buffer[256];
    ret = SQLBindCol(stmt, 1, SQL_CHAR, buffer, sizeof(buffer), &LenOrInd );
    ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(ODBCTestRequiresChorus, checkSQLBindColAfterSQLPrepareWithDual)
{
    setupTable();
    RETCODE ret = SQLPrepare(stmt, (SQLCHAR*)"select 1 from dual", SQL_NTS);
    ASSERT_EQ(ret, SQL_SUCCESS);

    SQLLEN LenOrInd = 5000;
    SQLCHAR buffer[256];
    ret = SQLBindCol(stmt, 1, SQL_CHAR, buffer, sizeof(buffer), &LenOrInd );
    ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(ODBCTestRequiresChorus, sqlBindBeforePrepareOrExecute)
{
    setupTable();
    SQLLEN LenOrInd = 5000;
    SQLCHAR buffer[256];
    RETCODE ret;
    ret = SQLBindCol(stmt, 1, SQL_CHAR, buffer, sizeof(buffer), &LenOrInd );
    ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(ODBCTestRequiresChorus, testMaxColumnNumberForBinding)
{
    setupTable();
    SQLLEN LenOrInd = 5000;
    SQLCHAR buffer[256];
    RETCODE ret;
    ret = SQLBindCol(stmt, 10001, SQL_CHAR, buffer, sizeof(buffer), &LenOrInd );
    ASSERT_EQ(ret, SQL_ERROR);
}
