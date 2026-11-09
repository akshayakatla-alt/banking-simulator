#include "Bank.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <chrono>

Bank::Bank(Database& database) : db(database) {}

std::string Bank::generateAccountNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10000, 99999);
    return "SJNA" + std::to_string(dis(gen));
}

std::string Bank::generateTransactionId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 999);
    return "TXN" + std::to_string(ms % 10000000) + std::to_string(dis(gen));
}

std::string Bank::generatePaymentId() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(10, 99);
    return "BILL" + std::to_string(ms % 1000000) + std::to_string(dis(gen));
}

std::string Bank::resolveCustomerId(const std::string& email) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT customer_id FROM customers WHERE email = $1",
            email
        );
        if (!res.empty()) {
            return res[0]["customer_id"].as<std::string>();
        }
    } catch (...) {}
    return "";
}

std::string Bank::createSavingsAccount(
    const std::string& customerId,
    double initialBalance,
    std::string& outMessage
) {
    if (customerId.empty()) {
        outMessage = "Customer ID is required.";
        return "";
    }
    if (initialBalance < 0) {
        outMessage = "Initial deposit cannot be negative.";
        return "";
    }

    try {
        pqxx::work txn(db.getConnection());

        // Verify customer exists
        auto custCheck = txn.exec_params(
            "SELECT customer_id FROM customers WHERE customer_id = $1",
            customerId
        );
        if (custCheck.empty()) {
            outMessage = "Customer not found.";
            return "";
        }

        std::string accNo = generateAccountNumber();

        txn.exec_params(
            "INSERT INTO accounts (account_number, customer_id, account_type, balance, overdraft_limit, status) "
            "VALUES ($1, $2, 'Savings', $3, 0.00, 'Active')",
            accNo, customerId, initialBalance
        );

        if (initialBalance > 0) {
            std::string txnId = generateTransactionId();
            txn.exec_params(
                "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
                "VALUES ($1, '-', $2, 'Deposit', $3, 'Successful')",
                txnId, accNo, initialBalance
            );
        }

        txn.commit();
        outMessage = "Savings account created successfully.";
        std::cout << "[SJNA BANK] Created Savings Account: " << accNo << " for " << customerId << std::endl;
        return accNo;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error: ") + e.what();
        std::cerr << "[SJNA BANK ERROR] Create Savings failed: " << e.what() << std::endl;
        return "";
    }
}

std::string Bank::createCurrentAccount(
    const std::string& customerId,
    double initialBalance,
    double overdraftLimit,
    std::string& outMessage
) {
    if (customerId.empty()) {
        outMessage = "Customer ID is required.";
        return "";
    }
    if (initialBalance < 0 || overdraftLimit < 0) {
        outMessage = "Initial balance and overdraft limit cannot be negative.";
        return "";
    }

    try {
        pqxx::work txn(db.getConnection());

        auto custCheck = txn.exec_params(
            "SELECT customer_id FROM customers WHERE customer_id = $1",
            customerId
        );
        if (custCheck.empty()) {
            outMessage = "Customer not found.";
            return "";
        }

        std::string accNo = generateAccountNumber();

        txn.exec_params(
            "INSERT INTO accounts (account_number, customer_id, account_type, balance, overdraft_limit, status) "
            "VALUES ($1, $2, 'Current', $3, $4, 'Active')",
            accNo, customerId, initialBalance, overdraftLimit
        );

        if (initialBalance > 0) {
            std::string txnId = generateTransactionId();
            txn.exec_params(
                "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
                "VALUES ($1, '-', $2, 'Deposit', $3, 'Successful')",
                txnId, accNo, initialBalance
            );
        }

        txn.commit();
        outMessage = "Current account created successfully.";
        std::cout << "[SJNA BANK] Created Current Account: " << accNo << " for " << customerId << std::endl;
        return accNo;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error: ") + e.what();
        std::cerr << "[SJNA BANK ERROR] Create Current failed: " << e.what() << std::endl;
        return "";
    }
}

double Bank::getBalance(const std::string& accountNumber) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT balance FROM accounts WHERE account_number = $1",
            accountNumber
        );
        if (!res.empty()) {
            return res[0]["balance"].as<double>();
        }
    } catch (const std::exception& e) {
        std::cerr << "[SJNA BANK ERROR] Get balance failed: " << e.what() << std::endl;
    }
    return -1.0;
}

bool Bank::accountExists(const std::string& accountNumber) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT account_number FROM accounts WHERE account_number = $1",
            accountNumber
        );
        return !res.empty();
    } catch (...) {
        return false;
    }
}

