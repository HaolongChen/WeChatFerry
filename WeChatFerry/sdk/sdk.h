#pragma once

extern "C" {
__declspec(dllexport) int WxInitSDK(bool debug, int port, int *status);
__declspec(dllexport) int WxDestroySDK();
}
