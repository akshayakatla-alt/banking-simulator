#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>
#include <memory>
#include "Database.h"
#include "crow.h"

class Bank {
private:
    Database& db;

    std::string generateAccountNumber();
    std::string generateTransactionId();
    std::string generatePaymentId();

public:
    Bank(Database& database);

    std::string resolveCustomerId(const std::string& email);

    std::string createSavingsAccount(
        const std::string& customerId,
        double initialBalance,
        std::string& outMessage
    );

    std::string createCurrentAccount(
        const std::string& customerId,
        double initialBalance,
        double overdraftLimit,
        std::string& outMessage
    );

    double getBalance(const std::string& accountNumber);

    bool accountExists(const std::string& accountNumber);

    bool getAccountDetails(const std::string& accountNumber, crow::json::wvalue& outAccount);

    bool getCustomerAccounts(const std::string& customerId, crow::json::wvalue::list& outAccounts);

    bool deposit(const std::string& accountNumber, double amount, std::string& outMessage);

    bool withdraw(const std::string& accountNumber, double amount, std::string& outMessage);

    bool transfer(
        const std::string& sender,
        const std::string& receiver,
        double amount,
        const std::string& remarks,
        std::string& outTxnId,
        std::string& outMessage
    );

    bool payBill(
        const std::string& accountNumber,
        const std::string& billCategory,
        const std::string& billerName,
        const std::string& consumerNumber,
        double amount,
        std::string& outTxnId,
        std::string& outMessage
    );

    bool getTransactions(const std::string& accountNumber, crow::json::wvalue::list& outTransactions);
};

#endif
