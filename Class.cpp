#include "stdafx.h"
#include "Class.h"
#include "scope.h"
#include "lang.h"
#include "Function.h"
#include "langException.h"
namespace lang
{
    Class::Class(std::string name,int index,membertype member,lang::scope* scope,membertype staticmember)
    {
        this->scope = scope;
        this->scope->refinc();
        this->type = new Type(PreType::_Class, (char*)"class");
        this->name = name;
        this->index = index;
        this->member = member;
        this->finalize = nullptr;
#if CPP11
        for(auto i : *member)
        {
#else
        for( auto it = member->begin(); it != member->end(); ++it )
        {
            auto i = *it;
#endif
            if(i.first == "finalize")
            {
                if(i.second is _Function)
                {
                    this->finalize = (langFunction)i.second;
                    break;
                }
            }
        }

        thisscope = new lang::scope(this->scope->parsers, this->scope, nullptr);
        this->thisscope->refinc();
FOREACH(i,*staticmember)//        foreach_(var_ i in_ *staticmember)//C# style foreach
//        {
            if(i.second->type->TypeEnum == _Function)
            {
                auto buf = new Function((langFunction)i.second,this->thisscope);
                buf->scope = thisscope;
                this->thisscope->variable.add(i.first, buf);
            }
            else
                this->thisscope->variable.add(i.first, i.second);
        }
        /*
        if(this->finalize != nullptr)
        {
        this->finalize = new Function((langFunction)this->finalize,this->thisscope);
        this->finalize->scope = thisscope;
        this->thisscope->variable.add("finalize", this->finalize);
        lang::gc->uncontroll(this->finalize);
        }*/
        delete staticmember;
    }
    Class::Class(Class* clas)
    {
        this->type = new Type(PreType::_Class, (char*)"class");
        this->name  = clas->name;
        this->index = clas->index;
        this->member= clas->member;
        this->scope = clas->scope;
        this->finalize = nullptr;
    }
    std::string Class::toString()
    {
        return "Class:" + this->name;
    }
    Class::~Class(void)
    {
        if(this->type->TypeEnum == PreType::_Class)
        {delete this->member;
        this->scope->refdec();}//delete this->name;
#if _DEBUG
        if(lang::gc_view)std::cout<<"remove"<<this<<this->type->name<<std::endl;
#endif
        this->thisscope->refdec();
        //delete this->type->name;
    }
    ClassObject::ClassObject(Class* type) : Class(type) , staticClass(type)//type->name,type->index,type->member,type->scope)
    {
        this->scope = type->scope;
        thisscope = new lang::scope(this->scope->parsers, this->scope,this);
        this->thisscope->refinc();
        this->type->TypeEnum = _ClassObject;
        FOREACH(i,*member)//foreach_(var_ i in_ *this->member)//C# style foreach
        //{
            if(i.second->type->TypeEnum == _Function)
            {
                auto buf = new Function((langFunction)i.second,this->thisscope);
                buf->scope = thisscope;
                this->thisscope->variable.add(i.first, buf);
            }
            else
                this->thisscope->variable.add(i.first, i.second);
        }
        if(type->finalize != nullptr)
        {
            this->finalize = new Function((langFunction)type->finalize,this->thisscope);
            this->finalize->scope = thisscope;
            this->thisscope->variable.add("finalize", this->finalize);
            lang::gc->uncontroll(this->finalize);
        }

    }

    std::string ClassObject::toString(void)
    {
        if(this->thisscope->variable["ToString"] is _Function)
        {
            auto arg = std::vector<langObject>();//普通にローカル変数のポインタ私で良かった
            auto result = ((langFunction)this->thisscope->variable["ToString"])->call(&arg);
            return result->toString();
            /*
            auto arg = new std::vector<langObject>;//例外処理で逆に冗長になる例
            try
            {
            auto result = ((langFunction)this->thisscope->variable["ToString"])->call(arg);
            delete arg;
            return result->toString();
            }
            catch(...)
            {
            delete arg;
            throw;
            }*/
        }
        return "ClassObject:" + this->name;//Class::toString();
    }
    ClassObject::~ClassObject(void)
    {
        if(this->finalize)//もう終了してたらファイナライザは呼ばれない
        {
            if(lang::running)
            {
                std::vector<langObject> arg;
                this->finalize->call(&arg);
            }
            delete this->finalize;
        }
        //this->scope->refdec();//scopeじゃなくてthisscopeでは
    }
}
