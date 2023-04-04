#pragma once

#ifndef BROWSER_STATE_H_
#define BROWSER_STATE_H_
#include <string>
#include <semaphore>

class BrowserState {
 public:
  BrowserState(int _browser_id) : _browser_id(_browser_id) {}

  int GetBrowserIdentifier();
  bool IsScreenshotDone();

  int GetWidth();
  int GetHeight();

  void SetScreenshotDone(bool value);
  void SetDimensions(int width, int height);
  void SetBuffer(void* data, int length);
  void* GetBuffer(int& length);

  std::string currentUrl;
  //Notification semaphore for JS execution
  std::binary_semaphore notify = std::binary_semaphore(1);
  //Notification semaphore for Page Load
  std::binary_semaphore notifyLoad = std::binary_semaphore(1);

  bool _error = false;

 private:
  int _browser_id;
  bool _screenshotDone = false;

  int width = 1920;
  int height = 1080;

  void* buffer;
  int bufferLength;
};

#endif  // !BROWSER_STATE_H_
