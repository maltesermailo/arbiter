#include "browser_state.h"
#include <iostream>

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

void BrowserState::SetBuffer(void* data, size_t length, int widthIn, int heightIn) {
  if (this->bufferLength > 0) {
    delete[] buffer;
  }

  std::cout << "Setting buffer" << std::endl;

  this->buffer = data;
  this->bufferLength = length;
  this->bufferWidth = widthIn;
  this->bufferHeight = heightIn;
}

void BrowserState::ClearBuffer() {
  if (this->bufferLength > 0) {
    delete[] buffer;
  }

  this->bufferLength = 0;
  this->bufferWidth = 0;
  this->bufferHeight = 0;
}

void* BrowserState::GetBuffer(size_t& length, int& widthOut, int& heightOut) {
  length = bufferLength;
  widthOut = bufferWidth;
  heightOut = bufferHeight;

  return this->buffer;
}