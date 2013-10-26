#define BOOST_TEST_MODULE tests
#include <boost/test/unit_test.hpp>

#include "../model.hpp"
using namespace IOSKJ;

struct modelFixture { 
	Model model;

	modelFixture(){
		model.startup();
		model.defaults();
	} 
};

BOOST_FIXTURE_TEST_SUITE(model,modelFixture)

	/**
	 * Test that when there is no substantial change in equilibrium
	 * conditions given further simulation.
	 */
	BOOST_AUTO_TEST_CASE(equilibrium_stable){
		model.track_open("equilibrium_stable.txt");

		model.spawning_uniform();
		model.init();
		auto biomass_equil = model.biomass;

		model.recruit_variation_on = false;
		model.exploitation_on = false;
		model.simulate(100);

		const double tolerance = 0.1; //0.1%
		BOOST_CHECK_CLOSE(biomass_equil(W),model.biomass(W),tolerance);
		BOOST_CHECK_CLOSE(biomass_equil(M),model.biomass(M),tolerance);
		BOOST_CHECK_CLOSE(biomass_equil(E),model.biomass(E),tolerance);
	}

	/**
	 * Test that when there is no movement and equal
	 * reruitment distribution that the equilibrium biomass
	 * is equal in all areas
	 */
	BOOST_AUTO_TEST_CASE(equilibrium_uniform){
		model.recruit_uniform();
		model.movement_none();
		model.init();

		const double tolerance = 0.01; //0.01%
		BOOST_CHECK_CLOSE(model.biomass(W),model.biomass(M),tolerance);
		BOOST_CHECK_CLOSE(model.biomass(M),model.biomass(E),tolerance);
		BOOST_CHECK_CLOSE(model.biomass(W),model.biomass(E),tolerance);
	}

BOOST_AUTO_TEST_SUITE_END()
