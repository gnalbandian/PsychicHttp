#ifndef PsychicStaticFileHandler_h
#define PsychicStaticFileHandler_h

#include "PsychicCore.h"
#include "PsychicWebHandler.h"
#include "PsychicRequest.h"
#include "PsychicResponse.h"
#include "PsychicFileResponse.h"

class PsychicStaticFileHandler : public PsychicWebHandler {
  // using File = fs::File;
  // using FS = fs::FS;
  private:
    bool _getFile(PsychicRequest *request);
    static size_t _getFileSize(FILE* f);
    bool _fileExists(const std::string& path);
    uint8_t _countBits(const uint8_t value) const;
  protected:
    // FS _fs;
    std::string _uri;
    std::string _path;
    std::string _cache_control;
    std::string _default_file;
    std::string _last_modified;
    FILE *_file;
    std::string _filename;
    
    
    bool _isDir;
    bool _gzipFirst;
    uint8_t _gzipStats;
  public:
    PsychicStaticFileHandler(const char* uri, const char* path, const char* cache_control);
    bool canHandle(PsychicRequest *request) override;
    esp_err_t handleRequest(PsychicRequest *request) override;
    PsychicStaticFileHandler& setIsDir(bool isDir);
    PsychicStaticFileHandler& setDefaultFile(const char* filename);
    PsychicStaticFileHandler& setCacheControl(const char* cache_control);
    PsychicStaticFileHandler& setLastModified(const char* last_modified);
    PsychicStaticFileHandler& setLastModified(struct tm* last_modified);
    //PsychicStaticFileHandler& setTemplateProcessor(AwsTemplateProcessor newCallback) {_callback = newCallback; return *this;}
};

#endif /* PsychicHttp_h */