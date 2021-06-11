#include "Beamline/MatrixSource.h"
#include "Beamline/PointSource.h"
#include "Beamline/ReflectionZonePlate.h"
#include "Debug.h"
#include "TracerInterface.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip> 
#include <sstream>

#define SHORTOUTPUT false

namespace RAY
{
    TracerInterface::TracerInterface() :
        m_Beamline(Beamline::get())
    {

        DEBUG(std::cout << "Creating TracerInterface..." << std::endl);
    }

    TracerInterface::~TracerInterface()
    {
        DEBUG(std::cout << "Deleting TracerInterface..." << std::endl);
    }

    void TracerInterface::addLightSource(LightSource* newSource) {
        m_LightSources.push_back(newSource);
    }

    void TracerInterface::generateRays(VulkanTracer* tracer, LightSource* source) {
        //only one Source for now
        if (!tracer) return;
        if (!source) return;
        std::vector<RAY::Ray> rays = (*source).getRays();
        std::cout << "add rays" << std::endl;
        (*tracer).addRayVector(rays.data(), rays.size());
    }

    // ! parameters are temporary and need to be removed again
    bool TracerInterface::run(double translationXerror, double translationYerror, double translationZerror)
    {

        const clock_t all_begin_time = clock();
        //create tracer instance
        VulkanTracer tracer;
        // readFromFile("../../io/input.csv", RayType);


        //add source to tracer
        //initialize matrix light source with default params
        int beamlinesSimultaneously = 1;
        int number_of_rays = 20000;
        //PointSource p = PointSource(0, "name", number_of_rays, 0.005, 0.005, 0, 20, 60, 1, 1, 0, 0, 100, 10, { 0,0,0,0 });
        // petes setup
        PointSource p = PointSource(0, "spec1_first_rzp4", number_of_rays, 1, 0.005, 0.005, 0, 0.02, 0.06, 1, 1, 0, 0, 640, 120, { 0,0,0,0 });
        ReflectionZonePlate rzp = ReflectionZonePlate("ReflectionZonePlateMis", 1, 0, 1, 1, 4, 60, 170, 2.2, 0, 90, p.getPhotonEnergy(), p.getPhotonEnergy(), -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 0, 1, { 0,0,0, 0,0,0 }, nullptr); // dx,dy,dz, dpsi,dphi,dchi //
        PlaneGrating plG = PlaneGrating("PlaneGratingDeviationAzMis", 0, 50, 200, 10, 0.0, 7.5, 10000, 100, 1000, 1, 2, { 0,0,0, 0,0,0 }, { 0,0,0,0,0,0 }, nullptr); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}

        //ImagePlane ip = ImagePlane("Image Plane", 385, nullptr); // one out of the bunch
        //PointSource m = PointSource(0, "Point source 1", number_of_rays, 0.065, 0.04, 1.0, 0.001, 0.001, 0, 0, 0, 0, 100, 10, {0,0,0,0});
        //std::cout << m.getName() << " with " << m.getNumberOfRays() << " Rays." << std::endl;std::cout.precision(15); // show 16 decimals

        addLightSource(&p);
        generateRays(&tracer, &p);

        std::cout << "add rays to tracer done" << std::endl;



        std::cout.precision(17);
        //ReflectionZonePlate p1 = ReflectionZonePlate("ReflectionZonePlate1", 1, 0, 50, 200, 170, 1, 10, 1000, 100, 100, -1, -1, 1, 1, 100, 500, 100, 500, 0, 0, 0, { 0,0,0, 0,0,0 }, NULL); // dx,dy,dz, dpsi,dphi,dchi // {1,2,3,0.001,0.002,0.003}
        //RAY::ReflectionZonePlate reflZonePlate = ReflectionZonePlate("ReflectionZonePlate", 1, 0, 4, 60, 170, 2.2, 0, 90, 640, 640, -1, -1, 2.2, 1, 90, 400, 90, 400, 0, 0, 0, { 0,0,0,0,0,0 }); // dx,dy,dz, dpsi,dphi,dchi // 
        // plane mirror with RAY-UI default values
        //PlaneMirror p1 = PlaneMirror("PlaneMirror1", 50, 200, 10, 0, 10000, { 0,0,0, 0,0,0 }, nullptr); // {1,2,3,0.01,0.02,0.03}
        /*
        PlaneMirror p2 = PlaneMirror("PlaneMirror2", 50, 200, 15, 4, 10000, {1,2,3, 0.001,0.002,0.003}, &p1); // {1,2,3,0.01,0.02,0.03}
        PlaneMirror p3 = PlaneMirror("PlaneMirror3", 50, 200, 7, 10, 10000, {0,0,0, 0,0,0}, &p2); // {1,2,3,0.01,0.02,0.03}
        PlaneMirror p4 = PlaneMirror("PlaneMirror4", 50, 200, 22, 17, 10000, {0,0,0, 0,0,0}, &p3); // {1,2,3,0.01,0.02,0.03}
        */
        ImagePlane ip1 = ImagePlane("ImagePlane1", 350.0, nullptr);

