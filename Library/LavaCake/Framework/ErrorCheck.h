#pragma once
#include <iostream>
#include <cstdint>
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


namespace LavaCake {
  namespace Framework {
    /**
     Class ErrorCheck :
     \brief a singleton that hold the description of the last error encountered by the program
     */
    class ErrorCheck {

      ErrorCheck() {};

      static ErrorCheck* m_instance;
      bool                     m_printError = false;
      const char* m_errorMessage = (char*)"";
      uint8_t                  m_errorLvl = 0;
      uint8_t                  m_minErrorLevel = 0;

    public:

      /**
      \brief Return the last error the program has encountered
      \brief a char* containing the description of the last error encontered by the program
      */
      static const char* getError() {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        return m_instance->m_errorMessage;
      }

      /**
      \brief Register an error
      \param msg : the error message
      \param lvl : the level of the error, the lower the level the more important the error is
      */
      static void setError(const char* msg, uint8_t lvl = 0) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_errorMessage = msg;
        m_instance->m_errorLvl = lvl;
        if (m_instance->m_printError) {
          if (m_instance->m_minErrorLevel >= m_instance->m_errorLvl) {
            switch (m_instance->m_errorLvl) {
            case 0:
              std::cout << RED << "Error : ";
              break;
            case 1:
              std::cout << YELLOW << "Warning 1 : ";
              break;
            case 2:
              std::cout << YELLOW << "Warning 2 : ";
              break;
            case 3:
              std::cout << YELLOW << "Warning 3 : ";
              break;
            case 4:
              std::cout << YELLOW << "Warning 4 : ";
              break;
            case 5:
              std::cout << GREEN << "Notes : ";
              break;
            default:
              break;
            }
            std::cout << m_instance->m_errorMessage << RESET << std::endl;
          }
        }
      }

      /**
      \brief Define if the ErrorCheck singleton can print error on console
      \param print a boolean that define if ErrorCheck can print error on console
      */
      static void printError(bool print = true, uint8_t lvl = 0) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_printError = print;
        m_instance->m_minErrorLevel = lvl;
      }
    };
  }
}
