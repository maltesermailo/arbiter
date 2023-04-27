#include "browser_state.h"

int BrowserState::GetBrowserIdentifier() {
  return this->_browser_id;
}

bool BrowserState::IsScreenshotDone() {
  return this->_screenshotDone;
}

int BrowserState::GetWidth() {
  return this->width;
}

int BrowserState::GetHeight() {
  return this->height;
}

void BrowserState::SetScreenshotDone(bool value) {
  this->_screenshotDone = value;
}

void BrowserState::SetDimensions(int widthIn, int heightIn) {
  this->width = widthIn;
  this->height = heightIn;
}

void BrowserState::SetBuffer(const void* data, size_t length, int widthIn, int heightIn) {
  this->buffer = data;
  this->bufferLength = length;
  this->bufferWidth = widthIn;
  this->bufferHeight = heightIn;
}

const void* BrowserState::GetBuffer(size_t& length, int& widthOut, int& heightOut) {
  length = bufferLength;
  widthOut = bufferWidth;
  heightOut = bufferHeight;

  return this->buffer;
}