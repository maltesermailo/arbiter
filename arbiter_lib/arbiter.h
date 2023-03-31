#pragma once

#ifndef ARBITER_H_
#define ARBITER_H_
#include <list>
#include <iostream>
#include "yaml-cpp/yaml.h"

using namespace std;

class Arbiter {
 public:
  Arbiter();

  //Runs the screenshot comparison and generates the gallery. This will be run on the main thread and block CEF. Consider running Arbiter::RunThread()
  void Run();
  //Adds a URL to the list of URLs
  void AddURL(std::string url);

  //Runs the main arbiter thread. Preferably call this over Run()
  void RunThread();

  //Methods run by Run()
  // 
  //Parses the spider.yaml file for the sitemap
  void ParseSpiderFile();

  //Prepares the data directory with the last run
  bool PrepareData(std::string dataDir, int lastRun);

  //Takes a screenshot in the browser
  void TakeScreenshot(CefRefPtr<CefBrowser> browser);

  //Takes screenshots in every open page
  void TakeScreenshots();

 private:
  std::list<string> urls;
  YAML::Node spiderFile;

};

#endif