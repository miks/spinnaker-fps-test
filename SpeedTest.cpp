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
  CFloatPtr   ptrExposureTime = nodeMap.GetNode("ExposureTime");

  // Set aqcuisition mode to continuous:
  pCam->AcquisitionMode.SetValue(AcquisitionModeEnums::AcquisitionMode_Continuous);

  CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
  CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
  CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");

  bool customSettings = false;

  int width = ptrWidth->GetMax();
  int height = ptrHeight->GetMax();
  float exposureTime = ptrExposureTime->GetMin();
  string pixelFormat = "BayerRG8";
  string autoExposure = "Off";

  if(customSettings) {
    width = 100;
    height = 100;
    exposureTime = 4;
    pixelFormat = "Mono8";
  }

  // set exposure time
  ptrExposureTime->SetValue(exposureTime);

  // set width
  ptrWidth->SetValue(width);

  // set height
  ptrHeight->SetValue(height);

  //
  // set pixel format
  //
  // Retrieve the desired entry node from the enumeration node
  CEnumEntryPtr ptrPixelFormatNode = ptrPixelFormat->GetEntryByName(pixelFormat.c_str());
  // Set integer as new value for enumeration node
  ptrPixelFormat->SetIntValue(ptrPixelFormatNode->GetValue());

  //
  // set auto exposure mode
  //
  // Retrieve the desired entry node from the enumeration node
  CEnumEntryPtr ptrExposureAutoNode = ptrExposureAuto->GetEntryByName(autoExposure.c_str());
  // Set integer as new value for enumeration node
  ptrExposureAuto->SetIntValue(ptrExposureAutoNode->GetValue());

  // Get camera device information.
  cout << "Camera device information" << endl
    << "=========================" << endl;
  cout << "Model            : "
    << CStringPtr( nodeMap.GetNode( "DeviceModelName") )->GetValue() << endl;
  cout << "Firmware version : "
    << CStringPtr( nodeMap.GetNode( "DeviceFirmwareVersion") )->GetValue() << endl;
  cout << "Serial number    : "
    << CStringPtr( nodeMap.GetNode( "DeviceSerialNumber") )->GetValue() << endl;
  cout << endl;

  // Camera settings
  cout << "Camera device settings" << endl << "======================" << endl;
  cout << "Acquisition mode : "
    << ptrAcquisitionMode->GetCurrentEntry()->GetSymbolic() << endl;
  cout << "Pixel format     : "
    << ptrPixelFormat->GetCurrentEntry()->GetSymbolic() << endl;
  cout << "Auto exposure    : "
    << ptrExposureAuto->GetCurrentEntry()->GetSymbolic() << endl;
  cout << "Exposure time    : "
    << ptrExposureTime->GetValue() << endl;
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

  cout << "Camera fps measuring" << endl
    << "====================" << endl;

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
        cout << frameCounter << "fps" << endl;
      }
      frameCounter = 0;
    }
  }

  cout << endl;

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
