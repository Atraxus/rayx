#pragma once
#pragma pack(16)

#include <stdexcept>
#include <iostream>
#include <vector>



class Ray
{
public:
	Ray(double xpos, double ypos, double zpos, double xdir, double ydir, double zdir, double w);
	Ray(double* location);
	Ray();
	~Ray();
	std::vector<double> getRayInformation();
	double getxDir();
	double getyDir();
	double getzDir();
	double getxPos();
	double getyPos();
	double getzPos();
	double getWeight();

private:
	struct vec3
	{
		double x, y, z;
	};
	vec3 position;
	double weight;
	vec3 direction;
	double placeholder;
};
