#pragma once
#include <functional>
#include <vector>

#include "RenderObject.h"
#include "Tracer/Tracer.h"
#define MAX_RAYS 1000

// Definition of the RayFilterFunction type
using RayFilterFunction = std::function<std::vector<size_t>(const RAYX::BundleHistory&, size_t)>;

/**
 * @brief Generates visual representations of rays based on bundle history and optical elements.
 * @param bundleHist RAYX-Core type, providing details of ray interactions in the beamline.
 * @param elements A vector of optical elements used for coordinate conversions.
 * @return A vector of lines, which visually represents the paths of rays in the beamline.
 */

void displayFilterSlider(int* amountOfRays, int maxAmountOfRays);

std::vector<Line> getRays(const RAYX::BundleHistory& bundleHist, const std::vector<RAYX::OpticalElement>& elements, RayFilterFunction filterFunction,
                          int amountOfRays);

std::vector<size_t> findMostCentralRays(const std::vector<std::vector<double>>& features, const std::vector<size_t>& clusterAssignments,
                                        const std::vector<std::vector<double>>& centroids, size_t k);

std::vector<size_t> kMeansFilter(const RAYX::BundleHistory& bundleHist, size_t k);

float euclideanDistance(const std::vector<float>& a, const std::vector<float>& b);

void initializeCentroids(std::vector<std::vector<float>>& centroids, const std::vector<std::vector<float>>& features, size_t k);

std::pair<std::vector<size_t>, std::vector<std::vector<float>>> kMeansClustering(const std::vector<std::vector<float>>& features, size_t k);
