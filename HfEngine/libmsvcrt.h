#pragma once
#include "windows.h"
#include "stdio.h"

/*
	为了解决一些链接库的冲突（比如fclose在vcruntime140-ruby与msvcrt中都存在，并且不可以通用，使用msvcrt的_wfopn必须使用
	msvcrt的fclose才能正常运行）而设立的模块。
				这ruby，编译代码的时候都坑死人了，变成二进制代码之后继续坑人。。。

*/

namespace MSVCRT {

	extern int(*fclose) (FILE *fp);

	void GetFunctions();
}