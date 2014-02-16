#define BOOST_TEST_MODULE tests
#include <boost/test/unit_test.hpp>

#include "../model.hpp"
#include "../statistics.hpp"
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
	 * @class IOSKJ::Model
	 * @test equilibrium_stable
	 * 
	 * Test that when there is no substantial change in equilibrium
	 * conditions given further simulation.
	 */
	BOOST_AUTO_TEST_CASE(equilibrium_stable){
		model.track_open("equilibrium_stable.txt");

		model.init();
		auto biomass_equil = model.biomass;

		model.recruits_variation_on = false;
		model.exploitation_on = false;
		model.years(0,100);

		const double tolerance = 0.01; //0.01%
		BOOST_CHECK_CLOSE(biomass_equil(W),model.biomass(W),tolerance);
		BOOST_CHECK_CLOSE(biomass_equil(M),model.biomass(M),tolerance);
		BOOST_CHECK_CLOSE(biomass_equil(E),model.biomass(E),tolerance);
	}

	/**
	 * @class IOSKJ::Model
	 * @test equilibrium_uniform
	 * 
	 * Test that when there is no movement and equal
	 * reruitment distribution that the equilibrium biomass
	 * is equal in all areas
	 */
	BOOST_AUTO_TEST_CASE(equilibrium_uniform){
		model.recruits_uniform();
		model.movement_none();
		model.init();

		const double tolerance = 0.01; //0.01%
		BOOST_CHECK_CLOSE(model.biomass(W),model.biomass(M),tolerance);
		BOOST_CHECK_CLOSE(model.biomass(M),model.biomass(E),tolerance);
		BOOST_CHECK_CLOSE(model.biomass(W),model.biomass(E),tolerance);
	}

	/**
	 * @class IOSKJ::Model
	 * @test recruiment_variation
	 * 
	 * Test that recruitment variation has the right mean and 
	 * standard deviation.
	 */
	BOOST_AUTO_TEST_CASE(recruiment_variation){
		model.track_open("recruiment_variation.txt");

		model.init();
		
		model.exploitation_on = false;
		Stats stats;
		for(int year=0;year<1000;year++){
			model.year(year);
			stats(model.recruits_deviation);
		}

		BOOST_CHECK_CLOSE(mean(stats),1,5);
		BOOST_CHECK_CLOSE(std::pow(variance(stats),0.5),model.recruits_sd,10);
	}

	/**
	 * @class IOSKJ::Model
	 * @test exploitation_specified
	 * 
	 */
	BOOST_AUTO_TEST_CASE(exploitation_specified){
		model.track_open("exploitation_specified.txt");

		model.init();

		model.recruits_variation_on = false;
		model.exploitation_rate_set(0);
		model.years(0,20);
	}

BOOST_AUTO_TEST_SUITE_END()
