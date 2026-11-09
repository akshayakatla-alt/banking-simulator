#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <iostream>
#include <string>

using namespace std;

class Transaction {

private:

    string transactionId;
    string senderAccount;
    string receiverAccount;
    string transactionType;

    double amount;

    string transactionDate;
    string status;

public:

    // Constructor
    Transaction(
        string id,
        string sender,
        string receiver,
        string type,
        double amt,
        string date
    );

    // Get transaction ID
    string getTransactionId() const;

    // Get transaction type
    string getTransactionType() const;

    // Get transaction amount
    double getAmount() const;

    // Get transaction status
    string getStatus() const;

    // Get sender account
    string getSenderAccount() const;

    // Get receiver account
    string getReceiverAccount() const;

    // Get transaction date
    string getTransactionDate() const;

    // Update transaction status
    void setStatus(string newStatus);

    // Display transaction details
    void displayTransaction() const;

};

#endif