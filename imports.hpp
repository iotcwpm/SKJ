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

//... array and query classes
#include <stencila/array.hpp>
#include <stencila/query.hpp>
using namespace Stencila;


// Fisheries Simulation Library (https://github.com/trident-systems/fsl) for...

//... probability distributions
#include <fsl/math/probability/uniform.hpp>
#include <fsl/math/probability/normal.hpp>
#include <fsl/math/probability/lognormal.hpp>
#include <fsl/math/probability/truncated.hpp>
using namespace Fsl::Math::Probability;

//... estimation scaffolding
#include <fsl/estimation/estimation.hpp>
using namespace Fsl::Estimation;
