#ifndef __LANG_PARSEOBJ_H__
#define __LANG_PARSEOBJ_H__
//#pragma once
#include "parserEnum.h"
#include "Object.h"
#include <memory>
namespace lang
{
    class parseObj
    {
    private:
    public:
        int sourcestartindex;
        int sourceendindex;
        langObject ptr;
        std::string* name;
        parserEnum pEnum;
        int getInt();
        std::string getString();
        double getDouble();
        char getChar();
        parseObj(parserEnum p,std::string* n,int i,int j);
        parseObj(parserEnum p,std::string* n,int i,int j,langObject obj);
        parseObj(int obj,int i,int j);
        parseObj(std::string obj,int i,int j);
        parseObj(double obj,int i,int j);
        parseObj(char obj,int i,int j);
        parseObj(wchar obj,int i,int j);
        ~parseObj(void);
        std::string toString(void);
        #if _DEBUG
        int line;
        #endif
    };
	//exp ? true-exp : false-exp   ;
	//               ^colonindex   ^endindex
	//expがfalseならgoto colon
	//expがtrueなら次の式を評価してgoto endindex
	class Conditional : Object
	{
	private:
		//void *__vfptr;
		int _isconditional;
		//Type* type;
	public:
		int colonindex;
		int endindex;
		Conditional(int, int);
		~Conditional();
		bool isconditional();
	};
	struct Catcherc
	{
		std::string *type;
		std::string *varname;
		int index;
	};
	class Catcher : public Object
	{
	private:
		//void *__vfptr;
		int _iscatcher;
		//Type* type;
	public:
		std::vector<Catcherc> Catchers;
		Catcher();
		~Catcher();
		bool iscatcher();
		void Add(std::string *type, std::string *varname, int index);
	};

}
#endif
