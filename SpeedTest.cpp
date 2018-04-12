#include <iostream>
#include <ctime>
#include <csignal>
#include "Spinnaker.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

bool flagLoop = true;

void run(CameraPtr pCam)
{
  // Initialize camera
  pCam->Init();

  // Retrieve GenICam nodemap
  INodeMap & nodeMap = pCam->GetNodeMap();

  CIntegerPtr ptrWidth = nodeMap.GetNode("Width");
  CIntegerPtr ptrHeight = nodeMap.GetNode("Height");
  CIntegerPtr ptrOffsetX = nodeMap.GetNode("OffsetX");
  CIntegerPtr ptrOffsetY = nodeMap.GetNode("OffsetY");

  // Set aqcuisition mode to continuous:
  pCam->AcquisitionMode.SetValue(AcquisitionModeEnums::AcquisitionMode_Continuous);

  CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
  CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");


  bool customSettings = false;

  int width = ptrWidth->GetMax();
  int height = ptrHeight->GetMax();
  string pixelFormat = "BayerRG8";

  if(customSettings) {
    width = 100;
    height = 100;
    pixelFormat = "Mono8";
  }

  // set width
  ptrWidth->SetValue(width);

  // set height
  ptrHeight->SetValue(height);

  // Retrieve the desired entry node from the enumeration node
  CEnumEntryPtr ptrPixelFormatNode = ptrPixelFormat->GetEntryByName(pixelFormat.c_str());
  // Retrieve the integer value from the entry node
  int64_t ptrPixelFormatValue = ptrPixelFormatNode->GetValue();
  // Set integer as new value for enumeration node
  ptrPixelFormat->SetIntValue(ptrPixelFormatValue);


  // Get camera device information.
  cout << "Camera Device Information" << endl
    << "=========================" << endl;
  cout << "Model            : "
    << CStringPtr( nodeMap.GetNode( "DeviceModelName") )->GetValue() << endl;
  cout << "Firmware version : "
    << CStringPtr( nodeMap.GetNode( "DeviceFirmwareVersion") )->GetValue() << endl;
  cout << "Serial number    : "
    << CStringPtr( nodeMap.GetNode( "DeviceSerialNumber") )->GetValue() << endl;
  cout << endl;

  // Camera settings
  cout << "Camera Device Settings" << endl << "======================" << endl;
  cout << "Acquisition mode : "
    << ptrAcquisitionMode->GetCurrentEntry()->GetSymbolic() << endl;
  cout << "Pixel format     : "
    << ptrPixelFormat->GetCurrentEntry()->GetSymbolic() << endl;
  cout << "Width            : "
    << ptrWidth->GetValue() << endl;
  cout << "Height           : "
    << ptrHeight->GetValue() << endl;
  cout << "Offset X         : "
    << ptrOffsetX->GetValue() << endl;
  cout << "Offset Y         : "
    << ptrOffsetY->GetValue() << endl;
  cout << endl;

  // Start aqcuisition
  pCam->BeginAcquisition();

  time_t timeBegin = time(0);
  int tick = 0;
  bool warmup = true;
  long frameCounter = 0;

  cout << "Real time fps measuring" << endl
    << "=========================" << endl;

  while (flagLoop)
  {
    ImagePtr pResultImage = pCam->GetNextImage();
    pResultImage->Release();

    frameCounter++;
    time_t timeNow = time(0) - timeBegin;

    if (timeNow - tick >= 1)
    {
      tick++;
      // skip first measurment as it will not have correct fps due to warm-up
      if(warmup == true) {
        warmup = false;
      }
      else {
        cout << "FPS: " << frameCounter << endl;
      }
      frameCounter = 0;
    }
  }

  // Deinitialize camera
  pCam->EndAcquisition();
  pCam->DeInit();
}

void raiseFlag(int param)
{
    flagLoop = false;
}

int main()
{
  // Catch interupt to stop test loop
  signal(SIGINT, raiseFlag);

  // Retrieve singleton reference to system object
  SystemPtr system = System::GetInstance();

  // Retrieve list of cameras from the system
  CameraList camList = system->GetCameras();

  if(camList.GetSize() == 0) {
    cerr << "No camera connected" << endl;
    return -1;
  }
  else {
    // Run tests on first available camera
    run(camList.GetByIndex(0));
  }

  // Clear camera list before releasing system
  camList.Clear();
  // Release system
  system->ReleaseInstance();
}
