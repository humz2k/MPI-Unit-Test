#ifndef _MPITEST_ASSERTION_HPP_
#define _MPITEST_ASSERTION_HPP_

#include "common.hpp"
#include "ioformat.hpp"
#include "test.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace mpitest {

/**
 * @class BaseAssertion
 * @brief Stores fundamental data regarding an assertion in a test.
 *
 * This class captures the test reference, the textual representation
 * of the assertion, file/line information, and whether the assertion
 * holds. Subclasses (LocalAssertion, GlobalAssertion) determine
 * reporting logic.
 */
class BaseAssertion {
  private:
    std::stringstream m_stream;
    Test& m_test;
    std::string m_assertion;
    std::string m_file;
    int m_line;
    bool m_condition;

  public:
    /**
     * @brief Constructs a BaseAssertion.
     * @param test Reference to the test in which this assertion is raised.
     * @param assertion String version of the assertion code (e.g. "x == 42").
     * @param file Filename where the assertion is located (usually __FILE__).
     * @param line Line number where the assertion is located (usually
     * __LINE__).
     * @param condition Boolean result of the actual test condition.
     */
    BaseAssertion(Test& test, std::string assertion, std::string file, int line,
                  bool condition)
        : m_test(test), m_assertion(assertion), m_file(file), m_line(line),
          m_condition(condition) {}

    virtual ~BaseAssertion() = default;

    /**
     * @brief Stream insertion operator for building a message string.
     */
    template <typename T> BaseAssertion& operator<<(T str) {
        m_stream << str;
        return *this;
    }

    /**
     * @brief Get the final assertion message.
     */
    std::string message() const { return m_stream.str(); }

    /**
     * @brief Accessor for the associated test.
     */
    Test& test() { return m_test; }

    /**
     * @brief Const accessor for the associated test.
     */
    const Test& test() const { return m_test; }

    /**
     * @brief Returns the boolean result of the assertion condition.
     */
    bool condition() const { return m_condition; }

    /**
     * @brief Returns the textual representation of the assertion (e.g. "x ==
     * 42").
     */
    const std::string& assertion() const { return m_assertion; }

    /**
     * @brief Returns the source file where the assertion was made.
     */
    const std::string& file() const { return m_file; }

    /**
     * @brief Returns the line number where the assertion was made.
     */
    int line() const { return m_line; }
};

/**
 * @class LocalAssertion
 * @brief Checks a condition only on the local MPI rank.
 *
 * If condition() fails on any given rank, that rank will print an error,
 * mark the test as failed, but does not attempt a collective check or reduce.
 */
class LocalAssertion : public BaseAssertion {
  public:
    using BaseAssertion::BaseAssertion;

    /**
     * @brief Destructor. Triggers printing and test failure if the assertion
     * fails locally.
     */
    virtual ~LocalAssertion() {
        if (!condition()) {
            test().fail();
            std::cout << "   - " << format::Red() << "Assertion " << assertion()
                      << " in test " << test().test_name() << " (" << file()
                      << "::" << line() << ")" << " FAILED." << std::endl;
            auto msg = message();
            if (msg.size() > 0) {
                std::cout << "      - rank " << mpihelpers::comm_rank() << ": " << msg
                          << std::endl;
            }
            std::cout << format::Clear() << std::flush;
        }
    }
};

/**
 * @class GlobalAssertion
 * @brief Checks a condition across all MPI ranks collectively.
 *
 * If any rank fails condition(), the test is marked as failed and
 * rank 0 collects and prints messages from all ranks.
 */
class GlobalAssertion : public BaseAssertion {
  public:
    using BaseAssertion::BaseAssertion;

    /**
     * @brief Destructor. Triggers a global reduction, printing, and test
     * failure if any rank fails.
     */
    virtual ~GlobalAssertion() {
        // The key difference from LocalAssertion: we do a global_reduce.
        if (!mpihelpers::global_reduce(condition())) {
            test().fail();
            int size = mpihelpers::comm_size();
            int rank = mpihelpers::comm_rank();
            for (int i = 0; i < size; i++) {
                if (i == 0) {
                    if (rank == 0) {
                        std::cout << "   - " << format::Red() << "Assertion "
                                  << assertion() << " in test "
                                  << test().test_name() << " (" << file()
                                  << "::" << line() << ")" << " FAILED."
                                  << std::endl;
                        auto msg = message();
                        if (msg.size() > 0) {
                            std::cout << "      - rank " << i << ": " << msg
                                      << std::endl;
                        }
                    }
                    continue;
                }
                if (i == rank) {
                    // Each non-zero rank sends its message to rank 0.
                    mpihelpers::send_string(message(), 0);
                }
                if (rank == 0) {
                    // Rank 0 receives and prints the message from rank i.
                    auto msg = mpihelpers::recv_string(i);
                    if (msg.size() > 0) {
                        std::cout << "      - rank " << i << ": " << msg
                                  << std::endl;
                    }
                }
            }
            if (rank == 0) {
                std::cout << format::Clear() << std::flush;
            }
        }
    }
};

} // namespace mpitest

/**
 * @brief Global assertion macro: checks the condition across all ranks.
 */
#define GLOBAL_ASSERT(assertion)                                               \
    mpitest::GlobalAssertion(*this, #assertion, __FILE__, __LINE__, assertion)

/**
 * @brief Local assertion macro: checks the condition on the local rank only.
 */
#define LOCAL_ASSERT(assertion)                                                \
    mpitest::LocalAssertion(*this, #assertion, __FILE__, __LINE__, assertion)

#endif // _MPITEST_ASSERTION_HPP_