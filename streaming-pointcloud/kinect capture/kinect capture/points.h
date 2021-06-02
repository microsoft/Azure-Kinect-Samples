#pragma once

#include <vector>
//Ensuring that only code licenced under MPL2.0 is used
//Will cause a compiler error if trying to access code that is not MPL2.0
#define EIGEN_MPL2_ONLY
#include <eigen3/Eigen/Geometry>

class points {
public: //Functions
	points();
	~points();
	bool importPoints(std::vector<Eigen::Vector3f>& newPoints);	
	void translate(Eigen::Vector3f& t);
	void rotate(Eigen::Vector3f& rot);
	void threshold(Eigen::Vector3f& min, Eigen::Vector3f& max);
	void translateRotateThreshold(Eigen::Vector3f& t, Eigen::Vector3f& rot, Eigen::Vector3f& min, Eigen::Vector3f& max);
	std::vector<Eigen::Vector3f>& give_points();
	void sparseFilter(int& width, int& height);

private: //Variables
	std::vector<Eigen::Vector3f> current_points;
};