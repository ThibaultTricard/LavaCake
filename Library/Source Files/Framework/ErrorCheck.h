#pragma once
#include <iostream>

namespace LavaCake {
  namespace Framework {
  /**
   Class ErrorCheck :
   \brief a singleton that hold the description of the last error encountered by the program
   */
    class ErrorCheck {

      ErrorCheck(){};

      static ErrorCheck*       m_instance ;
      bool                     m_printError = false;
      char*                    m_errorMessage = (char*)"";

    public :

			/**
      \brief Return the last error the program has encountered
      \brief a char* containing the description of the last error encontered by the program
			*/
      static char* getError() {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        return m_instance->m_errorMessage;
      }

			/**
      \brief Register an error
      \param msg : the error message
			*/
      static void setError(char* msg) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_errorMessage = msg;

        if (m_instance->m_printError) {
          std::cout << "Error : " << m_instance->m_errorMessage << std::endl;
        }
      }

			/**
       \brief Define if the ErrorCheck singleton can print error on console
       \param print a boolean that define if ErrorCheck can print error on console
			*/
      static void PrintError(bool print = true) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_printError = print;
      }
    };
  }
}
