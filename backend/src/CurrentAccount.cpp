#include "CurrentAccount.h"

// Constructor
CurrentAccount::CurrentAccount(
    string acc, string name,
    double bal, double limit
) : BankAccount(acc, name, bal) {

    overdraftLimit = (limit >= 0) ? limit : 0;
}

// Withdrawal function
bool CurrentAccount::withdraw(double amount) {

    if (amount <= 0) {

        cout << "Invalid withdrawal amount!" << endl;
        return false;
    }

    if (amount > balance + overdraftLimit) {

        cout << "Overdraft Limit Exceeded!" << endl;
        return false;
    }

    balance -= amount;

    cout << "Current Account Withdrawal Successful!" << endl;

    return true;
}

// Get overdraft limit
double CurrentAccount::getOverdraftLimit() const {

    return overdraftLimit;
}