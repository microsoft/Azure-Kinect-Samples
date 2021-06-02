#include "points.h"
#include <iostream>
//-----Points default contstructor------//
/*
This constructor will set flat to false (used to signify if flat points have been made) and
reserve the previous points to hold up to 10 frames for averaging the amount of points
*/
points::points() {
	this->current_points;	
}

//-----Points default deconstructor------//
/*
This deconstructor will free all memory and then set their points to 
their 0 respective value
*/
points::~points() {
	this->current_points.clear();
	this->current_points.shrink_to_fit();
}

//-----importPoints------//
/*
This will go through and import the points from 3 different types which will be a vector of either:
Eigen::Vector3f

It will first check to see that the new points is not an empty list,
then it will call add previous to get the current point added into the queue (this will have the first frame in there twice
until it is cleared past the frame average points),
Then it will set the current points to the newPoints value by converting to the
std::vector<Eigen::Vector3f> that is used for current points.

Returns true if succeeded, which means it doesn't recieve a nonempty amount of points from newPoints
*/
bool points::importPoints(std::vector<Eigen::Vector3f>& newPoints) {
	if (newPoints.size() == 0) {
		return false;
	}

	this->current_points = newPoints;

	return true;
}

//-----translation------//
/*
Translates all values in this->current) points
*/
void points::translate(Eigen::Vector3f& t) {
	for (auto&& i : this->current_points) {
		i = i - t;
	}
}

//-----rotate------//
/*
This function Will rotate the points around the x,y,z axis separately or all at once depending on the use case.
*/
void points::rotate(Eigen::Vector3f& rot) {
	//Get the degrees to rotate by
	float anglex = (float)0.0174533 * rot.x();
	float angley = (float)0.0174533 * rot.y();
	float anglez = (float)0.0174533 * rot.z();

	//Get the rotations for each axis
	Eigen::AngleAxisf xrot(anglex, Eigen::Vector3f::UnitX());
	Eigen::AngleAxisf yrot(angley, Eigen::Vector3f::UnitY());
	Eigen::AngleAxisf zrot(anglez, Eigen::Vector3f::UnitZ());

	//Set all the rotations into a single quaterniond and pass to the cuda function
	Eigen::Quaternionf quat = xrot * yrot * zrot;

	for (auto&& i : this->current_points) {
		i = quat * i;
	}


}

//-----threshold------//
/*
This function will apply a boundrary onto the 3d points and set them to origin if they are out of bounds.
Points at origin are considered invalid and will be discarded when sent over mqtt, they are not removed to maintain ordering
*/
void points::threshold(Eigen::Vector3f& min, Eigen::Vector3f& max) {
	Eigen::Vector3f allZero(0, 0, 0);
	float* minArr = min.data();
	float* maxArr = max.data();
	for (auto&& i : this->current_points) {
		float* idata = i.data();
		if (i[0] < minArr[0] || i[0] > maxArr[0] || i[1] < minArr[1] || i[1] > maxArr[1] || i[2] < minArr[2] || i[2] > maxArr[2]) {
			i = allZero;
		}
	}
}

//-----threshold------//
/*
This function is the translate, rotate, and threshold functions all combined into one function call.
Sets points to origin if they are out of the thresholding cube, points at origin are considered invalid and
will not be sent over mqtt. They are not deleted from the vector to maintain ordering
*/
void points::translateRotateThreshold(Eigen::Vector3f& t, Eigen::Vector3f& rot, Eigen::Vector3f& min, Eigen::Vector3f& max) {
	//Get the degrees to rotate by
	float anglex = (float)0.0174533 * rot.x();
	float angley = (float)0.0174533 * rot.y();
	float anglez = (float)0.0174533 * rot.z();

	//Get the rotations for each axis
	Eigen::AngleAxisf xrot(anglex, Eigen::Vector3f::UnitX());
	Eigen::AngleAxisf yrot(angley, Eigen::Vector3f::UnitY());
	Eigen::AngleAxisf zrot(anglez, Eigen::Vector3f::UnitZ());

	//Set all the rotations into a single quaterniond and pass to the cuda function
	Eigen::Quaternionf const quat = xrot * yrot * zrot;

	Eigen::Vector3f allZero(0, 0, 0);

	float* minArr = min.data();
	float* maxArr = max.data();
	for (auto&& i : this->current_points) {
		i -= t;
		i = quat * i;
		float* idata = i.data();
		if (i[0] < minArr[0] || i[0] > maxArr[0] || i[1] < minArr[1] || i[1] > maxArr[1] || i[2] < minArr[2] || i[2] > maxArr[2]) {
			i = allZero;
		}
	}
}


//-----give_points------//
/*
This will return the current points in a vector of Eigen::Vector3f points
*/
std::vector<Eigen::Vector3f>& points::give_points() {
	return this->current_points;
}

//-----sparseFilter------//
/*
This function will go through and see if there at least [validNeightbor]s around a point. If there is not, the point is considered an error
in the camera and set to origin and is considered invalid.
*/
void points::sparseFilter(int& width, int& height) {
	static int validNeighbor = 4;
	static int size = width * height;
	Eigen::Vector3f allZero(0, 0, 0);
	for (int i = 0; i < size; ++i) {
		//Check that the point is not already invalid
		//If it isn't get the x,y in relation to the image
		if (this->current_points[i] != allZero) {
			int idxwidth = i % width;
			int idxheight = i / width;

			//Make sure its not around the border of the image
			if (0 < idxwidth && idxwidth < width - 1 && 0 < idxheight && idxheight < height - 1) {
				int neighborCount = 0;
				//Get the neighbors
				int neighbors[8] = {
					((width * (idxheight - 1)) + idxwidth - 1), (idxwidth + (width * (idxheight - 1))), (idxwidth + 1 + (width * (idxheight - 1))),
					(idxwidth - 1 + (width * idxheight)), (idxwidth + 1 + (width * idxheight)),
					(idxwidth - 1 + (width * (idxheight + 1))), (idxwidth + (width * (idxheight + 1))), (idxwidth + 1 + (width * (idxheight + 1)))
				};
				//Count to see how many neighbors are valid
				for (auto&& j : neighbors) {
					if (this->current_points[j] != allZero) {
						++neighborCount;
					}
				}
				//If the point doesn't have enough valid neighbors, that point is invalid
				if (neighborCount < validNeighbor) {
					this->current_points[i] = allZero;
				}
			}
		}
	}
}