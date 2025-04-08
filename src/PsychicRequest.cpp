#include "PsychicRequest.h"
#include "http_status.h"
#include "PsychicHttpServer.h"
#include "esp_log.h"

PsychicRequest::PsychicRequest(PsychicHttpServer *server, httpd_req_t *req) : _server(server),
                                                                              _req(req),
                                                                              _method(HTTP_GET),
                                                                              _query(""),
                                                                              _body(""),
                                                                              _tempObject(NULL)
{
  // load up our client.
  this->_client = server->getClient(req);

  // handle our session data
  if (req->sess_ctx != NULL)
    this->_session = (SessionData *)req->sess_ctx;
  else
  {
    this->_session = new SessionData();
    req->sess_ctx = this->_session;
  }

  // callback for freeing the session later
  req->free_ctx = this->freeSession;

  // load up some data
  this->_uri = this->_req->uri;
}

PsychicRequest::~PsychicRequest()
{
  // temorary user object
  if (_tempObject != NULL)
    free(_tempObject);

  // our web parameters
  // for (auto *param : _params)
  //   delete (param);
  // _params.clear();
}

void PsychicRequest::freeSession(void *ctx)
{
  if (ctx != NULL)
  {
    SessionData *session = (SessionData *)ctx;
    delete session;
  }
}

PsychicHttpServer *PsychicRequest::server()
{
  return _server;
}

httpd_req_t *PsychicRequest::request()
{
  return _req;
}

PsychicClient *PsychicRequest::client()
{
  return _client;
}

const char *PsychicRequest::getFilename()
{
  // parse the content-disposition header
  if (this->hasHeader("Content-Disposition"))
  {
    ContentDisposition cd = this->getContentDisposition();
    // if (cd.filename != "")
    // return cd.filename;
    return "";
  }

  // fall back to passed in query string
  //  PsychicWebParameter *param = getParam("_filename");
  //  if (param != NULL)
  //    return param->name();

  // fall back to parsing it from url (useful for wildcard uploads)
  //  String uri = this->uri();
  //  int filenameStart = uri.lastIndexOf('/') + 1;
  //  String filename = uri.substring(filenameStart);
  //  if (filename != "")
  //    return filename;

  // finally, unknown.
  ESP_LOGE(PH_TAG, "Did not get a valid filename from the upload.");
  return "unknown.txt";
}

const ContentDisposition PsychicRequest::getContentDisposition()
{
    ContentDisposition cd;
    std::string header = this->header("Content-Disposition");

    // Set disposition type
    if (header.find("form-data") == 0)
        cd.disposition = FORM_DATA;
    else if (header.find("attachment") == 0)
        cd.disposition = ATTACHMENT;
    else if (header.find("inline") == 0)
        cd.disposition = INLINE;
    else
        cd.disposition = NONE;

    // Parse filename
    size_t start = header.find("filename=");
    if (start != std::string::npos)
    {
        size_t firstQuote = header.find('"', start + 9);
        size_t endQuote = header.find('"', firstQuote + 1);
        if (firstQuote != std::string::npos && endQuote != std::string::npos)
        {
            cd.filename = header.substr(firstQuote + 1, endQuote - firstQuote - 1);
        }
    }

    // Parse name
    start = header.find("name=");
    if (start != std::string::npos)
    {
        size_t firstQuote = header.find('"', start + 5);
        size_t endQuote = header.find('"', firstQuote + 1);
        if (firstQuote != std::string::npos && endQuote != std::string::npos)
        {
            cd.name = header.substr(firstQuote + 1, endQuote - firstQuote - 1);
        }
    }

    return cd;
}

esp_err_t PsychicRequest::loadBody()
{
  esp_err_t err = ESP_OK;

  this->_body.clear();

  size_t remaining = this->_req->content_len;
  size_t actuallyReceived = 0;

  char *buf = static_cast<char *>(malloc(remaining + 1));
  if (buf == nullptr)
  {
    ESP_LOGE(PH_TAG, "Failed to allocate memory for body");
    return ESP_FAIL;
  }

  while (remaining > 0)
  {
    int received = httpd_req_recv(this->_req, buf + actuallyReceived, remaining);

    if (received == HTTPD_SOCK_ERR_TIMEOUT)
    {
      continue;
    }
    else if (received <= 0)
    {
      ESP_LOGE(PH_TAG, "Failed to receive data.");
      err = ESP_FAIL;
      break;
    }

    remaining -= received;
    actuallyReceived += received;
  }

  buf[actuallyReceived] = '\0';
  this->_body = std::string(buf);
  free(buf);
  return err;
}

