#include "arbiter.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "libpng/png.h"
#include <filesystem>

Arbiter::~Arbiter() {}

void Arbiter::ParseSpiderFile() {
  Log("Parsing spider file...");

  this->spiderFile = YAML::LoadFile("spider.yaml");

  YAML::Node pathsNode = this->spiderFile["paths"];
  YAML::const_iterator iter;

  Log(std::format(
      "[Arbiter] Loading %d urls...",
      pathsNode.size()));

  for (iter = pathsNode.begin(); iter != pathsNode.end(); ++iter) {
    std::string url = iter->as<std::string>();

    Log(std::format("[Arbiter] Adding %s...", url));
    this->AddURL(url);
  }
}

void Arbiter::PrepareData(std::string dataDirIn, int lastRunIn) {
  this->dataDirPath = dataDirIn;

  if (!std::filesystem::exists(dataDirIn)) {
    throw "Data directory at path " + dataDirIn + " doesn't exist!";
  }

  for (auto const& url : this->urls) {
    std::filesystem::path filepath = std::string(dataDirIn)
                                         .append("/")
                                         .append(std::to_string(lastRun))
                                         .append("/")
                                         .append(url)
                                         .append(".png");
    if (!std::filesystem::exists(filepath)) {
      logFile << "[Arbiter] "
           << "Warning! File " << url
           << ".png has no screenshot. A new screenshot will be taken and "
              "saved but not compared" << endl;
    }
  }

  this->lastRun = lastRunIn;
  this->currentRun = this->lastRun + 1;

  if (!std::filesystem::create_directory(
          std::string(dataDirIn)
              .append("/")
              .append(std::to_string(this->currentRun))
              .append("/"))) {
    throw "Can't create data directory at path " + dataDirIn + "/" +
        std::to_string(this->currentRun);
  }
}

void Arbiter::TakeScreenshot(CefRefPtr<CefBrowser> browser, std::shared_ptr<BrowserState> state) {
  CefRefPtr<CefProcessMessage> message =
      CefProcessMessage::Create("GET_DIMENSIONS");
  browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);

  state->notify.acquire();

  //If last load timer is higher than 5 seconds, dont continue and wait a second, a picture or JS script might have not finished loading
  while ((std::chrono::system_clock::now().time_since_epoch().count() - state->lastLoadTimeMillis) < 5000) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // DEBUG: Wait till loading is finished
  std::this_thread::sleep_for(std::chrono::seconds(20));

  //Resize browser and start paint process
  browser->GetHost()->WasResized();

  while (!state->IsScreenshotDone()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  size_t length;
  int bufferWidth;
  int bufferHeight;
  uint8_t* buffer =
      (uint8_t*)state->GetBuffer(length, bufferWidth, bufferHeight);

  int supposedLength = state->GetWidth() * state->GetHeight() * 4;

  if (length > 0) {
    if (bufferWidth != state->GetWidth() || bufferHeight != state->GetHeight()) {
      this->Log(std::format(
          "[Arbiter] [%d] Screenshot of url %s hasn't got proper width and "
          "height. Should be: %d x %d pixels, Is: %d x %d pixels",
          browser->GetIdentifier(), state->currentUrl, bufferWidth,
          bufferHeight, state->GetWidth(), state->GetHeight()));
    }

    if (supposedLength != length) {
      this->Log(std::format(
          "[Arbiter] [%d] Screenshot of url %s hasn't got proper size. Should be: %d bytes, Is: %d bytes",
          browser->GetIdentifier(), state->currentUrl, supposedLength, length));
    }

    this->Log(std::format(
        "[Arbiter] [%d] Converting from BGRA to RGBA and saving to png file...",
        browser->GetIdentifier(), state->currentUrl, supposedLength, length));

    png_bytep* pngBuf = new png_bytep[bufferHeight];

    for (int y = 0; y < bufferHeight; y++) {
      png_bytep row = new png_byte[bufferWidth*4];
      pngBuf[y] = row;

      for (int x = 0; x < bufferWidth; x++) {
        int i = x * 4;
        uint8_t blue = *(buffer++);
        uint8_t green = *(buffer++);
        uint8_t red = *(buffer++);
        uint8_t alpha = *(buffer++);

        row[i] = red;
        row[i + 1] = green;
        row[i + 2] = blue;
        row[i + 3] = alpha;
      }
    }

    png_structp png_ptr;
    png_infop info_ptr;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
      throw "Can't create png write context, out of memory!";
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
      throw "Can't create png info context, out of memory!";
  }
}

void Arbiter::AddURL(std::string url) {
  this->urls.push_back(url);
}

void Arbiter::Run(CefRefPtr<CefBrowser> browser) {
  std::shared_ptr<BrowserState> state = make_shared<BrowserState>(browser->GetIdentifier());
  state
      ->SetScreenshotDone(false);
  state
      ->SetDimensions(1920, 8640);

  this->browserStates[browser->GetIdentifier()] = state;

  Arbiter::GetInstance()->Log(
      std::format("[Arbiter] [%d] Starting...", browser->GetIdentifier()));

  while (!this->toBeDone.empty()) {
    std::string url;

    Arbiter::GetInstance()->Log(
        std::format("[Arbiter] [%d] Getting next url", browser->GetIdentifier()));

    try {
      //Acquire queue lock
      this->queueMutex.lock();

      //Check for remaining urls
      if (!this->toBeDone.empty()) {
        url = this->toBeDone.front();

        this->toBeDone.pop();
      }

      this->queueMutex.unlock();
    } catch (...) {
      this->queueMutex.unlock();
    }

    //If url is empty, toBeDone is empty
    if (!url.empty()) {
      Arbiter::GetInstance()->Log(
          std::format("[Arbiter] [%d] Loading url %s...", browser->GetIdentifier(), url));
      browser->GetMainFrame()->LoadURL(url);

      state->currentUrl = url;

      state->notifyLoad.acquire();

      //If page didnt load, continue with next one, else take screenshot
      if (state->_error) {
        Arbiter::GetInstance()->Log(std::format(
            "[Arbiter] [%d] Warning: Page %s didn't load properly! Skipping...", browser->GetIdentifier(), url));
        continue;
      }

      Arbiter::GetInstance()->Log(std::format(
          "[Arbiter] [%d] Creating screenshot for %s...", browser->GetIdentifier(), url));

      //Take screenshot
      this->TakeScreenshot(browser, state);
    }
  }
}

void Arbiter::Log(const char* str) {
  //logFile << str << endl;
  cout << str << endl;
}

void Arbiter::Log(const std::string& str) {
  // logFile << str << endl;
  cout << str << endl;
}


BrowserStateList Arbiter::getStateList() {
  return this->browserStates;
}

std::shared_ptr<Arbiter> Arbiter::GetInstance() {
  return std::move(g_Arbiter);
}