#ifndef JWT_HELPER_H
#define JWT_HELPER_H

#include <string>
#include <chrono>

struct JwtPayload {
    std::string customerId;
    std::string email;
    std::string role;
    int64_t exp;
};

class JwtHelper {
private:
    std::string secretKey;

    static std::string base64UrlEncode(const std::string& input);
    static std::string base64UrlDecode(const std::string& input);
    std::string createHmac(const std::string& data) const;

public:
    explicit JwtHelper(const std::string& secret);

    std::string generateToken(const std::string& customerId, const std::string& email, const std::string& role = "customer", int validHours = 24) const;
    bool verifyToken(const std::string& token, JwtPayload& outPayload) const;
};

#endif
