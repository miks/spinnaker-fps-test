#include <iostream>
#include <ctime>
#include <csignal>
#include "Spinnaker.h"
#include "cpptoml/cpptoml.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

bool run = true;
const string APPLICATION_NAME = "speed_test";

void RunTest(CameraPtr pCam, string config_file) {
  try {
    // Initialize camera
    pCam->Init();

    // Initialize configuration
    auto config = cpptoml::parse_file(config_file);

    // Retrieve GenICam node_map
    INodeMap & node_map = pCam->GetNodeMap();

    CIntegerPtr ptr_width = node_map.GetNode("Width");
    CIntegerPtr ptr_height = node_map.GetNode("Height");
    CIntegerPtr ptr_offset_x = node_map.GetNode("OffsetX");
    CIntegerPtr ptr_offset_y = node_map.GetNode("OffsetY");
    CFloatPtr   ptr_exposure_time = node_map.GetNode("ExposureTime");

    CEnumerationPtr ptr_pixel_format = node_map.GetNode("PixelFormat");
    CEnumerationPtr ptr_acquisition_mode = node_map.GetNode("AcquisitionMode");
    CEnumerationPtr ptr_auto_exposure = node_map.GetNode("ExposureAuto");
    CEnumerationPtr ptr_auto_gain = node_map.GetNode("GainAuto");
    CEnumerationPtr ptr_adc_bit_depth = node_map.GetNode("AdcBitDepth");

    int width = config->get_qualified_as<int>("camera.width").value_or(0);
    int height = config->get_qualified_as<int>("camera.height").value_or(0);
    int offset_x = config->get_qualified_as<int>("camera.offset_x").value_or(0);
    int offset_y = config->get_qualified_as<int>("camera.offset_y").value_or(0);
    int exposure_time = config->get_qualified_as<int>("camera.exposure_time").value_or(0);
    string pixel_format = config->get_qualified_as<string>("camera.pixel_format").value_or("");
    string acquisition_mode = config->get_qualified_as<string>("camera.acquisition_mode").value_or("");
    string auto_exposure = config->get_qualified_as<string>("camera.auto_exposure").value_or("");
    string auto_gain = config->get_qualified_as<string>("camera.auto_gain").value_or("");
    string adc_bit_depth = config->get_qualified_as<string>("camera.adc_bit_depth").value_or("");

    // set width
    ptr_width->SetValue(width);

    // set height
    ptr_height->SetValue(height);

    // set x offset
    ptr_offset_x->SetValue(offset_x);

    // set y offset
    ptr_offset_y->SetValue(offset_y);

    // Set auto gain mode
    CEnumEntryPtr ptr_auto_gain_node = ptr_auto_gain->GetEntryByName(auto_gain.c_str());
    ptr_auto_gain->SetIntValue(ptr_auto_gain_node->GetValue());

    // Set adc bit depth
    CEnumEntryPtr ptr_adc_bit_depth_node = ptr_adc_bit_depth->GetEntryByName(adc_bit_depth.c_str());
    ptr_adc_bit_depth->SetIntValue(ptr_adc_bit_depth_node->GetValue());

    // Set aqcuisition mode
    CEnumEntryPtr ptr_acquisition_mode_node = ptr_acquisition_mode->GetEntryByName(acquisition_mode.c_str());
    ptr_acquisition_mode->SetIntValue(ptr_acquisition_mode_node->GetValue());

    // set pixel format
    CEnumEntryPtr ptr_pixel_format_node = ptr_pixel_format->GetEntryByName(pixel_format.c_str());
    ptr_pixel_format->SetIntValue(ptr_pixel_format_node->GetValue());

    // set auto exposure mode
    CEnumEntryPtr ptr_auto_exposure_node = ptr_auto_exposure->GetEntryByName(auto_exposure.c_str());
    ptr_auto_exposure->SetIntValue(ptr_auto_exposure_node->GetValue());

    // set exposure time
    ptr_exposure_time->SetValue(exposure_time); // pass value in microseconds

    // Get camera device information.
    cout << "Camera device information" << endl
      << "=========================" << endl;
    cout << "Model             : "
      << CStringPtr( node_map.GetNode( "DeviceModelName") )->GetValue() << endl;
    cout << "Firmware version  : "
      << CStringPtr( node_map.GetNode( "DeviceFirmwareVersion") )->GetValue() << endl;
    cout << "Serial number     : "
      << CStringPtr( node_map.GetNode( "DeviceSerialNumber") )->GetValue() << endl;
    cout << "Max resolution    : "
      << ptr_width->GetMax() << " x " << ptr_height->GetMax() << endl;
    cout << "Min exposure time : "
      << ptr_exposure_time->GetMin() << endl;
    cout << endl;

    // Camera settings
    cout << "Camera device settings" << endl << "======================" << endl;
    cout << "Acquisition mode  : "
      << ptr_acquisition_mode->GetCurrentEntry()->GetSymbolic() << endl;
    cout << "Pixel format      : "
      << ptr_pixel_format->GetCurrentEntry()->GetSymbolic() << endl;
    cout << "ADC bit depth     : "
      << ptr_adc_bit_depth->GetCurrentEntry()->GetSymbolic() << endl;
    cout << "Auto gain         : "
      << ptr_auto_gain->GetCurrentEntry()->GetSymbolic() << endl;
    cout << "Auto exposure     : "
      << ptr_auto_exposure->GetCurrentEntry()->GetSymbolic() << endl;
    cout << "Exposure time     : "
      << ptr_exposure_time->GetValue() << endl;
    cout << "Width             : "
      << ptr_width->GetValue() << endl;
    cout << "Height            : "
      << ptr_height->GetValue() << endl;
    cout << "Offset X          : "
      << ptr_offset_x->GetValue() << endl;
    cout << "Offset Y          : "
      << ptr_offset_y->GetValue() << endl;
    cout << endl;

    // Start aqcuisition
    pCam->BeginAcquisition();

    time_t time_begin = time(0);
    int tick = 0;
    long frame_counter = 0;

    cout << "Camera fps measuring" << endl
      << "====================" << endl;

    while (run) {
      ImagePtr pResultImage = pCam->GetNextImage();
      pResultImage->Release();

      frame_counter++;
      time_t timeNow = time(0) - time_begin;

      if (timeNow - tick >= 1) {
        // skip first measurment as it will not have correct fps due to warm-up
        if(tick > 0)
          cout << frame_counter << "fps" << endl;

        tick++;
        frame_counter = 0;
      }
    }

    cout << endl;

    // Deinitialize camera
    pCam->EndAcquisition();
    pCam->DeInit();
  }
  catch (Spinnaker::Exception &e) {
    cout << "Error: " << e.what() << endl;
  }
}

void stop(int param) {
  run = false;
}

int main(int argc, char **argv) {
  // Catch interupt to stop test loop
  signal(SIGINT, stop);

  // Check if config file path is passed as argument
  if (argc != 2) {
    cout << "usage: ";
    cout << string(APPLICATION_NAME);
    cout << " /path/to/config.toml" << endl;
    exit(1);
  }

  // Assign config file value as string
  string config_file = string(argv[1]);

  // Check whether config file is readable
  if (access(config_file.c_str(), R_OK) == -1) {
    cerr << "Config file is not readable" << endl;
    exit(0);
  }

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
    RunTest(camList.GetByIndex(0), config_file);
  }

  // Clear camera list before releasing system
  camList.Clear();
  // Release system
  system->ReleaseInstance();
}
