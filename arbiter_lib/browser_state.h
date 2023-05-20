#pragma once

#ifndef BROWSER_STATE_H_
#define BROWSER_STATE_H_
#include <string>
#include <semaphore>

class BrowserState {
 public:
  BrowserState(int _browser_id) : _browser_id(_browser_id), buffer(0), bufferLength(0), bufferWidth(0), bufferHeight(0) {}

  int GetBrowserIdentifier();
  bool IsScreenshotDone();

  int GetWidth();
  int GetHeight();

  void SetScreenshotDone(bool value);
  void SetDimensions(int width, int height);
  void SetBuffer(void* data, size_t length, int width, int height);
  void ClearBuffer();
  void* GetBuffer(size_t& length, int& width, int& height);

  std::string currentUrl;
  //Notification semaphore for JS execution
  std::binary_semaphore notify = std::binary_semaphore(0);
  //Notification semaphore for Page Load
  std::binary_semaphore notifyLoad = std::binary_semaphore(0);

  bool _error = false;

  // Represents the last time a resource has been loaded
  unsigned long lastLoadTimeMillis = 0;

 private:
  int _browser_id;
  bool _screenshotDone = false;

  int width = 1920;
  int height = 1080;

  int bufferWidth;
  int bufferHeight;

  void* buffer;
  size_t bufferLength;
};

#endif  // !BROWSER_STATE_H_
