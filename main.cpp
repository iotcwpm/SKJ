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
	model.years(0,2013-1950);

	model.shutdown();
	Data::shutdown();

	return 0;
}