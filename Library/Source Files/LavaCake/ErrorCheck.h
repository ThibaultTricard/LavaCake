#pragma once
#include <iostream>

namespace LavaCake {
  namespace Framework {
    class ErrorCheck {

      ErrorCheck(){};

      static ErrorCheck*       m_instance ;
      bool                     m_printError = false;
      int                      m_errorCode = 0;
      char*                    m_errorMessage = "";

    public :
      static int getErrorCode() {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        return m_instance->m_errorCode;
      }

      static char* getErrorMessage() {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        return m_instance->m_errorMessage;
      }

      static void setError(int err, char* msg) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_errorCode = err;
        m_instance->m_errorMessage = msg;

        if (m_instance->m_printError) {
          std::cout << "Error " << m_instance->m_errorCode << " : " << m_instance->m_errorMessage << std::endl;
        }
      }

      static void PrintError(bool print = true) {
        if (m_instance == nullptr) {
          m_instance = new ErrorCheck();
        }
        m_instance->m_printError = print;
      }
    };
  }
}