#ifndef SAVINGSACCOUNT_H
#define SAVINGSACCOUNT_H

#include "BankAccount.h"

class SavingsAccount : public BankAccount {

public:

    // Constructor
    SavingsAccount(string acc, string name, double bal);

    // Override withdrawal function
    bool withdraw(double amount) override;

};

#endif