#pragma once

#ifndef ARBITER_H_
#define ARBITER_H_
#include <list>
#include <iostream>
#include "include/yaml-cpp/yaml.h"

using namespace std;

class Arbiter {
 public:
  //Runs the screenshot comparison and generates the gallery.
  void Run();
  //Adds a URL to the list of URLs
  void AddURL(std::string url);

 private:
  std::list<string> urls;

};

#endif