http_method PsychicRequest::method() const
{
  return (http_method)_req->method;
}

std::string PsychicRequest::methodStr()
{
  return std::string(http_method_str((http_method)this->_req->method));
}

std::string PsychicRequest::path() {
  const char* q = strchr(_uri, '?');
  if (q == nullptr) {
      return std::string(_uri);
  } else {
      return std::string(_uri, q - _uri);  // construct from pointer + length
  }
}

const char *PsychicRequest::uri() const
{
  return _uri;
}

const std::string &PsychicRequest::query() const
{
  return _query;
}

// no way to get list of headers yet....
// int PsychicRequest::headers()
// {
// }

std::string PsychicRequest::header(const char *name)
{
  size_t header_len = httpd_req_get_hdr_value_len(this->_req, name);

  if (header_len)
  {
    std::string header(header_len + 1, '\0'); // Allocate space, including null terminator
    esp_err_t err = httpd_req_get_hdr_value_str(this->_req, name, &header[0], header_len + 1);

    if (err == ESP_OK)
    {
      header.resize(strnlen(header.c_str(), header_len)); // shrink to real content length
      return header;
    }
  }

  return {};
}

bool PsychicRequest::hasHeader(const char *name)
{
  return httpd_req_get_hdr_value_len(this->_req, name) > 0;
}

std::string PsychicRequest::host()
{
  return this->header("Host");
  // return nullptr;
}

std::string PsychicRequest::contentType()
{
  return header("Content-Type");
  // return nullptr;
}

size_t PsychicRequest::contentLength() const
{
  return this->_req->content_len;
}

const std::string& PsychicRequest::body()
{
  return this->_body;
}

bool PsychicRequest::isMultipart()
{
  const std::string &type = this->contentType();
  return type.find("multipart/form-data") != std::string::npos;
}

esp_err_t PsychicRequest::redirect(const char *url)
{
  PsychicResponse response(this);
  response.setCode(301);
  response.addHeader("Location", url);

  return response.send();
}

bool PsychicRequest::hasCookie(const char *key)
{
  char cookie[MAX_COOKIE_SIZE];
  size_t cookieSize = MAX_COOKIE_SIZE;
  esp_err_t err = httpd_req_get_cookie_val(this->_req, key, cookie, &cookieSize);

  // did we get anything?
  if (err == ESP_OK)
    return true;
  else if (err == ESP_ERR_HTTPD_RESULT_TRUNC)
    ESP_LOGE(PH_TAG, "cookie too large (%d bytes).\n", cookieSize);

  return false;
}

std::string PsychicRequest::getCookie(const char *key)
{
  char cookie[MAX_COOKIE_SIZE];
  size_t cookieSize = MAX_COOKIE_SIZE;
  esp_err_t err = httpd_req_get_cookie_val(this->_req, key, cookie, &cookieSize);

  // did we get anything?
  if (err == ESP_OK)
    return std::string(cookie);
  else
    return std::string();
}

void PsychicRequest::loadParams()
{
    // Check if there's a query string
    size_t query_len = httpd_req_get_url_query_len(_req);
    if (query_len)
    {
        std::string query(query_len + 1, '\0');
        if (httpd_req_get_url_query_str(_req, &query[0], query_len + 1) == ESP_OK)
        {
            // query.resize(strnlen(query.c_str(), query_len));  // Trim any extra nulls
            _query += query; // Append to _query (make sure _query is a std::string)
            _addParams(_query, false);
        }
    }

    // Check for URL-encoded form data in POST body
    if (this->method() == HTTP_POST && this->contentType().rfind("application/x-www-form-urlencoded", 0) == 0)
    {
        _addParams(_body, true);  // Ensure _body is also a std::string
    }
}

void PsychicRequest::_addParams(const std::string& params, bool post)
{
    size_t start = 0;
    while (start < params.length()) {
        size_t end = params.find('&', start);
        if (end == std::string::npos) end = params.length();

        size_t equal = params.find('=', start);
        if (equal == std::string::npos || equal > end) equal = end;

        std::string name = params.substr(start, equal - start);
        std::string value = (equal + 1 < end) ? params.substr(equal + 1, end - equal - 1) : "";

        addParam(name, value, true, post);

        start = end + 1;
    }
}

