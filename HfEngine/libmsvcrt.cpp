#include "libmsvcrt.h"

namespace MSVCRT {
	int(*fclose) (FILE *fp);

	void GetFunctions() {
		HMODULE h = GetModuleHandle(TEXT("msvcrt"));

		fclose = (int(*)(FILE*))GetProcAddress(h, "fclose");
	}
}
