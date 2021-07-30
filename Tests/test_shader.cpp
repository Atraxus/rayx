#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Beamline/Beamline.h"
#include "Surface/Quadric.h"
#include "Beamline/Objects/ReflectionZonePlate.h"
#include "Beamline/Objects/PlaneMirror.h"
#include "Beamline/Objects/SphereMirror.h"
#include "Beamline/Objects/MatrixSource.h"
#include "Tracer/TracerInterface.h"
#include "Core.h"
#include "Ray.h"
// #include "Tracer/TracerInterface.h"
#include "VulkanTracer.h"
#include <fstream>
#include <sstream>
//std::vector<double> zeros = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

using ::testing::ElementsAre;
std::vector<double> zeros = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
std::vector<double> zeros7 = { 0,0,0,0,0, 0,0 }; // for slope error

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


std::list<double> runTracer(std::vector<RAYX::Ray> testValues, std::vector<std::shared_ptr<RAYX::OpticalElement>> elements) {
    for (int i = 0;i < 16;i++) {
        std::cout << "elements[0]: " << elements[0]->getSurfaceParams()[i] << std::endl;
    }
    RAYX::TracerInterface ti;
    VulkanTracer tracer;

    std::list<std::vector<RAYX::Ray>> rayList;
    tracer.setBeamlineParameters(1, elements.size(), testValues.size());
    std::cout << "testValues.size(): " << testValues.size() << std::endl;
    (tracer).addRayVector(testValues.data(), testValues.size());
    std::cout << "add rays to tracer done" << std::endl;

    for (std::shared_ptr<RAYX::OpticalElement> e : elements) {
        ti.addOpticalElementToTracer(&tracer, e);
    }
    tracer.run(); //run tracer
    std::list<double> outputRays;
    std::vector<Ray> outputRayVector = *(tracer.getOutputIteratorBegin());
    for (auto iter = outputRayVector.begin();iter != outputRayVector.end();iter++) {
        outputRays.push_back((*iter).getxPos());
        outputRays.push_back((*iter).getyPos());
        outputRays.push_back((*iter).getzPos());
        outputRays.push_back((*iter).getWeight());
        outputRays.push_back((*iter).getxDir());
        outputRays.push_back((*iter).getyDir());
        outputRays.push_back((*iter).getzDir());
        outputRays.push_back((*iter).getEnergy());
    }
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    tracer.cleanup();
    return outputRays;
}

void writeToFile(std::list<double> outputRays, std::string name)
{
    std::cout << "writing to file..." << name <<  std::endl;
    std::ofstream outputFile;
    outputFile.precision(17);
    std::cout.precision(17);
    std::string filename = "../../Tests/output/";
    filename.append(name);
    filename.append(".csv");
    outputFile.open(filename);
    char sep = ';'; // file is saved in .csv (comma seperated value), excel compatibility is manual right now
    outputFile << "Index" << sep << "Xloc" << sep << "Yloc" << sep << "Zloc" << sep << "Weight" << sep << "Xdir" << sep << "Ydir" << sep << "Zdir" << sep << "Energy" << std::endl;
    // outputFile << "Index,Xloc,Yloc,Zloc,Weight,Xdir,Ydir,Zdir" << std::endl;

    size_t counter = 0;
    int print = 0; // whether to print on std::out (0=no, 1=yes)
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end(); i++) {
        if (counter % 8 == 0) {
            outputFile << counter / VULKANTRACER_RAY_DOUBLE_AMOUNT;
            if (print == 1) std::cout << ")" << std::endl;
            if (print == 1) std::cout << "(";
        }
        outputFile << sep << *i;
        if (counter % 8 == 7) {
            outputFile << std::endl;
            counter++;
            continue;
        }
        if (counter % 8 == 3) {
            if (print == 1) std::cout << ") ";
        }
        else if (counter % 8 == 4) {
            if (print == 1) std::cout << " (";
        }
        else if (counter % 8 != 0) {
            if (print == 1) std::cout << ", ";
        }
        if (print == 1) std::cout << *i;
        counter++;
    }
    if (print == 1) std::cout << ")" << std::endl;
    outputFile.close();
    std::cout << "done!" << std::endl;
}

