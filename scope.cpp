﻿#pragma once
#include "stdafx.h"
#include "scope.h"
//#include "variable.h"
//#include <tchar.h>
#include <map>
#include <memory>
#include <vector>
#ifdef CPP11
#include <thread>
#include <mutex>
#endif
#include <sstream>
#include <iosfwd>
#include "Function.h"
#include "GC.h"
#include "langException.h"
#include "lang.h"
#include "Class.h"
#include "Array.h"
#include "GTKOLWindow.h"
#include "OLR.h"
namespace lang
{//VisualStadio

	langObject BuitInFunction(std::string name, std::vector<langObject> arg)
	{
		auto func = (*BuiltFunction)[name];
		if (func)
		{
			return func(arg);
		}
		else
		{
			throw_langRuntimeException("関数%sが見つかりません。", name.c_str());
		}
	}
	void scope::refinc()
	{
		this->refcount++;
	}
	void scope::refdec()
	{
		this->refcount--;
		if (this->refcount <= 0)
		{
			if (this->variable.parentVariable != nullptr && gc->removeRoot(this))
				delete this;
		}
	}
	void scope::del()
	{
		if (this->refcount <= 0)
		{
			if (this->variable.parentVariable != nullptr && gc->removeRoot(this))
				delete this;
		}
	}
	scope::scope(std::vector<parseObj*>& v) :parsers(v)
	{
		parent = nullptr;
		this->isClass() = nullptr;
		refcount = 0;
		//this->variable = lang::variable();
		//this->parsers = v;
		this->index = 0;
		this->startIndex = 0;
		this->status = en::returnStatus::none_;
		this->type = en::scopeType::_none_;
#if _DEBUG
		if (gc_view)
			std::cout << "変数スコープを作成" << this << std::endl;
#endif
	}
	scope::scope(std::vector<parseObj*>& v, scope* parent, langClassObject _this) :parsers(v)
	{
		this->parent = parent;
		this->isClass() = isClass();
		gc->addRoot(this);//gc->roots[this] = 0;
		refcount = 0;
		this->variable.parentVariable = &parent->variable;//this->variable = new lang::variable(parent->variable);
		//this->parsers = v;
		this->index = 0;
		this->startIndex = 0;
		this->status = en::returnStatus::none_;
		this->type = en::scopeType::_none_;
#if _DEBUG
		if (gc_view)
			std::cout << "変数スコープを作成" << this << std::endl;
#endif
	}