        //m_Beamline.addQuadric(reflZonePlate.getName(), reflZonePlate.getAnchorPoints(), reflZonePlate.getInMatrix(), reflZonePlate.getOutMatrix(), reflZonePlate.getTempMisalignmentMatrix(), reflZonePlate.getInverseTempMisalignmentMatrix(), reflZonePlate.getParameters());
        m_Beamline.addQuadric(rzp); //rzp.getName(), rzp.getAnchorPoints(), rzp.getInMatrix(), rzp.getOutMatrix(), rzp.getTempMisalignmentMatrix(), rzp.getInverseTempMisalignmentMatrix(), rzp.getParameters());
        //m_Beamline.addQuadric(ip); //ip.getName(), ip.getAnchorPoints(), ip.getInMatrix(), ip.getOutMatrix(), ip.getTempMisalignmentMatrix(), ip.getInverseTempMisalignmentMatrix(), ip.getParameters());
        //add beamline to tracer
        std::vector<RAY::Quadric> Quadrics = m_Beamline.getObjects();
        tracer.setBeamlineParameters(beamlinesSimultaneously, Quadrics.size(), number_of_rays);
        for (int i = 0; i<int(Quadrics.size()); i++) {
            tracer.addQuadric(Quadrics[i].getAnchorPoints(), Quadrics[i].getInMatrix(), Quadrics[i].getOutMatrix(), Quadrics[i].getTempMisalignmentMatrix(), Quadrics[i].getInverseTempMisalignmentMatrix(), Quadrics[i].getParameters());//, Quadrics[i].getInverseMisalignmentMatrix()
        }

        const clock_t begin_time = clock();
        tracer.run(); //run tracer
        std::cout << "tracer run time: " << float(clock() - begin_time) << " ms" << std::endl;

        DEBUG(std::cout << "run succeeded" << std::endl);

        DEBUG(std::cout << "tracerInterface run without output: " << float(clock() - all_begin_time) << " ms" << std::endl);

        //get rays from tracer
        auto outputRayIterator = tracer.getOutputIteratorBegin();
        // transform in to usable data
        auto doubleVecSize = RAY_MAX_ELEMENTS_IN_VECTOR * 8;
        std::vector<double> doubleVec(doubleVecSize);
        int index = 0;

        std::ofstream outputFile("output.csv");//, std::ios::app
        // outputFile.precision(17);
        if (SHORTOUTPUT) {
            outputFile << "Index;Xloc;Yloc\n";
        }
        else {
            outputFile << "Index;Xloc;Yloc;Zloc;Weight;Xdir;Ydir;Zdir;Energy\n";
        }

        for (; outputRayIterator != tracer.getOutputIteratorEnd(); outputRayIterator++) {
            // std::cout << "ray 16384 xpos: " << (*outputRayIterator)[16384].getxPos() << std::endl;
            // std::cout << "ray 16383 xpos: " << (*outputRayIterator)[16383].getxPos() << std::endl;
            // std::cout << "ray 16385 xpos: " << (*outputRayIterator)[16385].getxPos() << std::endl;
            // std::cout << "ray 16386 xpos: " << (*outputRayIterator)[16386].getxPos() << std::endl;


            DEBUG(std::cout << "(*outputRayIterator).size(): " << (*outputRayIterator).size() << std::endl);
            memcpy(doubleVec.data(), (*outputRayIterator).data(), (*outputRayIterator).size() * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double));

