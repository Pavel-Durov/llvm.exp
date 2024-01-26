#ifndef Logger_h
#define Logger_h

#include <iostream>
#include <sstream>

class ErrLogMessage : public std::basic_ostringstream<char> {
public:
  ~ErrLogMessage() { std::cerr << "Fatal error:" << str().c_str(); }
};
#define DIE ErrLogMessage()

#endif // Logger_h
