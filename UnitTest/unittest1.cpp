#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest{		
	TEST_CLASS(HelloWorld){
	public:
		
		TEST_METHOD(TestHelloWorld){
			Tests::TestHelloWorld();
		}

	};

    TEST_CLASS(HFWindow) {
    public:
        TEST_METHOD(TestHFWindow) {
            Tests::TestHFWindow();
        }
    };

    TEST_CLASS(DX) {
    public:
        TEST_METHOD(TestShader) {
            Tests::TestShaderBasic();
        }
    };

}