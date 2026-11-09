#include "Transaction.h"

// Constructor
Transaction::Transaction(
    string id,
    string sender,
    string receiver,
    string type,
    double amt,
    string date
) {

    transactionId = id;
    senderAccount = sender;
    receiverAccount = receiver;
    transactionType = type;
    amount = amt;
    transactionDate = date;
    status = "Pending";
}

// Get Transaction ID
string Transaction::getTransactionId() const {
    return transactionId;
}

// Get Transaction Type
string Transaction::getTransactionType() const {
    return transactionType;
}

// Get Amount
double Transaction::getAmount() const {
    return amount;
}

// Get Status
string Transaction::getStatus() const {
    return status;
}

// Get Sender Account
string Transaction::getSenderAccount() const {
    return senderAccount;
}

// Get Receiver Account
string Transaction::getReceiverAccount() const {
    return receiverAccount;
}

// Get Transaction Date
string Transaction::getTransactionDate() const {
    return transactionDate;
}

// Update Transaction Status
void Transaction::setStatus(string newStatus) {
    status = newStatus;
}

// Display Transaction Details
void Transaction::displayTransaction() const {

    cout << "\n===== SJNA BANK TRANSACTION =====" << endl;

    cout << "Transaction ID: "
         << transactionId << endl;

    cout << "Sender Account: "
         << senderAccount << endl;

    cout << "Receiver Account: "
         << receiverAccount << endl;

    cout << "Transaction Type: "
         << transactionType << endl;

    cout << "Amount: Rs. "
         << amount << endl;

    cout << "Transaction Date: "
         << transactionDate << endl;

    cout << "Status: "
         << status << endl;
}