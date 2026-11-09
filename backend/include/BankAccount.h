
#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <iostream>
#include <string>
using namespace std;

class BankAccount {

protected:
    string accountNumber;
    string holderName;
    double balance;

public:
    // Constructor
    BankAccount(string acc, string name, double bal);

    // Deposit money
    virtual void deposit(double amount);

    // Pure virtual function
    virtual bool withdraw(double amount) = 0;

    // Display account details
    void displayDetails() const;

    // Getters
    double getBalance() const;
    string getAccountNumber() const;
    string getHolderName() const;

    // Virtual destructor
    virtual ~BankAccount();
};

#endif