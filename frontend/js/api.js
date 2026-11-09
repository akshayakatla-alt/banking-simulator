/**
 * SJNA BANK - API Client Layer
 * Handles authentication headers, JWT tokens, and REST API calls
 */

const API_BASE_URL = window.API_BASE_URL || "http://localhost:8080";

const API = {
  // Authentication token management
  getToken() {
    return localStorage.getItem("sjna_token") || "";
  },

  setToken(token) {
    if (token) {
      localStorage.setItem("sjna_token", token);
    } else {
      localStorage.removeItem("sjna_token");
    }
  },

  getCurrentUser() {
    const raw = localStorage.getItem("sjna_user");
    try {
      return raw ? JSON.parse(raw) : null;
    } catch (e) {
      return null;
    }
  },

  setCurrentUser(user) {
    if (user) {
      localStorage.setItem("sjna_user", JSON.stringify(user));
    } else {
      localStorage.removeItem("sjna_user");
    }
  },

  getActiveAccount() {
    return localStorage.getItem("sjna_active_acc") || "";
  },

  setActiveAccount(accountNumber) {
    if (accountNumber) {
      localStorage.setItem("sjna_active_acc", accountNumber);
    } else {
      localStorage.removeItem("sjna_active_acc");
    }
  },

  // Core Request Helper
  async request(endpoint, options = {}) {
    const url = `${API_BASE_URL}${endpoint}`;
    const headers = {
      "Content-Type": "application/json",
      ...(options.headers || {})
    };

    const token = this.getToken();
    if (token) {
      headers["Authorization"] = `Bearer ${token}`;
    }

    try {
      const response = await fetch(url, {
        ...options,
        headers
      });

      const data = await response.json().catch(() => ({
        success: false,
        message: "Failed to parse server response"
      }));

      if (response.status === 401) {
        // Token expired or invalid
        if (this.getToken()) {
          this.setToken(null);
          this.setCurrentUser(null);
          window.dispatchEvent(new CustomEvent("sjna:unauthorized"));
        }
      }

      return {
        ok: response.ok,
        status: response.status,
        data
      };
    } catch (err) {
      console.error("Network or API Error:", err);
      return {
        ok: false,
        status: 0,
        data: {
          success: false,
          message: "Unable to connect to SJNA Bank server. Please ensure the backend is running on port 8080."
        }
      };
    }
  },

  // Auth Endpoints
  async register(name, email, phone, password) {
    return this.request("/api/register", {
      method: "POST",
      body: JSON.stringify({ name, email, phone, password })
    });
  },

  async login(email, password) {
    const res = await this.request("/api/login", {
      method: "POST",
      body: JSON.stringify({ email, password })
    });

    if (res.ok && res.data && res.data.token) {
      this.setToken(res.data.token);
      if (res.data.user) {
        this.setCurrentUser(res.data.user);
      }
    }
    return res;
  },

  logout() {
    this.setToken(null);
    this.setCurrentUser(null);
    this.setActiveAccount(null);
  },

  // Account Endpoints
  async createSavingsAccount(initialBalance) {
    const user = this.getCurrentUser() || {};
    return this.request("/api/account/savings", {
      method: "POST",
      body: JSON.stringify({
        name: user.name || "",
        email: user.email || "",
        phone: user.phone || "",
        initialBalance: parseFloat(initialBalance) || 0
      })
    });
  },

  async createCurrentAccount(initialBalance, overdraftLimit) {
    const user = this.getCurrentUser() || {};
    return this.request("/api/account/current", {
      method: "POST",
      body: JSON.stringify({
        name: user.name || "",
        email: user.email || "",
        phone: user.phone || "",
        initialBalance: parseFloat(initialBalance) || 0,
        overdraftLimit: parseFloat(overdraftLimit) || 0
      })
    });
  },

  async getBalance(accountNumber) {
    return this.request(`/api/account/balance/${encodeURIComponent(accountNumber)}`, {
      method: "GET"
    });
  },

  async getMyAccounts() {
    return this.request("/api/account/my-accounts", {
      method: "GET"
    });
  },

  async deposit(accountNumber, amount) {
    return this.request("/api/account/deposit", {
      method: "POST",
      body: JSON.stringify({
        accountNumber,
        amount: parseFloat(amount)
      })
    });
  },

  async withdraw(accountNumber, amount) {
    return this.request("/api/account/withdraw", {
      method: "POST",
      body: JSON.stringify({
        accountNumber,
        amount: parseFloat(amount)
      })
    });
  },

  // Fund Transfer & Transactions
  async transferFunds(senderAccount, receiverAccount, amount, remarks = "") {
    return this.request("/api/transactions/transfer", {
      method: "POST",
      body: JSON.stringify({
        senderAccount,
        receiverAccount,
        amount: parseFloat(amount),
        remarks
      })
    });
  },

  async getTransactionHistory(accountNumber = "") {
    const query = accountNumber ? `?accountNumber=${encodeURIComponent(accountNumber)}` : "";
    return this.request(`/api/transactions/history${query}`, {
      method: "GET"
    });
  },

  async payBill(accountNumber, billCategory, billerName, consumerNumber, amount) {
    return this.request("/api/payments/bill", {
      method: "POST",
      body: JSON.stringify({
        accountNumber,
        billCategory,
        billerName,
        consumerNumber,
        amount: parseFloat(amount)
      })
    });
  },

  // Profile Endpoints
  async getProfile() {
    return this.request("/api/profile", {
      method: "GET"
    });
  },

  async updateProfile(name, phone) {
    return this.request("/api/profile", {
      method: "PUT",
      body: JSON.stringify({ name, phone })
    });
  },

  async changePassword(oldPassword, newPassword) {
    return this.request("/api/profile/change-password", {
      method: "POST",
      body: JSON.stringify({ oldPassword, newPassword })
    });
  }
};

window.API = API;
