#pragma once

#ifndef BROWSER_STATE_H_
#define BROWSER_STATE_H_
#include <string>

class BrowserState {
 public:
  BrowserState(int _browser_id) : _browser_id(_browser_id) {}

  int GetBrowserIdentifier();
  bool IsScreenshotDone();
  void SetScreenshotDone(bool value);
  void SetDimensions(int width, int height);

  std::string currentUrl;

 private:
  int _browser_id;
  bool _screenshotDone = false;

  int width = 1920;
  int height = 1080;
};

#endif  // !BROWSER_STATE_H_
