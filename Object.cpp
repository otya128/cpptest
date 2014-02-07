#pragma once
#include "stdafx.h"
#include "Object.h"
//#include "parserEnum.h"
#include <sstream>
#include <memory>
#define langObject std::shared_ptr<Object>
#define langInt std::shared_ptr<Int>
#define langString std::shared_ptr<String>

namespace lang
{
Object::Object(void* ptr)
{
    this->type = new Type(PreType::_Object);
	this->ptr=ptr;
}
Object::Object(void)
{
}

std::string _toString(Object* arg)
{
   // return arg->toString();for(i=0;i<10;i++){print(i);}
   if(arg==nullptr)return "null";
        if(arg->type->TypeEnum==PreType::_Int)
        {
            std::stringstream ss;
            ss<<(static_cast<Int*>(arg))->getInt();
            return (ss.str());
        }
        if(arg->type->TypeEnum==PreType::_String)
        {
        return *(static_cast<String*>(arg))->getString();
        }
}
#define GABEKORE 1
Object::~Object(void)
{
#if GABEKORE
    std::cout<<"がべこれ中..."<<this<<"\t"<<this->type->name<<"\t"<<_toString(this)<<std::endl;
#endif
    delete this->type;
	delete this->ptr;
}
void* Object::getPointer(void)
{
    return this->ptr;
}
void Object::setPointer(void* ptr)
{
    this->ptr = ptr;
    return;
}
std::string Object::toString()
{
    return this->type->name;
}
Int::Int(int i)
{
    this->type = new Type(PreType::_Int);
	this->ptr = (void*)new int(i);
}
Int::~Int(void)
{
    //delete this->type;
	//delete this->ptr;
}
int Int::getInt(void)
{
    return *(int*)(this->ptr);
}
void Int::setInt(int i)
{
    delete this->ptr;
    this->ptr = new int(i);
    return;
}
std::string Int::toString()
{
    std::stringstream ss;
    ss<<this->getInt();
    return (ss.str());
}

String::String(std::string* i)
{
    this->type = new Type(PreType::_String);
	this->ptr = (void*)new std::string(*i);
    
}
String::~String(void)
{
    //delete this->type;
	//delete this->ptr;
}
std::string* String::getString(void)
{
    return (std::string*)(this->ptr);
}
void String::setString(std::string* i)
{
    delete this->ptr;
    this->ptr = (void*)i;
    return;
}
std::string String::toString(void)
{
    return *((std::string*)(this->ptr));
}
    int Int::toInt(std::shared_ptr<Object> obj)
    {
        if(obj->type->TypeEnum==PreType::_Int)
        return (static_cast<Int*>(obj.get()))->getInt();
        //if(obj->type->TypeEnum==PreType::_Int)
            //return (static_cast<Int*>(obj))->getInt();
        return 0;//変換不可
    }
    std::shared_ptr<Object> Object::plus(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) + Int::toInt(obj2));
            case _String:
                return std::make_shared<String>(&(obj1->toString() + obj2->toString()));
        }
        throw "出来ない";
        //変換不可
    }
    std::shared_ptr<Object> Object::multiply(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) * Int::toInt(obj2));
        }
        throw "出来ない";
        //変換不可
    }
    std::shared_ptr<Object> Object::greater(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) > Int::toInt(obj2));
        }
        throw "出来ない";
        //変換不可
    }
    std::shared_ptr<Object> Object::less(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) < Int::toInt(obj2));
        }
        throw "出来ない";
        //変換不可
    }
    std::shared_ptr<Object> Object::greaterEqual(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) >= Int::toInt(obj2));
        }
        throw "出来ない";
    }
    std::shared_ptr<Object> Object::lessEqual(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) <= Int::toInt(obj2));
        }
        throw "出来ない";
    }
    std::shared_ptr<Object> Object::equal(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) == Int::toInt(obj2));
        }
        throw "出来ない";
    }
    std::shared_ptr<Object> Object::modulo(std::shared_ptr<Object> obj1,std::shared_ptr<Object> obj2)
    {
        switch (obj1->type->TypeEnum)
        {
        case PreType::_Int:
            return std::make_shared<Int>(Int::toInt(obj1) % Int::toInt(obj2));
        }
        throw "出来ない";
    }
}