PsychicWebParameter* PsychicRequest::addParam(const std::string& name, const std::string& value, bool decode, bool post)
{
    if (decode)
      return nullptr;  
        // return addParam(new PsychicWebParameter(urlDecode(name), urlDecode(value), post));
    else
      return nullptr;
        // return addParam(new PsychicWebParameter(name, value, post));
}

PsychicWebParameter* PsychicRequest::addParam(PsychicWebParameter* param)
{
    _params.push_back(param);
    return param;
}

bool PsychicRequest::hasParam(const char *key)
{
  // return getParam(key) != NULL;
  return false;
}

PsychicWebParameter* PsychicRequest::getParam(const char* key)
{
    for (auto* param : _params)
    {
        if (param->name() == key)
            return param;
    }
    return nullptr;
}

bool PsychicRequest::hasSessionKey(const char *key)
{
  return this->_session->find(key) != this->_session->end();
}

const std::string PsychicRequest::getSessionKey(const std::string& key)
{
    auto it = this->_session->find(key);
    if (it != this->_session->end())
        return it->second;
    else
        return std::string();
}

void PsychicRequest::setSessionKey(const std::string &key, const std::string &value)
{
  this->_session->insert(std::pair<std::string, std::string>(key, value));
}

// static const String md5str(const String &in)
// {
//   MD5Builder md5 = MD5Builder();
//   md5.begin();
//   md5.add(in);
//   md5.calculate();
//   return md5.toString();
// }

