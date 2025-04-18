#include "PsychicResponse.h"
#include "PsychicRequest.h"
#include <http_status.h>
#include "esp_log.h"
#include "urlencode.h"

PsychicResponse::PsychicResponse(PsychicRequest *request) :
  _request(request),
  _code(200),
  _status(""),
  _contentLength(0),
  _body("")
{
}

PsychicResponse::~PsychicResponse()
{
  //clean up our header variables.  we have to do this on desctruct since httpd_resp_send doesn't store copies
  for (HTTPHeader header : _headers)
  {
    free(header.field);
    free(header.value);
  }
  _headers.clear();
}

void PsychicResponse::addHeader(const char *field, const char *value)
{
  //these get freed after send by the destructor
  HTTPHeader header;
  header.field =(char *)malloc(strlen(field)+1);
  header.value = (char *)malloc(strlen(value)+1);

  strlcpy(header.field, field, strlen(field)+1);
  strlcpy(header.value, value, strlen(value)+1);

  _headers.push_back(header);
}

void PsychicResponse::setCookie(const char *name, const char *value, unsigned long secondsFromNow, const char *extras)
{
    time_t now = time(nullptr);

    // Build the cookie value: URL-encode name and value
    std::string output = urlEncode(name) + "=" + urlEncode(value);

    // If the current time is before a "modern" timestamp, use Max-Age
    if (now < 1700000000) {
        output += "; Max-Age=" + std::to_string(secondsFromNow);
    }
    // Otherwise, use an Expires date
    else {
        time_t expirationTimestamp = now + secondsFromNow;
        struct tm tmInfo;
        // Use thread-safe gmtime_r instead of gmtime
        gmtime_r(&expirationTimestamp, &tmInfo);
        char expires[30];
        strftime(expires, sizeof(expires), "%a, %d %b %Y %H:%M:%S GMT", &tmInfo);
        output += "; Expires=" + std::string(expires);
    }

    // If any extras were provided, append them
    if (extras && strlen(extras) > 0) {
        output += "; " + std::string(extras);
    }

    // Finally, add the header to the response.
    // Assume addHeader is a member function that takes a const char* key and value.
    addHeader("Set-Cookie", output.c_str());
}

void PsychicResponse::setCode(int code)
{
  _code = code;
}


void PsychicResponse::setContentType(const char *contentType)
{
  httpd_resp_set_type(_request->request(), contentType);
}

void PsychicResponse::setContent(const char *content)
{
  _body = content;
  setContentLength(strlen(content));
}

void PsychicResponse::setContent(const uint8_t *content, size_t len)
{
  _body = (char *)content;
  setContentLength(len);
}

const char * PsychicResponse::getContent()
{
  return _body;
}

size_t PsychicResponse::getContentLength()
{
  return _contentLength;
}

esp_err_t PsychicResponse::send()
{
  //esp-idf makes you set the whole status.
  sprintf(_status, "%u %s", _code, http_status_reason(_code));
  httpd_resp_set_status(_request->request(), _status);

  //our headers too
  this->sendHeaders();

  //now send it off
  esp_err_t err = httpd_resp_send(_request->request(), getContent(), getContentLength());

  //did something happen?
  if (err != ESP_OK)
    ESP_LOGE(PH_TAG, "Send response failed (%s)", esp_err_to_name(err));

  return err;
}

void PsychicResponse::sendHeaders()
{
  //get our global headers out of the way first
  for (HTTPHeader header : DefaultHeaders::Instance().getHeaders())
    httpd_resp_set_hdr(_request->request(), header.field, header.value);

  //now do our individual headers
  for (HTTPHeader header : _headers)
    httpd_resp_set_hdr(this->_request->request(), header.field, header.value);

  // DO NOT RELEASE HEADERS HERE... released in the PsychicResponse destructor after they have been sent.
  // httpd_resp_set_hdr just passes on the pointer, but its needed after this call.
  // clean up our header variables after send
  // for (HTTPHeader header : _headers)
  // {
  //   free(header.field);
  //   free(header.value);
  // }
  // _headers.clear();
}

esp_err_t PsychicResponse::sendChunk(uint8_t *chunk, size_t chunksize)
{
  /* Send the buffer contents as HTTP response chunk */
  esp_err_t err = httpd_resp_send_chunk(this->_request->request(), (char *)chunk, chunksize);
  if (err != ESP_OK)
  {
    ESP_LOGE(PH_TAG, "File sending failed (%s)", esp_err_to_name(err));

    /* Abort sending file */
    httpd_resp_sendstr_chunk(this->_request->request(), NULL);

    /* Respond with 500 Internal Server Error */
    httpd_resp_send_err(this->_request->request(), HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
  }

  return err;
}

esp_err_t PsychicResponse::finishChunking()
{
  /* Respond with an empty chunk to signal HTTP response completion */
  return httpd_resp_send_chunk(this->_request->request(), NULL, 0);
}