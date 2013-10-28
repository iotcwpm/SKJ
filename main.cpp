#define DEBUG 1

#include "model.hpp"
#include "data.hpp"

int main(void){
	using namespace IOSKJ;
	Model model;

	model.startup();
	Data::startup();

	model.defaults();
	model.init();
	model.simulate(0,100);

	model.shutdown();
	Data::shutdown();

	return 0;
}