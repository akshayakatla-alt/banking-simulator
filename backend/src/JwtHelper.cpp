#include "JwtHelper.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>

JwtHelper::JwtHelper(const std::string& secret) : secretKey(secret) {}

std::string JwtHelper::base64UrlEncode(const std::string& input) {
    static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    return out;
}

std::string JwtHelper::base64UrlDecode(const std::string& input) {
    std::string out;
    std::vector<int> T(256, -1);
    const char* lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (int i = 0; i < 64; i++) T[lookup[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string JwtHelper::createHmac(const std::string& data) const {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    HMAC(EVP_sha256(),
         secretKey.c_str(), secretKey.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hash, &hashLen);

    return std::string(reinterpret_cast<char*>(hash), hashLen);
}

std::string JwtHelper::generateToken(const std::string& customerId, const std::string& email, const std::string& role, int validHours) const {
    std::string header = R"({"alg":"HS256","typ":"JWT"})";

    auto now = std::chrono::system_clock::now();
    int64_t exp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + (validHours * 3600);

    std::ostringstream payloadStream;
    payloadStream << R"({"sub":")" << customerId
                  << R"(","email":")" << email
                  << R"(","role":")" << role
                  << R"(","exp":)" << exp << "}";

    std::string encodedHeader = base64UrlEncode(header);
    std::string encodedPayload = base64UrlEncode(payloadStream.str());

    std::string unsignedToken = encodedHeader + "." + encodedPayload;
    std::string signature = base64UrlEncode(createHmac(unsignedToken));

    return unsignedToken + "." + signature;
}

bool JwtHelper::verifyToken(const std::string& token, JwtPayload& outPayload) const {
    size_t firstDot = token.find('.');
    size_t secondDot = token.find('.', firstDot + 1);

    if (firstDot == std::string::npos || secondDot == std::string::npos) {
        return false;
    }

    std::string unsignedToken = token.substr(0, secondDot);
    std::string signature = token.substr(secondDot + 1);

    std::string expectedSig = base64UrlEncode(createHmac(unsignedToken));
    if (signature != expectedSig) {
        return false;
    }

    std::string payloadJson = base64UrlDecode(token.substr(firstDot + 1, secondDot - firstDot - 1));

    // Simple JSON extraction
    auto extractField = [&payloadJson](const std::string& field) -> std::string {
        std::string pattern = "\"" + field + "\":\"";
        size_t pos = payloadJson.find(pattern);
        if (pos == std::string::npos) return "";
        pos += pattern.length();
        size_t end = payloadJson.find('"', pos);
        if (end == std::string::npos) return "";
        return payloadJson.substr(pos, end - pos);
    };

    auto extractIntField = [&payloadJson](const std::string& field) -> int64_t {
        std::string pattern = "\"" + field + "\":";
        size_t pos = payloadJson.find(pattern);
        if (pos == std::string::npos) return 0;
        pos += pattern.length();
        size_t end = payloadJson.find_first_of(",}", pos);
        if (end == std::string::npos) return 0;
        try {
            return std::stoll(payloadJson.substr(pos, end - pos));
        } catch (...) {
            return 0;
        }
    };

    outPayload.customerId = extractField("sub");
    outPayload.email = extractField("email");
    outPayload.role = extractField("role");
    outPayload.exp = extractIntField("exp");

    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (outPayload.exp < nowSec) {
        return false; // Expired
    }

    return !outPayload.customerId.empty();
}
