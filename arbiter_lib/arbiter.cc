#include "arbiter.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include <filesystem>

Arbiter::~Arbiter() {}

void Arbiter::ParseSpiderFile() {
  Log("Parsing spider file...");
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

  //Wait till loading is finished
  std::this_thread::sleep_for(std::chrono::seconds(20));

  //Resize browser and start paint process
  browser->GetHost()->WasResized();

  while (!state->IsScreenshotDone()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //CONVERT TO PNG
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
        std::string url = this->toBeDone.front();

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