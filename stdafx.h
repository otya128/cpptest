// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#ifndef __STDAFX_H__//#pragma once
#define __STDAFX_H__
#include "targetver.h"

#include <stdio.h>
//#include <tchar.h>
//add
#include <iostream>
#include <string>
#if _DEBUG && _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
