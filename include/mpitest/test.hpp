#ifndef _MPITEST_TEST_HPP_
#define _MPITEST_TEST_HPP_

#include "common.hpp"
#include "ioformat.hpp"
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mpitest {

/**
 * @brief A simple MPI-based test class.
 *
 * This class provides a framework for registering and running tests
 * across multiple MPI ranks. Each test is a subclass of @c Test and
 * implements the @c test() method.
 *
 * The static method @c run_all() orchestrates running all registered tests,
 * printing a summary of results at rank 0.
 */
class Test {
  private:
    std::string m_suite;
    std::string m_name;
    bool m_passed = true;
    static inline std::unordered_map<std::string, std::vector<mpitest::Test*>>
        m_tests;

  public:
    /**
     * @brief Registers a test with the global registry of tests.
     * @param test A pointer to the test instance.
     */
    static void register_test(Test* test) {
        m_tests[test->m_suite].push_back(test);
    }

    /**
     * @brief Initializes MPI, runs all registered tests, prints summary, then
     * finalizes MPI.
     *
     * This function calls @c MPI_Init and @c MPI_Finalize under the hood, which
     * means it takes ownership of the MPI lifecycle.
     */
    static void run_all() {
        MPI_Init(NULL, NULL);
        if (mpihelpers::comm_rank() == 0) {
            std::cout << "Testing (world_size = " << mpihelpers::comm_size()
                      << "):" << std::endl;
        }

        std::unordered_map<std::string, int> m_total_count;
        std::unordered_map<std::string, int> m_passed_count;
        for (auto& [suite, tests] : m_tests) {
            for (auto* test : tests) {
                m_total_count[suite]++;
                if (test->run())
                    m_passed_count[suite]++;
            }
        }

        if (mpihelpers::comm_rank() == 0) {
            std::cout << format::Clear() << std::endl << "Summary:" << std::endl;
            for (auto& [suite, total] : m_total_count) {
                if (m_passed_count[suite] == total) {
                    std::cout << format::Green();
                } else {
                    std::cout << format::Red();
                }
                std::cout << "   - " << suite << " " << m_passed_count[suite]
                          << "/" << total << " passed" << format::Clear() << std::endl;
            }
        }
        MPI_Finalize();
    }

    /**
     * @brief Constructor for a @c Test object.
     * @param suite Name of the suite to which this test belongs.
     * @param name  Name of the test.
     */
    Test(std::string suite, std::string name) : m_suite(suite), m_name(name) {}
    virtual ~Test() = default;

    /**
     * @brief Pure virtual method that should implement the test logic.
     */
    virtual void test() = 0;

    /**
     * @brief Runs the test (calls @c test()), synchronizes ranks,
     *        and does a global reduction to see if the test passed.
     * @return Whether the test ultimately passed across *all* ranks.
     */
    bool run() {
        if (mpihelpers::comm_rank() == 0) {
            std::cout << "   - " << format::Blue() << "Running " << test_name()
                      << format::Clear() << std::endl;
        }

        MPI_Barrier(MPI_COMM_WORLD);

        this->test();

        MPI_Barrier(MPI_COMM_WORLD);

        m_passed = mpihelpers::global_reduce(m_passed);
        if (mpihelpers::comm_rank() == 0) {
            if (m_passed) {
                std::cout << "   - " << test_name() << ": " << format::Green()
                          << " passed" << format::Clear() << std::endl;
            } else {
                std::cout << "   - " << test_name() << ": " << format::Red() << "failed"
                          << format::Clear() << std::endl;
            }
        }
        return m_passed;
    }

    /**
     * @brief Forces the test to be marked as failed (local to the rank that calls it).
     */
    void fail() { m_passed = false; }

    /**
     * @brief Helper to retrieve a "Suite::TestName" formatted string.
     */
    std::string test_name() const { return m_suite + "::" + m_name; }
};

} // namespace mpitest

/**
 * @brief A macro to declare and define a new test.
 *
 * @param suite The suite name.
 * @param name  The test name.
 *
 * This macro creates a derived class of @c mpitest::Test, registers it automatically,
 * and defines its @c test() method, which you can implement inline.
 */
#define TEST(suite, name)                                                      \
    namespace mpitest::suite {                                                 \
    class name : public mpitest::Test {                                        \
      public:                                                                  \
        name() : mpitest::Test::Test(#suite, #name) {                          \
            mpitest::Test::register_test(this);                                \
        };                                                                     \
        void test() override;                                                  \
    };                                                                         \
    name test##name;                                                           \
    }                                                                          \
    inline void mpitest::suite::name::test()

#endif // _MPITEST_TEST_HPP_