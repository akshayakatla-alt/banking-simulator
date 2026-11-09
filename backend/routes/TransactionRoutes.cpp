#include "TransactionRoutes.h"
#include <iostream>

void registerTransactionRoutes(BankApp& app, Bank& bank) {

    // 1. Fund Transfer API
    CROW_ROUTE(app, "/api/transactions/transfer").methods("POST"_method)
    ([&bank](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("senderAccount") || !data.has("receiverAccount") || !data.has("amount")) {
            res["success"] = false;
            res["message"] = "Sender account, receiver account, and amount are required.";
            return crow::response(400, res);
        }

        std::string sender = data["senderAccount"].s();
        std::string receiver = data["receiverAccount"].s();
        double amount = data["amount"].d();
        std::string remarks = data.has("remarks") ? std::string(data["remarks"].s()) : "";

        std::string txnId, message;
        bool success = bank.transfer(sender, receiver, amount, remarks, txnId, message);

        res["success"] = success;
        res["message"] = message;
        if (success) {
            res["transactionId"] = txnId;
            res["senderAccount"] = sender;
            res["receiverAccount"] = receiver;
            res["amount"] = amount;
        }

        return crow::response(success ? 200 : 400, res);
    });

    // 2. Transaction History API
    CROW_ROUTE(app, "/api/transactions/history").methods("GET"_method)
    ([&bank](const crow::request& req) {
        crow::json::wvalue res;
        crow::json::wvalue::list transactionList;

        std::string acc = "";
        if (req.url_params.get("accountNumber") != nullptr) {
            acc = req.url_params.get("accountNumber");
        }

        bank.getTransactions(acc, transactionList);

        res["success"] = true;
        res["transactions"] = std::move(transactionList);
        return crow::response(200, res);
    });

    // 3. Bill Payment Simulation API
    CROW_ROUTE(app, "/api/payments/bill").methods("POST"_method)
    ([&bank](const crow::request& req) {
        auto data = crow::json::load(req.body);
        crow::json::wvalue res;

        if (!data || !data.has("accountNumber") || !data.has("billCategory") ||
            !data.has("billerName") || !data.has("consumerNumber") || !data.has("amount")) {
            res["success"] = false;
            res["message"] = "Account number, category, biller name, consumer number, and amount are required.";
            return crow::response(400, res);
        }

        std::string acc = data["accountNumber"].s();
        std::string category = data["billCategory"].s();
        std::string biller = data["billerName"].s();
        std::string consumer = data["consumerNumber"].s();
        double amount = data["amount"].d();

        std::string txnId, message;
        bool success = bank.payBill(acc, category, biller, consumer, amount, txnId, message);

        res["success"] = success;
        res["message"] = message;
        if (success) {
            res["transactionId"] = txnId;
            res["billerName"] = biller;
            res["category"] = category;
            res["amount"] = amount;
        }

        return crow::response(success ? 200 : 400, res);
    });
}
