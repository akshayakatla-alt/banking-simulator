#ifndef CURRENTACCOUNT_H
#define CURRENTACCOUNT_H

#include "BankAccount.h"

class CurrentAccount : public BankAccount {

private:
    double overdraftLimit;

public:

    // Constructor
    CurrentAccount(string acc, string name,
                   double bal, double limit);

    // Override withdrawal function
    bool withdraw(double amount) override;

    // Display overdraft limit
    double getOverdraftLimit() const;

};

#endif