#include "EasyRenderer.h"
#include <fstream>
std::ofstream logfile("/storage/emulated/0/HfEngine/STGFramework/log.txt");

int main(){
	SDL_Window *wnd = nullptr;
	wnd = SDL_CreateWindow("STG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, SDL_WINDOW_SHOWN);
	if(wnd == nullptr){
		logfile << "233";
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	
	SDL_Event eve;
	while(true){
  	if(SDL_PullEvent(&eve)){
		if(eve.type == SDL_QUIT)break;
  	}
  	
	}
	SDL_DestroyWindow(wnd);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}