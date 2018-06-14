// No idea whats' this...
#ifndef PIPES_HPP_
#define PIPES_HPP_

#include "Indicators.hpp"
#include "Image.hpp"

typedef struct
{
  bool   IsControlling;
  bool   IsNotPause;
  double Speed;
  double Steering;
  double Accelerating;
  double Breaking;
  bool   IsAIControlled;
  bool   ShowGroundTruth;
  bool   IsRecording;
} TorcsData_t;

class CPipes
{
  public:
    /// @brief Constructor.
    CPipes(std::string nameOfInputPipe, std::string nameOfOutputPipe);

    /// @brief Destructor.
    ~CPipes();

    /// @brief Try a read from the input pipe. This shall read speed value, amount of bytes for the image, image bytes
    void read();

    /// @brief Writes the driving commands to the pipe. This shall write acc, brake, and steer.
    void write();

    // @brief Mark the presence of new data after we try a read
    bool isDataUpdated();
    // Stop the process when the input pipe closes
    bool isAvailable();

    /// @brief The indicators from the shared memory.
    // Is this for the training ?
    Indicators_t Indicators;

    /// @brief The other data which comes from torcs.
    TorcsData_t TorcsData;

    /// @brief The image coming from the input pipe.
    CImage Image;

  private:
    const char* fin;
    const char* fout;
    std::ifstream FIN;
	std::ofstream FOUT;

//    void * pMemory;
//    int    SharedMemoryID;
    bool   IsDataUpdated;
    bool   IsAvailable;

    // void attach();
    // void detach();
    void initPipes();
};

#endif /* SHAREDMEMORY_HPP_ */
