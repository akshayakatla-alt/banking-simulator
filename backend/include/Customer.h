#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <iostream>
#include <string>
using namespace std;

class Customer {

private:
    string customerId;
    string fullName;
    string email;
    string phone;
    string accountNumber;

public:

    // Constructor
    Customer(string id, string name,
             string mail, string mobile);

    // Set bank account number
    void setAccountNumber(string acc);

    // Get customer details
    string getCustomerId() const;
    string getFullName() const;
    string getEmail() const;
    string getPhone() const;
    string getAccountNumber() const;

    // Update customer details
    void updateDetails(string name, string mobile);

    // Display customer information
    void displayCustomer() const;
};

#endif