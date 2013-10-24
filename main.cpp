#define DEBUG 1
#define TRACKING 1

#include "model.hpp"
#include "data.hpp"

int main(void){
	using namespace IOSKJ;
	Model model;

	model.startup();
	Data::startup();

	model.simulate();

	model.shutdown();
	Data::shutdown();

	return 0;
}