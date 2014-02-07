#include "stdafx.h"
#include "scope.h"
#include "variable.h"
#include <map>
#include <memory>
#include <vector>
namespace lang
{

langObject BuidInFunction(std::string name,std::vector<langObject> arg)
{
    if(name=="print")
    {
        for (auto var : arg)
        {
            if(var != nullptr)std::cout<<var->toString(); else std::cout<< "null";
        }
        std::cout<<"\t";
    }
    return 0;
}
scope::scope(std::vector<parseObj*> v)
{
    this->parsers = v;
    this->index = 0;
    this->startIndex = 0;
}
scope::scope(std::vector<parseObj*> v,scope* parent)
{
    this->variable.parentVariable = &parent->variable;//this->variable = new lang::variable(parent->variable);
    this->parsers = v;
    this->index = 0;
    this->startIndex = 0;
}


scope::~scope(void)
{
}
    int scope::parentSkip(int index)
    {
    int j=0;
        while(this->parsers.size()>index)
        {
            index++;
            switch (this->parsers[index]->pEnum)
            {
                case parserEnum::leftparent:
                    j++;
                break;
                case parserEnum::rightparent:
                if(j==0)
                    return index;
                    j--;
                break;
            }
        }
        return index;
    }
    int scope::blockSkip(int index)
    {
    int j=0;
        while(this->parsers.size()>index)
        {
            index++;
            switch (this->parsers[index]->pEnum)
            {
                case parserEnum::blockstart:
                    j++;
                break;
                case parserEnum::blockend:
                if(j==0)
                    return index;
                    j--;
                break;
            }
        }
        return index;
    }
void scope::run(void)
{
    this->index = this->startIndex;
    auto status = en::scopeStatus::none;
    scope* forscope;
    int forindex[3],findex = -1;
    forindex[0]=-1;forindex[1]=-1;forindex[2]=-1;
    for(this->index = this->startIndex;index < parsers.size();index++)
    {
        auto j = this->parsers[index];
        switch (status)
        {
        case en::none:
            switch (j->pEnum)
            {
                case parserEnum::identifier:
                    if(*j->name == "for")
                    {
                        status = en::_for;
                    }else if(*j->name == "if")
                    {
                        status = en::_if;
                    }else
                    status = en::iden;
                break;
            case parserEnum::blockend:
                
                return;
            case parserEnum::blockstart:
                    auto sc = new scope(this->parsers,this);
                    sc->startIndex = this->index + 1;
                    sc->run();
                    this->index = sc->index;
                    delete sc;
                break;
            }
            break;
        case en::iden:
            switch (j->pEnum)
            {
                case parserEnum::identifier:
                    status = en::var;
                break;
                default:
                    int i=index-1;
                    langObject buf = eval(this->parsers[this->index-1]->ptr,i);
                    index=i;status = en::none;
                    //if(buf!=nullptr)std::cout<<"result:"<<(buf)->toString()<<std::endl;
                break;
            }
        break;
        case en::var:
            switch (j->pEnum)
            {
                case parserEnum::equal:
                    int i=index-1;
                    this->variable.add(*this->parsers[this->index-1]->name,NULLOBJECT);
                    //this->variable[*this->parsers[this->index-1]->name]=
                    eval(this->parsers[this->index-1]->ptr,i);
                    status = en::none;
                break;
                }
        break;
        #ifndef forΜ
            case en::_for:
                switch (findex)
                {
                case 4:
                case 3:
                    switch (j->pEnum)
                    {
                        case parserEnum::blockstart:
                        if(findex == 3)
                        {
                            forscope = new scope(this->parsers,this);
                            forscope->startIndex = this->index + 1;
                            forscope->index = forindex[0];
                            forscope->eval(NULLOBJECT,forscope->index);
                        }
                        forscope->index = forindex[1];
                        if(Int::toInt(forscope->eval(NULLOBJECT,forscope->index)))
                        {
                            forscope->run();
                            this->index--;//this->index = forscope->index;
                            findex = 4;
                            forscope->index = forindex[2];
                            forscope->eval(NULLOBJECT,forscope->index);
                        }
                        else
                        {
                            this->index = this->blockSkip(this->index);
                            status = en::none;
                            delete forscope;
                        }
                        break;
                    }
                break;
                case 2:
                    switch (j->pEnum)
                    {
                    case semicolon:
                        forindex[2] = index + 1;
                        findex = 3;
                        break;
                    }
                break;
                case 1:
                    switch (j->pEnum)
                    {
                    case semicolon:
                        forindex[1] = index + 1;
                        findex = 2;
                        break;
                    }
                break;
                case -1:
                    if(j->pEnum==parserEnum::leftparent)
                    {
                        forindex[0] = index+1;
                        findex = 1;
                    }
                    else throw "forΜγΙΝ(ͺKv";
                    break;
                }
            break;
        #endif
        case en::_if:
            langObject buf = eval(this->parsers[this->index]->ptr,this->index);
            if(Int::toInt(buf))
            {
                status = en::none;
            }
            else
            {
                this->index = this->blockSkip(this->index + 1);
                status = en::none;
            }
        break;
        }
    }
    //auto buf = eval(this->parsers[0]->ptr,this->index);
    //if(buf!=nullptr)std::cout<<"result:"<<(buf)->toString()<<std::endl;
    //delete buf;
}
#if 1
        const int EqualPrece = 16;
        const int PlusPrece = 6;
        const int MinusPrece = 6;
        const int GreaterPrece = 8;
        const int GreaterEqualPrece = 8;
        const int LessPrece = 8;
        const int LessEqualPrece = 8;
        const int EqualEqualPrece = 9;
        const int MultiplyPrece = 5;
        const int ModuloPrece = 5;
        const int DivisionPrece = 5;
        const int PlusEqualPrece = 16;
        const int DotEqualPrece = 1;
        const int ArrayPrece = 2;//2;
        const int IncrementPrece = 2;
        const int PointerPrece = 3;
        int Operator(parserEnum op)
        {
            switch (op)
            {
                case parserEnum::dot:
                    return 1;
                case parserEnum::leftbracket:
                    return ArrayPrece;
                case parserEnum::plusplus:
                case parserEnum::minusminus:
                    return IncrementPrece;
                case parserEnum::modulo:
                case parserEnum::multiply:
                case parserEnum::division:
                    return 5;
                case parserEnum::minus:
                case parserEnum::plus:
                    return 6;
                case parserEnum::leftshift:
                case parserEnum::rightshift:
                    return 7;
                case parserEnum::greater:
                case parserEnum::greaterequal:
                case parserEnum::less:
                case parserEnum::lessequal:
                    return 8;
                case parserEnum::equalequal:
                case parserEnum::notequal:
                    return 9;
                case parserEnum::and:
                    return 10;
                case parserEnum::xor:
                    return 11;
                case parserEnum::or:
                    return 12;
                case parserEnum::andand:
                    return 13;
                case parserEnum::oror:
                    return 14;
                case parserEnum::equal://=    _
                case parserEnum::plusequal://+= _
                case parserEnum::minusequal://-   _
                case parserEnum::divisionequal:// /=_
                case parserEnum::leftshiftequal://<<= _
                case parserEnum::rightshiftequal://>>= /
                case parserEnum::multiplyequal: //*=  /
                case parserEnum::moduloequal:  //%=  /
                case parserEnum::andequal:    //&=  /
                case parserEnum::xorequal:   //^=  /
                case parserEnum::orequal:   //|=  /
                    return 16;
                default:
                    return 0;
            }
        }
#endif
std::shared_ptr<Object> scope::eval(std::shared_ptr<Object> object,int& index,int opera,bool isbinaryoperation)
{
    //int index = object->index;
    int binaryoperation = index + 1;
    if(!isbinaryoperation)
    {
        int i;
        switch (this->parsers[index]->pEnum)
        {
            case parserEnum::identifier:
                if(this->parsers.size()>binaryoperation&&this->parsers[binaryoperation]->pEnum==leftparent)
                {
                    i = this->parentSkip(binaryoperation);
                    std::vector<langObject> arg;
                    index = index + 2;
                    while (index<i)
                    {
                        arg.push_back(eval(NULLOBJECT,index,17));
                        index++;
                        if(this->parsers[index]->pEnum==parserEnum::comma)index++;
                    }
                    BuidInFunction(*this->parsers[binaryoperation - 1]->name,arg);
                    //object = 
                    index = i;
                    binaryoperation = index + 1;

                }
                else
                object = this->variable[*this->parsers[index]->name];
            break;
            case parserEnum::leftparent:
                i = this->parentSkip(index);
                object = eval(NULLOBJECT,binaryoperation,17);
                index = i;
                binaryoperation = index + 1;
            break;
            case parserEnum::num:
            case parserEnum::str:
            case parserEnum::chr:
                object = this->parsers[index]->ptr;
            break;
        }
        if(this->parsers[index] != nullptr)
        {
            //delete object;
        }
    }
    if(this->parsers.size()>binaryoperation)
    {
        std::shared_ptr<Object> buf;
        int i=index+2;
        int thisop = Operator(this->parsers[binaryoperation]->pEnum);
    switch (this->parsers[binaryoperation]->pEnum)
    {
        case parserEnum::plus:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::plus(object,buf));
            index  = i;
            //delete buf;
        break;
        case parserEnum::multiply:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::multiply(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::modulo:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::modulo(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::greater:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::greater(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::less:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::less(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::greaterequal:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::greaterEqual(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::lessequal:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::lessEqual(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::equalequal:
            if (opera < thisop) break;
            buf = eval(object,i,thisop);
            object = (Object::equal(object,buf));
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::equal:
            if (opera < thisop) break;
            object = eval(object,i,thisop);
            this->variable.set(*this->parsers[index]->name,object);//this->variable[*this->parsers[index]->name] = object;
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
        case parserEnum::plusplus:
            if (opera < thisop) break;
            buf = std::make_shared<Int>(1);
            object = Object::plus(object,buf);
            this->variable.set(*this->parsers[index]->name,object);
            index = i;
            if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) > thisop) object = eval(object,i,17,true);
        break;
    }
    }
    return object;
}
}
