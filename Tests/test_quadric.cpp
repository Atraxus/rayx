#include "Tracer/Vulkan/Material.h"
#include "setupTests.h"

#if RUN_TEST_QUADRIC

// TODO(rudi): These tests previously also tested World/GeometricUserParams (see
// commit 18f6af0). Is it possible to re-add this part while still using RML in
// a simple way?
TEST(Quadric, testTransformationMatrices) {
    auto b = RAYX::importBeamline(resolvePath(
        "Tests/rml_files/test_quadric/testTransformationMatrices.rml"));

    auto plM = b.m_OpticalElements[0];

    std::array<double, 4 * 4> correctInMat = {
        0.9997500170828264,    -0.0093954937290516224, 0.02028861849598634, 0,
        0.0051669667654668724, 0.97994613741550907,    0.199195557728752,   0,
        -0.0217532939421341,   -0.19904093162465691,   0.97974971382524823, 0,
        260.14829377531987,    2387.4863841540064,     -11764.895314472104, 1};
    CHECK_EQ(plM->getInMatrix(), correctInMat);

    std::array<double, 4 * 4> correctOutMat = {
        0.9997500170828264,     0.0051669667654668724, -0.0217532939421341,  0,
        -0.0093954937290516224, 0.97994613741550907,   -0.19904093162465691, 0,
        0.02028861849598634,    0.199195557728752,     0.97974971382524823,  0,
        1.0418248851126821,     2.562645914782741,     12007.519413984284,   1};
    CHECK_EQ(plM->getOutMatrix(), correctOutMat);
}

TEST(Quadric, testGlobalCoordinates) {
    auto b = RAYX::importBeamline(
        resolvePath("Tests/rml_files/test_quadric/testGlobalCoordinates.rml"));
    auto p1 = b.m_OpticalElements[0];
    auto p2 = b.m_OpticalElements[1];
    auto p3 = b.m_OpticalElements[2];
    auto p4 = b.m_OpticalElements[3];

    std::array<double, 4 * 4> correctInMat = {
        0.98549875516199115,   -0.16900296657661762, 0.015172371682388559, 0,
        0.16532344425841758,   0.97647242956597469,  0.13845487740074897,  0,
        -0.038214687656708428, -0.13393876058044285, 0.99025251631160971,  0,
        75.929375313416855,    267.1775211608857,    -1981.5050326232194,  1};
    std::array<double, 4 * 4> correctOutMat = {
        0.98549875516199115,  0.16532344425841758, -0.038214687656708428, 0,
        -0.16900296657661762, 0.97647242956597469, -0.13393876058044285,  0,
        0.015172371682388559, 0.13845487740074897, 0.99025251631160971,   0,
        0.38961967265975178,  0.90464730022614015, 2000.877388040077,     1};
    CHECK_EQ(p1->getInMatrix(), correctInMat);
    CHECK_EQ(p1->getOutMatrix(), correctOutMat);

    correctInMat = {
        0.99293120507151145,   -0.11097960343033685, -0.042085028426756203, 0,
        0.11609472530507925,   0.8343464348616344,   0.53887664765697407,   0,
        -0.024690823225925775, -0.5399532889575962,  0.84133275758899373,   0,
        223.62868377282729,    2612.60301401578,     -8515.6341722842517,   1};
    correctOutMat = {
        0.99293120507151167,  0.11609472530507926, -0.024690823225925786, 0,
        -0.11097960343033686, 0.8343464348616344,  -0.5399532889575962,   0,
        -0.04208502842675621, 0.53887664765697396, 0.84133275758899373,   0,
        -290.48295826317462,  2383.0982743679397,  8580.687147244611,     1};
    CHECK_EQ(p2->getInMatrix(), correctInMat);
    CHECK_EQ(p2->getOutMatrix(), correctOutMat);

    correctInMat = {
        0.98543342847582116, -0.15825553171931203, -0.06225869194438529, 0,
        0.13887040159030783, 0.53751266264821851,  0.83174223714740403,  0,
        -0.0981629747131509, -0.82827249391311997, 0.55165995524635025,  0,
        983.58748092948872,  6671.4190423504515,   -14677.230524351115,  1};
    correctOutMat = {
        0.98543342847582127,   0.13887040159030789, -0.098162974713150941, 0,
        -0.15825553171931206,  0.53751266264821851, -0.82827249391312019,  0,
        -0.062258691944385297, 0.83174223714740392, 0.55165995524635025,   0,
        -827.25618948103272,   8485.1091498802671,  13719.145095369648,    1};
    CHECK_EQ(p3->getInMatrix(), correctInMat);
    CHECK_EQ(p3->getOutMatrix(), correctOutMat);

    correctInMat = {
        0.95892551908867896,  -0.24148295699748484, -0.14882147130121315, 0,
        0.16352170556644466,  0.041905034073013991, 0.98564933923174713,  0,
        -0.23178114817462611, -0.96949984507429876, 0.079671511544351148, 0,
        2630.3284873799857,   13109.341026432205,   -10506.281074894196,  1,
    };
    correctOutMat = {
        0.95892551908867907,  0.16352170556644474, -0.23178114817462619, 0,
        -0.24148295699748484, 0.04190503407301399, -0.96949984507429876, 0,
        -0.14882147130121315, 0.98564933923174713, 0.079671511544351137, 0,
        -920.16688225314476,  9376.0458164086922,  14156.215944980175,   1};
    CHECK_EQ(p4->getInMatrix(), correctInMat);
    CHECK_EQ(p4->getOutMatrix(), correctOutMat);
}

#endif
