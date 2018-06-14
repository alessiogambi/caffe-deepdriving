////////////////////////////////////////////////
//
//  Gets frame/image and speed values from the input pipe and send driving commands to the output pipe.
//  This version does not rely on shared memory and can be used with any simulator which can provide the inputs as expected
//
////////////////////////////////////////////////

#include <glog/logging.h>

#include "caffe/caffe.hpp"
#include "caffe/util/db_leveldb.hpp"

#include "torcs/Arguments.hpp"
#include "torcs/Database.hpp"
#include "torcs/Semantic.hpp"
#include "torcs/DriveController.hpp"
#include "torcs/NeuralNet.hpp"

#include <opencv2/highgui/highgui.hpp>

#define ImageWidth  280
#define ImageHeight 210

using namespace caffe;
using std::string;

// Declare functions
bool processKeys();
int run(string ModelPath, string WeightsPath, string MeanPath, int Lanes,
		int GPUDevice, string nameOfInputPipe, string nameOfOutputPipe);

// Implement functions
int main(int argc, char** argv) {

	::google::InitGoogleLogging(argv[0]);

	string const ModelPath = getArgument(argc, argv, "--model");

	if (ModelPath.empty()) {
		std::cout << "Please define a path to the model description." << std::endl;
		std::cout << "Example: " << std::endl << std::endl;
		std::cout << argv[0]
				<< " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3"
				<< std::endl << std::endl;
		return -1;
	}

	string const WeightsPath = getArgument(argc, argv, "--weights");

	if (WeightsPath.empty()) {
		std::cout << "Please define a path to the model weights." << std::endl;
		std::cout << "Example: " << std::endl << std::endl;
		std::cout << argv[0]
				<< " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3"
				<< std::endl << std::endl;
		return -1;
	}

	string const MeanPath = getArgument(argc, argv, "--mean");

	if (MeanPath.empty()) {
		std::cout << "Please define a path to the mean file." << std::endl;
		std::cout << "Example: " << std::endl << std::endl;
		std::cout << argv[0]
				<< " --model pre_trained/modelfile.prototxt --weights pre_trained/weightsfile.binaryproto --mean pre_trained/meanfile.binaryproto --lanes 3"
				<< std::endl << std::endl;
		return -1;
	} else {
		std::cout << "Mean path " << MeanPath << std::endl;

	}

	string const LaneString = getArgument(argc, argv, "--lanes");

	if (LaneString.empty()) {
		std::cout << "Please specify the number of lanes (from 1 to 3)."
				<< std::endl;
		std::cout << "Example: " << std::endl << std::endl;
		std::cout << argv[0]
				<< " --data-path pre_trained/TORCS_Training_1F --lanes 3"
				<< std::endl << std::endl;
		return -1;
	}

	int Lanes = atoi(LaneString.c_str());

	if (Lanes < 1 || Lanes > 3) {
		std::cout << "Please specify the number of lanes (from 1 to 3)."
				<< std::endl;
		std::cout << "Example: " << std::endl << std::endl;
		std::cout << argv[0]
				<< " --data-path pre_trained/TORCS_Training_1F --lanes 3"
				<< std::endl << std::endl;
		return -1;
	}

	int GPUDevice = -1;
	string const GPUString = getArgument(argc, argv, "--gpu");

	if (!GPUString.empty()) {
		GPUDevice = atoi(GPUString.c_str());

		if (GPUDevice < 0) {
			GPUDevice = -1;
		}
	} else {
		std::cout
				<< "WARNING: GPU usage is disabled. Enable it with --gpu <DeviceNumber> or disable it explicitly with --gpu -1."
				<< std::endl;
	}

	string const nameOfInputPipe = getArgument(argc, argv, "--input");

	if (nameOfInputPipe.empty()) {
		std::cout
				<< "Please specify the location of input named pipe to retrieve images from "
				<< std::endl;
		return -1;
	}

	string const nameOfOutputPipe = getArgument(argc, argv, "--output");

	if (nameOfOutputPipe.empty()) {
		std::cout
				<< "Please specify the location of output named pipe to send driving controls to "
				<< std::endl;
		return -1;
	}

	return run(ModelPath, WeightsPath, MeanPath, Lanes, GPUDevice, nameOfInputPipe, nameOfOutputPipe);
}

int run(string ModelPath, string WeightsPath, string MeanPath, int Lanes,
		int GPUDevice, string nameOfInputPipe, string nameOfOutputPipe) {

	/*
	 * Note: Somehow this call prevents Semantic, i.e., the GUI, to be visualized as long as a client connects to the pipes
	 */
	CPipes TorcsMemory(nameOfInputPipe, nameOfOutputPipe);

	CSemantic Semantic;

	CDriveController DriveController;
	CNeuralNet NeuralNet(ModelPath, WeightsPath, MeanPath, GPUDevice);
	CErrorMeasurement ErrorMeas;

	Indicators_t * pGroundTruth = &TorcsMemory.Indicators;
	Indicators_t * pEstimatedIndicators = 0;
	Indicators_t EstimatedIndicators;

	bool IsEnd = false;

	Semantic.setFrameImage(&TorcsMemory.Image);

	Semantic.setAdditionalData(&TorcsMemory.TorcsData);
	Semantic.setErrorMeasurement(&ErrorMeas);
	Semantic.show(0, 0, false);

	// Main loop
	while (!IsEnd && TorcsMemory.isAvailable() ) {
		TorcsMemory.read();

		if (TorcsMemory.isDataUpdated()) {
			std::cout << "READ MEMORY" << std::endl;

			pEstimatedIndicators = &EstimatedIndicators;

			// This is the call to the CNN
			NeuralNet.process(pEstimatedIndicators, TorcsMemory.Image);

			// Not sure about this
			ErrorMeas.measure(&TorcsMemory.Indicators, pEstimatedIndicators);

			// Use the estimated indicators to compute a driving action
			DriveController.control(*pEstimatedIndicators, TorcsMemory.TorcsData, Lanes);

			Semantic.show(pGroundTruth, pEstimatedIndicators, true);

			std::cout << std::endl << "Estimated: " << std::endl;
			std::cout << "============= " << std::endl;
			EstimatedIndicators.print(std::cout);

			std::cout << "WRITE DRIVING COMMANDS TO MEMORY" << std::endl;
			std::cout << std::endl << "New Driving Actions: " << std::endl;
			std::cout << "Acc: : " << TorcsMemory.TorcsData.Accelerating
					<< std::endl;
			std::cout << "Brake: " << TorcsMemory.TorcsData.Breaking
					<< std::endl;
			std::cout << "Steer: " << TorcsMemory.TorcsData.Steering
					<< std::endl;
			// Write command back and disable the controller till the next image arrives
			TorcsMemory.write();
		}
		IsEnd = processKeys();

	}
	ErrorMeas.print(std::cout);
	NeuralNet.printTimeSummery(std::cout);

	return 0;
}

bool processKeys() {
	static const char EscKey = 27;
	static int KeyTime = 20;

	char Key = cvWaitKey(KeyTime);

	if (Key == EscKey) {
		return true;
	}

	return false;
}

