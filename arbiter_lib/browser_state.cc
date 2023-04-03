#include "browser_state.h"

int BrowserState::GetBrowserIdentifier() {
  return this->_browser_id;
}

bool BrowserState::IsScreenshotDone() {
  return this->_screenshotDone;
}

void BrowserState::SetScreenshotDone(bool value) {
  this->_screenshotDone = value;
}

void BrowserState::SetDimensions(int widthIn, int heightIn) {
  this->width = widthIn;
  this->height = heightIn;
}