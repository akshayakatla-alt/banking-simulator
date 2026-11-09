#include "AccountRoutes.h"
#include <iostream>

static std::string extractBearerToken(const crow::request& req) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.rfind("Bearer ", 0) == 0) {
        return authHeader.substr(7);
    }
    return "";
}

void registerAccountRoutes(BankApp& app, Bank& bank, Auth& auth) {

    // 1. Create Savings Account
    CROW_ROUTE(app, "/api/account/savings").methods("POST"_method)
    ([&bank, &auth](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data) {
            res["success"] = false;
            res["message"] = "Invalid JSON payload.";
            return crow::response(400, res);
        }

        std::string customerId;
        std::string token = extractBearerToken(req);
        JwtPayload payload;
        if (!token.empty() && auth.verifyToken(token, payload)) {
            customerId = payload.customerId;
        } else if (data.has("email")) {
            customerId = bank.resolveCustomerId(data["email"].s());
        }

        if (customerId.empty()) {
            res["success"] = false;
            res["message"] = "Unable to resolve customer account. Please ensure customer is registered.";
            return crow::response(400, res);
        }

        double initialBalance = data.has("initialBalance") ? data["initialBalance"].d() : 0.0;
        std::string msg;
        std::string accNo = bank.createSavingsAccount(customerId, initialBalance, msg);

        bool success = !accNo.empty();
        res["success"] = success;
        res["accountNumber"] = accNo;
        res["message"] = msg;

        return crow::response(success ? 201 : 400, res);
    });

    // 2. Create Current Account
    CROW_ROUTE(app, "/api/account/current").methods("POST"_method)
    ([&bank, &auth](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data) {
            res["success"] = false;
            res["message"] = "Invalid JSON payload.";
            return crow::response(400, res);
        }

        std::string customerId;
        std::string token = extractBearerToken(req);
        JwtPayload payload;
        if (!token.empty() && auth.verifyToken(token, payload)) {
            customerId = payload.customerId;
        } else if (data.has("email")) {
            customerId = bank.resolveCustomerId(data["email"].s());
        }

        if (customerId.empty()) {
            res["success"] = false;
            res["message"] = "Unable to resolve customer account. Please ensure customer is registered.";
            return crow::response(400, res);
        }

        double initialBalance = data.has("initialBalance") ? data["initialBalance"].d() : 0.0;
        double overdraftLimit = data.has("overdraftLimit") ? data["overdraftLimit"].d() : 0.0;
        std::string msg;
        std::string accNo = bank.createCurrentAccount(customerId, initialBalance, overdraftLimit, msg);

        bool success = !accNo.empty();
        res["success"] = success;
        res["accountNumber"] = accNo;
        res["message"] = msg;

        return crow::response(success ? 201 : 400, res);
    });

    // 3. Check Account Balance
    CROW_ROUTE(app, "/api/account/balance/<string>").methods("GET"_method)
    ([&bank](const std::string& accountNumber) {
        crow::json::wvalue res;

        if (!bank.accountExists(accountNumber)) {
            res["success"] = false;
            res["message"] = "Account Not Found";
            return crow::response(404, res);
        }

        double balance = bank.getBalance(accountNumber);
        res["success"] = true;
        res["accountNumber"] = accountNumber;
        res["balance"] = balance;
        res["currency"] = "INR";

        return crow::response(200, res);
    });

    // 4. Get Account Details
    CROW_ROUTE(app, "/api/account/details/<string>").methods("GET"_method)
    ([&bank](const std::string& accountNumber) {
        crow::json::wvalue res;
        crow::json::wvalue details;

        if (!bank.getAccountDetails(accountNumber, details)) {
            res["success"] = false;
            res["message"] = "Account Not Found";
            return crow::response(404, res);
        }

        res["success"] = true;
        res["account"] = std::move(details);
        return crow::response(200, res);
    });

    // 5. Get My Accounts (Protected)
    CROW_ROUTE(app, "/api/account/my-accounts").methods("GET"_method)
    ([&bank, &auth](const crow::request& req) {
        crow::json::wvalue res;
        std::string token = extractBearerToken(req);

        JwtPayload payload;
        if (token.empty() || !auth.verifyToken(token, payload)) {
            res["success"] = false;
            res["message"] = "Unauthorized. Please log in.";
            return crow::response(401, res);
        }

        crow::json::wvalue::list accountsList;
        bank.getCustomerAccounts(payload.customerId, accountsList);

        res["success"] = true;
        res["accounts"] = std::move(accountsList);
        return crow::response(200, res);
    });

    // 6. Cash Deposit
    CROW_ROUTE(app, "/api/account/deposit").methods("POST"_method)
    ([&bank](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("accountNumber") || !data.has("amount")) {
            res["success"] = false;
            res["message"] = "Account number and amount are required.";
            return crow::response(400, res);
        }

        std::string msg;
        bool success = bank.deposit(
            data["accountNumber"].s(),
            data["amount"].d(),
            msg
        );

        res["success"] = success;
        res["message"] = msg;
        return crow::response(success ? 200 : 400, res);
    });

    // 7. Cash Withdrawal
    CROW_ROUTE(app, "/api/account/withdraw").methods("POST"_method)
    ([&bank](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("accountNumber") || !data.has("amount")) {
            res["success"] = false;
            res["message"] = "Account number and amount are required.";
            return crow::response(400, res);
        }

        std::string msg;
        bool success = bank.withdraw(
            data["accountNumber"].s(),
            data["amount"].d(),
            msg
        );

        res["success"] = success;
        res["message"] = msg;
        return crow::response(success ? 200 : 400, res);
    });
}
