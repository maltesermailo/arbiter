#pragma once

#ifndef ARBITER_H_
#define ARBITER_H_
#include <list>
#include <iostream>
#include "include/cef_browser.h"
#include "yaml-cpp/yaml.h"
#include <queue>
#include <mutex>

using namespace std;

class Arbiter {
 public:
  Arbiter() : lastRun(0), currentRun(0) {}
  ~Arbiter();

  static std::shared_ptr<Arbiter> GetInstance();

  //Runs the screenshot comparison and generates the gallery. This will be run on the main thread and block CEF. Consider running Arbiter::RunThread()
  void Run(CefRefPtr<CefBrowser> browser);
  //Adds a URL to the list of URLs
  void AddURL(std::string url);

  //Runs the main arbiter thread. Preferably call this over Run()
  void RunThread(CefRefPtr<CefBrowser> browser);

  //Parses the spider.yaml file for the sitemap
  void ParseSpiderFile();

  //Prepares the data directory with the last run
  void PrepareData(std::string dataDir, int lastRun);

  //Takes a screenshot in the browser
  void TakeScreenshot(CefRefPtr<CefBrowser> browser);

 private:
  std::list<string> urls;
  YAML::Node spiderFile;

  std::string dataDirPath;
  int lastRun;
  int currentRun;

  std::queue<string> toBeDone;
  std::mutex queueMutex;

};

// Global Instance
namespace {
std::shared_ptr<Arbiter> g_Arbiter = make_shared<Arbiter>();
}

#endif