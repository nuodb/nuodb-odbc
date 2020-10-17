/**
 * (C) Copyright 2016-2020 NuoDB, Inc.  All Rights Reserved.
 *
 * This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
 * See the LICENSE file provided with this software.
 */

#include "ODBCTestBase.h"

#include <string>

class ODBCEscapeTestRequiresChorus : public ODBCTestBase
{
    void SetUp() override
    {
        ODBCTestBase::SetUp();
        execDirect("DROP TABLE IF EXISTS escape1");
        execDirect("CREATE TABLE escape1(f1 INT, f2 STRING)");
        ASSERT_EQ(SQL_SUCCESS, SQLPrepare(stmt, (SQLCHAR*)"INSERT INTO escape1 VALUES (?, ?)",
                                          SQL_NTS));

        int intVal;
        SQLLEN intInd = 0;
        char charVal[32];
        SQLLEN charInd = 0;

        for (int i = 0; i < 5; i++) {
            intVal = i;
            sprintf(charVal, "%d", i);

            ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                                                    1,
                                                    SQL_PARAM_INPUT,
                                                    SQL_C_LONG,
                                                    SQL_INTEGER,
                                                    0,
                                                    0,
                                                    &intVal,
                                                    0,
                                                    &intInd));

            ASSERT_EQ(SQL_SUCCESS, SQLBindParameter(stmt,
                                                    2,
                                                    SQL_PARAM_INPUT,
                                                    SQL_C_CHAR,
                                                    SQL_C_CHAR,
                                                    32,
                                                    0,
                                                    charVal,
                                                    32,
                                                    &charInd));
            ASSERT_EQ(SQL_SUCCESS, SQLExecute(stmt));
        }
        freeStmt();

        // This is the default, but the Java test sets it explicitly.
        ASSERT_EQ(SQL_SUCCESS, SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0));
    }
};

TEST_F(ODBCEscapeTestRequiresChorus, Timestamp)
{
    execDirectAndFetch("select {ts '1/1/12 14:30'} from dual");

    bool nullValue;
    SQL_TIMESTAMP_STRUCT ts1 = getTimestamp(1, nullValue);
    ASSERT_FALSE(nullValue);
    freeStmt();

    execDirectAndFetch("select cast('1/1/12 14:30' as timestamp) from dual");
    SQL_TIMESTAMP_STRUCT ts2 = getTimestamp(1, nullValue);
    ASSERT_FALSE(nullValue);
    freeStmt();
    ASSERT_EQ(ts1, ts2);
}


TEST_F(ODBCEscapeTestRequiresChorus, Date)
{
    execDirectAndFetch("select {d '2012-01-01'} from dual");

    bool nullValue;
    SQL_DATE_STRUCT d1 = getDate(1, nullValue);
    ASSERT_FALSE(nullValue);
    freeStmt();

    execDirectAndFetch("select cast('1/1/12' as date) from dual");

    SQL_DATE_STRUCT d2 = getDate(1, nullValue);
    ASSERT_FALSE(nullValue);
    freeStmt();

    ASSERT_EQ(d1, d2);
}

TEST_F(ODBCEscapeTestRequiresChorus, Time)
{
    execDirectAndFetch("select {t '12:01:01'} from dual");

    bool nullValue;
    SQL_TIME_STRUCT t1 = getTime(1, nullValue);
    ASSERT_FALSE(nullValue);

    freeStmt();

    execDirectAndFetch("select cast('12:01:01' as time) from dual");

    SQL_TIME_STRUCT t2 = getTime(1, nullValue);
    ASSERT_FALSE(nullValue);

    freeStmt();
    ASSERT_EQ(t1, t2);
}

TEST_F(ODBCEscapeTestRequiresChorus, OtherEscapes)
{
    execDirectAndFetch("select 'found' from dual where 'abc' like 'ab#c' {escape '#'}");
    bool nullValue;
    ASSERT_STREQ("found", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn abs(-12)} from dual");
    ASSERT_EQ(12, getIntData(1));
    freeStmt();
    execDirectAndFetch("select count(*) from {oj escape1 a left outer join escape1 t2 on a.f1 != t2.f1}");
    int count1 = getIntData(1);
    freeStmt();

    execDirectAndFetch("select count(*) from escape1 a left outer join escape1 t2 on a.f1 != t2.f1");
    int count2 = getIntData(1);
    freeStmt();

    ASSERT_EQ(count1, count2);
}

