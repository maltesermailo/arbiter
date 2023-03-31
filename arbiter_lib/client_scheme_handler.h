#pragma once
#include "include\cef_scheme.h"
#include "include\wrapper\cef_helpers.h"
#ifndef CLIENT_SCHEME_HANDLER_H_
#define CLIENT_SCHEME_HANDLER_H_

namespace scheme_handler {

// Create and register the custom scheme handler factory.
void RegisterSchemeHandlerFactory();

}  // namespace scheme_handler

const char UI_FILE_NAME[] = "ui.html";

class ClientRequestHandler : public CefResourceHandler {
     public:
        ClientRequestHandler() : offset_(0) {}

      bool ProcessRequest(CefRefPtr<CefRequest> request,
                          CefRefPtr<CefCallback> callback) override;

      void GetResponseHeaders(CefRefPtr<CefResponse> response,
                              int64& response_length,
                              CefString& redirectUrl) override;

      void Cancel() override;

      bool ReadResponse(void* data_out,
                        int bytes_to_read,
                        int& bytes_read,
                        CefRefPtr<CefCallback> callback) override;

     private:
      std::string data_;
      std::string mime_type_;
      size_t offset_;

      IMPLEMENT_REFCOUNTING(ClientRequestHandler);
      DISALLOW_COPY_AND_ASSIGN(ClientRequestHandler);
};

class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
    public:
      ClientSchemeHandlerFactory() {}

      // Return a new scheme handler instance to handle the request.
      CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           const CefString& scheme_name,
                                           CefRefPtr<CefRequest> request) override {
        CEF_REQUIRE_IO_THREAD();
        return new ClientRequestHandler();
      }

     private:
      IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
      DISALLOW_COPY_AND_ASSIGN(ClientSchemeHandlerFactory);
};

#endif
