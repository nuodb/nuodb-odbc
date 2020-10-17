/**
 * (C) Copyright 2016-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "ODBCTestBase.h"

#include <vector>
#include <algorithm>
#include <string>


struct TypeInfoToCheck {
    std::string name;
    int dataType;
    int sqlDataType;

    TypeInfoToCheck() {
        dataType = -1;
        sqlDataType = -1;
    }

    TypeInfoToCheck(const char *a_name, int a_dataType, int a_sqlDataType)
        :name(a_name), dataType(a_dataType), sqlDataType(a_sqlDataType)
    {
        //
    }

    bool operator==(const TypeInfoToCheck& other) const {
        return name == other.name
            && dataType == other.dataType
            && sqlDataType == other.sqlDataType;
    }

    bool operator<(const TypeInfoToCheck& other) const {
        return name < other.name;
    }
};

::std::ostream& operator<<(::std::ostream& os, const TypeInfoToCheck& t) {
    return os << "(" << t.name << " dt " << t.dataType << " sdt " << t.sqlDataType << ")";
}

class ODBCMetadataTestRequiresChorus : public ODBCTestBase
{
    void SetUp() override
    {
        ODBCTestBase::SetUp();
        execDirect("create table tt1(name varchar(32) not null, primary key(name))");
        execDirect("create table tt2(name varchar(32) not null, rank int not null, primary key(name, rank))");
        execDirect("create table indexed(file int, rank int not null, name varchar(32))");
        execDirect("create index rank_index on indexed(rank)");
    }

public:
    void fetchTypeInfo(TypeInfoToCheck& val, bool& done);
};

void ODBCMetadataTestRequiresChorus::fetchTypeInfo(TypeInfoToCheck& val, bool& done)
{
    done = false;
    RETCODE ret = SQLFetch(stmt);
    if (ret == SQL_NO_DATA) {
        done = true;
        return;
    }
    ASSERT_EQ(SQL_SUCCESS, ret);
    bool nullValue;
    val.name = getCharData(1, nullValue);
    val.dataType = getIntData(2, nullValue);
    val.sqlDataType = getIntData(16, nullValue);
}

TEST_F(ODBCMetadataTestRequiresChorus, Tables)
{
    RETCODE retcode;

    EXPECT_EQ(SQL_SUCCESS, SQLTables(stmt, NULL, 0, NULL, 0, (SQLCHAR*)"tt%", SQL_NTS, NULL, 0));

    // SQL_DESC_TYPE is not supported, so we check 'concise'.

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("REMARKS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    ASSERT_EQ(SQL_LONGVARCHAR, attrs.conciseType);

    TEST_CHAR_TYPE catalog[128];
    SQLLEN catInd;
    TEST_CHAR_TYPE schema[128];
    SQLLEN schemaInd;
    TEST_CHAR_TYPE table[128];
    SQLLEN tableInd;
    TEST_CHAR_TYPE type[128];
    SQLLEN typeInd;
    TEST_CHAR_TYPE remarks[128];
    SQLLEN remarksInd;

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 1, TEST_SQL_C_CHAR, catalog, 128, &catInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 2, TEST_SQL_C_CHAR, schema, 128, &schemaInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 3, TEST_SQL_C_CHAR, table, 128, &tableInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 4, TEST_SQL_C_CHAR, type, 128, &typeInd));

    EXPECT_EQ(SQL_SUCCESS, SQLBindCol(stmt, 5, SQL_C_CHAR, remarks, 128, &remarksInd));

    bool sawtt1 = false;
    bool sawtt2 = false;
    while ((retcode = SQLFetch(stmt)) == SQL_SUCCESS) {
        if (catInd == -1) {
            catInd = 0;
        }
        catalog[catInd] = 0;

        std::string ucat = convertToUTF8FromUnicode(catalog);
        if (schemaInd == -1) {
            schemaInd = 0;
        }
        schema[schemaInd] = 0;

        std::string uschema = convertToUTF8FromUnicode(schema);
        if (tableInd == -1) {
            tableInd = 0;
        }
        table[tableInd] = 0;
        std::string utable = convertToUTF8FromUnicode(table);
        if (typeInd == -1) {
            typeInd = 0;
        }
        type[typeInd] = 0;
        std::string utype =  convertToUTF8FromUnicode(type);
        if (remarksInd == -1) {
            remarksInd = 0;
        }
        remarks[remarksInd] = 0;
        std::string uremarks = convertToUTF8FromUnicode(table);
        if (utable == std::string("TT1")) {
            sawtt1 = true;
        }
        if (utable == std::string("TT2")) {
            sawtt2 = true;
        }
    }
    ASSERT_TRUE(sawtt1);
    ASSERT_TRUE(sawtt2);
}

// SetStmtAttr is not implemented, plus I can't find any case-sensitive behavior at all in nuosql.
TEST_F(ODBCMetadataTestRequiresChorus, DISABLED_TableCase)
{
    ASSERT_EQ(SQL_SUCCESS, SQLSetConnectAttr(hdbc1, SQL_ATTR_METADATA_ID, (SQLPOINTER)SQL_TRUE, 0));
    ASSERT_EQ(SQL_SUCCESS, SQLTables(stmt, NULL, 0, NULL, 0, (SQLCHAR*)"tt1", SQL_NTS, NULL, 0));
    ASSERT_EQ(SQL_SUCCESS, SQLFetch(stmt));
    bool nullValue;
    std::string name = getCharData(3, nullValue);
    EXPECT_STREQ("TT1", name.c_str());
    EXPECT_EQ(SQL_SUCCESS, SQLFreeStmt(stmt, SQL_CLOSE));

    EXPECT_EQ(SQL_SUCCESS, SQLSetConnectAttr(hdbc1, SQL_ATTR_METADATA_ID, (SQLPOINTER)SQL_FALSE, 0));
    EXPECT_EQ(SQL_SUCCESS, SQLTables(stmt, NULL, 0, NULL, 0, (SQLCHAR*)"tt1", SQL_NTS, NULL, 0));
    EXPECT_EQ(SQL_NO_DATA, SQLFetch(stmt));
    EXPECT_EQ(SQL_SUCCESS, SQLFreeStmt(stmt, SQL_CLOSE));
}


TEST_F(ODBCMetadataTestRequiresChorus, Columns)
{
    ASSERT_EQ(SQL_SUCCESS, SQLColumns(stmt,
                                      NULL, 0,
                                      (SQLCHAR*)SCHEMANAME,
                                      SQL_NTS,
                                      (SQLCHAR*)"TT2",
                                      SQL_NTS,
                                      (SQLCHAR*)"%",
                                      SQL_NTS));

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable, but isn't. DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable but isn't. DB-14416
    // EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable but isn't. DB-14416
    // EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable but isn't. DB-14416
    // EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    // doc says 'smallint', nuodb returns 'integer'
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(6);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TYPE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(7);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_SIZE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(8);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("BUFFER_LENGTH", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(9);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("DECIMAL_DIGITS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14414
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(10);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NUM_PREC_RADIX", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14414
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(11);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NULLABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14414
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(12);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("REMARKS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(13);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_DEF", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(14);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SQL_DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14414
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(15);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SQL_DATETIME_SUB", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14414
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(16);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("CHAR_OCTET_LENGTH", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(17);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("ORDINAL_POSITION", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14416
    //EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(18);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("IS_NULLABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    bool nullValue;
    std::string name = getCharData(4, nullValue);
    ASSERT_STREQ("NAME", name.c_str());
    EXPECT_EQ(SQL_SUCCESS, SQLFetch(stmt));

    name = getCharData(4, nullValue);
    ASSERT_STREQ("RANK", name.c_str());
    SQLFreeStmt(stmt, SQL_CLOSE);
}

TEST_F(ODBCMetadataTestRequiresChorus, DISABLED_SpecialColumns)
{
    RETCODE ret = SQLSpecialColumns(stmt, SQL_BEST_ROWID,
                                    NULL,
                                    0,
                                    (SQLCHAR*)SCHEMANAME,
                                    SQL_NTS,
                                    (SQLCHAR*)"TT2",
                                    SQL_NTS,
                                    SQL_SCOPE_CURROW,
                                    SQL_NULLABLE);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    char colbuf[128];
    SQLLEN len;
    ret = SQLGetData(stmt, 2, SQL_C_CHAR, colbuf, 128, &len);
    ASSERT_EQ(SQL_SUCCESS, ret);
    colbuf[len] = 0;
    ASSERT_STREQ("NAME", colbuf);
    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);

    ret = SQLGetData(stmt, 2, SQL_C_CHAR, colbuf, 128, &len);
    ASSERT_EQ(SQL_SUCCESS, ret);
    colbuf[len] = 0;
    ASSERT_STREQ("RANK", colbuf);
    SQLFreeStmt(stmt, SQL_CLOSE);
}

TEST_F(ODBCMetadataTestRequiresChorus, PrimaryKeys)
{
    RETCODE ret = SQLPrimaryKeys(stmt, NULL, 0, (SQLCHAR*)SCHEMANAME, SQL_NTS,
                                 (SQLCHAR*)"TT2", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable, but isn't. DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable but isn't. DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable, but isn't. DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // should be nullable, but isn't. DB-14416
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("KEY_SEQ", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14417
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(6);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PK_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14417
    //EXPECT_TRUE(attrs.nullable);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    // 4, 5, 6 -- col seq kname
    bool nullValue;
    std::string col = getCharData(4, nullValue);
    ASSERT_STREQ("NAME", col.c_str());
    SQLINTEGER seq = getIntData(5);
    ASSERT_EQ(1, seq);
    std::string kname = getCharData(6, nullValue);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);

    col = getCharData(4, nullValue);
    ASSERT_STREQ("RANK", col.c_str());
    seq = getIntData(5);
    ASSERT_EQ(2, seq);
    std::string kname2 = getCharData(6, nullValue);
    ASSERT_STREQ(kname.c_str(), kname2.c_str());
    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_NO_DATA, ret);
}

TEST_F(ODBCMetadataTestRequiresChorus, Procedures)
{
    execDirect("create procedure test(IN p1 integer, OUT p2 integer, INOUT p3 integer) RETURNS output(field integer) AS p2 = p1; p3 = p3 +1; insert into output values(p3); end_procedure;");

    RETCODE ret = SQLProcedures(stmt, NULL, 0, (SQLCHAR*)SCHEMANAME, SQL_NTS, NULL, 0);
    ASSERT_EQ(SQL_SUCCESS, ret);

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14418
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14418
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(7);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("REMARKS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14418
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(8);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14418
    //EXPECT_TRUE(attrs.nullable);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    bool nullValue;
    std::string name = getCharData(3, nullValue);
    ASSERT_STREQ("TEST", name.c_str());
    // it returns something, but it's not officially a function.
    ASSERT_EQ(SQL_PT_PROCEDURE, getIntData(8));
    // There can be procedures we don't know about, so we don't insist
    // on just one.
}

//db-14410
TEST_F(ODBCMetadataTestRequiresChorus, DISABLED_ProceduresFunctions)
{
    execDirect(
        "CREATE FUNCTION func_is_date (i_date string, i_format string )"
        "RETURNS BOOLEAN "
        "AS "
        "VAR l_out BOOLEAN = 'TRUE';"
        "RETURN l_out;"
        "END_FUNCTION;");

    RETCODE ret = SQLProcedures(stmt, NULL, 0, NULL, 0, NULL, 0);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    bool nullValue;
    std::string name = getCharData(3, nullValue);
    ASSERT_STREQ("FUNC_IS_DATE", name.c_str());
    ASSERT_EQ(SQL_PT_FUNCTION, getIntData(8));
    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_NO_DATA, ret);
}


TEST_F(ODBCMetadataTestRequiresChorus, Statistics)
{
    RETCODE ret = SQLStatistics(stmt, NULL, 0, NULL, 0, (SQLCHAR*)"INDEXED", SQL_NTS, SQL_INDEX_ALL, 0);
    ASSERT_EQ(SQL_SUCCESS, ret);

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TABLE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NON_UNIQUE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14419
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("INDEX_QUALIFIER", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(6);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("INDEX_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(7);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14419
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(8);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("ORDINAL_POSITION", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14419
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    // DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(9);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(10);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("ASC_OR_DESC", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14419
    //EXPECT_EQ(SQL_CHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(11);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("CARDINALITY", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(12);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PAGES", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(13);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("FILTER_CONDITION", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14419
    //EXPECT_TRUE(attrs.nullable);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    bool nullValue;
    std::string name = getCharData(3, nullValue);
    ASSERT_STREQ("INDEXED", name.c_str());
    SQLINTEGER u = getIntData(4);
    ASSERT_EQ(SQL_TRUE, u);
    name = getCharData(5, nullValue);
    // this should be null, as I read the spec, but it's not. DB-14411
    ASSERT_FALSE(nullValue);
    ASSERT_EQ((size_t)0, name.size());

    name = getCharData(6, nullValue);
    ASSERT_STREQ("RANK_INDEX", name.c_str());
    ASSERT_EQ(SQL_INDEX_OTHER, getIntData(7));
    ASSERT_EQ(1, getIntData(8));
    name = getCharData(9, nullValue);
    ASSERT_STREQ("RANK", name.c_str());
    //
    std::string ad = getCharData(10, nullValue);
    ASSERT_TRUE(nullValue);
}

TEST_F(ODBCMetadataTestRequiresChorus,ProcedureColumns)
{
    execDirect("create procedure test(IN p1 integer, OUT p2 integer, INOUT p3 datetime) RETURNS output(field integer) AS p2 = p1; insert into output values(p1); end_procedure;");

    RETCODE ret = SQLProcedureColumns(stmt, NULL, 0, NULL, 0, (SQLCHAR*)"TEST", SQL_NTS,
                                      (SQLCHAR*)"%", SQL_NTS);
    ASSERT_EQ(SQL_SUCCESS, ret);

    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_CAT", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_SCHEM", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("PROCEDURE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(6);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14420
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(7);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TYPE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(8);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //DB-14420
    //EXPECT_STREQ("COLUMN_SIZE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(9);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //DB-14420
    //EXPECT_STREQ("BUFFER_LENGTH", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(10);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //DB-14420
    //EXPECT_STREQ("DECIMAL_DIGITS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(11);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //DB-14420
    //EXPECT_STREQ("NUM_PREC_RADIX", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(12);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NULLABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(13);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("REMARKS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(14);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("COLUMN_DEF", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //DB-14420
    //EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(15);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SQL_DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(16);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //DB-14420
    //EXPECT_STREQ("SQL_DATETIME_SUB", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(17);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("CHAR_OCTET_LENGTH", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(18);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("ORDINAL_POSITION", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    // DB-14419
    // EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(19);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("IS_NULLABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_LONGVARCHAR, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //DB-14420
    //EXPECT_TRUE(attrs.nullable);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);

    bool nullValue;
    std::string schema = getCharData(2, nullValue);
    EXPECT_STREQ(SCHEMANAME, schema.c_str());
    std::string name = getCharData(3, nullValue);
    EXPECT_STREQ("TEST", name.c_str());
    name = getCharData(4, nullValue);
    EXPECT_STREQ("P1", name.c_str());
    EXPECT_EQ(SQL_PARAM_INPUT, getIntData(5));
    EXPECT_EQ(SQL_INTEGER, getIntData(6));
    EXPECT_STREQ("integer", getCharData(7, nullValue).c_str());
    //int colSize = getIntData(8, nullValue);
    //DB-14420
    //EXPECT_TRUE(nullValue);
    // The rest of the columns get hairy in a hurry.

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    schema = getCharData(2, nullValue);
    EXPECT_STREQ(SCHEMANAME, schema.c_str());
    name = getCharData(3, nullValue);
    EXPECT_STREQ("TEST", name.c_str());
    name = getCharData(4, nullValue);
    EXPECT_STREQ("P2", name.c_str());
    EXPECT_EQ(SQL_PARAM_OUTPUT, getIntData(5));
    EXPECT_EQ(SQL_INTEGER, getIntData(6));
    EXPECT_STREQ("integer", getCharData(7, nullValue).c_str());
    //colSize = getIntData(8, nullValue);
    //DB-14420
    //EXPECT_TRUE(nullValue);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    schema = getCharData(2, nullValue);
    EXPECT_STREQ(SCHEMANAME, schema.c_str());
    name = getCharData(3, nullValue);
    EXPECT_STREQ("TEST", name.c_str());
    name = getCharData(4, nullValue);
    EXPECT_STREQ("P3", name.c_str());
    EXPECT_EQ(SQL_PARAM_INPUT_OUTPUT, getIntData(5));
    // comes through as SQL_TYPE_TIMESTAMP/datetime
    //EXPECT_EQ(SQL_DATETIME, getIntData(6));
    //EXPECT_STREQ("datetime", getCharData(7, nullValue).c_str());
    //colSize = getIntData(8, nullValue);
    //DB-14420
    //EXPECT_TRUE(nullValue);

    ret = SQLFetch(stmt);
    ASSERT_EQ(SQL_SUCCESS, ret);
    EXPECT_STREQ(SCHEMANAME, schema.c_str());
    name = getCharData(3, nullValue);
    EXPECT_STREQ("TEST", name.c_str());
    name = getCharData(4, nullValue);
    EXPECT_STREQ("FIELD", name.c_str());
    EXPECT_EQ(SQL_RESULT_COL, getIntData(5));
    EXPECT_EQ(SQL_INTEGER, getIntData(6));
    EXPECT_STREQ("integer", getCharData(7, nullValue).c_str());
    //colSize = getIntData(8, nullValue);
    //DB-14420
    //EXPECT_TRUE(nullValue);
}

TEST_F(ODBCMetadataTestRequiresChorus, SQLGetTypeInfo)
{
    // all commented out below reports in DB-14489.
    RETCODE ret = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);
    ASSERT_EQ(SQL_SUCCESS, ret);
    ColAttributes attrs = colAttributes(1);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("TYPE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_EQ(127, attrs.length);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(2);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(3);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //EXPECT_STREQ("COLUMN_SIZE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(4);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("LITERAL_PREFIX", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_EQ(127, attrs.length);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(5);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("LITERAL_SUFFIX", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_EQ(127, attrs.length);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(6);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("CREATE_PARAMS", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_EQ(127, attrs.length);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(7);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NULLABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(8);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("CASE_SENSITIVE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(9);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SEARCHABLE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(10);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("UNSIGNED_ATTRIBUTE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(11);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("FIXED_PREC_SCALE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(12);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    //EXPECT_STREQ("AUTO_UNIQUE_VALUE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(13);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("LOCAL_TYPE_NAME", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_VARCHAR, attrs.conciseType);
    EXPECT_EQ(127, attrs.length);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(14);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("MINIMUM_SCALE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(15);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("MAXIMUM_SCALE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(16);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SQL_DATA_TYPE", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_FALSE(attrs.nullable);

    attrs = colAttributes(17);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("SQL_DATETIME_SUB", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    //EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

    attrs = colAttributes(18);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("NUM_PREC_RADIX", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_INTEGER, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    //EXPECT_TRUE(attrs.nullable);

#if 0 // missing
    attrs = colAttributes(19);
    EXPECT_TRUE(attrs.present(SQL_DESC_NAME));
    EXPECT_STREQ("INTERVAL_PRECISION", attrs.name.c_str());
    EXPECT_TRUE(attrs.present(SQL_DESC_CONCISE_TYPE));
    EXPECT_EQ(SQL_SMALLINT, attrs.conciseType);
    EXPECT_TRUE(attrs.present(SQL_DESC_NULLABLE));
    EXPECT_TRUE(attrs.nullable);
#endif

    std::vector<TypeInfoToCheck> results;
    bool doneFetching;
    TypeInfoToCheck val;
    // there's no order.
    while (!(fetchTypeInfo(val, doneFetching), doneFetching)) {
        results.push_back(val);
    }
    std::sort(results.begin(), results.end());
    int inx = 0;
    // 2003 might be a reasonable custom data type, is 0 correct?
    EXPECT_EQ(TypeInfoToCheck("BIGINT", -5, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("BINARY", -2, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("BLOB", -4, 0), results[inx++]);
    // 16 is unexplained.
    EXPECT_EQ(TypeInfoToCheck("BOOLEAN", 16, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("CHAR", SQL_CHAR, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("CLOB", -1, 0), results[inx++]);
    // shouldn't SQL_TYPE_DATE be in the SQL_DATA_TYPE, not the CONSISE one?
    EXPECT_EQ(TypeInfoToCheck("DATE", SQL_TYPE_DATE, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("DECIMAL", SQL_DECIMAL, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("DOUBLE", SQL_DOUBLE, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("INTEGER", SQL_INTEGER, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("NCHAR", SQL_VARCHAR, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("NCLOB", -1, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("NUMBER", SQL_NUMERIC, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("NUMERIC", SQL_NUMERIC, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("SMALLINT", SQL_SMALLINT, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("STRING", SQL_VARCHAR, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("TEXT", SQL_VARCHAR, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("TIME", SQL_TYPE_TIME, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("TIMESTAMP", SQL_TYPE_TIMESTAMP, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("VARBINARY", -3, 0), results[inx++]);
    EXPECT_EQ(TypeInfoToCheck("VARCHAR", SQL_VARCHAR, 0), results[inx++]);
}

TEST_F(ODBCMetadataTestRequiresChorus, SQLGetInfo)
{
    SQLUINTEGER uintVal;

    RETCODE ret = SQLGetInfo(hdbc1, SQL_TXN_ISOLATION_OPTION, &uintVal,
                             0, NULL);
    ASSERT_EQ(SQL_SUCCESS, ret);
    EXPECT_NE((unsigned int)0, uintVal & SQL_TXN_SERIALIZABLE);

    char ind[2];
    SQLSMALLINT outLen;
    ret = SQLGetInfo(hdbc1, SQL_DATA_SOURCE_READ_ONLY,
                     &ind[0],
                     2,
                     &outLen);
    ASSERT_EQ(SQL_SUCCESS, ret);
    EXPECT_EQ('N', ind[0]);
    EXPECT_EQ(1, outLen);
}

TEST_F(ODBCMetadataTestRequiresChorus, StmtAttrs)
{
    SQLLEN val;
    ASSERT_EQ(SQL_SUCCESS, SQLGetStmtAttr(stmt, SQL_ATTR_MAX_LENGTH, &val, sizeof(val),
                                          NULL));
    ASSERT_EQ(SQL_SUCCESS, SQLGetStmtAttr(stmt, SQL_ATTR_MAX_ROWS, &val, sizeof(val),
                                          NULL));
    ASSERT_EQ(SQL_SUCCESS, SQLGetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT, &val, sizeof(val),
                                          NULL));
    // why did JDBC expect 5?
    ASSERT_EQ(0, val);

    // the ODBC manager complains that the cursor is not available for setting these options unless we
    // use a brand new statement
    SQLHANDLE hstmt1;
    ASSERT_EQ(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    ASSERT_EQ(SQL_SUCCESS,
        SQLSetStmtAttr(hstmt1, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_READ_ONLY, SQL_IS_UINTEGER));
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO,
        SQLSetStmtAttr(hstmt1, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_LOCK, SQL_IS_UINTEGER));

    ASSERT_EQ(SQL_SUCCESS,
        SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_UINTEGER));
    ASSERT_EQ(SQL_SUCCESS_WITH_INFO,
        SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, SQL_IS_UINTEGER));

    ASSERT_EQ(SQL_SUCCESS, SQLFreeStmt(hstmt1, SQL_CLOSE));
}


TEST_F(ODBCMetadataTestRequiresChorus, ColumnTypeAndDisplaySize)
{
    execDirect("select count(*) as count from system.tables");
    SQLLEN displaySize;
    SQLLEN columnType;
    RETCODE ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_BIGINT);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 21);

    // max INT
    execDirect("select  2147483647 as MAX_INT from dual");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_INTEGER);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 11);

    // max BIGINT
    execDirect("select  9223372036854775807 as BIG_INT from dual");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_BIGINT);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 21);

    // varchar  - 10 chars in length - in a real table
    execDirect("create table small_string_table(astring varchar(256))");
    std::string small_string(10, 'a');
    std::string query = "insert into small_string_table (astring) values ('" + small_string + "')";
    execDirect(query.c_str());
    execDirect("select astring from small_string_table");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_VARCHAR);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 256);

    // varchar  - 42 chars in length - in a real table
    execDirect("create table medium_string_table(astring varchar(256))");
    std::string medium_string(42, 'a');
    query = "insert into medium_string_table (astring) values ('" + medium_string + "')";
    execDirect(query.c_str());
    execDirect("select astring from medium_string_table");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_VARCHAR);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 256);

    // varchar  - 40 chars in length - in a real table
    execDirect("create table medium_string_table_two(astring varchar(40))");
    std::string medium_string_two(35, 'a');
    query = "insert into medium_string_table_two (astring) values ('" + medium_string_two + "')";
    execDirect(query.c_str());
    execDirect("select astring from medium_string_table_two");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_VARCHAR);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 40);

    // varchar  - 255 chars in length - in a real table
    execDirect("create table large_string_table(astring varchar(256))");
    std::string large_string(255, 'a');
    query = "insert into large_string_table (astring) values ('" + large_string + "')";
    execDirect(query.c_str());
    execDirect("select astring from large_string_table");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_VARCHAR);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 256);

    // string  - 10 chars in length - in a real table
    execDirect("create table random_string_table(astring string)");
    std::string astring(10, 'a');
    query = "insert into random_string_table (astring) values ('" + astring + "')";
    execDirect(query.c_str());
    execDirect("select tablename from system.tables where tableid = 1");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_LONGVARCHAR);
    // display size is useless for LONGVARCHAR as it is unknown just from the metadata

    // Timestamp: displaySize = 26
    execDirect("create table tt(ts timestamp)");
    execDirect("insert into tt values (now())");
    execDirect("select ts from tt");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIMESTAMP);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 26);

    // Timestamp(3): displaySize = 23
    execDirect("create table ttTwo(ts timestamp(3))");
    execDirect("insert into ttTwo values (now())");
    execDirect("select ts from ttTwo");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIMESTAMP);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 23);

    // Timestamp(9): displaySize = 29
    execDirect("create table tt3(ts timestamp(9))");
    execDirect("insert into tt3 values (now())");
    execDirect("select ts from tt3");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIMESTAMP);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 29);

    // Dual tables - Timestamp
    execDirect("select now() from dual");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIMESTAMP);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 26);

    // Time: displaySize = 8
    execDirect("create table ttt(tm time)");
    execDirect("insert into ttt values (now())");
    execDirect("select tm from ttt");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIME);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 8);

    // Time(6): displaySize = 15
    execDirect("create table ttt2(tm time(6))");
    execDirect("insert into ttt2 values (now())");
    execDirect("select tm from ttt2");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIME);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 15);

    // Dual tables - Time
    execDirect("select current_time from dual");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_TIME);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 8);

    // Date: displaySize = 10
    execDirect("create table dt(d date)");
    execDirect("insert into dt values (current_date)");
    execDirect("select d from dt");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_DATE);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 10);

    // Dual tables - Date
    execDirect("select current_date from dual");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_TYPE_DATE);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 10);

    // Check that the column type for a LOWER node is LONGVARCHAR and the display size is 0
    execDirect("select lower(FIELD) lower_field from SYSTEM.FIELDS where tablename = 'FIELDS'");
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaySize );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(displaySize, 0);
    ret = SQLColAttributes(stmt, (SQLSMALLINT)1, SQL_COLUMN_TYPE, NULL, 0, NULL, &columnType );
    ASSERT_EQ(ret, SQL_SUCCESS);
    ASSERT_EQ(columnType, SQL_LONGVARCHAR);
}

TEST_F(ODBCMetadataTestRequiresChorus, PrepareStatementMetaData)
{
    //select
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"select tablename, schema, extends, type, subtype, tableid, currentversion, remarks, viewdefinition, cardinality, recordsbatchfactor  from system.tables", SQL_NTS));
    SQLSMALLINT numcols;

    // Check SQLNumResultCols
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numcols));
    ASSERT_EQ(11, numcols);

    //Check that we can call SQLColAttributes on all columns
    ColAttributes attrs  = colAttributes(1);
    attrs  = colAttributes(2);
    attrs  = colAttributes(3);
    attrs  = colAttributes(4);
    attrs  = colAttributes(5);
    attrs  = colAttributes(6);
    attrs  = colAttributes(7);
    attrs  = colAttributes(8);
    attrs  = colAttributes(9);
    attrs  = colAttributes(10);
    attrs  = colAttributes(11);

    execDirect("create table pstable(i integer)");
    //insert
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"insert into pstable values (1)", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numcols));
    ASSERT_EQ(0, numcols);
    attrs  = colAttributes(1);


    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"insert into pstable values (?)", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numcols));
    ASSERT_EQ(0, numcols);

    //update
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"update pstable set i = ? where i = ?", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numcols));
    ASSERT_EQ(0, numcols);

    //delete
    EXPECT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"delete from pstable where i = ?", SQL_NTS));
    EXPECT_EQ(SQL_SUCCESS, SQLNumResultCols(stmt, &numcols));
    ASSERT_EQ(0, numcols);
}