// Helper: trim whitespace from both ends of a string (you must implement this)
std::string trim(const std::string &s) {
  const char* whitespace = " \t\n\r";
  size_t start = s.find_first_not_of(whitespace);
  if (start == std::string::npos)
      return "";
  size_t end = s.find_last_not_of(whitespace);
  return s.substr(start, end - start + 1);
}
std::string md5str(const std::string& input)
{
    unsigned char digest[16]; // MD5 produces 128 bits (16 bytes)
    char hex_output[33];      // 2 chars per byte + null terminator

    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts(&ctx);
    mbedtls_md5_update(&ctx, reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    mbedtls_md5_finish(&ctx, digest);
    mbedtls_md5_free(&ctx);

    // Convert digest to hex string (lowercase)
    for (int i = 0; i < 16; ++i) {
        snprintf(hex_output + i * 2, 3, "%02x", digest[i]);
    }

    return std::string(hex_output);
}

bool PsychicRequest::authenticate(const char* username, const char* password)
{
    if (!hasHeader("Authorization")) return false;

    std::string authReq = header("Authorization");
    if (authReq.rfind("Basic", 0) == 0)
    {
        authReq = authReq.substr(6);
        // authReq.erase(0, authReq.find_first_not_of(" \t\r\n")); // trim left
        // authReq.erase(authReq.find_last_not_of(" \t\r\n") + 1); // trim right
        authReq = trim(authReq);

        std::string toEncode = std::string(username) + ":" + password;
        size_t encodedLen = 0;
        size_t outLen = 4 * ((toEncode.size() + 2) / 3);
        char* encoded = new char[outLen + 1];

        if (mbedtls_base64_encode(reinterpret_cast<unsigned char*>(encoded), outLen + 1, &encodedLen,
                                  reinterpret_cast<const unsigned char*>(toEncode.c_str()), toEncode.size()) == 0)
        {
            encoded[encodedLen] = '\0';
            bool match = authReq == encoded;
            delete[] encoded;
            return match;
        }

        delete[] encoded;
    }
    else if (authReq.rfind("Digest", 0) == 0)
    {
        authReq = authReq.substr(7);
        std::string _username = _extractParam(authReq, "username=\"", '\"');
        if (_username.empty() || _username != username) return false;

        std::string _realm = _extractParam(authReq, "realm=\"", '\"');
        std::string _nonce = _extractParam(authReq, "nonce=\"", '\"');
        std::string _uri = _extractParam(authReq, "uri=\"", '\"');
        std::string _resp = _extractParam(authReq, "response=\"", '\"');
        std::string _opaque = _extractParam(authReq, "opaque=\"", '\"');

        if (_realm.empty() || _nonce.empty() || _uri.empty() || _resp.empty() || _opaque.empty()) return false;

        if (_opaque != getSessionKey("opaque") ||
            _nonce != getSessionKey("nonce") ||
            _realm != getSessionKey("realm")) return false;

        std::string _nc, _cnonce;
        if (authReq.find("qop=auth") != std::string::npos || authReq.find("qop=\"auth\"") != std::string::npos)
        {
            _nc = _extractParam(authReq, "nc=", ',');
            _cnonce = _extractParam(authReq, "cnonce=\"", '\"');
        }

        std::string _H1 = md5str(username + std::string(":") + _realm + ":" + password);

        std::string _methodStr;
        switch (_method)
        {
            case HTTP_GET: _methodStr = "GET:"; break;
            case HTTP_POST: _methodStr = "POST:"; break;
            case HTTP_PUT: _methodStr = "PUT:"; break;
            case HTTP_DELETE: _methodStr = "DELETE:"; break;
            default: _methodStr = "GET:"; break;
        }

        std::string _H2 = md5str(_methodStr + _uri);

        std::string _responseCheck;
        if (authReq.find("qop=auth") != std::string::npos || authReq.find("qop=\"auth\"") != std::string::npos)
        {
            _responseCheck = md5str(_H1 + ':' + _nonce + ':' + _nc + ':' + _cnonce + ":auth:" + _H2);
        }
        else
        {
            _responseCheck = md5str(_H1 + ':' + _nonce + ':' + _H2);
        }

        return _resp == _responseCheck;
    }

    return false;
}


std::string PsychicRequest::_extractParam(const std::string &authReq, const std::string &param, char delimit)
{
    size_t begin = authReq.find(param);
    if (begin == std::string::npos)
        return "";
    begin += param.length();
    size_t end = authReq.find(delimit, begin);
    if (end == std::string::npos)
        return authReq.substr(begin);
    return authReq.substr(begin, end - begin);
}

const char *PsychicRequest::_getRandomHexString()
{
  char buffer[33]; // buffer to hold 32 Hex Digit + /0
  int i;
  for (i = 0; i < 4; i++)
  {
    sprintf(buffer + (i * 8), "%08lx", (unsigned long int)esp_random());
  }
  // return String(buffer);.
  return "";
}

esp_err_t PsychicRequest::requestAuthentication(HTTPAuthMethod mode, const char* realm, const char* authFailMsg)
{
    // Set realm: if empty, use "Login Required"
    if (strcmp(realm, "") == 0)
        this->setSessionKey("realm", "Login Required");
    else
        this->setSessionKey("realm", realm);

    // Create response object
    PsychicResponse response(this);
    std::string authStr;

    // For Basic authentication:
    if (mode == BASIC_AUTH) {
        // Build: "Basic realm=\"<realm>\""
        authStr = "Basic realm=\"" + this->getSessionKey("realm") + "\"";
        response.addHeader("WWW-Authenticate", authStr.c_str());
    }
    // For Digest authentication:
    else {
        // Create new nonce and opaque if they haven't been set yet.
        if (this->getSessionKey("nonce").empty())
            this->setSessionKey("nonce", _getRandomHexString());
        if (this->getSessionKey("opaque").empty())
            this->setSessionKey("opaque", _getRandomHexString());

        // Build: "Digest realm=\"<realm>\", qop=\"auth\", nonce=\"<nonce>\", opaque=\"<opaque>\""
        authStr = "Digest realm=\"" + this->getSessionKey("realm") +
                  "\", qop=\"auth\", nonce=\"" + this->getSessionKey("nonce") +
                  "\", opaque=\"" + this->getSessionKey("opaque") + "\"";
        response.addHeader("WWW-Authenticate", authStr.c_str());
    }

    response.setCode(401);
    response.setContentType("text/html");
    response.setContent(authStr.c_str());
    return response.send();
}

esp_err_t PsychicRequest::reply(int code)
{
  // PsychicResponse response(this);

  // response.setCode(code);
  // response.setContentType("text/plain");
  // response.setContent(http_status_reason(code));

  // return response.send();
  return ESP_OK;
}

esp_err_t PsychicRequest::reply(const char *content)
{
  PsychicResponse response(this);

  response.setCode(200);
  response.setContentType("text/html");
  response.setContent(content);

  return response.send();
}

esp_err_t PsychicRequest::reply(int code, const char *contentType, const char *content)
{
  // PsychicResponse response(this);

  // response.setCode(code);
  // response.setContentType(contentType);
  // response.setContent(content);

  // return response.send();
  return ESP_OK;
}