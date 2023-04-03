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

void Arbiter::TakeScreenshot(CefRefPtr<CefBrowser> browser) {
  //browser->GetMainFrame()->Exe
}

void Arbiter::AddURL(std::string url) {

}

void Arbiter::Run(CefRefPtr<CefBrowser> browser) {

}

void Arbiter::Log(char* str) {
  //logFile << str << endl;
  cout << str << endl;
}

std::shared_ptr<Arbiter> Arbiter::GetInstance() {
  return std::move(g_Arbiter);
}