// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "arbiter.h"
#include <format>

namespace {

SimpleHandler* g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," +
         CefURIEncode(CefBase64Encode(data.data(), data.size()), false)
             .ToString();
}

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
    : use_views_(use_views), is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window)
        window->SetTitle(title);
    }
  } else if (!IsChromeRuntimeEnabled()) {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);

  //Start Arbiter Thread on browser
  CefPostTask(TID_FILE_BACKGROUND,
              base::BindOnce(&Arbiter::Run, Arbiter::GetInstance(), browser));
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Allow Chrome to show the error page.
  //if (IsChromeRuntimeEnabled())
  //  return;

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  if (failedUrl.ToString().find("client://") == std::string::npos && frame->IsMain()) {
    BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
    if (stateList.contains(browser->GetIdentifier())) {
      std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];
      state->_error = true;

      state->notifyLoad.release();
    } else {
      Arbiter::GetInstance()->Log(std::format(
              "Unknown instance %d detected. Instance won't continue loading!",
          browser->GetIdentifier()));
    }
      return;
  }

  // Display a load error message using a data: URI.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";

  frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int httpStatusCode) {
  if (frame->IsMain()) {
      BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
      if (stateList.contains(browser->GetIdentifier())) {
          if (httpStatusCode == 0) {
              return;
          }

          if (frame->GetURL().ToString().find("client://ui/null") !=
              std::string::npos) {
              return;
          }

          Arbiter::GetInstance()->Log(
          std::format("[Arbiter] [{}] URL {} finished loading with status code {}",
          browser->GetIdentifier(), frame->GetURL().ToString(),
          httpStatusCode));

          std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];
          state->_error = false;

          state->notifyLoad.release();
      } else {
          Arbiter::GetInstance()->Log(
              std::format("Unknown instance %d detected. Instance "
                                      "won't continue loading!",
                          browser->GetIdentifier())
                  .c_str());
      }
      return;
  }
}

bool SimpleHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
  if (frame->IsMain()) {
      BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
      if (stateList.contains(browser->GetIdentifier())) {
          std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];

          if (!message->GetArgumentList()->GetBool(1)) {
              state->_error = true;

              std::string errorMsg = message->GetArgumentList()->GetString(2).ToString();

              Arbiter::GetInstance()->Log(
                  std::format("[Arbiter] [{}] Error while trying to execute "
                              "resize javascript. Error: ",
                              browser->GetIdentifier(), errorMsg)
                      .c_str());
          }

          state->notify.release();
      } else {
          Arbiter::GetInstance()->Log(
              std::format("Unknown instance {} detected. Instance "
                                      "won't continue loading!",
                          browser->GetIdentifier())
                  .c_str());
      }
      return true;
  }

  return false;
}

CefRefPtr<CefResourceHandler> SimpleHandler::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
  BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
  if (stateList.contains(browser->GetIdentifier())) {
      std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];

      state->lastLoadTimeMillis =
          std::chrono::system_clock::now().time_since_epoch().count();
  } else {
      Arbiter::GetInstance()->Log(
          std::format("Unknown instance %d detected. Instance "
                                  "will have arbitrary state!",
                      browser->GetIdentifier())
              .c_str());
  }
  return nullptr;
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
                                       force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
                                CefRect& cefRect) {

    BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
  if (stateList.contains(browser->GetIdentifier())) {
    std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];
    
    cefRect.width = state->GetWidth();
    cefRect.height = state->GetHeight();
  } else {
    cefRect.width = 1920;
    cefRect.height = 1080;
  }
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser,
    CefRenderHandler::PaintElementType type,
    const CefRenderHandler::RectList& dirtyRects,
    const void* buffer,
    int width,
    int height) {
  size_t length = width * height * 4;

  BrowserStateList stateList = Arbiter::GetInstance()->getStateList();
  if (stateList.contains(browser->GetIdentifier())) {
    std::shared_ptr<BrowserState> state = stateList[browser->GetIdentifier()];

    uint8_t* newBuffer = new uint8_t[length];
    std::memcpy(newBuffer, buffer, length);

    std::cout << "URL: " << browser->GetMainFrame()->GetURL().ToString() << std::endl;

    PrintPicture(newBuffer, width, height, length);

    state->SetBuffer(newBuffer, length, width, height);
    state->SetScreenshotDone(true);
  }
}

// static
bool SimpleHandler::IsChromeRuntimeEnabled() {
  static int value = -1;
  if (value == -1) {
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    value = command_line->HasSwitch("enable-chrome-runtime") ? 1 : 0;
  }
  return value == 1;
}
