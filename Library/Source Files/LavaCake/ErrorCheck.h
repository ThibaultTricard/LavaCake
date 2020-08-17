#pragma once
#include <iostream>

namespace LavaCake {
  namespace Framework {
    class ErrorCheck {

      ErrorCheck(){};

      static ErrorCheck*       m_instance ;
      bool                     m_printError = false;
      char*                    m_errorMessage = "";

    public :

			/*
			* return the last error the program has encountered
			*/
      static char* getError() {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        return m_instance->m_errorMessage;
      }

			/*
			*	raise and error
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

			/*
			*	enable automatic console error printing
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