/*

TEST(Tracer, testUniformRandom) {
    double settings = 17;

    RAYX::MatrixSource m = RAYX::MatrixSource(0, "Matrix source 1", 2000, 0, 0.065, 0.04, 0.0, 0.001, 0.001, 100, 0, { 0,0,0,0 });
    std::vector<RAYX::Ray> testValues = m.getRays();

    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("testRandomNumbers", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);
    std::list<double> outputRays = runTracer(testValues, { q });

    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end(); i++) {
        ASSERT_TRUE(*i <= 1.0);
        ASSERT_TRUE(*i >= 0.0);
    }
    std::string filename = "testFile_randomUniform";
    writeToFile(outputRays, filename);
}



TEST(Tracer, ExpTest) {
    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = -4;
    int high = 4;
    double settings = 18;
    RAYX::RandomRays random = RAYX::RandomRays(n, low, high);

    std::vector<RAYX::Ray> testValues = random.getRays();
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 1, -3), glm::dvec3(PI, 2, 3),4, 5);
    testValues.push_back(r);

    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("ExpTest", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-13;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_position.x), tolerance);
        }
        else if (counter % 8 == 1) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_position.y), tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_position.z), tolerance);
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_weight), tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_direction.x), tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_direction.y), tolerance);
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_direction.z), tolerance);
        }
        else if (counter % 8 == 7) {
            EXPECT_NEAR(*i, exp(testValues[int(counter / 8)].m_energy), tolerance);
        }
        counter++;
        i++;
    }
}

TEST(Tracer, LogTest) {
    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = 1;
    int high = 4;
    double settings = 19;
    RAYX::RandomRays random = RAYX::RandomRays(n, low, high);

    std::vector<RAYX::Ray> testValues = random.getRays();
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0.1, 1, 0.3), glm::dvec3(PI, 2, 3),4, 5);
    testValues.push_back(r);

    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("LogTest", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-13;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, log(testValues[int(counter / 8)].m_position.x), tolerance);
        }
        else if (counter % 8 == 1) {
            EXPECT_NEAR(*i, log(testValues[int(counter / 8)].m_position.y), tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i,  log(testValues[int(counter / 8)].m_position.z), tolerance);
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i,  log(testValues[int(counter / 8)].m_weight), tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i,  log(testValues[int(counter / 8)].m_direction.x), tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, log(testValues[int(counter / 8)].m_direction.y), tolerance);
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, log(testValues[int(counter / 8)].m_direction.z), tolerance);
        }
        else if (counter % 8 == 7) {
            EXPECT_NEAR(*i, log(testValues[int(counter / 8)].m_energy), tolerance);
        }
        counter++;
        i++;
    }
}


TEST(Tracer, testRefrac2D) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    std::vector<std::shared_ptr<RAYX::OpticalElement>> quadrics;
    double settings = 16;

    // ray.position = normal at intersection point, ray.direction = direction of ray, ray.weight = weight of ray before refraction
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.0001666666635802469, -0.017285764670739875, 0.99985057611723738), 0,1.0);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-0.012664171360811521, 0.021648721107426414, 0.99968542634078494),0, 1.0);
    correct.push_back(c);
    // one quadric for each ray to transport ax and az for that test ray to the shader
    double az = 0.00016514977645243345;
    double ax = 0.012830838024391771;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("testRefrac2D", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ az, ax,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);
    quadrics.push_back(q);

    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.00049999999722222275, -0.017285762731583675, 0.99985046502305308),0, 1.0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.00049999999722222275, -0.017285762731583675, 0.99985046502305308),0, 0.0);
    correct.push_back(c);
    az = -6.2949352042540596e-05;
    ax = 0.038483898782123105;
    q = std::make_shared<RAYX::OpticalElement>("testRefrac2D", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ az, ax,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);
    quadrics.push_back(q);

    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.0001666666635802469, -0.017619047234249029, 0.99984475864845179),0, 1.0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.0001666666635802469, -0.017619047234249029, 0.99984475864845179),0, 0.0);
    correct.push_back(c);
    az = -0.077169530850327184;
    ax = 0.2686127340088395;
    q = std::make_shared<RAYX::OpticalElement>("testRefrac2D", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ az, ax,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);
    quadrics.push_back(q);

    // normal != 0 (spherical RZP)
    r = RAYX::Ray(glm::dvec3(0.050470500672820856, 0.95514062789960541, -0.29182033770349547), glm::dvec3(-0.000499999916666667084, -0.016952478247434233, 0.99985617139734351),0, 1.0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0.080765992839840872, 0.57052382524991363, 0.81730007905468893), 0,1.0);
    correct.push_back(c);
    az = 0.0021599283476277926;
    ax = -0.050153240660177005;
    q = std::make_shared<RAYX::OpticalElement>("testRefrac2D", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ az, ax,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);
    quadrics.push_back(q);

    std::list<double> outputRays = runTracer(testValues, quadrics);

    int counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        std::cout << *i << ", ";
        if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
            std::cout << std::endl;
        }
        counter++;
        i++;
    }
}

TEST(Tracer, testNormalCartesian) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position.x = slopeX, ray.position.z = slopeZ. ray.direction = normal at intersection point from eg quad fct.
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    // store correct resulting normal[0:3] in ray.direction and fourth component (normal[3]) in weight
    // case: normal unchanged bc slope = 0
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 0, 0);
    correct.push_back(c);

    // normal != (0,1,0), slope still = 0
    r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537), 0, 0);
    testValues.push_back(r);
    // normal unchanged
    c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537), 0, 0.0);
    correct.push_back(c);

    // normal = (0,1,0), slopeX = 2, slopeZ = 3
    r = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(-0.90019762973551742, 0.41198224566568298, -0.14112000805986721), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(-9431.2371568647086, 4310.7269916467494, -1449.3435640204684), 0, 0);
    correct.push_back(c);

    double settings = 13;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testNormalCartesian", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    double tolerance = 1e-12; // smallest possible
    // return format = pos (0=x,1=y,2=z), 3=weight, dir (4=x,5=y,6=z), 7=0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
}

TEST(Tracer, testNormalCylindrical) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position.x = slopeX, ray.position.z = slopeZ. ray.direction = normal at intersection point from eg quad fct.
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    // store correct resulting normal[0:3] in ray.direction and fourth component (normal[3]) in weight
    // case: normal unchanged bc slope = 0
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0), 0, 0);
    correct.push_back(c);

    // normal != (0,1,0), slope still = 0
    r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537), 0, 0);
    testValues.push_back(r);
    // normal slightly unchanged in x (due to limited precision?!)
    c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(5.0465463027115769, 10470.451695989539, -28.532199794465537), 0, 0.0);
    correct.push_back(c);

    // normal = (0,1,0), slopeX = 2, slopeZ = 3
    r = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(0.90019762973551742, 0.41198224566568292, -0.14112000805986721), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(5.0465463027123736, 10470.451695989539, -28.532199794465537), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(2, 0, 3), glm::dvec3(9431.2169472441783, 4310.7711493493844, -1449.3437356459144), 0, 0);
    correct.push_back(c);

    double settings = 14;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testNormalCylindrical", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    double tolerance = 1e-11; // smallest possible
    // return format = pos (0=x,1=y,2=z), 3=weight, dir (4=x,5=y,6=z), 7=0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
}

TEST(Tracer, testRefrac) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    double a = 0.01239852;
    // encode: ray.position = normal at intersection point. ray.direction = direction of ray, ray.weigth = weight of ray
    // plane surface
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.00049999991666667084, -0.99558611855684065, 0.09385110834192622), 0, 1);
    testValues.push_back(r);
    // store correct resulting weight in c.weight and calculated direction in c.direction
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.00049999991666667084, 0.99667709206767885, 0.08145258834192623), 0, 1);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.000016666664506172893, -0.995586229182718, 0.093851118714515264), 0, 1.0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.000016666664506160693, 0.9966772027014974, 0.081452598714515267), 0, 1.0);
    correct.push_back(c);

    // spherical grating, same a
    r = RAYX::Ray(glm::dvec3(0.0027574667592826954, 0.99999244446428082, -0.0027399619384214182), glm::dvec3(-0.00049999991666667084, -0.99558611855684065, 0.093851108341926226), 0, 1);
    testValues.push_back(r);
    // pos does not matter
    c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0.0049947959329671825, 0.99709586573547515, 0.07599267429701162), 0, 1);
    correct.push_back(c);


    double settings = 15;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testRefrac", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ a,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-12;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
    std::cout << std::endl;
}

TEST(Tracer, testRefracBeyondHor) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position = normal at intersection point. ray.direction = direction of ray, ray.weigth = weight of ray
    // plane surface
    // beyond horizon
    double a = -0.038483898782123105;
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-0.99991341437509562, 0.013149667401360443, -0.00049999997222215965), 0, 1.0);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(-0.99991341437509562, 0.013149667401360443, -0.00049999997222215965), 0, 0.0);
    correct.push_back(c);


    double settings = 15;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testRefrac", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, std::vector<double>{ a,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-12;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
    std::cout << std::endl;
}

TEST(Tracer, testWasteBox) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    // encode: ray.position = position of intersection point. ray.direction.x = xLength of opt. element, ray.direction.z = zLength of optical element, ray.weigth = weight of ray before calling wastebox
    // case: intersection point on surface
    RAYX::Ray r = RAYX::Ray(glm::dvec3(-5.0466620698997637, 0, 28.760236725599515), glm::dvec3(50, 0, 200), 0, 1);
    testValues.push_back(r);
    // store correct resulting weight in weight of c
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0, 1);
    correct.push_back(c);

    // intersection point not on surface
    r = RAYX::Ray(glm::dvec3(-5.0466620698997637, 0, 28.760236725599515), glm::dvec3(5, 0, 20), 0, 1.0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1034.8685185321933, 0, -13.320120179862874), glm::dvec3(0, 0, 0), 0, 0.0);
    correct.push_back(c);

    // intersection point not on surface
    r = RAYX::Ray(glm::dvec3(-1.6822205656320104, 0, 28.760233508097873), glm::dvec3(5, 0, 20), 0, 1);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    // ray already had weight 0
    r = RAYX::Ray(glm::dvec3(-5.0466620698997637, 0, 28.760236725599515), glm::dvec3(50, 0, 200), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    double settings = 11;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testWasteBox", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-10;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
    std::cout << std::endl;
}

TEST(Tracer, testRZPLineDensityDefaulParams) { // point to point
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column}
    // {image_type, rzp_type, derivation_method, zOffsetCenter}, -> point2point(0), elliptical(0), formulas(0), 0
    // {risag, rosag, rimer, romer},
    // {d_alpha, d_beta, d_ord, wl}, {0,0,0,0}
    std::vector<double> inputValues = { 0,0,0,0, 100,500,100,500, 0.017453292519943295,0.017453292519943295,-1,12.39852 * 1e-06, 0,0,0,0 };

    // encode: ray.position = position of test ray. ray.direction = normal at intersection point.
    RAYX::Ray r = RAYX::Ray(glm::dvec3(-5.0805095016939532, 0, 96.032788311782269), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(3103.9106911246745, 0, 5.0771666329965663), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(-1.6935030407867075, 0, 96.032777495754004), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1034.8685185321933, 0, -13.320120179862874), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    // spherical (normal != (0,1,0))
    r = RAYX::Ray(glm::dvec3(-5.047050067282087, 4.4859372100394515, 29.182033770349552), glm::dvec3(0.05047050067282087, 0.95514062789960552, -0.29182033770349552), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(4045.0989844091873, 0, -174.20856260487483), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(-1.6802365843267262, 1.3759250917712356, 16.445931214643075), glm::dvec3(0.016802365843267261, 0.98624074908228765, -0.16445931214643075), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1418.1004208892471, 0, 253.09836635775156), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    double settings = 12;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testRZPpoint2point", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, inputValues, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-10;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.x, tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.z, tolerance);
        }
        counter++;
        i++;
    }
}

TEST(Tracer, testRZPLineDensityAstigmatic) { // astigmatic 2 astigmatic
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column} -> astigmatic2astigmatic(1), elliptical(0), formulas(0), 0
    // {image_type, rzp_type, derivation_method, zOffsetCenter},
    // {risag, rosag, rimer, romer},
    // {d_alpha, d_beta, d_ord, wl}, {0,0,0,0}
    std::vector<double> inputValues = { 1,0,0,0, 100,500,100,500, 0.017453292519943295,0.017453292519943295,-1,12.39852 * 1e-06, 0,0,0,0 };

    // encode: ray.position = position of test ray. ray.direction = normal at intersection point.
    RAYX::Ray r = RAYX::Ray(glm::dvec3(-5.0805095016939532, 0, 96.032788311782269), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(3103.9106911246745, 0, 5.0771666329965663), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(-1.6935030407867075, 0, 96.032777495754004), glm::dvec3(0, 1, 0), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1034.8685185321933, 0, -13.320120179862874), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    // spherical (normal != (0,1,0))
    r = RAYX::Ray(glm::dvec3(-5.047050067282087, 4.4859372100394515, 29.182033770349552), glm::dvec3(0.05047050067282087, 0.95514062789960552, -0.29182033770349552), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(4045.0989844091873, 0, -174.20856260487483), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(-1.6802365843267262, 1.3759250917712356, 16.445931214643075), glm::dvec3(0.016802365843267261, 0.98624074908228765, -0.16445931214643075), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1418.1004208892471, 0, 253.09836635775156), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    double settings = 12;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testRZPAstigmatic", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, inputValues, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-10;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.x, tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.z, tolerance);
            std::cout << *i << ", ";
        }
        std::cout << std::endl;
        counter++;
        i++;
    }
    std::cout << std::endl;
}


// test pow(a,b) = a^b function. ray position[i] ^ ray direction[i] for i in {0,1,2}
TEST(Tracer, testRayMatrixMult) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    // {1st column, 2nd column, 3rd column, 4th column}
    std::vector <double> matrix = { 1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16 };

    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(13, 14, 15), glm::dvec3(0, 0, 0), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(1, 1, 0), glm::dvec3(0, 1, 1), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1 + 5 + 13, 2 + 6 + 14, 3 + 7 + 15), glm::dvec3(5 + 9, 6 + 10, 7 + 11), 0, 0);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(1, 2, 3), glm::dvec3(4, 5, 6), 0, 0);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1 * 1 + 2 * 5 + 3 * 9 + 13, 1 * 2 + 2 * 6 + 3 * 10 + 14, 1 * 3 + 2 * 7 + 3 * 11 + 15), glm::dvec3(4 * 1 + 5 * 5 + 6 * 9, 4 * 2 + 5 * 6 + 6 * 10, 4 * 3 + 5 * 7 + 6 * 11), 0, 0);
    correct.push_back(c);

    double settings = 10;
    std::shared_ptr<RAYX::OpticalElement> q1 = std::make_shared<RAYX::OpticalElement>("testRayMatrixMult", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, matrix, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q1 });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-12;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.x, tolerance);
        }
        else if (counter % 8 == 1) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.y, tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.z, tolerance);
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
        }
        else if (counter % 8 == 0) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
        }
        counter++;
        i++;

    }
}

// test pow(a,b) = a^b function. ray position[i] ^ ray direction[i] for i in {0,1,2}
TEST(Tracer, testDPow) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0.0, 0, 0), glm::dvec3(0, 1, -1), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(2, 2, 3), glm::dvec3(0, 1, 7), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(4, -4, 2), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(0.2, 19.99 / 2, PI), glm::dvec3(4, 3, 6), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(-1.0, -1.0, -1.0), glm::dvec3(-4, 3, 0), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(-1.0, -1.0, -1.0), glm::dvec3(4, 5, 6), 0, 0);
    testValues.push_back(r);
    std::vector<double> correct = { 1,0,1,
    1,2,2187,
    0,1,0,
    0.0016,998.50074987499977,961.38919357530415,
    1, -1, 1,
    1, -1, 1 };

    double settings = 7;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("testDoublePow", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 < 3) {
            //std::cout << ", output" << *i <<", correct " << correct[corr_counter] << std::endl;
            EXPECT_NEAR(*i, correct[corr_counter], tolerance);
            corr_counter++;
        }
        counter++;
        i++;

    }
}

// test pow(a,b) = a^b function. ray position[i] ^ ray direction[i] for i in {0,1,2}
TEST(Tracer, testCosini) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    // phi, psi given in position.x, position.y
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(1, 1, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(1, 0, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(PI, PI, 0), glm::dvec3(0, 0, 0), 0, 0);
    testValues.push_back(r);


    double settings = 9;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("testCosini", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    int corr_counter = 0;
    double tolerance = 1e-12;
    // return format = pos (x,y,z), weight, dir (x,y,z), 0
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 4) { // xdir = sin(psi) * sin(phi)
            EXPECT_NEAR(*i, cos(testValues[int(counter / 8)].m_position.y) * sin(testValues[int(counter / 8)].m_position.x), tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 5) { // ydir = -sin(psi)
            EXPECT_NEAR(*i, -sin(testValues[int(counter / 8)].m_position.y), tolerance);
            std::cout << *i << ", ";
        }
        else if (counter % 8 == 6) { // zdir = cos(psi) * cos(phi)
            EXPECT_NEAR(*i, cos(testValues[int(counter / 8)].m_position.y) * cos(testValues[int(counter / 8)].m_position.x), tolerance);
            std::cout << *i;
        }
        else if (counter % 8 == 7) {
            std::cout << std::endl;
        }
        counter++;
        i++;

    }
}

// test factorial f(a) = a!
TEST(Tracer, factTest) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 1, 2), glm::dvec3(-1, 4, 17), 0, -2);
    testValues.push_back(r);

    // pos, weight, dir
    std::vector<double> correct = { 1,1,2, -2, -1,24,355687428096000 };

    double settings = 8;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("testPow", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        EXPECT_NEAR(*i, correct[counter], tolerance);
        counter++;
        i++;

    }
}

TEST(Tracer, bessel1Test) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    //RAYX::Ray r = RAYX::Ray(glm::dvec3(-12.123,20.1,100), glm::dvec3(20.0,0,23.1), 0);
    RAYX::Ray r = RAYX::Ray(glm::dvec3(-12.123, 20.1, 100), glm::dvec3(20.0, 0, 23.1), 0, -0.1);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(12.123, 2, 0.00000001), glm::dvec3(19.99, 10.2, PI), 0, 4);
    testValues.push_back(r);

    // pos(3), weight, dir(3) + 0
    std::vector<double> correct = { 0,0,0, 0, 0.066833545658411195,0,0,0
    ,-0.21368198451302897,0.57672480775687363,5e-09, -0.06604332802354923, 0.065192988349741882,-0.0066157432977083167,0.28461534317975273,0 };

    double settings = 6;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("TestBessel1", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;
    int counter = 0;
    double tolerance = 1e-08;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        std::cout << counter << ", output" << *i << ", correct " << correct[counter] << std::endl;
        EXPECT_NEAR(*i, correct[counter], tolerance);
        counter++;
        i++;

    }
}


TEST(Tracer, diffractionTest) {
    VulkanTracer tracer;
    std::vector<RAYX::Ray> testValues;
    RAYX::Ray r;
    // pos = (iopt,  xlenght, ylength) weight = wavelength
    //r = RAYX::Ray(glm::dvec3(1, 50,100), glm::dvec3(0.0,0.0,0.0), 0.1);
    //testValues.push_back(r);
    int n = 1;
    for (int i = 0; i < n; i++) {
        r = RAYX::Ray(glm::dvec3(1, 20, 2), glm::dvec3(0.0, 0.0, 0.0), 0, 12.39852);
        testValues.push_back(r);
    }
    double lowerDphi = 1e-10;
    double upperDphi = 1e-06;
    double lowerDpsi = 1e-08;
    double upperDpsi = 1e-05;

    double settings = 5;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("TestDiffraction", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        std::cout << *i << ", ";
        if (counter % 8 == 0) {
            //EXPECT_TRUE(abs(*i) < upperDphi);
            //EXPECT_TRUE(abs(*i) > lowerDphi);
        }
        else if (counter % 8 == 1) {
            //EXPECT_TRUE(abs(*i) < upperDpsi);
            //EXPECT_TRUE(abs(*i) > lowerDpsi);
        }
        counter++;
        i++;
    }
}

TEST(Tracer, TrigTest) {
    std::list<std::vector<RAYX::Ray>> rayList;
    int n = 10;
    int low = -1;
    int high = 1;
    RAYX::RandomRays random = RAYX::RandomRays(n, low, high);

    std::vector<RAYX::Ray> testValues = random.getRays();
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(1, 0, 1), 0, 1);
    testValues.push_back(r);
    r = RAYX::Ray(glm::dvec3(PI, PI, PI), glm::dvec3(PI, PI, PI), 0, PI);
    testValues.push_back(r);
    double settings = 1;

    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("qq", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, cos(testValues[int(counter / 8)].m_position.x), tolerance);
        }
        else if (counter % 8 == 1) {
            EXPECT_NEAR(*i, cos(testValues[int(counter / 8)].m_position.y), tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, sin(testValues[int(counter / 8)].m_position.z), tolerance);
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, sin(testValues[int(counter / 8)].m_weight), tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i, atan(testValues[int(counter / 8)].m_direction.x), tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, atan(testValues[int(counter / 8)].m_direction.y), tolerance);
        }
        else if (counter % 8 == 6) {
            if (testValues[int(counter / 8)].m_direction.z >= -1 && testValues[int(counter / 8)].m_direction.z <= 1) {
                EXPECT_NEAR(*i, asin(testValues[int(counter / 8)].m_direction.z), tolerance);
            }// else nan, how to test if nan?
        }
        counter++;
        i++;
    }
}

// test VLS function that calculates new a from given a, z-position and 6 vls parameters
TEST(Tracer, vlsGratingTest) {
    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;

    double z = 5.0020783775947848;
    double a = 0.01239852;
    double settings = 4;

    // encode vls parameters in ray direction and position, a = wl*linedensity*ord*1.e-6 is given as well (in weight of ray)
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, 0.0), 0, a);
    testValues.push_back(r);
    // a should remain unchanged if all vls parameters are 0
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, 0.0), 0, a);
    correct.push_back(c);

    // use some vls values and compare with A calculated by old ray UI
    r = RAYX::Ray(glm::dvec3(1, 2, 3), glm::dvec3(4, 5, 6), 0, a);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(1, 2, 3), glm::dvec3(4, 5, 6), 0, 9497.479959611925);
    correct.push_back(c);

    // give z position and setting=4 to start vls test on shader
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("TestVLS", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,z,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });

    int counter = 0;
    double tolerance = 1e-15;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        std::cout << *i << ", ";
        if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
        }
        counter++;
        i++;
    }

}

TEST(Tracer, planeRefracTest) {
    VulkanTracer tracer;
    std::list<std::vector<RAYX::Ray>> rayList;

    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 3;

    // normal (always 0,1,0) encoded in ray position, a encoded in direction.x, direction.y and direction.z are actual ray directions
    RAYX::Ray r = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0, -0.99558611855684065, 0.09385110834192662), 0, 0.01239852);
    testValues.push_back(r);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.0, 0.99667709206767885, 0.08145258834192623), 0, 0.01239852);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.01239852, -0.99558611855684065, 0.09385110834192662), 0, 0.01239852);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.0, 0.99667709206767885, 0.08145258834192623), 0, 0.01239852);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.01239852, -0.99567947186812988, 0.0928554753392902), 0, 0.01239852);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.0, 0.99675795875308415, 0.080456955339290204), 0, 0.01239852);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.01239852, -0.99567947186812988, 0.0928554753392902), 0, 0.01239852);
    testValues.push_back(r);
    c = RAYX::Ray(glm::dvec3(0.0, 1.0, 0.0), glm::dvec3(0.0, 0.99675795875308415, 0.080456955339290204), 0, 0.01239852);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.0004999999166666, -0.99558611855684065, 0.093851108341926226), 0, 0.01239852);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0, 0.99667709206767885, 0.08145258834192623), 0, 0.01239852);
    testValues.push_back(r);
    correct.push_back(c);

    r = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(-0.0004999999166666, -0.995586229182718, 0.093851118714515264), 0, 0.01239852);
    c = RAYX::Ray(glm::dvec3(0, 1, 0), glm::dvec3(0, 0.9966772027014974, 0.081452598714515267), 0, 0.01239852);
    testValues.push_back(r);
    correct.push_back(c);
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("TestPlaneRefrac", std::vector<double>{ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-12;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
        }
        counter++;
        i++;
    }
}

TEST(Tracer, iteratToTest) {
    VulkanTracer tracer;
    std::list<std::vector<RAYX::Ray>> rayList;

    std::vector<RAYX::Ray> testValues;
    std::vector<RAYX::Ray> correct;
    double settings = 20;

    // normal (always 0,1,0) encoded in ray position, a encoded in direction.x, direction.y and direction.z are actual ray directions
    RAYX::Ray r = RAYX::Ray(glm::dvec3(-0.0175, 1736.4751598838836, -9848.1551798768887), glm::dvec3(-0.00026923073232438285, -0.17315574581145807, 0.984894418304465), 1.0, 100);
    RAYX::Ray c = RAYX::Ray(glm::dvec3(-2.7173752216893443, 0.050407875158271054, 28.473736158432885), glm::dvec3(-0.00026923073232438285, -0.17315574581145807, 0.984894418304465), 1.0, 100);
    testValues.push_back(r);
    correct.push_back(c);

    double longRadius = 10470.491787499999;
    double shortRadius = 315.72395939400002;
    std::shared_ptr<RAYX::OpticalElement> q = std::make_shared<RAYX::OpticalElement>("TestPlaneRefrac", std::vector<double>{ longRadius,shortRadius,0,0, 0,0,0,0, 0,0,0,0, 0,settings,0,0 }, zeros, zeros, zeros, zeros, zeros, zeros);

    std::list<double> outputRays = runTracer(testValues, { q });
    std::cout << "got " << outputRays.size() << " values from shader" << std::endl;

    int counter = 0;
    double tolerance = 1e-09;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        if (counter % 8 == 0) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.x, tolerance);
        }
        else if (counter % 8 == 1) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.y, tolerance);
        }
        else if (counter % 8 == 2) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_position.z, tolerance);
        }
        else if (counter % 8 == 3) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_weight, tolerance);
        }
        else if (counter % 8 == 4) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.x, tolerance);
        }
        else if (counter % 8 == 5) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.y, tolerance);
        }
        else if (counter % 8 == 6) {
            EXPECT_NEAR(*i, correct[int(counter / 8)].m_direction.z, tolerance);
        }
        counter++;
        i++;
    }
}


void testOpticalElement(std::vector<std::shared_ptr<RAYX::OpticalElement>> elements, int n) {


    std::shared_ptr<RAYX::MatrixSource> m = std::make_shared<RAYX::MatrixSource>(0, "Matrix source 1", n, 0, 0.065, 0.04, 0.0, 0.001, 0.001, 100, 0, std::vector<double>{ 0,0,0,0 });


    std::list<double> outputRays = runTracer(m->getRays(), elements);
    std::string filename = "testFile_";
    std::cout << elements[0]->getName();
    filename.append(elements[0]->getName());
    writeToFile(outputRays, filename);
}
// test complete optical elements instead of single functions
// uses deterministic source (matrix source with source depth = 0)
// use name of optical element as file name

TEST(opticalElements, planeMirrorDefault) {
    std::shared_ptr<RAYX::PlaneMirror> plM = std::make_shared<RAYX::PlaneMirror>("PlaneMirrorDef", 50, 200, 10, 7.5, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, false); // {1,2,3,0.01,0.02,0.03}
    testOpticalElement({ plM }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, planeMirrorMis) {
    std::shared_ptr<RAYX::PlaneMirror> plM = std::make_shared<RAYX::PlaneMirror>("PlaneMirrorMis", 50, 200, 10, 0, 10000, std::vector<double>{ 1,2,3,0.001,0.002,0.003 }, zeros7, nullptr, false); // {1,2,3,0.01,0.02,0.03}
    testOpticalElement({ plM }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, sphereMirror) {
    std::shared_ptr<RAYX::SphereMirror> s = std::make_shared<RAYX::SphereMirror>("SphereMirrorDefault", 50, 200, 10, 0.0, 10000, 10000, 1000, std::vector<double>{ 0,0,0,0,0,0 }, zeros7, nullptr, false);
    testOpticalElement({ s }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, planeGratingDevDefault) {
    std::shared_ptr<RAYX::PlaneGrating> plG = std::make_shared<RAYX::PlaneGrating>("PlaneGratingDeviationDefault", 0, 50, 200, 10, 0.0, 0.0, 10000, 100, 1000, 1, 2, 0, std::vector<double>{ 0,0,0,0,0,0 }, std::vector<double>{ 0,0,0,0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ plG }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, planeGratingDevAzimuthal) {
    std::shared_ptr<RAYX::PlaneGrating> plG = std::make_shared<RAYX::PlaneGrating>("PlaneGratingDeviationAz", 0, 50, 200, 10, 0.0, 7.5, 10000, 100, 1000, 1, 2, 0, std::vector<double>{ 0,0,0,0,0,0 }, std::vector<double>{ 0,0,0,0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ plG }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, planeGratingDevMis) {
    std::shared_ptr<RAYX::PlaneGrating> plG = std::make_shared<RAYX::PlaneGrating>("PlaneGratingDeviationAzMis", 0, 50, 200, 10, 0.0, 7.5, 10000, 100, 1000, 1, 2, 0, std::vector<double>{ 1,2,3,0.001,0.002,0.003 }, std::vector<double>{ 0,0,0,0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ plG }, 20);
    ASSERT_TRUE(true);
}

// constant incidence angle mode, azimuthal angle and misalignment
TEST(opticalElements, planeGratingIncAzMis) {
    std::shared_ptr<RAYX::PlaneGrating> plG = std::make_shared<RAYX::PlaneGrating>("PlaneGratingIncAzMis", 1, 50, 200, 0.0, 10, 7.5, 10000, 100, 1000, 1, 2, 0, std::vector<double>{ 1,2,3,0.001,0.002,0.003 }, std::vector<double>{ 0,0,0,0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ plG }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, planeGratingDevMisVLS) {
    std::shared_ptr<RAYX::PlaneGrating> plG = std::make_shared<RAYX::PlaneGrating>("PlaneGratingDeviationMis", 0, 50, 200, 10, 0.0, 7.5, 10000, 100, 1000, 1, 2, 0, std::vector<double>{ 1,2,3,0.001,0.002,0.003 }, std::vector<double>{ 1,2,3,4,5,6 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ plG }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, RZPDefaultParams) {
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePlateDefault", 0, 1, 0, 0, 0, 50, 200, 170, 1, 0, 10000, 100, 100, -1, -1, 1, 1, 100, 500, 100, 500, 0, 0, 0, 0, 0, 0, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ rzp }, 20);
    ASSERT_TRUE(true);
}

TEST(opticalElements, RZPDefaultParams200) {
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePlateDefault200", 0, 1, 0, 0, 0, 50, 200, 170, 1, 0, 10000, 100, 100, -1, -1, 1, 1, 100, 500, 100, 500, 0, 0, 0, 0, 0, 0, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ rzp }, 200);
    ASSERT_TRUE(true);
}

TEST(opticalElements, RZPAzimuthal200) {
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePlateAzim200", 0, 1, 0, 0, 0, 50, 200, 170, 1, 10, 10000, 100, 100, -1, -1, 1, 1, 100, 500, 100, 500, 0, 0, 0, 0, 0, 0, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
    testOpticalElement({ rzp }, 200);
    ASSERT_TRUE(true);
}


TEST(opticalElements, RZPMis) {
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePlateMis", 0, 1, 0, 0, 0, 50, 200, 170, 1, 0, 10000, 100, 100, -1, -1, 1, 1, 100, 500, 100, 500, 0, 0, 0, 0, 0, 0, std::vector<double>{ 1,2,3,0.001,0.002,0.003 }, zeros7, nullptr, false); // dx,dy,dz, dpsi,dphi,dchi //
    testOpticalElement({ rzp }, 200);
    ASSERT_TRUE(true);
}

TEST(opticalElements, ImagePlane) {
    std::shared_ptr<RAYX::PlaneMirror> plM = std::make_shared<RAYX::PlaneMirror>("PlaneMirror_ImagePlane", 50, 200, 10, 0, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, false); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::ImagePlane> i = std::make_shared<RAYX::ImagePlane>("ImagePlane", 1000, plM, false);
    testOpticalElement({ plM, i }, 200);
    ASSERT_TRUE(true);
}


TEST(globalCoordinates, FourMirrors_9Rays) {
    std::shared_ptr<RAYX::PlaneMirror> p1 = std::make_shared<RAYX::PlaneMirror>("globalCoordinates_9rays", 50, 200, 10, 7, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p2 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror2", 50, 200, 15, 4, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p1, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p3 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror3", 50, 200, 7, 10, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p2, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p4 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror4", 50, 200, 22, 17, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p3, true); // {1,2,3,0.01,0.02,0.03}
    p4->setOutMatrix({ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 });
    testOpticalElement({ p1, p2, p3, p4 }, 9);
    ASSERT_TRUE(true);
}

TEST(globalCoordinates, FourMirrors_20Rays) {
    std::shared_ptr<RAYX::PlaneMirror> p1 = std::make_shared<RAYX::PlaneMirror>("globalCoordinates_20rays", 50, 200, 10, 7, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p2 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror2", 50, 200, 15, 4, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p1, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p3 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror3", 50, 200, 7, 10, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p2, true); // {1,2,3,0.01,0.02,0.03}
    std::shared_ptr<RAYX::PlaneMirror> p4 = std::make_shared<RAYX::PlaneMirror>("PlaneMirror4", 50, 200, 22, 17, 10000, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, p3, true); // {1,2,3,0.01,0.02,0.03}
    // to stay in element coordinates and make a comparison with old RAY possible
    p4->setOutMatrix({ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 });
    testOpticalElement({ p1, p2, p3, p4 }, 20);
    ASSERT_TRUE(true);
}


TEST(opticalElements, slit1) {
    std::shared_ptr<RAYX::MatrixSource> m = std::make_shared<RAYX::MatrixSource>(0, "matrix source", 200, 0, 0.065, 0.04, 0, 0.001, 0.001, 100, 0, std::vector<double>{ 0,0,0,0 });
    std::shared_ptr<RAYX::Slit> s = std::make_shared<RAYX::Slit>("slit", 0, 1, 20, 2, 0, 10000, 20, 1, m->getPhotonEnergy(), std::vector<double>{ 0,0,0, 0,0,0 }, nullptr, true);
    std::shared_ptr<RAYX::ImagePlane> ip = std::make_shared<RAYX::ImagePlane>("Image plane", 1000, s, true);
    std::list<double> outputRays = runTracer(m->getRays(), {s,ip});
    int counter = 0;
    for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end();) {
        // only if weight == 1
        if(counter & 8 == 0) {
            std::list<double>::iterator j = i;
            j++;
            j++;
            if(*(j) == 1)
                EXPECT_TRUE(abs(*i) <= 6);
        }else if (counter % 8 == 1) { // y loc
            std::list<double>::iterator j = i;
            j++;
            j++;
            j++;
            if(*j == 1) {
                EXPECT_TRUE(abs(*i) >= 0.5);
                EXPECT_TRUE(abs(*i) <= 1.3);
            }
        }
        counter++;
        i++;
    }
}

bool global = false;
// PETES SETUP
// spec1-first_rzp4mm
TEST(PeteRZP, spec1_first_rzp) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_rzp", 20000 , 1, 0.005,0.005,0, 0.02,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 4, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 1, 0, 0, 1, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, global); // dx,dy,dz, dpsi,dphi,dchi //
    std::list<double> outputRays = runTracer(p->getRays(), {rzp});
    std::string filename = "testFile_spec1_first_rzp";
    writeToFile(outputRays, filename);
}

TEST(PeteRZP, spec1_first_ip) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_rzp4",20000 , 1, 0.005,0.005,0, 0.02,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 4, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 1, 0, 0, 1, std::vector<double>{ 0,0,0, 0,0,0 }, zeros7, nullptr, global);  // dx,dy,dz, dpsi,dphi,dchi //
    std::shared_ptr<RAYX::ImagePlane> ip1 = std::make_shared<RAYX::ImagePlane>("ImagePlane1", 385.0, rzp, global);
    std::vector<RAYX::Ray> input = p->getRays();
    std::list<double> outputRays = runTracer(input, {rzp, ip1});
    std::string filename = "testFile_spec1_first_rzp_ip";
    writeToFile(outputRays, filename);
}

TEST(PeteRZP, spec1_first_plus_rzp) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_plus_rzp",20000 , 1, 0.005,0.005,0, 0.02,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 4, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), 1, 1, 2.2, 4.75, 90, 400, 90, 400, 0, 0, 1, 0, -24.35, 4.75, std::vector<double>{ 0,0,0, 0,0,0 },  std::vector<double>{0,0,0,0, 0,0,0}, nullptr, global);  // dx,dy,dz, dpsi,dphi,dchi //
    std::list<double> outputRays = runTracer(p->getRays(), {rzp});
    std::string filename = "testFile_spec1_first_plus_rzp";
    writeToFile(outputRays, filename);
}

TEST(PeteRZP, spec1_first_plus_ip) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_plus_rzp_ip",20000 , 1, 0.005,0.005,0, 0.02,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 4, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), 1, 1, 2.2, 4.75, 90, 400, 90, 400, 0, 0, 1, 0, -24.35, 4.75, std::vector<double>{ 0,0,0, 0,0,0 }, std::vector<double>{0,0,0,0, 0,0,0}, nullptr, global);  // dx,dy,dz, dpsi,dphi,dchi //
    std::shared_ptr<RAYX::ImagePlane> ip1 = std::make_shared<RAYX::ImagePlane>("ImagePlane1", 400.0, rzp, global);
    std::vector<RAYX::Ray> input = p->getRays();
    std::list<double> outputRays = runTracer(input, {rzp, ip1});
    std::string filename = "testFile_spec1_first_plus_rzp_ip";
    writeToFile(outputRays, filename);
}

TEST(PeteRZP, spec1_first_minus_rzp2) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_minus_rzp2",20000 , 1, 0.005,0.005,0, 0.001,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 2, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 1, 0, 0, 1, std::vector<double>{ 0,0,0, 0,0,0 },  std::vector<double>{0,0,0,0, 0,0,0}, nullptr, global);  // dx,dy,dz, dpsi,dphi,dchi //
    std::list<double> outputRays = runTracer(p->getRays(), {rzp});
    std::string filename = "testFile_spec1_first_minus_rzp2";
    writeToFile(outputRays, filename);
}

TEST(PeteRZP, spec1_first_minus_ip2) {
    std::shared_ptr<RAYX::PointSource> p = std::make_shared<RAYX::PointSource>(0, "spec1_first_minus_rzp_ip2",20000 , 1, 0.005,0.005,0, 0.001,0.06, 1,1,0,0, 640, 120, std::vector<double>{0,0,0,0});
    std::shared_ptr<RAYX::ReflectionZonePlate> rzp = std::make_shared<RAYX::ReflectionZonePlate>("ReflectionZonePete", 0, 1, 0, 1, 1, 2, 60, 170, 2.2, 0, 90, p->getPhotonEnergy(), p->getPhotonEnergy(), -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 1, 0, 0, 1, std::vector<double>{ 0,0,0, 0,0,0 },  std::vector<double>{0,0,0,0, 0,0,0}, nullptr, global);  // dx,dy,dz, dpsi,dphi,dchi //
    std::shared_ptr<RAYX::ImagePlane> ip1 = std::make_shared<RAYX::ImagePlane>("ImagePlane1", 400.0, rzp, global);
    std::vector<RAYX::Ray> input = p->getRays();
    std::list<double> outputRays = runTracer(input, {rzp, ip1});
    std::string filename = "testFile_spec1_first_minus_rzp_ip2";
    writeToFile(outputRays, filename);
}
*/