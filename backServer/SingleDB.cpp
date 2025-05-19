#include "SingleDB.h"

//静态成员定义
SingleDB* SingleDB::dataBase = NULL;
SingleDB* SingleDB::getInstance()
{
    if (nullptr == SingleDB::dataBase)
    {
        SingleDB::dataBase = new SingleDB();
    }
    return SingleDB::dataBase;
}

int SingleDB::doSelect(char* sqlCode, char**& result, int& row, int& col)
{
    char* errmsg = nullptr;
    int res = sqlite3_get_table(this->pdb, sqlCode, &result, &row, &col, &errmsg);
    if (res == SQLITE_OK)
    {
        cout << "do sql success";
    }
    else {
        cout << sqlite3_errcode(this->pdb);
        cout << sqlite3_errmsg(this->pdb);
    }
    //成功为0,不成功返回小于0的值
    return res;
}

int SingleDB::doupdateInsertDel(string sql)
{
    char* errmsg = nullptr;
    int res = sqlite3_exec(this->pdb, sql.c_str(), nullptr, nullptr, &errmsg);
    if (res == SQLITE_OK)
    {
        cout << "do sql success";     //sql语法没有问题
    }
    else {
        cout << sqlite3_errcode(this->pdb);
        cout << sqlite3_errmsg(this->pdb);
    }
    return res;
}

SingleDB::SingleDB()
{
    //1.数据库不存在则创建,存在就打开
    int res = sqlite3_open("server.db", &pdb);
    if (res == SQLITE_OK)
    {
        cout<< "打开数据库成功--新建"<<endl;
    }
    else {
        cout << "数据库失败--新建"<<endl;
    }
    
}

SingleDB::~SingleDB()
{
    sqlite3_close(this->pdb);
    this->pdb = nullptr;      //避免野指针
}
