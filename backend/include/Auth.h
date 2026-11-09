#ifndef AUTH_H
#define AUTH_H

#include <string>
#include "Database.h"
#include "JwtHelper.h"
#include "crow.h"

class Auth {
private:
    Database& db;
    JwtHelper jwt;

    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& storedHash);

public:
    Auth(Database& database, const std::string& jwtSecret);

    bool registerUser(
        const std::string& name,
        const std::string& email,
        const std::string& phone,
        const std::string& password,
        std::string& outCustomerId,
        std::string& outMessage
    );

    bool loginUser(
        const std::string& email,
        const std::string& password,
        std::string& outToken,
        crow::json::wvalue& outUser,
        std::string& outMessage
    );

    bool verifyToken(const std::string& token, JwtPayload& outPayload);

    bool getCustomerProfile(const std::string& customerId, crow::json::wvalue& outProfile);

    bool updateCustomerProfile(
        const std::string& customerId,
        const std::string& name,
        const std::string& phone,
        std::string& outMessage
    );

    bool changePassword(
        const std::string& customerId,
        const std::string& oldPassword,
        const std::string& newPassword,
        std::string& outMessage
    );
};

#endif