TEST_F(ODBCEscapeTestRequiresChorus, NumericFunctions)
{
    bool nullValue;

    execDirectAndFetch("select {fn abs(-12)} from dual");
    EXPECT_EQ(12, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn acos(1)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn asin(1)} from dual");
    double val = getDoubleData(1, nullValue);
    EXPECT_TRUE(val > 1.5);
    freeStmt();

    execDirectAndFetch("select {fn atan(0)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn atan2(0,0)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn ceiling(3.4)} from dual");
    EXPECT_EQ(4, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn cos(0)} from dual");
    EXPECT_EQ(1, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn cot(1)} from dual");
    double d = getDoubleData(1, nullValue);
    EXPECT_TRUE(d > 0 && d < 1);
    freeStmt();

    execDirectAndFetch("select {fn degrees(0)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn floor(2.13)} from dual");
    EXPECT_EQ(2, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn mod(10,3)} from dual");
    EXPECT_EQ(1, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn pi()} from dual");
    EXPECT_EQ(3, (int)getDoubleData(1, nullValue));
    freeStmt();

    execDirectAndFetch("select {fn power(2,3)} from dual");
    EXPECT_EQ(8, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn radians(180)} from dual");
    EXPECT_EQ(3, (int)getDoubleData(1, nullValue));
    freeStmt();

    execDirectAndFetch("select {fn rand()} from dual");
    getDoubleData(1, nullValue);
    EXPECT_FALSE(nullValue);
    freeStmt();


    execDirectAndFetch("select {fn rand(1000)} from dual");
    getDoubleData(1, nullValue);
    EXPECT_FALSE(nullValue);
    freeStmt();

    execDirectAndFetch("select {fn round(2.5)} from dual");
    EXPECT_EQ(3, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn sin(0)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn sqrt(4)} from dual");
    EXPECT_EQ(2, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn tan(0)} from dual");
    EXPECT_EQ(0, getIntData(1));
    freeStmt();
}

TEST_F(ODBCEscapeTestRequiresChorus, StringFunctions)
{
    bool nullValue;

    execDirectAndFetch("select {fn concat('abc','def')} from dual");
    ASSERT_STREQ("abcdef", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn lcase('ABC')} from dual");
    ASSERT_STREQ("abc", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn left('abcdef',2)} from dual");
    ASSERT_STREQ("ab", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn length('ABC')} from dual");
    ASSERT_EQ(3, getIntData(1, nullValue));
    freeStmt();

    execDirectAndFetch("select {fn locate('def','abcdef')} from dual");
    ASSERT_EQ(4, getIntData(1, nullValue));
    freeStmt();

    execDirectAndFetch("select {fn locate('def','defabcdef',4)} from dual");
    ASSERT_EQ(7, getIntData(1, nullValue));
    freeStmt();

    execDirectAndFetch("select {fn ltrim('   def')} from dual");
    ASSERT_STREQ("def", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn right('abcdef',2)} from dual");
    ASSERT_STREQ("ef", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn rtrim('abc   ')} from dual");
    ASSERT_STREQ("abc", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn substring('abcdef',3,2)} from dual");
    ASSERT_STREQ("cd", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn ucase('abcdef')} from dual");
    ASSERT_STREQ("ABCDEF", getCharData(1, nullValue).c_str());
    freeStmt();
}

TEST_F(ODBCEscapeTestRequiresChorus, DateTimeFunctions)
{
    execDirectAndFetch("select {fn hour('10:42:36')} from dual");
    ASSERT_EQ(10, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn minute('10:42:36')} from dual");
    ASSERT_EQ(42, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn month('September 30, 1964')} from dual");
    ASSERT_EQ(9, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn now()} from dual");
    freeStmt();

    execDirectAndFetch("select {fn second('10:42:36')} from dual");
    ASSERT_EQ(36, getIntData(1));
    freeStmt();

    execDirectAndFetch("select {fn year('September 30, 1964')} from dual");
    ASSERT_EQ(1964, getIntData(1));
    freeStmt();
}

TEST_F(ODBCEscapeTestRequiresChorus, SystemFunctions)
{
    bool nullValue;

    execDirectAndFetch("select {fn user()} from dual");
    ASSERT_TRUE(getCharData(1, nullValue).size() > 0);
    freeStmt();

    execDirectAndFetch("select {fn ifnull('abc','xyz')} from dual");
    ASSERT_STREQ("abc", getCharData(1, nullValue).c_str());
    freeStmt();

    execDirectAndFetch("select {fn ifnull(null,'xyz')} from dual");
    ASSERT_STREQ("xyz", getCharData(1, nullValue).c_str());
    freeStmt();
}
