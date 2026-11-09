CREATE TABLE IF NOT EXISTS customers (
    customer_id VARCHAR(30) PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(150) UNIQUE NOT NULL,
    phone VARCHAR(15) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS accounts (
    account_number VARCHAR(30) PRIMARY KEY,
    customer_id VARCHAR(30) REFERENCES customers(customer_id),
    account_type VARCHAR(20) NOT NULL,
    balance NUMERIC(15,2) DEFAULT 0,
    overdraft_limit NUMERIC(15,2) DEFAULT 0,
    status VARCHAR(20) DEFAULT 'Active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS transactions (
    transaction_id VARCHAR(30) PRIMARY KEY,
    sender_account VARCHAR(30),
    receiver_account VARCHAR(30),
    transaction_type VARCHAR(30) NOT NULL,
    amount NUMERIC(15,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'Successful',
    transaction_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    customer_id VARCHAR(30) REFERENCES customers(customer_id),
    email VARCHAR(150) UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    role VARCHAR(20) DEFAULT 'customer',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS bill_payments (
    payment_id VARCHAR(30) PRIMARY KEY,
    account_number VARCHAR(30) REFERENCES accounts(account_number),
    bill_category VARCHAR(50) NOT NULL,
    biller_name VARCHAR(100) NOT NULL,
    consumer_number VARCHAR(50) NOT NULL,
    amount NUMERIC(15,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'Successful',
    payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