            doubleVec.resize((*outputRayIterator).size() * VULKANTRACER_RAY_DOUBLE_AMOUNT);
            std::cout << "tracerInterface: sample ray: " << doubleVec[0] << ", " << doubleVec[1] << ", " << doubleVec[2] << ", " << doubleVec[3] << ", " << doubleVec[4] << ", " << doubleVec[5] << ", " << doubleVec[6] << ", energy: " << doubleVec[7] << std::endl;
            writeToFile(doubleVec, outputFile, index);
            index = index + (*outputRayIterator).size();

        }
        outputFile.close();
        std::cout << "tracer run incl load rays time: " << float(clock() - begin_time) << " ms" << std::endl;


        // while (true) {
        //     int i = 1;
        // }
        std::cout << std::endl;
        //clean up tracer to avoid memory leaks
        tracer.cleanup();
        return true;
    }

    //writes rays to file
    void TracerInterface::writeToFile(std::list<double> outputRays) const
    {
        std::cout << "writing to file..." << std::endl;
        std::ofstream outputFile;
        outputFile.precision(17);
        std::string filename = "../../output/output.csv";
        outputFile.open(filename);
        char sep = ';'; // file is saved in .csv (comma seperated value), excel compatibility is manual right now
        outputFile << "Index" << sep << "Xloc" << sep << "Yloc" << sep << "Zloc" << sep << "Weight" << sep << "Xdir" << sep << "Ydir" << sep << "Zdir" << "Energy" << std::endl;
        // outputFile << "Index,Xloc,Yloc,Zloc,Weight,Xdir,Ydir,Zdir" << std::endl;

        size_t counter = 0;
        int print = 0;
        for (std::list<double>::iterator i = outputRays.begin(); i != outputRays.end(); i++) {
            if (counter % 8 == 0) {
                outputFile << counter / VULKANTRACER_RAY_DOUBLE_AMOUNT;
                if (print == 1) std::cout << ")" << std::endl;
                if (print == 1) std::cout << "(";
            }
            if (counter % 8 == 7) {
                outputFile << std::endl;
                counter++;
                continue;
            }
            outputFile << sep << *i;
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

    //writes rays to file
    void TracerInterface::writeToFile(const std::vector<double>& outputRays, std::ofstream& file, int index) const
    {
        size_t size = outputRays.size();

        DEBUG(std::cout << "writing " << outputRays.size() / 8 << " rays to file..." << std::endl);

        if (SHORTOUTPUT) {
            char buff[64];
            for (size_t i = 0; i < size; i = i + 8) { // ! + 8 because of placeholder
                sprintf(buff, "%d;%.17f;%.17f\n", index, outputRays[i], outputRays[i + 1]);
                file << buff;
                index++;
            }
        }
        else {
            char buff[256];
            for (size_t i = 0; i < size; i = i + 8) { // ! + 8 because of placeholder 
                sprintf(buff, "%d;%.17f;%.17f;%.17f;%.17f;%.17f;%.17f;%.17f;%.17f\n", index,
                    outputRays[i], outputRays[i + 1], outputRays[i + 2], outputRays[i + 3],
                    outputRays[i + 4], outputRays[i + 5], outputRays[i + 6], outputRays[i + 7]);
                file << buff;
                index++;
            }
        }

        DEBUG(std::cout << "done!" << std::endl);
    }

    //reads from file. datatype (RayType, QuadricType) needs to be set
    //pretty ugly, should be rewritten later
    // void TracerInterface::readFromFile(std::string path, m_dataType dataType)
    // {
    //     std::ifstream inputFile;
    //     inputFile.open(path);
    //     switch (dataType) {
    //     case TracerInterface::RayType: {
    //         std::vector<Ray> newRayVector;
    //         newRayVector.resize(RAY_MAX_ELEMENTS_IN_VECTOR);
    //         std::string line;
    //         std::getline(inputFile, line);
    //         //std::cout<<line<<std::endl;
    //         uint32_t numberOfRays = 0;
    //         while (!inputFile.eof()) {
    //             std::getline(inputFile, line);
    //             if (line[0] == '\0') {
    //                 break;
    //             }
    //             int i = 0;
    //             char currentNumber[32];
    //             Ray newRay(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), 0);
    //             std::vector<double> newDoubles;
    //             for (int k = 0; k < VULKANTRACER_RAY_DOUBLE_AMOUNT; k++) {
    //                 int j = 0;
    //                 while ((line[i] != ',') && (line[i] != '\0')) {
    //                     currentNumber[j] = line[i];
    //                     j++;
    //                     i++;
    //                 }
    //                 i++;
    //                 currentNumber[j] = '\0';
    //                 newDoubles.emplace_back(std::stof(currentNumber));
    //             }
    //             newRay.m_position.x = newDoubles[1];
    //             newRay.m_position.y = newDoubles[2];
    //             newRay.m_position.z = newDoubles[3];
    //             newRay.m_weight = newDoubles[4];
    //             newRay.m_direction.x = newDoubles[5];
    //             newRay.m_direction.y = newDoubles[6];
    //             newRay.m_direction.z = newDoubles[7];
    //             //std::cout<<newDoubles[0]<<newDoubles[1]<<newDoubles[2]<<newDoubles[3]<<newDoubles[4]<<newDoubles[5]<<newRay.m_direction.y<<newRay.m_direction.z<<"test"<<std::endl;
    //             newRayVector.push_back(newRay);
    //             numberOfRays++;
    //             if (numberOfRays > RAY_MAX_ELEMENTS_IN_VECTOR) {
    //                 m_RayList.push_back(newRayVector);
    //                 numberOfRays = 0;
    //                 newRayVector.clear();
    //                 newRayVector.resize(RAY_MAX_ELEMENTS_IN_VECTOR);
    //             }

    //         }
    //         break;
    //     }
    //     case TracerInterface::QuadricType: {
    //         //int i = 1;
    //         break;
    //     }
    //     }

    // }

}
// namespace RAY
