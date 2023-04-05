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

void BrowserState::SetBuffer(const void* data, size_t length, int width, int height) {
  this->buffer = data;
  this->bufferLength = length;
}

const void* BrowserState::GetBuffer(size_t& length, int& width, int& height) {
  length = bufferLength;
  width = bufferWidth;
  height = bufferHeight;

  return this->buffer;
}