#include <windows.h>
#include <stdio.h>

#define CT_API __declspec(dllexport)

extern "C" {

CT_API void test(int a, int (*f)(int, int)) {
    char buf[256];
    int b = 3, c = 4;
    sprintf(buf, "a = %d, b = %d, c = %d\na + f(b, c) = %d", a, b, c, a + f(b, c));
    MessageBoxA(0, buf, "wow", 0);
    return;    
}

}