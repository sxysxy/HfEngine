#include <iostream>
#include "ReferPtr.h"
#include <algorithm>
#include <memory>
using namespace std;
using namespace Utility;
struct Test1;

struct Test : public ReferredObject {
	virtual void Release(){
		cout << "233" << endl;
	}
};

int main(){
	auto t = ReferPtr<Test>::New();
	return 0;
}