bool Bank::getAccountDetails(const std::string& accountNumber, crow::json::wvalue& outAccount) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT account_number, customer_id, account_type, balance, overdraft_limit, status, created_at "
            "FROM accounts WHERE account_number = $1",
            accountNumber
        );
        if (res.empty()) return false;

        outAccount["accountNumber"] = res[0]["account_number"].as<std::string>();
        outAccount["customerId"] = res[0]["customer_id"].as<std::string>();
        outAccount["accountType"] = res[0]["account_type"].as<std::string>();
        outAccount["balance"] = res[0]["balance"].as<double>();
        outAccount["overdraftLimit"] = res[0]["overdraft_limit"].as<double>();
        outAccount["status"] = res[0]["status"].as<std::string>();
        outAccount["createdAt"] = res[0]["created_at"].as<std::string>();
        return true;
    } catch (...) {
        return false;
    }
}

bool Bank::getCustomerAccounts(const std::string& customerId, crow::json::wvalue::list& outAccounts) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        auto res = txn.exec_params(
            "SELECT account_number, account_type, balance, overdraft_limit, status, created_at "
            "FROM accounts WHERE customer_id = $1 ORDER BY created_at ASC",
            customerId
        );

        for (const auto& row : res) {
            crow::json::wvalue acc;
            acc["accountNumber"] = row["account_number"].as<std::string>();
            acc["accountType"] = row["account_type"].as<std::string>();
            acc["balance"] = row["balance"].as<double>();
            acc["overdraftLimit"] = row["overdraft_limit"].as<double>();
            acc["status"] = row["status"].as<std::string>();
            acc["createdAt"] = row["created_at"].as<std::string>();
            outAccounts.push_back(std::move(acc));
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SJNA BANK ERROR] getCustomerAccounts failed: " << e.what() << std::endl;
        return false;
    }
}

bool Bank::deposit(const std::string& accountNumber, double amount, std::string& outMessage) {
    if (amount <= 0) {
        outMessage = "Deposit amount must be greater than zero.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());

        auto res = txn.exec_params(
            "SELECT balance FROM accounts WHERE account_number = $1 FOR UPDATE",
            accountNumber
        );
        if (res.empty()) {
            outMessage = "Account not found.";
            return false;
        }

        txn.exec_params(
            "UPDATE accounts SET balance = balance + $1 WHERE account_number = $2",
            amount, accountNumber
        );

        std::string txnId = generateTransactionId();
        txn.exec_params(
            "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
            "VALUES ($1, '-', $2, 'Deposit', $3, 'Successful')",
            txnId, accountNumber, amount
        );

        txn.commit();
        outMessage = "Deposit successful.";
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error during deposit: ") + e.what();
        return false;
    }
}

bool Bank::withdraw(const std::string& accountNumber, double amount, std::string& outMessage) {
    if (amount <= 0) {
        outMessage = "Withdrawal amount must be greater than zero.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());

        auto res = txn.exec_params(
            "SELECT balance, overdraft_limit FROM accounts WHERE account_number = $1 FOR UPDATE",
            accountNumber
        );
        if (res.empty()) {
            outMessage = "Account not found.";
            return false;
        }

        double balance = res[0]["balance"].as<double>();
        double overdraft = res[0]["overdraft_limit"].as<double>();

        if (balance + overdraft < amount) {
            outMessage = "Insufficient funds. Available limit exceeded.";
            return false;
        }

        txn.exec_params(
            "UPDATE accounts SET balance = balance - $1 WHERE account_number = $2",
            amount, accountNumber
        );

        std::string txnId = generateTransactionId();
        txn.exec_params(
            "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
            "VALUES ($1, $2, '-', 'Withdrawal', $3, 'Successful')",
            txnId, accountNumber, amount
        );

        txn.commit();
        outMessage = "Withdrawal successful.";
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error during withdrawal: ") + e.what();
        return false;
    }
}

