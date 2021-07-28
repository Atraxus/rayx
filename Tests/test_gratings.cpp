#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Beamline/Beamline.h"
#include "Beamline/Objects/PlaneGrating.h"
#include "Beamline/Objects/SphereGrating.h"
#include "Beamline/Objects/Ellipsoid.h"
#include "Core.h"
#include "Ray.h"

using ::testing::ElementsAre;

//! Using the google test framework, check all elements of two containers
#define EXPECT_ITERABLE_BASE( PREDICATE, REFTYPE, TARTYPE, ref, target) \
    { \
    const REFTYPE& ref_(ref); \
    const TARTYPE& target_(target); \
    REFTYPE::const_iterator refIter = ref_.begin(); \
    TARTYPE::const_iterator tarIter = target_.begin(); \
    unsigned int i = 0; \
    while(refIter != ref_.end()) { \
        if ( tarIter == target_.end() ) { \
            ADD_FAILURE() << #target " has a smaller length than " #ref ; \
            break; \
        } \
        PREDICATE(* refIter, * tarIter) \
            << "Containers " #ref  " (refIter) and " #target " (tarIter)" \
               " differ at index " << i; \
        ++refIter; ++tarIter; ++i; \
    } \
    EXPECT_TRUE( tarIter == target_.end() ) \
        << #ref " has a smaller length than " #target ; \
    }

//! Check that all elements of two same-type containers are equal
#define EXPECT_ITERABLE_EQ( TYPE, ref, target) \
    EXPECT_ITERABLE_BASE( EXPECT_EQ, TYPE, TYPE, ref, target )

//! Check that all elements of two different-type containers are equal
#define EXPECT_ITERABLE_EQ2( REFTYPE, TARTYPE, ref, target) \
    EXPECT_ITERABLE_BASE( EXPECT_EQ, REFTYPE, TARTYPE, ref, target )

//! Check that all elements of two same-type containers of doubles are equal
#define EXPECT_ITERABLE_DOUBLE_EQ( TYPE, ref, target) \
    EXPECT_ITERABLE_BASE( EXPECT_DOUBLE_EQ, TYPE, TYPE, ref, target )

