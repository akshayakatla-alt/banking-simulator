cmake_minimum_required(VERSION 3.20)

if(WIN32 AND NOT DEFINED CMAKE_CXX_COMPILER AND EXISTS "C:/msys64/ucrt64/bin/g++.exe")
    set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")
endif()

project(BankingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DASIO_STANDALONE -DASIO_ENABLE_OLD_SERVICES -D_WIN32_WINNT=0x0601)

set(SOURCES
    src/main.cpp
    src/Auth.cpp
    src/Bank.cpp
    src/BankAccount.cpp
    src/SavingsAccount.cpp
    src/CurrentAccount.cpp
    src/Customer.cpp
    src/Transaction.cpp
    src/Database.cpp
    src/JwtHelper.cpp
    routes/AuthRoutes.cpp
    routes/AccountRoutes.cpp
    routes/TransactionRoutes.cpp
)

add_executable(BankingSystem ${SOURCES})

target_include_directories(BankingSystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/routes
)

if(WIN32)
    target_include_directories(BankingSystem PRIVATE C:/msys64/ucrt64/include)
    target_link_directories(BankingSystem PRIVATE C:/msys64/ucrt64/lib)
    target_link_libraries(BankingSystem PRIVATE
        pqxx
        pq
        ssl
        crypto
        ws2_32
        mswsock
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
    find_package(OpenSSL REQUIRED)
    find_package(Threads REQUIRED)

    target_link_libraries(BankingSystem PRIVATE
        PkgConfig::PQXX
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
    )
endif()
