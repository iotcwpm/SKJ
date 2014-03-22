/**
 * File for importing necessary external libraries
 */

#pragma once

// C++ standard library
#include <cmath>
#include <fstream>


// Boost library (http://www.boost.org/) for ...

// ... string conversion
#include <boost/format.hpp>
using boost::format;


// Stencila library (https://github.com/stencila/stencila) for ...

//... grid and query classes
#include <stencila/grid.hpp>
#include <stencila/query.hpp>
using namespace Stencila;


// Fisheries Simulation Library (https://github.com/trident-systems/fsl) for...

//... probability distributions
#include <fsl/math/probability/uniform.hpp>
#include <fsl/math/probability/normal.hpp>
#include <fsl/math/probability/lognormal.hpp>
#include <fsl/math/probability/truncated.hpp>
using namespace Fsl::Math::Probability;

//... functions
#include <fsl/math/functions/power.hpp>
using Fsl::Math::Functions::Power;
#include <fsl/math/functions/logistic.hpp>
using Fsl::Math::Functions::Logistic;
#include <fsl/math/functions/spline.hpp>
using Fsl::Math::Functions::Spline;

//... stock-recruitment relationship
#include <fsl/population/recruitment/beverton-holt.hpp>
using Fsl::Population::Recruitment::BevertonHolt;

//... estimation scaffolding
#include <fsl/estimation/parameters.hpp>
#include <fsl/estimation/data.hpp>
using namespace Fsl::Estimation;
using Fsl::Estimation::Links::Log;
using Fsl::Estimation::Priors::Fixed;
