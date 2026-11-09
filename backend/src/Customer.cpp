#include "Customer.h"

// Constructor
Customer::Customer(string id, string name,
                   string mail, string mobile) {

    customerId = id;
    fullName = name;
    email = mail;
    phone = mobile;
    accountNumber = "";
}

// Set account number
void Customer::setAccountNumber(string acc) {

    accountNumber = acc;
}

// Get customer ID
string Customer::getCustomerId() const {
    return customerId;
}

// Get customer name
string Customer::getFullName() const {
    return fullName;
}

// Get email
string Customer::getEmail() const {
    return email;
}

// Get phone
string Customer::getPhone() const {
    return phone;
}

// Get account number
string Customer::getAccountNumber() const {
    return accountNumber;
}

// Update details
void Customer::updateDetails(string name, string mobile) {

    fullName = name;
    phone = mobile;

    cout << "Customer Details Updated Successfully!" << endl;
}

// Display customer information
void Customer::displayCustomer() const {

    cout << "\n===== SJNA BANK CUSTOMER =====" << endl;

    cout << "Customer ID: " << customerId << endl;
    cout << "Name: " << fullName << endl;
    cout << "Email: " << email << endl;
    cout << "Phone: " << phone << endl;
    cout << "Account Number: " << accountNumber << endl;
}