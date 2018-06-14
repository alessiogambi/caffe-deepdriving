/**
 * Semantic.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Andre Netzeband
 *
 *  Attention: This is a reimplementation of the code the DeepDriving project.
 *  See http://deepdriving.cs.princeton.edu for more details.
 *  Thus much code comes from Chenyi Chen.
 *
 *  Take the original DeepDriving license into account!
 */

#include "Pipes.hpp"

#include <glog/logging.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <cstring>

#include <sys/stat.h>

#define TORCS_IMAGE_WIDTH   640
#define TORCS_IMAGE_HEIGHT  480
#define RESIZE_IMAGE_WIDTH  280
#define RESIZE_IMAGE_HEIGHT 210

typedef struct {
	int written;  //a label, if 1: available to read, if 0: available to write

	uint8_t data[TORCS_IMAGE_WIDTH * TORCS_IMAGE_HEIGHT * 3]; // image data field

//	int control;
//	int pause;
//	double fast;

	// Indicators and other values?
	double dist_L;
	double dist_R;

	double toMarking_L;
	double toMarking_M;
	double toMarking_R;

	double dist_LL;
	double dist_MM;
	double dist_RR;

	double toMarking_LL;
	double toMarking_ML;
	double toMarking_MR;
	double toMarking_RR;

	double toMiddle;
	double angle;
	double speed;

	double steerCmd;
	double accelCmd;
	double brakeCmd;
} SharedMemoryLayout_t;

CPipes::CPipes(std::string nameOfInputPipe, std::string nameOfOutputPipe) :
		fin(nameOfInputPipe.c_str()), fout(nameOfOutputPipe.c_str()), FIN(fin), FOUT(
				fout) {
	IsAvailable = true;
	IsDataUpdated = false;
	initPipes();

}

CPipes::~CPipes() {
	// No need to explicitly close the streams
}

void CPipes::initPipes() {
	// Initialize the internal objects
	Indicators.Angle = 0;
	Indicators.Fast = 0;

	Indicators.DistanceToLeftObstacle = Indicators.MaxObstacleDist;
	Indicators.DistanceToRightObstacle = Indicators.MaxObstacleDist;

	Indicators.DistanceToLeftMarking = Indicators.MaxL;
	Indicators.DistanceToCenterMarking = Indicators.MaxM;
	Indicators.DistanceToRightMarking = Indicators.MaxR;

	Indicators.DistanceToLeftObstacleInLane = Indicators.MaxObstacleDist;
	Indicators.DistanceToCenterObstacleInLane = Indicators.MaxObstacleDist;
	Indicators.DistanceToRightObstacleInLane = Indicators.MaxObstacleDist;

	Indicators.DistanceToLeftMarkingOfLeftLane = Indicators.MaxLL;
	Indicators.DistanceToLeftMarkingOfCenterLane = -Indicators.LaneWidth / 2;
	Indicators.DistanceToRightMarkingOfCenterLane = Indicators.LaneWidth / 2;
	Indicators.DistanceToRightMarkingOfRightLane = Indicators.MaxRR;

	// Input data
	Image.setNoVideo(RESIZE_IMAGE_WIDTH, RESIZE_IMAGE_HEIGHT);
	TorcsData.Speed = 0;

	TorcsData.IsNotPause = true;
	TorcsData.ShowGroundTruth = false;

	TorcsData.IsControlling = true;
	TorcsData.IsAIControlled = true;

	TorcsData.Accelerating = 0;
	TorcsData.Breaking = 0;
	TorcsData.Steering = 0;



	//
	mknod(fin, S_IFIFO | 0666, 0);
	mknod(fout, S_IFIFO | 0666, 0);
}

// Read from the input pipe if anything and fill up the TORCS DATA structure
void CPipes::read() {
	if (FIN.is_open()) {
		std::string line;
		getline(FIN, line);

		if (!FIN.eof() && line.length() > 0) {

			std::cout << "Got new input " << std::endl;
			// Read speed value
			auto speed = ::atof(line.c_str());
			std::cout << "Speed: " << speed << std::endl;

			if (speed < 0) {
				// This is the signal we have to go out
				IsAvailable = false;
				return;
			}

			// Read Image size
			getline(FIN, line);
			auto data_size = std::stoi(line);
			std::cout << "IMG Size: " << data_size << std::endl;
			// Read Image Data
			std::string data;
			{
				std::vector<char> buf(data_size);
				// Read all the bytes here
				FIN.read(buf.data(), data_size);
				// write to vector data is valid since C++11
				data.assign(buf.data(), buf.size());
			}
			// FIXME Handle exceptions ?
//			if (!FIN.good()) {
//				std::cerr << "Read failed" << std::endl;
//				// TODO How to break ?!
//			}

			const char* imageData = data.c_str();

			// This is enough to have the Image stored and accessible to controller
			Image.readFromMemory((uint8_t*) imageData, TORCS_IMAGE_WIDTH,
			TORCS_IMAGE_HEIGHT,
			RESIZE_IMAGE_WIDTH, RESIZE_IMAGE_HEIGHT);
			TorcsData.Speed = speed;

			std::cout << "Done reading new input data" << std::endl;

			IsDataUpdated = true;

		} else {
			IsDataUpdated = false;
		}
	} else {
		std::cout << "Input Pipe is closed " << std::endl;
	}
}

// Write from to TORCS DATA structure to the output pipe only the control values
void CPipes::write() {
	if (FOUT.is_open()) {
		// The C++11 way:
		std::string doubleAsString = std::to_string(TorcsData.Accelerating);
		FOUT << doubleAsString << std::endl;

		doubleAsString = std::to_string(TorcsData.Breaking);
		FOUT << doubleAsString << std::endl;

		doubleAsString = std::to_string(TorcsData.Steering);
		FOUT << doubleAsString << std::endl;

		std::cout << "Done sending output commands " << std::endl;
	} else {
		std::cout << "Output Pipe is closed " << std::endl;
	}
}

bool CPipes::isDataUpdated() {
	return IsDataUpdated;
}

// Ideally this should close and stop the loop as soon as the sender dies
bool CPipes::isAvailable() {
	return IsAvailable && FIN.is_open() && FOUT.is_open();
}

