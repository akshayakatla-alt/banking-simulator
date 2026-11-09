#include "AuthRoutes.h"
#include <iostream>

static std::string extractBearerToken(const crow::request& req) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.rfind("Bearer ", 0) == 0) {
        return authHeader.substr(7);
    }
    return "";
}

void registerAuthRoutes(BankApp& app, Auth& auth) {

    // 1. Customer Registration
    CROW_ROUTE(app, "/api/register").methods("POST"_method)
    ([&auth](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("name") || !data.has("email") || !data.has("phone") || !data.has("password")) {
            res["success"] = false;
            res["message"] = "Invalid request payload. Name, email, phone, and password are required.";
            return crow::response(400, res);
        }

        std::string customerId, message;
        bool success = auth.registerUser(
            data["name"].s(),
            data["email"].s(),
            data["phone"].s(),
            data["password"].s(),
            customerId,
            message
        );

        res["success"] = success;
        res["message"] = message;
        if (success) {
            res["customerId"] = customerId;
        }

        return crow::response(success ? 201 : 400, res);
    });

    // 2. Customer Login
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([&auth](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("email") || !data.has("password")) {
            res["success"] = false;
            res["message"] = "Email and password are required.";
            return crow::response(400, res);
        }

        std::string token, message;
        crow::json::wvalue user;
        bool success = auth.loginUser(
            data["email"].s(),
            data["password"].s(),
            token,
            user,
            message
        );

        res["success"] = success;
        res["message"] = message;
        if (success) {
            res["token"] = token;
            res["user"] = std::move(user);
        }

        return crow::response(success ? 200 : 401, res);
    });

    // 3. Customer Profile View (Protected)
    CROW_ROUTE(app, "/api/profile").methods("GET"_method)
    ([&auth](const crow::request& req) {
        crow::json::wvalue res;
        std::string token = extractBearerToken(req);

        JwtPayload payload;
        if (token.empty() || !auth.verifyToken(token, payload)) {
            res["success"] = false;
            res["message"] = "Unauthorized. Invalid or expired token.";
            return crow::response(401, res);
        }

        crow::json::wvalue profile;
        if (!auth.getCustomerProfile(payload.customerId, profile)) {
            res["success"] = false;
            res["message"] = "Customer profile not found.";
            return crow::response(404, res);
        }

        res["success"] = true;
        res["profile"] = std::move(profile);
        return crow::response(200, res);
    });

    // 4. Customer Profile Update (Protected)
    CROW_ROUTE(app, "/api/profile").methods("PUT"_method)
    ([&auth](const crow::request& req) {
        crow::json::wvalue res;
        std::string token = extractBearerToken(req);

        JwtPayload payload;
        if (token.empty() || !auth.verifyToken(token, payload)) {
            res["success"] = false;
            res["message"] = "Unauthorized. Invalid or expired token.";
            return crow::response(401, res);
        }

        auto data = crow::json::load(req.body);
        if (!data || !data.has("name") || !data.has("phone")) {
            res["success"] = false;
            res["message"] = "Name and phone are required.";
            return crow::response(400, res);
        }

        std::string msg;
        bool success = auth.updateCustomerProfile(
            payload.customerId,
            data["name"].s(),
            data["phone"].s(),
            msg
        );

        res["success"] = success;
        res["message"] = msg;
        return crow::response(success ? 200 : 400, res);
    });

    // 5. Change Password (Protected)
    CROW_ROUTE(app, "/api/profile/change-password").methods("POST"_method)
    ([&auth](const crow::request& req) {
        crow::json::wvalue res;
        std::string token = extractBearerToken(req);

        JwtPayload payload;
        if (token.empty() || !auth.verifyToken(token, payload)) {
            res["success"] = false;
            res["message"] = "Unauthorized. Invalid or expired token.";
            return crow::response(401, res);
        }

        auto data = crow::json::load(req.body);
        if (!data || !data.has("oldPassword") || !data.has("newPassword")) {
            res["success"] = false;
            res["message"] = "Current password and new password are required.";
            return crow::response(400, res);
        }

        std::string msg;
        bool success = auth.changePassword(
            payload.customerId,
            data["oldPassword"].s(),
            data["newPassword"].s(),
            msg
        );

        res["success"] = success;
        res["message"] = msg;
        return crow::response(success ? 200 : 400, res);
    });
}
