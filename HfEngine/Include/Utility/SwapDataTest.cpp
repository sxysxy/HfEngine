#include "swapdata.h"
#include <cstdio>
struct Test {
	int a, b;
};
int main() {
	SwapData<Test> t;
	t.WriteRef() = Test {2, 3};
	t.Swap();
	auto t2 = t.ReadRef();
	printf("%d %d\n", t2.a, t2.b);
	t.WriteRef() = Test {1, 2};
	t.Swap();
	t2 = t.ReadRef();
	printf("%d %d\n", t2.a, t2.b);
	return 0;
}