bool Bank::transfer(
    const std::string& sender,
    const std::string& receiver,
    double amount,
    const std::string& remarks,
    std::string& outTxnId,
    std::string& outMessage
) {
    if (sender == receiver) {
        outMessage = "Sender and receiver accounts cannot be the same.";
        return false;
    }
    if (amount <= 0) {
        outMessage = "Transfer amount must be greater than zero.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());

        // Lock sender
        auto senderRes = txn.exec_params(
            "SELECT balance, overdraft_limit, status FROM accounts WHERE account_number = $1 FOR UPDATE",
            sender
        );
        if (senderRes.empty()) {
            outMessage = "Sender account not found.";
            return false;
        }

        // Lock receiver
        auto receiverRes = txn.exec_params(
            "SELECT balance, status FROM accounts WHERE account_number = $1 FOR UPDATE",
            receiver
        );
        if (receiverRes.empty()) {
            outMessage = "Receiver account not found in SJNA Bank.";
            return false;
        }

        double senderBal = senderRes[0]["balance"].as<double>();
        double senderOverdraft = senderRes[0]["overdraft_limit"].as<double>();

        if (senderBal + senderOverdraft < amount) {
            outMessage = "Transfer failed: Insufficient funds in sender account.";
            return false;
        }

        // Debit sender
        txn.exec_params(
            "UPDATE accounts SET balance = balance - $1 WHERE account_number = $2",
            amount, sender
        );

        // Credit receiver
        txn.exec_params(
            "UPDATE accounts SET balance = balance + $1 WHERE account_number = $2",
            amount, receiver
        );

        // Record transaction
        outTxnId = generateTransactionId();
        txn.exec_params(
            "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
            "VALUES ($1, $2, $3, 'Fund Transfer', $4, 'Successful')",
            outTxnId, sender, receiver, amount
        );

        txn.commit();
        outMessage = "Fund transfer completed successfully.";
        std::cout << "[SJNA BANK] Transfer Successful: " << sender << " -> " << receiver << " (₹" << amount << ") Txn: " << outTxnId << std::endl;
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database transaction error: ") + e.what();
        return false;
    }
}

bool Bank::payBill(
    const std::string& accountNumber,
    const std::string& billCategory,
    const std::string& billerName,
    const std::string& consumerNumber,
    double amount,
    std::string& outTxnId,
    std::string& outMessage
) {
    if (amount <= 0) {
        outMessage = "Bill payment amount must be greater than zero.";
        return false;
    }

    try {
        pqxx::work txn(db.getConnection());

        auto res = txn.exec_params(
            "SELECT balance, overdraft_limit FROM accounts WHERE account_number = $1 FOR UPDATE",
            accountNumber
        );
        if (res.empty()) {
            outMessage = "Payment account not found.";
            return false;
        }

        double balance = res[0]["balance"].as<double>();
        double overdraft = res[0]["overdraft_limit"].as<double>();

        if (balance + overdraft < amount) {
            outMessage = "Insufficient account balance for bill payment.";
            return false;
        }

        // Debit account
        txn.exec_params(
            "UPDATE accounts SET balance = balance - $1 WHERE account_number = $2",
            amount, accountNumber
        );

        // Record in bill_payments
        std::string payId = generatePaymentId();
        txn.exec_params(
            "INSERT INTO bill_payments (payment_id, account_number, bill_category, biller_name, consumer_number, amount, status) "
            "VALUES ($1, $2, $3, $4, $5, $6, 'Successful')",
            payId, accountNumber, billCategory, billerName, consumerNumber, amount
        );

        // Record in transactions
        outTxnId = generateTransactionId();
        txn.exec_params(
            "INSERT INTO transactions (transaction_id, sender_account, receiver_account, transaction_type, amount, status) "
            "VALUES ($1, $2, $3, 'Bill Payment', $4, 'Successful')",
            outTxnId, accountNumber, billerName, amount
        );

        txn.commit();
        outMessage = "Bill payment processed successfully.";
        std::cout << "[SJNA BANK] Bill Paid: " << billerName << " (" << billCategory << ") - ₹" << amount << std::endl;
        return true;
    } catch (const std::exception& e) {
        outMessage = std::string("Database error during bill payment: ") + e.what();
        return false;
    }
}

bool Bank::getTransactions(const std::string& accountNumber, crow::json::wvalue::list& outTransactions) {
    try {
        pqxx::read_transaction txn(db.getConnection());
        pqxx::result res;

        if (accountNumber.empty()) {
            res = txn.exec(
                "SELECT transaction_id, sender_account, receiver_account, transaction_type, amount, status, transaction_date "
                "FROM transactions ORDER BY transaction_date DESC LIMIT 100"
            );
        } else {
            res = txn.exec_params(
                "SELECT transaction_id, sender_account, receiver_account, transaction_type, amount, status, transaction_date "
                "FROM transactions WHERE sender_account = $1 OR receiver_account = $1 ORDER BY transaction_date DESC LIMIT 100",
                accountNumber
            );
        }

        for (const auto& row : res) {
            crow::json::wvalue item;
            item["transactionId"] = row["transaction_id"].as<std::string>();
            item["senderAccount"] = row["sender_account"].as<std::string>();
            item["receiverAccount"] = row["receiver_account"].as<std::string>();
            item["transactionType"] = row["transaction_type"].as<std::string>();
            item["amount"] = row["amount"].as<double>();
            item["status"] = row["status"].as<std::string>();
            item["date"] = row["transaction_date"].as<std::string>();
            outTransactions.push_back(std::move(item));
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SJNA BANK ERROR] getTransactions failed: " << e.what() << std::endl;
        return false;
    }
}
