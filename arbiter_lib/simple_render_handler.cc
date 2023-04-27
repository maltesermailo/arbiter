#include "simple_render_handler.h"

namespace {
SimpleRenderHandler* g_instance = nullptr;
}

SimpleRenderHandler::SimpleRenderHandler() {
  g_instance = this;
}

SimpleRenderHandler::~SimpleRenderHandler() {
  g_instance = nullptr;
}

SimpleRenderHandler* SimpleRenderHandler::GetInstance() {
    if (g_instance == nullptr) {
        new SimpleRenderHandler();
    }

	return g_instance;
}

bool SimpleRenderHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
        if (message->GetName().ToString().find("GET_DIMENSIONS") != std::string::npos) {
          int height;
          int width;

          CefRefPtr<CefV8Value> retval;
          CefRefPtr<CefV8Exception> exception;

          //browser->GetMainFrame()->GetV8Context()->Enter();
          if (browser->GetMainFrame()->GetV8Context()->Eval(
                  "return Math.max(document.body.scrollWidth, "
                  "document.body.offsetWidth, "
                  "document.documentElement.clientWidth, "
                  "document.documentElement.scrollWidth, "
                  "document.documentElement.offsetWidth);",
                  "display.js", 1, retval, exception)) {
            width = retval->GetIntValue();
          } else {
            CefRefPtr<CefProcessMessage> procMessage = CefProcessMessage::Create("GET_DIMENSIONS");
            CefRefPtr<CefListValue> arguments = procMessage->GetArgumentList();
            arguments
                ->SetBool(1, false);
            arguments->SetString(2, exception->GetMessageW());
            arguments->SetInt(3, browser->GetIdentifier());

            browser->GetMainFrame()->SendProcessMessage(PID_BROWSER,
                                                        procMessage);

            return true;
          }

          if (browser->GetMainFrame()->GetV8Context()->Eval(
                  "return Math.max(document.body.scrollHeight, document.body.offsetHeight, document.documentElement.clientHeight, document.documentElement.scrollHeight, document.documentElement.offsetHeight);",
                  "display.js", 1, retval, exception)) {
            height = retval->GetIntValue();
          } else {
            CefRefPtr<CefProcessMessage> procMessage =
                CefProcessMessage::Create("GET_DIMENSIONS");
            CefRefPtr<CefListValue> arguments = procMessage->GetArgumentList();
            arguments->SetBool(1, false);
            arguments->SetString(2, exception->GetMessageW());
            arguments->SetInt(3, browser->GetIdentifier());

            browser->GetMainFrame()->SendProcessMessage(PID_BROWSER,
                                                        procMessage);

            return true;
          }

          CefRefPtr<CefProcessMessage> procMessage =
              CefProcessMessage::Create("GET_DIMENSIONS");
          CefRefPtr<CefListValue> arguments = procMessage->GetArgumentList();
          arguments->SetBool(1, true);
          arguments->SetInt(2, browser->GetIdentifier());
          arguments->SetInt(3, width);
          arguments->SetInt(4, height);

          browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, procMessage);

          return true;
        }

        return false;
}