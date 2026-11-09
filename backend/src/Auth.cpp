#include "Auth.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <random>

Auth::Auth(Database& database, const std::string& jwtSecret)
    : db(database), jwt(jwtSecret) {}

std::string Auth::hashPassword(const std::string& password) {
    unsigned char salt[16];
    unsigned char hash[32];

    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Salt generation failed");
    }

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        100000,
        EVP_sha256(),
        sizeof(hash),
        hash
    ) != 1) {
        throw std::runtime_error("Password hashing failed");
    }

    std::stringstream result;
    result << std::hex << std::setfill('0');
    for (unsigned char c : salt)
        result << std::setw(2) << (int)c;

    result << "$";

    for (unsigned char c : hash)
        result << std::setw(2) << (int)c;

    return result.str();
}

bool Auth::verifyPassword(const std::string& password, const std::string& storedHash) {
    size_t separator = storedHash.find('$');
    if (separator == std::string::npos)
        return false;

    std::string saltHex = storedHash.substr(0, separator);
    std::string hashHex = storedHash.substr(separator + 1);

    if (saltHex.size() != 32 || hashHex.size() != 64)
        return false;

    unsigned char salt[16];
    unsigned char expected[32];
    unsigned char actual[32];

    auto decodeHex = [](const std::string& input, unsigned char* output) {
        for (size_t i = 0; i < input.size() / 2; i++) {
            try {
                output[i] = static_cast<unsigned char>(
                    std::stoi(input.substr(i * 2, 2), nullptr, 16)
                );
            } catch (...) {
                return false;
            }
        }
        return true;
    };

    if (!decodeHex(saltHex, salt) || !decodeHex(hashHex, expected))
        return false;

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        100000,
        EVP_sha256(),
        sizeof(actual),
        actual
    ) != 1)
        return false;

    return CRYPTO_memcmp(actual, expected, sizeof(actual)) == 0;
}

bool Auth::registerUser(
    const std::string& name,
    const std::string& email,
    const std::string& phone,
    const std::string& password,
    std::string& outCustomerId,
    std::string& outMessage
) {
    if (name.empty() || email.empty() || phone.empty() || password.length() < 8) {
        outMessage = "Invalid input: Name, Email, Phone are required and password must be at least 8 characters.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());

        // Check duplicate email in users
        auto checkEmail = txn.exec_params(
            "SELECT email FROM users WHERE email = $1",
            email
        );
        if (!checkEmail.empty()) {
            outMessage = "Registration failed: An account with this email already exists.";
            return false;
        }

        // Check duplicate phone in customers
        auto checkPhone = txn.exec_params(
            "SELECT phone FROM customers WHERE phone = $1",
            phone
        );
        if (!checkPhone.empty()) {
            outMessage = "Registration failed: An account with this phone number already exists.";
            return false;
        }

        // Generate customer ID
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);
        std::string customerId = "CUST" + std::to_string(dis(gen));

        std::string passwordHash = hashPassword(password);

        txn.exec_params(
            "INSERT INTO customers (customer_id, full_name, email, phone) VALUES ($1, $2, $3, $4)",
            customerId, name, email, phone
        );

        txn.exec_params(
            "INSERT INTO users (customer_id, email, password_hash, role) VALUES ($1, $2, $3, 'customer')",
            customerId, email, passwordHash
        );

        txn.commit();

        outCustomerId = customerId;
        outMessage = "Customer registered successfully.";
        std::cout << "[SJNA BANK] Customer Registered: " << customerId << " (" << email << ")" << std::endl;
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error during registration: ") + e.what();
        std::cerr << "[SJNA BANK ERROR] Registration failed: " << e.what() << std::endl;
        return false;
    }
}

bool Auth::loginUser(
    const std::string& email,
    const std::string& password,
    std::string& outToken,
    crow::json::wvalue& outUser,
    std::string& outMessage
) {
    if (email.empty() || password.empty()) {
        outMessage = "Email and password are required.";
        return false;
    }

    try {
        pqxx::read_transaction txn(db.getConnection());

        auto result = txn.exec_params(
            "SELECT u.password_hash, u.customer_id, u.role, c.full_name, c.phone "
            "FROM users u "
            "JOIN customers c ON u.customer_id = c.customer_id "
            "WHERE u.email = $1",
            email
        );

        if (result.empty()) {
            outMessage = "Invalid email or password.";
            return false;
        }

        std::string storedHash = result[0]["password_hash"].as<std::string>();
        if (!verifyPassword(password, storedHash)) {
            outMessage = "Invalid email or password.";
            return false;
        }

        std::string customerId = result[0]["customer_id"].as<std::string>();
        std::string role = result[0]["role"].as<std::string>();
        std::string name = result[0]["full_name"].as<std::string>();
        std::string phone = result[0]["phone"].as<std::string>();

        outToken = jwt.generateToken(customerId, email, role);

        outUser["customerId"] = customerId;
        outUser["name"] = name;
        outUser["email"] = email;
        outUser["phone"] = phone;
        outUser["role"] = role;

        outMessage = "Login successful.";
        std::cout << "[SJNA BANK] Customer Login Successful: " << customerId << std::endl;
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error during login: ") + e.what();
        std::cerr << "[SJNA BANK ERROR] Login failed: " << e.what() << std::endl;
        return false;
    }
}

bool Auth::verifyToken(const std::string& token, JwtPayload& outPayload) {
    return jwt.verifyToken(token, outPayload);
}

bool Auth::getCustomerProfile(const std::string& customerId, crow::json::wvalue& outProfile) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT customer_id, full_name, email, phone, created_at FROM customers WHERE customer_id = $1",
            customerId
        );

        if (res.empty()) return false;

        outProfile["customerId"] = res[0]["customer_id"].as<std::string>();
        outProfile["name"] = res[0]["full_name"].as<std::string>();
        outProfile["email"] = res[0]["email"].as<std::string>();
        outProfile["phone"] = res[0]["phone"].as<std::string>();
        outProfile["createdAt"] = res[0]["created_at"].as<std::string>();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SJNA BANK ERROR] Profile fetch failed: " << e.what() << std::endl;
        return false;
    }
}

bool Auth::updateCustomerProfile(
    const std::string& customerId,
    const std::string& name,
    const std::string& phone,
    std::string& outMessage
) {
    if (name.empty() || phone.empty()) {
        outMessage = "Full name and phone cannot be empty.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());
        txn.exec_params(
            "UPDATE customers SET full_name = $1, phone = $2 WHERE customer_id = $3",
            name, phone, customerId
        );
        txn.commit();
        outMessage = "Customer profile updated successfully.";
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Error updating profile: ") + e.what();
        return false;
    }
}

bool Auth::changePassword(
    const std::string& customerId,
    const std::string& oldPassword,
    const std::string& newPassword,
    std::string& outMessage
) {
    if (newPassword.length() < 8) {
        outMessage = "New password must be at least 8 characters.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT password_hash FROM users WHERE customer_id = $1",
            customerId
        );
        if (res.empty()) {
            outMessage = "User not found.";
            return false;
        }

        std::string currentHash = res[0]["password_hash"].as<std::string>();
        if (!verifyPassword(oldPassword, currentHash)) {
            outMessage = "Current password does not match.";
            return false;
        }

        std::string newHash = hashPassword(newPassword);
        txn.exec_params(
            "UPDATE users SET password_hash = $1 WHERE customer_id = $2",
            newHash, customerId
        );
        txn.commit();
        outMessage = "Password changed successfully.";
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Error changing password: ") + e.what();
        return false;
    }
}
