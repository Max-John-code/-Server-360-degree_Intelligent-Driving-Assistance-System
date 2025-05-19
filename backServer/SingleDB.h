#pragma once
#include<cstring>
#include<stdio.h>
#include <iostream>
#include <sqlite3.h>

using namespace std;
class SingleDB
{
public:
    static SingleDB* getInstance();                                 //外部接口--获取该类唯一实例
    /** 查询
     * @brief doSelect
     * @param sql       执行的sql
     * @param qres      结果集引用
     * @param row       数据行引用
     * @param col       数据列引用
     * @return  0--sql执行成功 else 执行失败
     */
    int doSelect(char* sqlCode, char**& result, int& row, int& col); //执行sql语句   返回1-sql语句执行成功   返回0-sql语句执行失败
    /**
     * @brief doupdateInsertDel
     * @param sql   执行的sql
     * @return 0--sql执行成功 else 执行失败
     */
    int doupdateInsertDel(string sql);

private:
    SingleDB();                        //私有构造函数--保证该类只有一个实例
    ~SingleDB();

    static SingleDB* dataBase;         //该类唯一实例
    sqlite3* pdb;                    //数据库指针
};

