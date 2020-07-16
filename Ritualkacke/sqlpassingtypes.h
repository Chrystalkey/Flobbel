//
// Created by Chrystalkey on 13.07.2020.
//

#ifndef FLOBBEL_SQLPASSINGTYPES_H
#define FLOBBEL_SQLPASSINGTYPES_H

enum SQLPassingType{
    Integer,
    String,
    Invalid
};

class sql_basetype{
public:
    sql_basetype() = default;
    const SQLPassingType type() const{return _type;}
protected:
    SQLPassingType _type = Invalid;
};

class sql_str: public sql_basetype {
public:
    sql_str(const std::wstring& str): str(str){ _type = String;}
    operator const std::wstring&()const{return str;}
    operator const wchar_t*()const{return str.c_str();}

private:
    std::wstring str;
};
class sql_int: public sql_basetype{
public:
    sql_int(int64_t num):num(num){_type = Integer;}
    operator const int64_t()const{return num;}
private:
    int64_t num;
};

#endif //FLOBBEL_SQLPASSINGTYPES_H
