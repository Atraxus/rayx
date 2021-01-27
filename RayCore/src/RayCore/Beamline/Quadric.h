#ifndef QUADRIC_H
#define QUADRIC_H

#include "Core.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#define RAYCORE_QUADRIC_DOUBLE_AMOUNT 48;
typedef std::vector<std::vector<double>> Matrix;
#include "utils.h"

namespace RAY {
    class RAY_API Quadric {
        public:
            Quadric(std::vector<double> inputPoints, std::vector<double> inputInMatrix, std::vector<double> inputOutMatrix, std::vector<double> misalignmentMatrix, std::vector<double> inverseMisalignmentMatrix);
            Quadric(std::vector<double> inputPoints, std::vector<double> parameters, double alpha, double chi, double beta, double distanceToPreceedingElement);
            
            std::vector<double> getQuadric();
            void setParameters(std::vector<double> params);
            void editQuadric(std::vector<double> inputPoints);
            std::vector<double> getAnchorPoints();
            void setInMatrix(std::vector<double> inputMatrix);
            void setOutMatrix(std::vector<double> inputMatrix);
            std::vector<double> getInMatrix();
            std::vector<double> getOutMatrix();
            std::vector<double> getMisalignmentMatrix();
            void calcTransformationMatrices(double alpha, double chi, double beta, double distanceToPreceedingElement);
            void setMisalignment(std::vector<double> misalignment);
            std::vector<double> getMatrixProduct(Matrix A, Matrix B);
            std::vector<double> getInverseMisalignmentMatrix();
    
            Quadric();
            ~Quadric();
        private:
            std::vector<double> m_anchorPoints;
            std::vector<double> m_misalignmentParams;
            std::vector<double> m_misalignmentMatrix;
            std::vector<double> m_inverseMisalignmentMatrix;
            std::vector<double> m_inMatrix;
            std::vector<double> m_outMatrix;
            std::vector<double> m_parameters;
    };
}
#endif