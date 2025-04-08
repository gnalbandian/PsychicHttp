#include "esp_log.h"

#ifndef PsychicCore_h
#define PsychicCore_h

#define PH_TAG "psychic"

// version numbers
#define PSYCHIC_HTTP_VERSION_MAJOR 1
#define PSYCHIC_HTTP_VERSION_MINOR 1
#define PSYCHIC_HTTP_VERSION_PATCH 0

#ifndef MAX_COOKIE_SIZE
#define MAX_COOKIE_SIZE 512
#endif

#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 8 * 1024
#endif

#ifndef STREAM_CHUNK_SIZE
#define STREAM_CHUNK_SIZE 1024
#endif

#ifndef MAX_UPLOAD_SIZE
#define MAX_UPLOAD_SIZE (2048 * 1024) // 2MB
#endif

#ifndef MAX_REQUEST_BODY_SIZE
#define MAX_REQUEST_BODY_SIZE (16 * 1024) // 16K
#endif

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <esp_http_server.h>
#include <map>
#include <list>
// #include <libb64/cencode.h>
#include "esp_random.h"
// #include "MD5Builder.h"
// #include <UrlEncode.h>
// #include "FS.h"
#include <ArduinoJson.h>
#include <functional> // Required for std::function
#include "esp_err.h"  // Required for esp_err_t

enum HTTPAuthMethod
{
  BASIC_AUTH,
  DIGEST_AUTH
};

<<<<<<< HEAD
void urlDecode(const char* encoded, char* decoded, size_t buffer_size);
=======
char* urlDecode(const char *encoded);
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847

class PsychicHttpServer;
class PsychicRequest;
// class PsychicWebSocketRequest;
class PsychicClient;

<<<<<<< HEAD
// Filter function definition
typedef bool (*PsychicRequestFilterFunction)(PsychicRequest *request);

// Client connect callback
typedef void (*PsychicClientCallback)(PsychicClient *client);

// // Callback definitions
typedef esp_err_t (*PsychicHttpRequestCallback)(PsychicRequest *request);
typedef esp_err_t (*PsychicJsonRequestCallback)(PsychicRequest *request, JsonVariant json);
=======
// //filter function definition
// typedef std::function<bool(PsychicRequest *request)> PsychicRequestFilterFunction;

// //client connect callback
// typedef std::function<void(PsychicClient *client)> PsychicClientCallback;

// //callback definitions
// typedef std::function<esp_err_t(PsychicRequest *request)> PsychicHttpRequestCallback;
// typedef std::function<esp_err_t(PsychicRequest *request, JsonVariant &json)> PsychicJsonRequestCallback;
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847

typedef bool (*PsychicRequestFilterFunction)(PsychicRequest *request);
typedef void (*PsychicClientCallback)(PsychicClient *client);
typedef esp_err_t (*PsychicHttpRequestCallback)(PsychicRequest *request);
typedef esp_err_t (*PsychicJsonRequestCallback)(PsychicRequest *request, JsonVariant &json);

struct HTTPHeader
{
  char *field;
  char *value;
};

class DefaultHeaders
{
  std::list<HTTPHeader> _headers;

public:
  DefaultHeaders() {}

<<<<<<< HEAD
  void addHeader(const char * field, const char * value)
=======
  // void addHeader(const String& field, const String& value)
  // {
  //   addHeader(field.c_str(), value.c_str());
  // }

  void addHeader(const char *field, const char *value)
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847
  {
    HTTPHeader header;
    if (field && value)
    {
      // these are just going to stick around forever.
      header.field = (char *)malloc(strlen(field) + 1);
      header.value = (char *)malloc(strlen(value) + 1);

      strlcpy(header.field, field, strlen(field) + 1);
      strlcpy(header.value, value, strlen(value) + 1);

      _headers.push_back(header);
    }
    else
    {
      ESP_LOGW(PH_TAG, "CHECK THIS, NULLPTR");
    }
  }

<<<<<<< HEAD
  const std::list<HTTPHeader>& getHeaders() const { return _headers; }
=======
  const std::list<HTTPHeader> &getHeaders() { return _headers; }
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847

  // delete the copy constructor, singleton class
  DefaultHeaders(DefaultHeaders const &) = delete;
  DefaultHeaders &operator=(DefaultHeaders const &) = delete;

  // single static class interface
  static DefaultHeaders &Instance()
  {
    static DefaultHeaders instance;
    return instance;
  }
};

#endif // PsychicCore_h