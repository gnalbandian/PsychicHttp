#ifndef PsychicWebParameter_h
#define PsychicWebParameter_h
#include <string>
/*
 * PARAMETER :: Chainable object to hold GET/POST and FILE parameters
 * */

class PsychicWebParameter {
  private:
<<<<<<< HEAD
    std::string _name;
    std::string _value;
=======
    const char* _name;
    const char* _value;
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847
    size_t _size;
    bool _isForm;
    bool _isFile;

  public:
<<<<<<< HEAD
    PsychicWebParameter(const std::string& name, const std::string& value, bool form=false, bool file=false, size_t size=0): _name(name), _value(value), _size(size), _isForm(form), _isFile(file){}
    const std::string& name() const { return _name; }
    const std::string& value() const { return _value; }
=======
    PsychicWebParameter(const char* name, const char* value, bool form=false, bool file=false, size_t size=0): _name(name), _value(value), _size(size), _isForm(form), _isFile(file){}
    const char* name() const { return _name; }
    const char* value() const { return _value; }
>>>>>>> 1bff4fdd0a1375b77fbcbd0cf4e94fe2aa08f847
    size_t size() const { return _size; }
    bool isPost() const { return _isForm; }
    bool isFile() const { return _isFile; }
};

#endif //PsychicWebParameter_h