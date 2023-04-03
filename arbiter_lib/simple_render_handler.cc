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
	return g_instance;
}

bool SimpleRenderHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
        return true;
}