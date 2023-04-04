#include "client_scheme_handler.h"
#include <iostream>
#include <fstream>

bool ClientRequestHandler::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) {
  CEF_REQUIRE_IO_THREAD();

  bool handled = false;

  std::string url = request->GetURL();

  if (strstr(url.c_str(), UI_FILE_NAME) != nullptr) {
    std::ifstream file(strcat("./html/", UI_FILE_NAME));
    
    if (file.good()) {
        std::stringstream stream;
        stream << file.rdbuf();
        std::string content = stream.str();

        data_ = content;
        mime_type_ = "text/html";
        handled = true;
    } else {
        throw "No Such File";
    }
  } else if (strstr(url.c_str(), NULL_FILE_NAME) != nullptr) {
    data_ = "";
    mime_type_ = "text/html";
    handled = true;
  }

  if (handled)
    callback->Continue();

  return handled;
}

void ClientRequestHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
    int64& response_length,
    CefString& redirectUrl) {
      CEF_REQUIRE_IO_THREAD();

      DCHECK(!data_.empty());

      response->SetMimeType(mime_type_);
      response->SetStatus(200);

      // Set the resulting response length.
      response_length = data_.length();
}

void ClientRequestHandler::Cancel() {
      CEF_REQUIRE_IO_THREAD();
}

bool ClientRequestHandler::ReadResponse(void* data_out,
                  int bytes_to_read,
                  int& bytes_read,
                  CefRefPtr<CefCallback> callback) {
    CEF_REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    if (offset_ < data_.length()) {
        // Copy the next block of data into the buffer.
        int transfer_size =
            std::min(bytes_to_read, static_cast<int>(data_.length() - offset_));
        memcpy(data_out, data_.c_str() + offset_, transfer_size);
        offset_ += transfer_size;

        bytes_read = transfer_size;
        has_data = true;
    }

    return has_data;
}

void scheme_handler::RegisterSchemeHandlerFactory() {
  CefRegisterSchemeHandlerFactory(
      "client", "ui", new ClientSchemeHandlerFactory());
}