	scope::~scope(void)
	{
		gc->removeRoot(this);//gc->roots.erase(this);
#if _DEBUG
		if (gc_view)
			std::cout << "変数スコープを廃棄" << this << std::endl;
#endif
	}
	int scope::parentSkip(int index)
	{
		int j = 0;
		while (this->parsers.size() - 1 > index)
		{
			index++;
			switch (this->parsers[index]->pEnum)
			{
				case parserEnum::leftparent:
					j++;
					break;
				case parserEnum::rightparent:
					if (j == 0)
						return index;
					j--;
					break;
			}
		}
		return index;
	}
	int scope::bracketSkip(int index)
	{
		int j = 0;
		while (this->parsers.size() - 1 > index)
		{
			index++;
			switch (this->parsers[index]->pEnum)
			{
				case parserEnum::leftbracket:
					j++;
					break;
				case parserEnum::rightbracket:
					if (j == 0)
						return index;
					j--;
					break;
			}
		}
		return index;
	}
	int scope::blockSkip(int index, int j)
	{
		while (this->parsers.size() - 1 > index)
		{
			index++;
			switch (this->parsers[index]->pEnum)
			{
				case parserEnum::blockstart:
					j++;
					break;
				case parserEnum::blockend:
					if (j == 0)
						return index;
					j--;
					break;
			}
		}
		return index;
	}
	//変数宣言と式しか実行できない文
	void scope::statement(void)
	{
		auto status = en::scopeStatus::none;
		for (/*this->index = this->startIndex*/; index < parsers.size(); index++)
		{
			auto j = this->parsers[index];
			if (j->pEnum == parserEnum::semicolon) break;
			switch (status)
			{
				case en::none:
					switch (j->pEnum)
					{
						case parserEnum::_new:
							eval(this->parsers[this->index]->ptr, this->index);
							break;
						case parserEnum::identifier:
							status = en::iden;
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
							int i = index - 1;
							langObject buf = eval(this->parsers[this->index - 1]->ptr, i);
							index = i; status = en::none;
							break;
					}
					break;
				case en::var:
					switch (j->pEnum)
					{
						case parserEnum::equal:
						{
												  int i = index - 1;
												  this->variable.add(*this->parsers[this->index - 1]->name, NULLOBJECT);
												  //this->variable[*this->parsers[this->index-1]->name]=
												  eval(this->parsers[this->index - 1]->ptr, i);
												  status = en::none; index = i;
						}

							break;
						case parserEnum::leftparent:
							index = this->parentSkip(index);
							status = en::none;
							break;
					}
			}
		}
	}
#define LANG_SCOPE_RUN_RET return result ? result : NULLOBJECT
	langObject scope::run(void)
	{
		langObject result = nullptr;
		this->index = this->startIndex;
		auto status = en::scopeStatus::none;
		/*std::shared_ptr<scope>*/scope* forscope = 0;
		scope* whilescope = 0;
		scope* foreachscope = 0;
		int forindex[3] = { -1, -1, -1 }, findex = -1, whileindex = -1, whileflag = 0,
			foreahflag = 0;
		std::string* foreach_var = 0;
		langObject foreach_object = 0;
		langFunction Current = 0, MoveNext = 0;
		try
		{
			for (this->index = this->startIndex; index < parsers.size(); index++)
			{
				auto j = this->parsers[index];
				switch (status)
				{
					case en::none:
						switch (j->pEnum)
						{
							case _class:
								if (this->parsers[this->index]->ptr)
								{
									result = eval(this->parsers[this->index]->ptr, this->index);
								}
								else
								index = this->blockSkip(index, -1);
								break;
							case parserEnum::num:
							case parserEnum::chr:
							case parserEnum::str:
							case parserEnum::_this:
							case parserEnum::base:
							case parserEnum::_new:
								result = eval(this->parsers[this->index]->ptr, this->index);
								break;
							case parserEnum::identifier:
								/*if(*j->name == "for")
								{
								status = en::_for;
								}else if(*j->name == "if")
								{
								status = en::_if;
								}
								else if(*j->name == "return")
								{
								this->status = en::returnStatus::_return;
								index++;
								auto buf = eval(NULLOBJECT,index);

								return /*langObject*//*(buf);
								}
								else*/
								status = en::iden;
								break;
							case parserEnum::_if:
								status = en::_if;
								break;
							case parserEnum::_for:
								status = en::_for;
								break;
							case parserEnum::_while:
								status = en::_while;
								break;
							case parserEnum::_foreach:
								status = en::_foreach;
								foreahflag = 0;
								break;
							case parserEnum::_return:
								//こう自動成形されるのはバグ？
							{
														this->status = en::returnStatus::_return;
														index++;
														auto buf = eval(NULLOBJECT, index);
														return /*langObject*/(buf);
							}
								break;
							case parserEnum::_break:
								this->status = en::_break;
								LANG_SCOPE_RUN_RET;
							case parserEnum::_continue:
								this->status = en::_continue;
								LANG_SCOPE_RUN_RET;
							case parserEnum::blockend:
								if (this->type == en::ctor)return this->_this;
								/*if (result)
								{
								auto parent = this->parent;
								while (parent)
								{
								if (parent->type == en::_function)
								{
								this->status = en::_return;
								return result;
								}
								parent = parent->parent;
								}
								}*/
								LANG_SCOPE_RUN_RET;
							case parserEnum::semicolon:
								break;
							case parserEnum::blockstart:
							{
														   auto sc = /*std::make_shared<scope>*/new scope(this->parsers, this, this->_this);
														   sc->refinc();
														   sc->startIndex = this->index + 1;
														   auto buf = sc->run();
														   this->index = sc->index;
														   if (sc->status != en::returnStatus::none_)
														   {
															   this->status = sc->status;
															   sc->refdec();
															   //delete sc;
															   return /*langObject*/(buf);
														   }
														   //delete sc;
														   sc->refdec();
							}
								break;
							default:
								result = eval(this->parsers[this->index]->ptr, this->index);
								break;
							case none:
								break;
						}
						break;
					case en::iden:
						switch (j->pEnum)
						{
							case parserEnum::identifier:
								if (*this->parsers[this->index - 1]->name == "function")
								{
									int i = index - 1;
									result = eval(this->parsers[this->index - 1]->ptr, i);
									index = i; status = en::none;
									break;
								}
								status = en::var;
								break;
							default:
								int i = index - 1;
								result = eval(this->parsers[this->index - 1]->ptr, i);
								index = i; status = en::none;
								//if(buf!=nullptr)std::cout<<"result:"<<(buf)->toString()<<std::endl;
								break;
						}
						break;
					case en::var:
						switch (j->pEnum)
						{
							case parserEnum::equal:
							{
													  int i = index - 1;
													  this->variable.add(*this->parsers[this->index - 1]->name, NULLOBJECT);
													  //this->variable[*this->parsers[this->index-1]->name]=
													  result = eval(this->parsers[this->index - 1]->ptr, i);
													  status = en::none; index = i;
							}

								break;
							case parserEnum::leftparent:
								index = this->parentSkip(index);
								status = en::none;
								break;
							default:
							{
									   int i = index - 2;
									   result = eval(this->parsers[i]->ptr, i);
									   index = i; status = en::none;
									   //if(buf!=nullptr)std::cout<<"result:"<<(buf)->toString()<<std::endl;
							}
								break;
						}
						break;
					case en::_for:
						__v('ω')v___
#pragma region for文
							switch (findex)
						{
								case 4:
								case 3:
									switch (j->pEnum)
									{
										case parserEnum::blockstart:
											if (findex == 3)
											{
												forscope = /*std::make_shared<scope>*/new scope(this->parsers, this, this->_this);
												forscope->refinc();
												forscope->startIndex = this->index + 1;
												forscope->index = forindex[0];
												forscope->statement();//forscope->eval(NULLOBJECT, forscope->index);
											}
											forscope->index = forindex[1];
											//  for(;;)で無限ループにする
											if (forindex[1] + 1 == forindex[2] ||
												Int::toInt(forscope->eval(NULLOBJECT, forscope->index)))
											{
												auto buf = forscope->run();
												if (forscope->status == en::returnStatus::_return)
												{
													this->status = forscope->status;
													//delete forscope;
													forscope->refdec();
													return buf;
												}
												else
												if (forscope->status == en::returnStatus::_break)
												{
													this->index = this->blockSkip(this->index);
													status = en::none;
													findex = -1;
													forscope->refdec();
													continue;
												}
												else
												if (forscope->status != en::returnStatus::_continue && forscope->status != en::returnStatus::none_)
												{
													this->status = forscope->status;
													return buf;
												}
												this->index--;//this->index = forscope->index;
												findex = 4;
												forscope->index = forindex[2];
												forscope->eval(NULLOBJECT, forscope->index);
											}
											else
											{
												this->index = this->blockSkip(this->index);
												status = en::none;
												findex = -1;
												forscope->refdec();
												//delete forscope;
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
									if (j->pEnum == parserEnum::leftparent)
									{
										forindex[0] = index + 1;
										findex = 1;
									}
									else throw "forの後には(が必要";
									break;
						}
						break;
#pragma endregion
					case en::_if:
#pragma region if文 
					{
									langObject buf = eval(this->parsers[this->index]->ptr, this->index);
									if (Int::toInt(buf))
									{
										status = en::none;
										auto sc = new scope(this->parsers, this, this->_this);
										sc->refinc();
										sc->startIndex = this->index + 2;
										auto buf = sc->run();
										this->index = sc->index;
										if (sc->status != en::returnStatus::none_)
										{
											this->status = sc->status;
											sc->refdec();
											return buf;
										}
										sc->refdec();
										if (this->parsers[this->index + 1]->pEnum == parserEnum::_else)
										{
											this->index++;
											this->index = this->blockSkip(this->index + 1);
										}

									}
									else
									{
										this->index = this->blockSkip(this->index + 1);
										if (this->parsers[this->index + 1]->pEnum == parserEnum::_else)
										{
											this->index++;
										}
										status = en::none;
									}
					}
						break;
#pragma endregion
					case en::_while:
#pragma region while文
						switch (whileflag)
						{
							case 0:
								whileindex = this->index;
								whileflag = 1;
								break;
							case 1:
								if (j->pEnum == parserEnum::blockstart)
								{
									whilescope = new scope(this->parsers, this, this->_this);
									whilescope->startIndex = this->index + 1;
									whilescope->refinc();
									whilescope->index = whileindex;
									if (Int::toInt(whilescope->eval(NULLOBJECT, whilescope->index)))
									{
										auto buf = whilescope->run();
										if (whilescope->status == en::returnStatus::_return)
										{
											this->status = whilescope->status;
											whilescope->refdec();
											return buf;
										}
										else
										if (whilescope->status == en::returnStatus::_break)
										{
											this->index = this->blockSkip(this->index);
											status = en::none;
											whilescope->refdec();
											whileflag = 0;
											continue;
										}
										else
										if (whilescope->status != en::returnStatus::_continue && whilescope->status != en::returnStatus::none_)
										{
											this->status = whilescope->status;
											whilescope->refdec();
											return buf;
										}
										this->index--;
									}
									else
									{
										this->index = this->blockSkip(this->index);
										status = en::none;
										whilescope->refdec();
										whileflag = 0;
									}
								}
								break;
						}
#pragma endregion
						break;
#pragma region for each
					case en::_foreach:
						//酷いコードだ...
						//先読みでmatchしたほうが良かったかも
						//this->match(parserEnum::leftparent,parserEnum::identifier,...);
						switch (foreahflag)
						{
							case 0:
								if (this->parsers[index]->pEnum == parserEnum::leftparent)
								{
									foreahflag++;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 1:
								if (this->parsers[index]->pEnum == parserEnum::identifier)
								{
									foreahflag++;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 2:
								if (this->parsers[index]->pEnum == parserEnum::identifier)
								{
									foreahflag++;
									foreach_var = this->parsers[index]->name;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 3:
								if (this->parsers[index]->pEnum == parserEnum::_in)
								{
									foreahflag++;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 4:
								foreach_object = eval(NULLOBJECT, index);
								foreahflag++;
								break;
							case 5:
								if (this->parsers[index]->pEnum == parserEnum::rightparent)
								{
									foreahflag++;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 6:
								if (this->parsers[index]->pEnum == parserEnum::blockstart)
								{
									if (foreach_object is _ClassObject)
									{
										auto instance = (langClassObject)foreach_object;
										if (instance->thisscope->variable.definedVar("GetEnumerator"))
										{
											auto e = instance->thisscope->variable["GetEnumerator"];
											if (e is _Function)
											{
												auto arg = std::vector<langObject>();
												auto en = (langClassObject)((langFunction)e)->call(&arg);
												if (en is _ClassObject)
												{
													Current = (langFunction)(en->thisscope->variable["Current"]);
													MoveNext = (langFunction)(en->thisscope->variable["MoveNext"]);
													if (!(MoveNext is _Function))
													{
														throw langRuntimeException("MoveNextは関数である必要があります。");
													}
													if (!(Current is _Function))
													{
														throw langRuntimeException("Currentは関数である必要があります。");
													}
												}
												else
												{
													throw langRuntimeException("GetEnumeratorの返り値はClassObjectである必要があります。");
												}
											}
											else
											{
												throw langRuntimeException("GetEnumeratorは関数である必要があります。");
											}
										}
										else
										{
											throw langRuntimeException("foreachの対象となる変数はEnumerableである必要があります。");
										}
									}
									foreahflag++;
									foreachscope = new scope(this->parsers, this, this->_this);
									foreachscope->startIndex = this->index + 1;
									foreachscope->refinc();
									foreachscope->variable.add(*foreach_var, NULLOBJECT);
									goto _foreach_7;
								}
								else
								{
									throw langRuntimeException("構文エラーforeach");
								}
								break;
							case 7:
							_foreach_7 :
							{auto arg = std::vector<langObject>();
									   int result = Int::toInt(MoveNext->call(&arg));
									   if (result)
									   {
										   auto arg = std::vector<langObject>();
										   foreachscope->variable.add(*foreach_var, Current->call(&arg));
										   auto buf = foreachscope->run();
										   if (foreachscope->status == en::returnStatus::_return)
										   {
											   this->status = foreachscope->status;
											   foreachscope->refdec();
											   return buf;
										   }
										   else
										   if (foreachscope->status == en::returnStatus::_break)
										   {
											   this->index = this->blockSkip(this->index);
											   status = en::none;
											   foreachscope->refdec();
											   foreahflag = 0;
											   continue;
										   }
										   else
										   if (foreachscope->status != en::returnStatus::_continue && foreachscope->status != en::returnStatus::none_)
										   {
											   this->status = foreachscope->status;
											   foreachscope->refdec();
											   return buf;
										   }
										   this->index--;
									   }
									   else
									   {
										   this->index = this->blockSkip(this->index);
										   status = en::none;
										   foreachscope->refdec();
										   foreahflag = 0;
									   }
									   break;
							}
									   break;
						}
#pragma endregion
				}
			}
		}
		catch (langRuntimeException ex)
		{
			throw langRuntimeException(ex.what(), startIndex, index, this->parsers, ex.stacktrace, ex.funcstacktrace);
		}
		LANG_SCOPE_RUN_RET;
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
	int UnaryOperator(parserEnum op)
	{
		switch (op)
		{
			case parserEnum::minusminus:
			case parserEnum::plusplus:
			case parserEnum::notnot:
			case parserEnum::_not:
			case parserEnum::plus:
			case parserEnum::minus:
			case parserEnum::_and:
			case parserEnum::multiply:
			case parserEnum::_new:
				return 3;
		}
		return 0;
	}
	int Operator(parserEnum op)
	{
		switch (op)
		{
			case parserEnum::dot:
				return 1;
			case parserEnum::leftparent://return 17;
			case parserEnum::leftbracket:
				return ArrayPrece;
			case parserEnum::plusplus:
			case parserEnum::minusminus:
				return IncrementPrece;
			case parserEnum::_is:
			case parserEnum::_as:
				return 7;
			case parserEnum::modulo:
			case parserEnum::multiply:
			case parserEnum::division:
				return 5;
			case parserEnum::minus:
			case parserEnum::plus:
				return 6;
			case parserEnum::leftshift://<<
			case parserEnum::rightshift://>>
				return 7;
			case parserEnum::greater:
			case parserEnum::greaterequal:
			case parserEnum::less:
			case parserEnum::lessequal:
				return 8;
			case parserEnum::equalequal:
			case parserEnum::notequal:
				return 9;
			case parserEnum::_and:
				return 10;
			case parserEnum::_xor:
				return 11;
			case parserEnum::_or:
				return 12;
			case parserEnum::andand:
				return 13;
			case parserEnum::oror:
				return 14;
			case parserEnum::equal://=    ＼
			case parserEnum::plusequal://+= ＼
			case parserEnum::minusequal://-   ＼
			case parserEnum::divisionequal:// /=＼
			case parserEnum::leftshiftequal://<<= ＼
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
#define OP if (opera </*=*/ thisop) break
#define UOP if (unoopera </*=*/ thisop) break
#define OP2 i = index;if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) >= thisop) object = eval(object,i,17,evals::isbinaryoperation),index = i;
#define OP3 i=index;if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) >= thisop||this->parsers[index+1]->pEnum==leftparent) object = eval(object,i,17,evals::isbinaryoperation),index = i;//OP2
#define OP4 if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) >= /*thisop/*/1/*test*/||this->parsers[index+1]->pEnum==leftparent) object = eval(object,i,17,evals::isbinaryoperation),index = i;//OP2//i = index + 2;
#define OP5 i = index;if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) >= thisop||this->parsers[index+1]->pEnum==leftbracket) object = eval(object,i,17,evals::isbinaryoperation),index = i;//OP2//i = index + 2;
	langObject scope::eval(langObject object, int& index, int opera, evals ev, int unoopera)
	{
		//int index = object->index;
		int binaryoperation = index + 1;
		int i, j;
		bool isbinaryoperation = (bool)((int)ev & 1);
		langObject buf;
		if (!isbinaryoperation)
		{
			if (this->parsers[index]->ptr && this->parsers[index]->ptr->type->TypeEnum == _Function)
			{
				//ラムダ式だ
				langLambda l = (langLambda)this->parsers[index]->ptr;// new Lambda((langLambda)this->parsers[index]->ptr, this);
				if (l->is_lambda)
				{
					index = l->endindex ;
					binaryoperation = index + 1;
					object = new Lambda(l, this);
				}
				else
				{
					//今のところありえないが匿名関数も同様の方法で処理する場合はここに
				}
			}
			int thisop = UnaryOperator(this->parsers[index]->pEnum);
			i = index + 1;
			switch (this->parsers[index]->pEnum)
			{
				case parserEnum::identifier:
					/*if(this->parsers.size()>binaryoperation&&this->parsers[binaryoperation]->pEnum==leftparent)
					{
					j=index;
					i = this->parentSkip(binaryoperation);
					std::vector<langObject> arg;
					index = index + 2;
					while (index<i)
					{
					arg.push_back(eval(NULLOBJECT,index,17));
					index++;
					if(this->parsers[index]->pEnum==parserEnum::comma)index++;
					}
					if(this->variable[*this->parsers[j]->name]!=nullptr && this->variable[*this->parsers[j]->name]->type->TypeEnum == PreType::_Function)
					{
					object = static_cast<Function*>(this->variable[*this->parsers[j]->name].get())->call(&arg);
					}
					else
					object = BuitInFunction(*this->parsers[binaryoperation - 1]->name,arg);
					//object =
					index = i;
					binaryoperation = index + 1;

					}
					else*/
					if (*this->parsers[index]->name == "function")
					{
						object = /*std::make_shared<Function>*/(/**/anonymousFunction(index));
						binaryoperation = index + 1;
					}
					else
						object = this->variable[*this->parsers[index]->name];
					break;
				case parserEnum::leftparent:
					i = this->parentSkip(index);
					object = eval(NULLOBJECT, binaryoperation, 17);
					index = i;
					binaryoperation = index + 1;
					break;
				case _new:
					/*if (*this->parsers[binaryoperation]->name == "Array")
					{
					binaryoperation++;
					object = eval(NULLOBJECT, binaryoperation, 17);
					//binaryoperation+=2;
					index = binaryoperation;
					binaryoperation = index + 1;
					object = newArray(Int::toInt(object));
					}
					else*/
				{
							 object = eval(NULLOBJECT, binaryoperation, 0);
							 index = binaryoperation;
							 binaryoperation = index + 1;

							 {
								 auto buf = (Class*)object;
								 if (this->parsers[binaryoperation]->pEnum == parserEnum::leftparent)
								 {
									 int thisop = Operator(this->parsers[binaryoperation]->pEnum);
									 j = index;
									 i = this->parentSkip(binaryoperation);
									 std::vector<langObject> arg;
									 index = index + 2;
									 while (index < i)
									 {
										 arg.push_back(eval(NULLOBJECT, index, 17));
										 index++;
										 if (this->parsers[index]->pEnum == parserEnum::comma)index++;
									 }
									 if (object->type->TypeEnum == PreType::_Class)
									 {
										 object = newClassObject(buf);
										 auto ctor = ((langClassObject)object)->thisscope->variable["ctor"];
										 if (ctor->type->TypeEnum == _Function)
										 {
											 try
											 {
												 object = langObject(static_cast<Function*>(ctor)->ctorcall(&arg));
											 }
											 catch (langRuntimeException ex)
											 {
												 throw langRuntimeException(ex.what(), ex.tokens, ex.funcstacktrace, static_cast<Function*>(ctor)->name.c_str(), ex.stacktrace);
											 }
										 }
									 }
									 else
									 {

										 if (object->type->TypeEnum == _Type)
										 {
											 auto type = ((ObjectType*)object);
											 try
											 {
												 object = type->create(arg);
											 }
											 catch (langRuntimeException ex)
											 {
												 std::stringstream ss;
												 ss << lang::PreTypeName[type->TypeClass.TypeEnum] << " ctor";
												 throw langRuntimeException(ex.what(), ex.tokens, ex.funcstacktrace, ss.str().c_str(), ex.stacktrace);
											 }
										 }
									 }
									 index = i;
									 binaryoperation = index + 1;
									 i = index - 1;
									 OP4
								 }
								 else
								 {
									 if (object->type->TypeEnum == PreType::_Class)
									 {
										 object = newClassObject(buf);
									 }
									 else if (object->type->TypeEnum == _Type)
									 {
										 auto type = ((ObjectType*)object);
										 try
										 {
											 auto arg = std::vector<langObject>();
											 object = type->create(arg);
										 }
										 catch (langRuntimeException ex)
										 {
											 std::stringstream ss;
											 ss << lang::PreTypeName[type->TypeClass.TypeEnum] << " ctor";
											 throw langRuntimeException(ex.what(), ex.tokens, ex.funcstacktrace, ss.str().c_str(), ex.stacktrace);
										 }
									 }
								 }
							 }
							 /*
		else
		{
		throw lang::langRuntimeException("new はClass型でのみ有効です。");
		}*/
				}
					break;
				case parserEnum::_class:
					if (this->parsers[index]->ptr)
					{
						object = this->parsers[index]->ptr;
						index = this->blockSkip(index,-1);
						binaryoperation = index + 1;
					}
					break;
				case parserEnum::minus:
					UOP;
					buf = eval(object, i, 17, (evals)0, thisop);
					object = (Object::unaryminus(buf));
					index = i; binaryoperation = index + 1;
					OP2
						break;
				case parserEnum::plus:
					UOP;
					buf = eval(object, i, 17, (evals)0, thisop);
					object = (Object::unaryplus(buf));
					index = i; binaryoperation = index + 1;
					OP2
						break;
				case parserEnum::_this:
					object = _this;
					if (this->isClass())
					{

					}
					else
						throw lang::langRuntimeException("thisは使えません。");
					index = index + 0;
					//throw lang::langRuntimeException("new はClass型でのみ有効です。");
					break;
				case parserEnum::_false:
					object = this->parsers[index]->ptr;
					break;
				case parserEnum::base:
					object = this->_this->base;
					break;
				case parserEnum::_null:
				case parserEnum::_true:
				case parserEnum::num:
				case parserEnum::str:
				case parserEnum::chr:
					object = this->parsers[index]->ptr;
					break;
			}
			if (this->parsers[index] != nullptr)
			{
				//delete object;
			}
		}
		if (this->parsers.size() > binaryoperation)
		{
			int i = index + 2;
			int thisop = Operator(this->parsers[binaryoperation]->pEnum);
			bool isBuilt = true;
			switch (this->parsers[binaryoperation]->pEnum)
			{
				case leftparent:
					OP;
					{
						j = index;
						i = this->parentSkip(binaryoperation);
						std::vector<langObject> arg;
						index = index + 2;
						while (index < i)
						{
							arg.push_back(eval(NULLOBJECT, index, 17));
							index++;
							if (index > i)break;
							if (this->parsers[index]->pEnum == parserEnum::comma)index++;
						}
						/*if(this->variable[*this->parsers[j]->name]!=nullptr && this->variable[*this->parsers[j]->name]->type->TypeEnum == PreType::_Function)
						{
						object = static_cast<Function*>(this->variable[*this->parsers[j]->name].get())->call(&arg);
						}*/
						if (object != nullptr && object->type->TypeEnum == _Function)
						{
							try
							{
								object = langObject(static_cast<Function*>(object/*.get()*/)->call(&arg));
							}
							catch (langRuntimeException ex)
							{
								throw langRuntimeException(ex.what(), ex.tokens, ex.funcstacktrace, static_cast<Function*>(object)->name.c_str(), ex.stacktrace);
							}
							isBuilt = false;
						}
						else
						{
							object = BuitInFunction(*this->parsers[binaryoperation - 1]->name, arg);
							isBuilt = true;
						}
						//object = 
						index = i;
						binaryoperation = index + 1;
					}__v('ω')v___
					if (isBuilt)
						i = index - 1;
					OP4
						//if(this->parsers.size()>index+1&& Operator(this->parsers[index+1]->pEnum) >= thisop ||this->parsers[index+1]->pEnum==leftparent)
						// object = eval(object,i,17,true),index = i;
						break;
				case parserEnum::plus:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::plus(object, buf));
					index = i;
					//delete buf;
					OP2
						break;
				case parserEnum::minus:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::minus(object, buf));
					index = i;
					//delete buf;
					OP2
						break;
				case parserEnum::multiply:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::multiply(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::division:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::division(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::modulo:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::modulo(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::greater:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::greater(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::less:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::less(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::greaterequal:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::greaterEqual(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::lessequal:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::lessEqual(object, buf));
					index = i;
					OP2//if(this->parsers.size()>index+1&&Operator(this->parsers[index+1]->pEnum) >= thisop) object = eval(object,i,17,true),index = i;
						break;
				case parserEnum::equalequal:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::equal(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::leftshift:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::leftShift(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::rightshift:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::rightShift(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::equal:
					OP;
					object = eval(object, i, thisop);
					this->variable.set(*this->parsers[index]->name, object);//this->variable[*this->parsers[index]->name] = object;
					index = i;
					OP2
						break;
				case parserEnum::plusplus:
					OP;
					//buf = newInt(1);
					object = Object::inc(object);//Object::plus(object,buf);
					this->variable.set(*this->parsers[index]->name, object);
					index = i;
					OP2
						break;
				case parserEnum::_is:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::_is(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::_as:
					OP;
					buf = eval(object, i, thisop);
					object = (Object::as(object, buf));
					index = i;
					OP2
						break;
				case parserEnum::leftbracket:
					OP;
					{
						if (object is _Array)
						{
							buf = eval(object, i, 17);
							auto ob = object;
							//object = ((langArray)object)->ary[Int::toInt(buf)];
							//object = (Object::plus(object,buf));
							index = i + 1;
							binaryoperation = index + 1;
							//object = ((langArray)object)->ary[Int::toInt(object)];
							if (this->parsers.size() > index + 1)
							{
								if (this->parsers[index + 1]->pEnum == equal)
								{
									//binaryoperation++;
									auto set = (langArray)object;
									//index++;
									binaryoperation++;
									int ind = Int::toInt(buf);
									if (set->ary.size() <= ind)

										throw_langRuntimeException("配列の範囲外にアクセス%xのサイズは%dで、 %dにアクセスしようとしました。:VAR_DUMP%s", set, set->ary.size(), ind, set->toString().c_str())

										set->ary[ind] = eval(NULLOBJECT, binaryoperation);
									object = set->ary[Int::toInt(object)];
									//object = buf->thisscope->variable[*bufbuf->name];
									index = binaryoperation;
									binaryoperation++;
								}
							}
							object = ((langArray)ob)->ary[Int::toInt(buf)];
						}
						else
						if (object is _ClassObject)
						{
							j = index/* - 3*/;
							i = this->bracketSkip(binaryoperation/* - 2*/);
							std::vector<langObject> arg;
							index = index + 2;
							while (index < i)
							{
								arg.push_back(eval(NULLOBJECT, index, 17));
								index++;
								if (this->parsers[index]->pEnum == parserEnum::comma)index++;
							}
							if (this->parsers[i + 1]->pEnum == parserEnum::equal)
							{
								i = i + 2;
								arg.push_back(eval(NULLOBJECT, i, 17));
								object = Object::bracketequal(object, arg);
							}
							else
								object = Object::bracket(object, arg);
						}
						else
							throw langRuntimeException("[]を使えない");
					}
					OP5
						break;
				case parserEnum::dot:
					OP;
					if (object->type->TypeEnum == PreType::_ClassObject || object->type->TypeEnum == PreType::_Class)
					{
						auto buf = (Class*)object;
						if (this->parsers.size() > binaryoperation + 1)
						{
							auto bufbuf = this->parsers[binaryoperation + 1];
							if (bufbuf->pEnum == identifier || bufbuf->pEnum == _static || bufbuf->pEnum == base)
							{
								if (bufbuf->pEnum == identifier)
								{
									object = buf->getMember(*bufbuf->name);//buf->thisscope->variable[*bufbuf->name];
									index++;
									binaryoperation++;
									if (this->parsers.size() > binaryoperation + 1)
									{
										if (this->parsers[binaryoperation + 1]->pEnum == equal)
										{
											//binaryoperation++;
											buf->setMember(*bufbuf->name, eval(NULLOBJECT, binaryoperation));//buf->thisscope->variable.set(*bufbuf->name, eval(NULLOBJECT, binaryoperation));
											//object = buf->thisscope->variable[*bufbuf->name];
											index = binaryoperation;
											//index++;
											binaryoperation++;
										}
										else
										{
											index++;
											binaryoperation++;
										}
									}
								}
								else
								{
									if (bufbuf->pEnum == base)
									{
										object = ((langClassObject)object)->base;
									}
									else
									{
										if (object->type->TypeEnum == PreType::_ClassObject)
											object = ((langClassObject)object)->staticClass;
										//else ; //staticクラスからstaticを取ろうとしている
										index++;
										binaryoperation++;
									}
								}
							}
						}
						index++;
						binaryoperation++;
					}
					else
					{
						if (this->parsers.size() > binaryoperation + 1)
						{
							auto bufbuf = this->parsers[binaryoperation + 1];
							if (bufbuf->pEnum == identifier)
							{
								auto _this = object;
								object = object->getMember(*bufbuf->name);
								index++;
								binaryoperation++;

								if (object != nullptr && object is _BuiltFunc && this->parsers.size() > binaryoperation + 1)
								{
									auto bufbuf = this->parsers[binaryoperation + 1];
									if (bufbuf->pEnum == leftparent)
									{
										index++;
										binaryoperation++;
										j = index;
										i = this->parentSkip(binaryoperation);
										std::vector<langObject> arg;
										index = index + 2;
										while (index < i)
										{
											arg.push_back(eval(NULLOBJECT, index, 17));
											index++;
											if (this->parsers[index]->pEnum == parserEnum::comma)index++;
										}
										index = i;
										binaryoperation = index + 1;
										try
										{
											object = langObject(static_cast<SpecialFunction*>(object)->call(_this, &arg));
										}
										catch (langRuntimeException ex)
										{
											throw langRuntimeException(ex.what(), ex.tokens, ex.funcstacktrace, static_cast<Function*>(object)->name.c_str(), ex.stacktrace);
										}
									}
								}
							}
						}
						//throw lang::langRuntimeException(".はClass型でのみ有効です。");
					}
					//i = index + 2;
					OP3
						//if(this->parsers.size() > index + 1 && Operator(this->parsers[index + 1]->pEnum) >= thisop) object = eval(object, i, 17, true), index = i;
						break;
			}
		}
		return langObject(object);
	}
}
