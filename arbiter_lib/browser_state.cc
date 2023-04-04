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

void BrowserState::SetBuffer(void* data, int length) {
  this->buffer = data;
  this->bufferLength = length;
}

void* BrowserState::GetBuffer(int& length) {
  length = bufferLength;

  return this->buffer;
}