TEST(planeGrating, testParams) {
    // mount = 0, use deviation angle
    int mount = 0; // { GM_DEVIATION, GM_INCIDENCE, GM_CCF, GM_CCF_NO_PREMIRROR}
    double width = 125.23;
    double height = 51.56;
    double deviation = 12.4;
    double normalIncidence = 15.12;
    double azimuthal = 61.142;
    double dist = 1245.71;
    double designEnergyMounting = 100;
    double lineDensity = 812.2;
    int orderOfDiffraction = 1; // use others
    double fixFocusConstantCFF = 12.1;
    int add_order = 0;
    std::vector<double> sE = { 0,0,0,0,0, 0,0 };
    std::vector<double> mis = { 5.212,7.3,0.35, 0.23,0.011,0.0006 };
    std::vector<double> vls = { 2.1, 0.12, 12.2, 8.3, 5.1, 7.23 };
    RAYX::PlaneGrating p1 = RAYX::PlaneGrating("planegrating", mount, width, height, deviation, normalIncidence, azimuthal, dist, designEnergyMounting, lineDensity, orderOfDiffraction, fixFocusConstantCFF, add_order, mis, vls, sE, NULL, false); 

    double beta = 1.467650597188248;
    double alpha = 1.457521229154248;
    double a = 0.010070077944000002;
    std::vector<double> quad = { 0,0,0,0, 0,0,0,-1, 0,0,0,0, 1,0,0,0 };
    std::vector<double> params = { width,height,lineDensity,double(orderOfDiffraction), abs(hvlam(designEnergyMounting)),0,vls[0],vls[1], vls[2],vls[3],vls[4],vls[5], 0,0,0,double(add_order) };
    ASSERT_DOUBLE_EQ(p1.getWidth(), width);
    ASSERT_DOUBLE_EQ(p1.getHeight(), height);
    ASSERT_DOUBLE_EQ(p1.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(p1.getChi(), rad(azimuthal));
    ASSERT_DOUBLE_EQ(p1.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(p1.getBeta(), beta);
    EXPECT_NEAR(p1.getA(), a, 0.00000000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p1.getSurfaceParams(), quad);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p1.getElementParameters(), params);
    //EXPECT_NEAR (e.getRadius(),  radius, 0.00000001);

    // mount = 1, use incidence Angle
    mount = 1; // { GM_DEVIATION, GM_INCIDENCE, GM_CCF, GM_CCF_NO_PREMIRROR}
    add_order = 1;
    RAYX::PlaneGrating p2 = RAYX::PlaneGrating("planegrating", mount, width, height, deviation, normalIncidence, azimuthal, dist, designEnergyMounting, lineDensity, orderOfDiffraction, fixFocusConstantCFF, add_order, mis, vls, sE, NULL, false); 
    alpha = 1.3069025438933539;
    beta = 1.317319261832787;

    ASSERT_DOUBLE_EQ(p2.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(p2.getBeta(), beta);

    // higher order of diffraction, mount = 1
    orderOfDiffraction = 3;
    a = 0.030210233832000003;
    beta = 1.3380699314613769;
    quad = { 0,0,0,0, 0,0,0,-1, 0,0,0,0, 1,0,0,0 };
    params = { width,height,lineDensity,double(orderOfDiffraction), abs(hvlam(designEnergyMounting)),0,vls[0],vls[1], vls[2],vls[3],vls[4],vls[5], 0,0,0,double(add_order) };

    RAYX::PlaneGrating p3 = RAYX::PlaneGrating("planegrating", mount, width, height, deviation, normalIncidence, azimuthal, dist, designEnergyMounting, lineDensity, orderOfDiffraction, fixFocusConstantCFF, add_order, mis, vls, sE, NULL, false); 

    ASSERT_DOUBLE_EQ(p3.getWidth(), width);
    ASSERT_DOUBLE_EQ(p3.getHeight(), height);
    ASSERT_DOUBLE_EQ(p3.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(p3.getChi(), rad(azimuthal));
    ASSERT_DOUBLE_EQ(p3.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(p3.getBeta(), beta);
    EXPECT_NEAR(p3.getA(), a, 0.00000000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p3.getSurfaceParams(), quad);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p3.getElementParameters(), params);

    // mount = 0, use deviation angle, with higher order of diffraction
    mount = 0;
    a = 0.030210233832000003;
    alpha = 1.4473913414095938;
    beta = 1.4777804849329026;
    quad = { 0,0,0,0, 0,0,0,-1, 0,0,0,0, 1,0,0,0 };
    params = { width,height,lineDensity,double(orderOfDiffraction), abs(hvlam(designEnergyMounting)),0,vls[0],vls[1], vls[2],vls[3],vls[4],vls[5], 0,0,0,double(add_order) };

    RAYX::PlaneGrating p4 = RAYX::PlaneGrating("planegrating", mount, width, height, deviation, normalIncidence, azimuthal, dist, designEnergyMounting, lineDensity, orderOfDiffraction, fixFocusConstantCFF, add_order, mis, vls, sE, nullptr, false); 

    ASSERT_DOUBLE_EQ(p4.getWidth(), width);
    ASSERT_DOUBLE_EQ(p4.getHeight(), height);
    ASSERT_DOUBLE_EQ(p4.getAlpha(), alpha);
    ASSERT_DOUBLE_EQ(p4.getChi(), rad(azimuthal));
    ASSERT_DOUBLE_EQ(p4.getDistanceToPreceedingElement(), dist);
    ASSERT_DOUBLE_EQ(p4.getBeta(), beta);
    EXPECT_NEAR(p4.getA(), a, 0.00000000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p4.getSurfaceParams(), quad);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, p4.getElementParameters(), params);

}

TEST(PlaneGrating, testHvlam) {
    double hv = 100;
    double linedensity = 1000;
    double orderOfDiff = 1;
    double a = abs(hvlam(hv)) * abs(linedensity) * orderOfDiff * 1e-06;
    ASSERT_DOUBLE_EQ(a, 0.01239852);
}

TEST(SphereGrating, testParams) {
    int mount = 0;
    double width = 241.623;
    double height = 836.213;
    double deviation = 9.215;
    double incidence = 12.512;
    double azimuthal = 123.2;
    double distance = 2151.62;
    double entranceArm = 6134.1;
    double exitArm = 623.12;
    double designEnergy = 232.2;
    double linedensity = 432.2;
    int add_order = 0;
    int order = 1;
    std::vector<double> mis = { 1.41,5.3,1.5, 0.2,1.0,1.4 };
    std::vector<double> vls = { 2.1, 0.12, 12.2, 8.3, 5.1, 7.23 };
    std::vector<double> sE = { 0.1,0.5,0.1,0.2,0.5, 1,3 };

    RAYX::SphereGrating s1 = RAYX::SphereGrating("spheregrating", mount, width, height, deviation, incidence, azimuthal, distance, entranceArm, exitArm, designEnergy, linedensity, double(order), mis,vls,sE, NULL, false);
    double alpha = 1.4892226555787231;
    double beta = 1.4915379074397925;
    double chi = 2.1502456384570143;
    double theta = 4.6738270800745312;
    double radius = 1134.9852832410934;
    double a = 0.002307769312661499;
    std::vector<double> quad = { 1,0,0,0, 0,1,0,-radius, 0,0,1,0, 2,0,0,0 };
    std::vector<double> params = { width,height,linedensity,double(order), abs(hvlam(designEnergy)),0,vls[0],vls[1], vls[2],vls[3],vls[4],vls[5], 0,0,0,0 };

    ASSERT_DOUBLE_EQ(s1.getWidth(), width);
    ASSERT_DOUBLE_EQ(s1.getHeight(), height);
    ASSERT_DOUBLE_EQ(s1.getGratingMount(), mount);
    ASSERT_DOUBLE_EQ(s1.getChi(), rad(azimuthal));
    ASSERT_DOUBLE_EQ(s1.getDeviation(), rad(deviation));
    ASSERT_DOUBLE_EQ(s1.getChi(), chi);
    ASSERT_DOUBLE_EQ(s1.getDistanceToPreceedingElement(), distance);
    ASSERT_DOUBLE_EQ(s1.getLineDensity(), linedensity);
    ASSERT_DOUBLE_EQ(s1.getDesignEnergyMounting(), designEnergy);
    ASSERT_DOUBLE_EQ(s1.getOrderOfDiffraction(), double(order));
    EXPECT_NEAR(s1.getBeta(), beta, 0.00000001);
    EXPECT_NEAR(alpha, s1.getAlpha(), 0.000000001);
    EXPECT_NEAR(radius, s1.getRadius(), 0.0000001);
    EXPECT_NEAR(s1.getA(), a, 0.000000001);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, s1.getSurfaceParams(), quad);
    EXPECT_ITERABLE_DOUBLE_EQ(std::vector<double>, s1.getElementParameters(), params);

    mount = 1;
    RAYX::SphereGrating s2 = RAYX::SphereGrating("spheregrating", mount, width, height, deviation, incidence, azimuthal, distance, entranceArm, exitArm, designEnergy, linedensity, double(order), mis,vls, sE,NULL, false);
    /*double alpha = 1.4892226555787231;
    double beta = 1.4915379074397925;
    double chi = 2.1502456384570143;
    double theta = 4.6738270800745312;
    double radius = 1134.9852832410934;
    double a = 0.002307769312661499;
    std::vector<double> quad = {1,0,0,0, width,1,0,-radius, height,a,1,0, 2,0,0,0};
    std::vector<double> params = {width,height,linedensity,double(order), designEnergy,a,vls[0],vls[1], vls[2],vls[3],vls[4],vls[5], 0,0,0,0};*/

}
