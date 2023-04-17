// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_handler.h"
#include "simple_app.h"
#include "client_scheme_handler.h"
#include "arbiter.h"

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(800, 600);
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  SimpleBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(
        new SimpleWindowDelegate(popup_browser_view));

    // We created the Window.
    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

}  // namespace

SimpleApp::SimpleApp() {}

void SimpleApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {
  // Register "client" as a standard scheme.
  int options = CEF_SCHEME_OPTION_STANDARD;
  registrar->AddCustomScheme("client", options);
}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Create the browser using the Views framework if "--use-views" is specified
  // via the command-line. Otherwise, create the browser using the native
  // platform framework.
  const bool use_views = command_line->HasSwitch("use-views");

  scheme_handler::RegisterSchemeHandlerFactory();

  //Create browser-global Arbiter instance
  g_Arbiter = make_shared<Arbiter>();

  Arbiter::GetInstance()->ParseSpiderFile();

  CefString dataDir;
  CefString lastRunParam;
  CefString threadsParam;

  dataDir = command_line->GetSwitchValue("data-dir");
  if (dataDir.empty()) {
    Arbiter::GetInstance()->Log("No data dir specified with --data-dir. Example: --data-dir=C:\\test\\");

    CefShutdown();
    std::exit(-1);
    return;
  }

  lastRunParam = command_line->GetSwitchValue("last-run");
  int lastRun = -1;
  if (lastRunParam.empty()) {
    Arbiter::GetInstance()->Log("No last run specified with --last-run, generating new screenshots without comparison. Example: --last-run=1");
  } else {
    try {
      lastRun = stoi(lastRunParam.ToString());
    } catch (const std::invalid_argument& e) {
      UNREFERENCED_PARAMETER(e);
      Arbiter::GetInstance()->Log("Last run is not an integer!");

      CefShutdown();
      std::exit(-1);
      return;
    }
  }

  threadsParam = command_line->GetSwitchValue("threads");
  int threads =
      std::thread::hardware_concurrency();
  if (threads <= 0)
    threads = 4;

  if (threadsParam.empty()) {
    Arbiter::GetInstance()->Log(
        "No thread count specified, using 4");
  } else {
    try {
      threads = stoi(threadsParam.ToString());
    } catch (const std::invalid_argument& e) {
      UNREFERENCED_PARAMETER(e);
      Arbiter::GetInstance()->Log("Threads is not an integer!");

      CefShutdown();
      std::exit(-1);
      return;
    }
  }

  std::string data = dataDir.ToString();

  //Setup Data Directory
  try {
    Arbiter::GetInstance()->PrepareData(data, lastRun);
  } catch (const std::string& e) {
    Arbiter::GetInstance()->Log(e.c_str());
    CefShutdown();
    std::exit(-1);
    return;
  }

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_views));

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  browser_settings.javascript = STATE_ENABLED;
  browser_settings.local_storage = STATE_DISABLED;
  browser_settings.windowless_frame_rate = 1;
  CefString(&browser_settings.accept_language_list).FromString("de-DE,de");

  for (int i = 0; i < threads; i++) {
    if (use_views) {
      // Create the BrowserView.
      CefRefPtr<CefBrowserView> browser_view =
          CefBrowserView::CreateBrowserView(handler, "client://ui/null",
                                            browser_settings, nullptr, nullptr,
                                            new SimpleBrowserViewDelegate());

      // Create the Window. It will show itself after creation.
      CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
    } else {
      // Information used when creating the native window.
      CefWindowInfo window_info;

#if defined(OS_WIN)
      // On Windows we need to specify certain flags that will be passed to
      // CreateWindowEx().
      window_info.SetAsPopup(nullptr, "Arbiter");
#endif
      window_info.SetAsWindowless(nullptr);

      // Create the browser window.
      CefBrowserHost::CreateBrowser(window_info, handler, "client://ui/null",
                                    browser_settings, nullptr, nullptr);
    }
  }
}

CefRefPtr<CefClient> SimpleApp::GetDefaultClient() {
  // Called when a new browser window is created via the Chrome runtime UI.
  return SimpleHandler::GetInstance();
}
