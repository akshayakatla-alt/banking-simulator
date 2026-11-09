#include "SavingsAccount.h"

// Constructor
SavingsAccount::SavingsAccount(
    string acc, string name, double bal
) : BankAccount(acc, name, bal) {

}

// Withdrawal function
bool SavingsAccount::withdraw(double amount) {

    if (amount <= 0) {
        cout << "Invalid withdrawal amount!" << endl;
        return false;
    }

    if (amount > balance) {
        cout << "Insufficient Balance!" << endl;
        return false;
    }

    balance -= amount;

    cout << "Withdrawal Successful!" << endl;

    return true;
}