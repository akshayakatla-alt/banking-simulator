
#include "BankAccount.h"

// Constructor
BankAccount::BankAccount(string acc, string name, double bal) {

    accountNumber = acc;
    holderName = name;
    balance = bal;
}

// Deposit function
void BankAccount::deposit(double amount) {

    if (amount > 0) {
        balance += amount;
        cout << "Deposit Successful!" << endl;
    }
    else {
        cout << "Invalid Deposit Amount!" << endl;
    }
}

// Display account details
void BankAccount::displayDetails() const {

    cout << "\n----- SJNA BANK ACCOUNT -----" << endl;
    cout << "Account Number: " << accountNumber << endl;
    cout << "Account Holder: " << holderName << endl;
    cout << "Balance: Rs. " << balance << endl;
}

// Get balance
double BankAccount::getBalance() const {
    return balance;
}

// Get account number
string BankAccount::getAccountNumber() const {
    return accountNumber;
}

// Get holder name
string BankAccount::getHolderName() const {
    return holderName;
}

// Destructor
BankAccount::~BankAccount() {
    cout << "Bank Account Object Destroyed" << endl;
}