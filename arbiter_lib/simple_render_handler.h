#pragma once
#ifndef SIMPLE_RENDER_HANDLER_H_
#define SIMPLE_RENDER_HANDLER_H_

#include "include/cef_app.h"
#include "include/cef_client.h"

class SimpleRenderHandler : public CefRenderProcessHandler {
 public:
  SimpleRenderHandler();
  ~SimpleRenderHandler();

  // Provide access to the single global instance of this object.
  static SimpleRenderHandler* GetInstance();

  bool SimpleRenderHandler::OnProcessMessageReceived(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) override;

  private:
  // Include the default reference counting implementation.
   IMPLEMENT_REFCOUNTING(SimpleRenderHandler);
};

#endif  // !SIMPLE_RENDER_HANDLER_H_
