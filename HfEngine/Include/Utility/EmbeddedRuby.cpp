
#include "embeddedruby.h"
#include "windows.h"
#include "ws2def.h"

int main(int argc, char *argv[]){
	printf("%d", argc);
	for(int i = 1; i < argc; i++){
		printf(argv[i]);
	}
    //ruby_sysinit(&argc, &argv);
    //{
	//	RUBY_INIT_STACK;
	//	ruby_init();
	//	return ruby_run_node(ruby_options(argc, argv));
	//}

}
