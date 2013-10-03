#define DEBUG 1
#define TRACKING 1

#include "model.hpp"

int main(void){
	using namespace IOSKJ;
	Model model;
	model.startup();
	model.simulate();
	model.shutdown();
	return 0;
}