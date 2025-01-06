#ifndef _MPITEST_IOFORMAT_HPP_
#define _MPITEST_IOFORMAT_HPP_

#include <iostream>
#include <string>

namespace mpitest::format {

class Formatter {
  private:
    std::string m_raw;

  public:
    Formatter(std::string raw) : m_raw(raw) {}

    friend std::ostream& operator<<(std::ostream& os, const Formatter& a) {
        os << a.m_raw;
        return os;
    }
};

class Green : public Formatter {
  public:
    Green() : Formatter("\033[0;32m") {}
};

class Red : public Formatter {
  public:
    Red() : Formatter("\033[0;31m") {}
};

class Yellow : public Formatter {
  public:
    Yellow() : Formatter("\033[0;33m") {}
};

class Blue : public Formatter {
  public:
    Blue() : Formatter("\033[0;34m") {}
};

class Magenta : public Formatter {
  public:
    Magenta() : Formatter("\033[0;35m") {}
};

class Cyan : public Formatter {
  public:
    Cyan() : Formatter("\033[0;36m") {}
};

class Bold : public Formatter {
  public:
    Bold() : Formatter("\033[1m") {}
};

class Clear : public Formatter {
  public:
    Clear() : Formatter("\033[0m") {}
};

} // namespace mpitest

#endif // _MPITEST_IOFORMAT_HPP_