#include "setupTests.h"
#include "Tracer/CpuTracer.h"

// TODO(rudi): shader tests

namespace RAYX {
	namespace CPP_TRACER {
		double r8_exp(double);
		double squaresDoubleRNG(uint64_t&);
	}
}

using namespace RAYX::CPP_TRACER;

TEST_F(TestSuite, testUniformRandom) {
	uint64_t ctr = 13;
	for (int i = 0; i < 100; i++) {
		double d = squaresDoubleRNG(ctr);
		if (d < 0.0 || d > 1.0) {
			ADD_FAILURE() << "invalid d";
		}
	}
}

TEST_F(TestSuite, ExpTest) {
	std::vector<double> args = {10.0, 5.0, 2.0, 1.0, 0.5, 0.0001, 0.0};
	for (auto x : args) {
		CHECK_EQ(r8_exp(x), exp(x));
		CHECK_EQ(r8_exp(-x), exp(-x));
	}
}
