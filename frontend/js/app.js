/**
 * SJNA BANK - Main Application Logic
 * Comprehensive Online Banking Transaction Simulator
 */

document.addEventListener("DOMContentLoaded", () => {
  // State variables
  let currentUser = API.getCurrentUser();
  let accountsList = [];
  let currentActiveAccount = null;
  let allTransactions = [];
  let selectedBillCategory = "electricity";
  let pendingTransferData = null;

  // DOM Elements
  const authSection = document.getElementById("auth-section");
  const dashboardSection = document.getElementById("dashboard-section");
  const userGreeting = document.getElementById("user-greeting");
  const userHeaderName = document.getElementById("user-header-name");
  const userHeaderAvatar = document.getElementById("user-header-avatar");
  const headerUserSection = document.getElementById("header-user-section");
  const accountSelect = document.getElementById("account-select");

  // Navigation Items & Views
  const navItems = document.querySelectorAll(".nav-item");
  const viewSections = document.querySelectorAll(".view-section");

  // Init App
  init();

  function init() {
    setupEventListeners();
    checkAuthState();
  }

  function checkAuthState() {
    currentUser = API.getCurrentUser();
    const token = API.getToken();

    if (currentUser && token) {
      authSection.style.display = "none";
      dashboardSection.style.display = "block";
      headerUserSection.style.display = "flex";

      const displayName = currentUser.name || "Customer";
      userGreeting.textContent = displayName;
      userHeaderName.textContent = displayName;
      userHeaderAvatar.textContent = displayName.charAt(0).toUpperCase();

      loadCustomerData();
    } else {
      authSection.style.display = "block";
      dashboardSection.style.display = "none";
      headerUserSection.style.display = "none";
    }
  }

  // Load all accounts and transactions
  async function loadCustomerData() {
    await fetchAccounts();
    await fetchTransactions();
    await fetchProfile();
  }

  // 1. Fetch Accounts
  async function fetchAccounts() {
    const res = await API.getMyAccounts();
    if (res.ok && res.data && res.data.success) {
      accountsList = res.data.accounts || [];
      renderAccountDropdown();
      renderAccountsView();
      updateActiveAccountCard();
    } else {
      // If none exist or backend returns empty, attempt to check active account by balance
      const savedAcc = API.getActiveAccount();
      if (savedAcc) {
        const balRes = await API.getBalance(savedAcc);
        if (balRes.ok && balRes.data && balRes.data.success) {
          accountsList = [{
            accountNumber: savedAcc,
            accountType: "Savings",
            balance: balRes.data.balance,
            overdraftLimit: 0,
            status: "Active"
          }];
          renderAccountDropdown();
          renderAccountsView();
          updateActiveAccountCard();
        }
      }
    }
  }

  // 2. Render Account Dropdown in Dashboard Header & Forms
  function renderAccountDropdown() {
    accountSelect.innerHTML = "";
    const transferSender = document.getElementById("transfer-sender");
    const atmAccount = document.getElementById("atm-account");
    const billAccount = document.getElementById("bill-account");

    if (transferSender) transferSender.innerHTML = "";
    if (atmAccount) atmAccount.innerHTML = "";
    if (billAccount) billAccount.innerHTML = "";

    if (accountsList.length === 0) {
      accountSelect.innerHTML = `<option value="">No Accounts Found</option>`;
      return;
    }

    const savedAcc = API.getActiveAccount();
    let foundSaved = false;

    accountsList.forEach((acc) => {
      const isSelected = acc.accountNumber === savedAcc;
      if (isSelected) foundSaved = true;

      const optText = `${acc.accountNumber} (${acc.accountType} - ₹${formatINR(acc.balance)})`;

      const opt = new Option(optText, acc.accountNumber, false, isSelected);
      accountSelect.add(opt);

      if (transferSender) {
        transferSender.add(new Option(optText, acc.accountNumber, false, isSelected));
      }
      if (atmAccount) {
        atmAccount.add(new Option(optText, acc.accountNumber, false, isSelected));
      }
      if (billAccount) {
        billAccount.add(new Option(optText, acc.accountNumber, false, isSelected));
      }
    });

    if (!foundSaved && accountsList.length > 0) {
      accountSelect.selectedIndex = 0;
      API.setActiveAccount(accountsList[0].accountNumber);
      currentActiveAccount = accountsList[0];
    } else {
      currentActiveAccount = accountsList.find(a => a.accountNumber === savedAcc) || accountsList[0];
    }
  }

  // 3. Update the Active Card on Dashboard
  function updateActiveAccountCard() {
    if (!currentActiveAccount && accountsList.length > 0) {
      currentActiveAccount = accountsList[0];
    }

    const cardAccNumber = document.getElementById("card-acc-number");
    const cardHolderName = document.getElementById("card-holder-name");
    const cardBalance = document.getElementById("card-balance");
    const cardTypeBadge = document.getElementById("card-type-badge");
    const statTotalBalance = document.getElementById("stat-total-balance");

    if (currentActiveAccount) {
      if (cardAccNumber) cardAccNumber.textContent = maskAccountNumber(currentActiveAccount.accountNumber);
      if (cardHolderName) cardHolderName.textContent = currentUser ? currentUser.name : "Account Holder";
      if (cardBalance) cardBalance.textContent = `₹ ${formatINR(currentActiveAccount.balance)}`;
      if (cardTypeBadge) cardTypeBadge.textContent = `${currentActiveAccount.accountType} A/C`;

      // Update stat card
      const totalBal = accountsList.reduce((acc, curr) => acc + (parseFloat(curr.balance) || 0), 0);
      if (statTotalBalance) statTotalBalance.textContent = `₹ ${formatINR(totalBal)}`;
    } else {
      if (cardAccNumber) cardAccNumber.textContent = "NO ACCOUNT";
      if (cardBalance) cardBalance.textContent = "₹ 0.00";
    }
  }

  // 4. Render Accounts View
  function renderAccountsView() {
    const container = document.getElementById("accounts-list-container");
    if (!container) return;

    if (accountsList.length === 0) {
      container.innerHTML = `
        <div style="text-align: center; padding: 2.5rem; color: var(--text-secondary);">
          <p style="font-size: 1.1rem; margin-bottom: 1rem;">No bank accounts found for your profile.</p>
          <button class="btn btn-primary" onclick="window.openCreateAccountModal()">+ Open Your First Bank Account</button>
        </div>
      `;
      return;
    }

    container.innerHTML = accountsList.map(acc => `
      <div class="content-card" style="margin-bottom: 1rem; border-left: 4px solid var(--primary);">
        <div class="card-header-flex">
          <div>
            <h4 style="font-size: 1.05rem; font-weight: 700; color: var(--primary);">${acc.accountNumber}</h4>
            <span class="badge ${acc.accountType === 'Savings' ? 'badge-info' : 'badge-warning'}">${acc.accountType} Account</span>
            <span class="badge badge-success" style="margin-left: 0.35rem;">${acc.status || 'Active'}</span>
          </div>
          <div style="text-align: right;">
            <span style="font-size: 0.75rem; color: var(--text-muted); display: block;">AVAILABLE BALANCE</span>
            <span style="font-size: 1.4rem; font-weight: 800; color: var(--success);">₹ ${formatINR(acc.balance)}</span>
          </div>
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.85rem; color: var(--text-secondary); margin-top: 0.5rem;">
          <div>
            ${acc.accountType === 'Current' ? `<span>Overdraft Limit: <strong>₹ ${formatINR(acc.overdraftLimit || 0)}</strong></span>` : `<span>Interest Rate: <strong>4.0% p.a.</strong></span>`}
            <span style="margin: 0 0.5rem;">•</span>
            <span>IFSC: <strong>SJNA0004567</strong></span>
          </div>
          <div>
            <button class="btn btn-outline" style="padding: 0.4rem 0.8rem; font-size: 0.78rem;" onclick="window.selectAccount('${acc.accountNumber}')">
              ${acc.accountNumber === (currentActiveAccount ? currentActiveAccount.accountNumber : "") ? 'Active Account' : 'Set as Primary'}
            </button>
          </div>
        </div>
      </div>
    `).join("");
  }

  // 5. Fetch & Render Transactions
  async function fetchTransactions() {
    const res = await API.getTransactionHistory();
    if (res.ok && res.data && res.data.success) {
      allTransactions = res.data.transactions || [];
      renderTransactionsTable(allTransactions);
      updateTransactionStats();
    }
  }

  function renderTransactionsTable(transactions) {
    const tbody = document.getElementById("transaction-history-tbody");
    const dashTbody = document.getElementById("dashboard-recent-txns-tbody");

    const rowsHtml = (list) => {
      if (!list || list.length === 0) {
        return `<tr><td colspan="7" style="text-align: center; color: var(--text-muted); padding: 2rem;">No transaction history available.</td></tr>`;
      }
      return list.map(tx => {
        const isCredit = tx.transactionType === "Deposit" ||
          (tx.receiverAccount === (currentActiveAccount ? currentActiveAccount.accountNumber : ""));

        const typeBadgeClass =
          tx.transactionType === "Deposit" ? "badge-success" :
          tx.transactionType === "Withdrawal" ? "badge-warning" :
          tx.transactionType === "Bill Payment" ? "badge-info" : "badge-primary";

        return `
          <tr>
            <td><strong style="font-family: monospace;">${tx.transactionId}</strong></td>
            <td>${tx.date || "-"}</td>
            <td><span class="badge ${typeBadgeClass}">${tx.transactionType}</span></td>
            <td><span style="font-family: monospace;">${tx.senderAccount || "-"}</span></td>
            <td><span style="font-family: monospace;">${tx.receiverAccount || "-"}</span></td>
            <td class="${isCredit ? 'amount-credit' : 'amount-debit'}">
              ${isCredit ? '+' : '-'} ₹ ${formatINR(tx.amount)}
            </td>
            <td><span class="badge badge-success">${tx.status || "Completed"}</span></td>
            <td>
              <button class="btn btn-outline" style="padding: 0.25rem 0.6rem; font-size: 0.75rem;" onclick="window.showReceiptModal('${tx.transactionId}')">
                Receipt
              </button>
            </td>
          </tr>
        `;
      }).join("");
    };

    if (tbody) tbody.innerHTML = rowsHtml(transactions);
    if (dashTbody) dashTbody.innerHTML = rowsHtml(transactions.slice(0, 5));
  }

  function updateTransactionStats() {
    const statCredit = document.getElementById("stat-total-credit");
    const statDebit = document.getElementById("stat-total-debit");
    const statCount = document.getElementById("stat-txn-count");

    let totalCredit = 0;
    let totalDebit = 0;

    allTransactions.forEach(t => {
      const amt = parseFloat(t.amount) || 0;
      if (t.transactionType === "Deposit") {
        totalCredit += amt;
      } else {
        totalDebit += amt;
      }
    });

    if (statCredit) statCredit.textContent = `₹ ${formatINR(totalCredit)}`;
    if (statDebit) statDebit.textContent = `₹ ${formatINR(totalDebit)}`;
    if (statCount) statCount.textContent = allTransactions.length;
  }

  // 6. Profile Handling
  async function fetchProfile() {
    const res = await API.getProfile();
    if (res.ok && res.data && res.data.success) {
      const prof = res.data.profile;
      document.getElementById("profile-name-display").textContent = prof.name;
      document.getElementById("profile-email-display").textContent = prof.email;
      document.getElementById("profile-phone-display").textContent = prof.phone;
      document.getElementById("profile-id-display").textContent = prof.customerId;

      // Fill edit inputs
      const editName = document.getElementById("edit-profile-name");
      const editPhone = document.getElementById("edit-profile-phone");
      if (editName) editName.value = prof.name;
      if (editPhone) editPhone.value = prof.phone;
    }
  }

  // Event Listeners Setup
  function setupEventListeners() {
    // Auth Tab switching
    const authTabs = document.querySelectorAll(".auth-tab");
    const formLogin = document.getElementById("form-login");
    const formRegister = document.getElementById("form-register");

    authTabs.forEach(tab => {
      tab.addEventListener("click", () => {
        authTabs.forEach(t => t.classList.remove("active"));
        tab.classList.add("active");
        const target = tab.getAttribute("data-tab");
        if (target === "login") {
          formLogin.style.display = "block";
          formRegister.style.display = "none";
        } else {
          formLogin.style.display = "none";
          formRegister.style.display = "block";
        }
      });
    });

    // Login Form
    if (formLogin) {
      formLogin.addEventListener("submit", async (e) => {
        e.preventDefault();
        const email = document.getElementById("login-email").value.trim();
        const password = document.getElementById("login-password").value;

        if (!email || !password) {
          showToast("Please enter email and password.", "error");
          return;
        }

        const btn = formLogin.querySelector("button[type='submit']");
        btn.disabled = true;
        btn.textContent = "Authenticating...";

        const res = await API.login(email, password);
        btn.disabled = false;
        btn.textContent = "Sign In to SJNA Bank";

        if (res.ok && res.data && res.data.success) {
          showToast("Authentication successful! Welcome to SJNA Bank.", "success");
          checkAuthState();
        } else {
          showToast(res.data?.message || "Invalid email or password.", "error");
        }
      });
    }

    // Register Form
    if (formRegister) {
      formRegister.addEventListener("submit", async (e) => {
        e.preventDefault();
        const name = document.getElementById("reg-name").value.trim();
        const email = document.getElementById("reg-email").value.trim();
        const phone = document.getElementById("reg-phone").value.trim();
        const password = document.getElementById("reg-password").value;
        const initialBalance = parseFloat(document.getElementById("reg-initial-balance").value) || 1000;

        if (!name || !email || !phone || !password) {
          showToast("Please fill all required registration fields.", "error");
          return;
        }

        if (password.length < 8) {
          showToast("Password must be at least 8 characters long.", "error");
          return;
        }

        const btn = formRegister.querySelector("button[type='submit']");
        btn.disabled = true;
        btn.textContent = "Creating Account...";

        const res = await API.register(name, email, phone, password);
        btn.disabled = false;
        btn.textContent = "Register Customer Account";

        if (res.ok && res.data && res.data.success) {
          showToast("Registration successful! Logging in...", "success");
          // Auto login after registration
          const loginRes = await API.login(email, password);
          if (loginRes.ok && loginRes.data && loginRes.data.success) {
            // Create default Savings account
            await API.createSavingsAccount(initialBalance);
            checkAuthState();
          }
        } else {
          showToast(res.data?.message || "Registration failed. Duplicate email or phone.", "error");
        }
      });
    }

    // Logout Button
    const btnLogout = document.getElementById("btn-logout");
    if (btnLogout) {
      btnLogout.addEventListener("click", () => {
        API.logout();
        showToast("Logged out successfully.", "info");
        checkAuthState();
      });
    }

    // Navigation Items
    navItems.forEach(item => {
      item.addEventListener("click", () => {
        const targetView = item.getAttribute("data-view");
        switchView(targetView);
      });
    });

    // Account selector change
    if (accountSelect) {
      accountSelect.addEventListener("change", (e) => {
        const accNo = e.target.value;
        API.setActiveAccount(accNo);
        currentActiveAccount = accountsList.find(a => a.accountNumber === accNo) || null;
        updateActiveAccountCard();
        renderAccountsView();
      });
    }

    // Create Account Form inside Modal
    const formCreateAccount = document.getElementById("form-create-account");
    if (formCreateAccount) {
      const accTypeSelect = document.getElementById("new-account-type");
      const overdraftGroup = document.getElementById("group-overdraft-limit");

      if (accTypeSelect) {
        accTypeSelect.addEventListener("change", (e) => {
          if (e.target.value === "Current") {
            overdraftGroup.style.display = "block";
          } else {
            overdraftGroup.style.display = "none";
          }
        });
      }

      formCreateAccount.addEventListener("submit", async (e) => {
        e.preventDefault();
        const type = accTypeSelect.value;
        const initialBal = parseFloat(document.getElementById("new-initial-balance").value) || 0;
        const overdraft = parseFloat(document.getElementById("new-overdraft-limit").value) || 0;

        let res;
        if (type === "Savings") {
          res = await API.createSavingsAccount(initialBal);
        } else {
          res = await API.createCurrentAccount(initialBal, overdraft);
        }

        if (res.ok && res.data && res.data.success) {
          showToast(`New ${type} Account created successfully: ${res.data.accountNumber}`, "success");
          closeModal("modal-create-account");
          await fetchAccounts();
        } else {
          showToast(res.data?.message || "Failed to create account.", "error");
        }
      });
    }

    // Fund Transfer Form
    const formTransfer = document.getElementById("form-transfer");
    if (formTransfer) {
      formTransfer.addEventListener("submit", (e) => {
        e.preventDefault();
        const sender = document.getElementById("transfer-sender").value;
        const receiver = document.getElementById("transfer-receiver").value.trim().toUpperCase();
        const amount = parseFloat(document.getElementById("transfer-amount").value);
        const remarks = document.getElementById("transfer-remarks").value.trim();

        if (!sender) {
          showToast("Please select your sender account.", "error");
          return;
        }

        if (!receiver) {
          showToast("Please enter a valid receiver account number (e.g. SJNA10002).", "error");
          return;
        }

        if (sender === receiver) {
          showToast("Sender and receiver accounts cannot be the same.", "error");
          return;
        }

        if (!amount || amount <= 0) {
          showToast("Please enter a valid positive transfer amount.", "error");
          return;
        }

        // Store data for confirmation modal
        pendingTransferData = { sender, receiver, amount, remarks };

        // Populate confirmation modal
        document.getElementById("confirm-transfer-sender").textContent = sender;
        document.getElementById("confirm-transfer-receiver").textContent = receiver;
        document.getElementById("confirm-transfer-amount").textContent = `₹ ${formatINR(amount)}`;
        document.getElementById("confirm-transfer-remarks").textContent = remarks || "None";

        openModal("modal-transfer-confirm");
      });
    }

    // Confirm Transfer Execution
    const btnExecuteTransfer = document.getElementById("btn-execute-transfer");
    if (btnExecuteTransfer) {
      btnExecuteTransfer.addEventListener("click", async () => {
        if (!pendingTransferData) return;

        btnExecuteTransfer.disabled = true;
        btnExecuteTransfer.textContent = "Processing Transfer...";

        const res = await API.transferFunds(
          pendingTransferData.sender,
          pendingTransferData.receiver,
          pendingTransferData.amount,
          pendingTransferData.remarks
        );

        btnExecuteTransfer.disabled = false;
        btnExecuteTransfer.textContent = "Confirm & Execute Transfer";
        closeModal("modal-transfer-confirm");

        if (res.ok && res.data && res.data.success) {
          showToast("Fund transfer processed successfully!", "success");
          document.getElementById("form-transfer").reset();

          // Refresh data
          await loadCustomerData();

          // Display Receipt Modal
          showReceiptModal(res.data.transactionId || "TXN-SJNA");
        } else {
          showToast(res.data?.message || "Transfer failed. Insufficient balance or invalid receiver account.", "error");
        }
      });
    }

    // Bill Category Selection
    const catCards = document.querySelectorAll(".bill-category-card");
    catCards.forEach(card => {
      card.addEventListener("click", () => {
        catCards.forEach(c => c.classList.remove("selected"));
        card.classList.add("selected");
        selectedBillCategory = card.getAttribute("data-category");
        updateBillFormLabels(selectedBillCategory);
      });
    });

    // Bill Payment Form
    const formBill = document.getElementById("form-bill-payment");
    if (formBill) {
      formBill.addEventListener("submit", async (e) => {
        e.preventDefault();
        const account = document.getElementById("bill-account").value;
        const biller = document.getElementById("bill-provider").value.trim();
        const consumerNo = document.getElementById("bill-consumer-no").value.trim();
        const amount = parseFloat(document.getElementById("bill-amount").value);

        if (!account || !biller || !consumerNo || !amount || amount <= 0) {
          showToast("Please fill all required bill payment details.", "error");
          return;
        }

        const btn = formBill.querySelector("button[type='submit']");
        btn.disabled = true;
        btn.textContent = "Processing Payment...";

        const res = await API.payBill(account, selectedBillCategory, biller, consumerNo, amount);
        btn.disabled = false;
        btn.textContent = "Pay Bill Securely";

        if (res.ok && res.data && res.data.success) {
          showToast("Bill payment successful!", "success");
          formBill.reset();
          await loadCustomerData();
          showReceiptModal(res.data.transactionId || "BILL-SJNA");
        } else {
          showToast(res.data?.message || "Payment failed. Insufficient account balance.", "error");
        }
      });
    }

    // ATM Simulation: Deposit
    const formAtmDeposit = document.getElementById("form-atm-deposit");
    if (formAtmDeposit) {
      formAtmDeposit.addEventListener("submit", async (e) => {
        e.preventDefault();
        const acc = document.getElementById("atm-account").value;
        const amount = parseFloat(document.getElementById("atm-deposit-amount").value);

        if (!acc || !amount || amount <= 0) {
          showToast("Please specify a valid positive amount to deposit.", "error");
          return;
        }

        const res = await API.deposit(acc, amount);
        if (res.ok && res.data && res.data.success) {
          showToast(`₹ ${formatINR(amount)} deposited successfully!`, "success");
          document.getElementById("atm-deposit-amount").value = "";
          await loadCustomerData();
        } else {
          showToast(res.data?.message || "Deposit failed.", "error");
        }
      });
    }

    // ATM Simulation: Withdraw
    const formAtmWithdraw = document.getElementById("form-atm-withdraw");
    if (formAtmWithdraw) {
      formAtmWithdraw.addEventListener("submit", async (e) => {
        e.preventDefault();
        const acc = document.getElementById("atm-account").value;
        const amount = parseFloat(document.getElementById("atm-withdraw-amount").value);
        const pin = document.getElementById("atm-pin").value;

        if (!acc || !amount || amount <= 0) {
          showToast("Please specify a valid positive amount to withdraw.", "error");
          return;
        }

        if (!pin || pin.length < 4) {
          showToast("Please enter your 4-digit ATM PIN.", "error");
          return;
        }

        const res = await API.withdraw(acc, amount);
        if (res.ok && res.data && res.data.success) {
          showToast(`₹ ${formatINR(amount)} cash dispensed successfully!`, "success");
          document.getElementById("atm-withdraw-amount").value = "";
          document.getElementById("atm-pin").value = "";
          await loadCustomerData();
        } else {
          showToast(res.data?.message || "Withdrawal failed. Insufficient balance or overdraft exceeded.", "error");
        }
      });
    }

    // Profile Edit Form
    const formEditProfile = document.getElementById("form-edit-profile");
    if (formEditProfile) {
      formEditProfile.addEventListener("submit", async (e) => {
        e.preventDefault();
        const name = document.getElementById("edit-profile-name").value.trim();
        const phone = document.getElementById("edit-profile-phone").value.trim();

        const res = await API.updateProfile(name, phone);
        if (res.ok && res.data && res.data.success) {
          showToast("Profile details updated successfully!", "success");
          if (currentUser) {
            currentUser.name = name;
            currentUser.phone = phone;
            API.setCurrentUser(currentUser);
          }
          checkAuthState();
        } else {
          showToast(res.data?.message || "Failed to update profile.", "error");
        }
      });
    }

    // Change Password Form
    const formChangePassword = document.getElementById("form-change-password");
    if (formChangePassword) {
      formChangePassword.addEventListener("submit", async (e) => {
        e.preventDefault();
        const oldPass = document.getElementById("pass-old").value;
        const newPass = document.getElementById("pass-new").value;
        const confirmPass = document.getElementById("pass-confirm").value;

        if (newPass !== confirmPass) {
          showToast("New passwords do not match.", "error");
          return;
        }

        if (newPass.length < 8) {
          showToast("New password must be at least 8 characters long.", "error");
          return;
        }

        const res = await API.changePassword(oldPass, newPass);
        if (res.ok && res.data && res.data.success) {
          showToast("Password changed successfully!", "success");
          formChangePassword.reset();
        } else {
          showToast(res.data?.message || "Password change failed. Check current password.", "error");
        }
      });
    }

    // Transaction History Filters
    const historySearch = document.getElementById("history-search");
    const historyTypeFilter = document.getElementById("history-type-filter");
    const historySort = document.getElementById("history-sort");

    function applyHistoryFilters() {
      let filtered = [...allTransactions];
      const query = historySearch ? historySearch.value.trim().toLowerCase() : "";
      const type = historyTypeFilter ? historyTypeFilter.value : "ALL";
      const sort = historySort ? historySort.value : "DATE_DESC";

      if (query) {
        filtered = filtered.filter(t =>
          (t.transactionId && t.transactionId.toLowerCase().includes(query)) ||
          (t.senderAccount && t.senderAccount.toLowerCase().includes(query)) ||
          (t.receiverAccount && t.receiverAccount.toLowerCase().includes(query))
        );
      }

      if (type !== "ALL") {
        filtered = filtered.filter(t => t.transactionType === type);
      }

      if (sort === "AMOUNT_DESC") {
        filtered.sort((a, b) => parseFloat(b.amount) - parseFloat(a.amount));
      } else if (sort === "AMOUNT_ASC") {
        filtered.sort((a, b) => parseFloat(a.amount) - parseFloat(b.amount));
      } else if (sort === "DATE_ASC") {
        filtered.reverse();
      }

      renderTransactionsTable(filtered);
    }

    if (historySearch) historySearch.addEventListener("input", applyHistoryFilters);
    if (historyTypeFilter) historyTypeFilter.addEventListener("change", applyHistoryFilters);
    if (historySort) historySort.addEventListener("change", applyHistoryFilters);

    // Quick Amount Pill handlers
    document.querySelectorAll(".quick-amount-pill").forEach(pill => {
      pill.addEventListener("click", () => {
        const val = pill.getAttribute("data-val");
        const targetInputId = pill.getAttribute("data-target");
        const input = document.getElementById(targetInputId);
        if (input) input.value = val;
      });
    });

    // Global Unauthorized event
    window.addEventListener("sjna:unauthorized", () => {
      showToast("Session expired. Please sign in again.", "warning");
      checkAuthState();
    });
  }

  // Dynamic Bill Form Labels based on selected category
  function updateBillFormLabels(category) {
    const providerLabel = document.getElementById("bill-provider-label");
    const providerInput = document.getElementById("bill-provider");
    const consumerLabel = document.getElementById("bill-consumer-label");
    const consumerInput = document.getElementById("bill-consumer-no");

    switch (category) {
      case "electricity":
        if (providerLabel) providerLabel.textContent = "Electricity Board / Provider";
        if (providerInput) providerInput.placeholder = "e.g. Tata Power, Adani Electricity, BESCOM";
        if (consumerLabel) consumerLabel.textContent = "Consumer Number (CA Number)";
        if (consumerInput) consumerInput.placeholder = "e.g. 100984729";
        break;
      case "water":
        if (providerLabel) providerLabel.textContent = "Water Supply Board";
        if (providerInput) providerInput.placeholder = "e.g. Delhi Jal Board, BWSSB, HMWSSB";
        if (consumerLabel) consumerLabel.textContent = "Connection / Consumer ID";
        if (consumerInput) consumerInput.placeholder = "e.g. WTR-884920";
        break;
      case "mobile":
        if (providerLabel) providerLabel.textContent = "Telecom Operator";
        if (providerInput) providerInput.placeholder = "e.g. Jio, Airtel, Vi, BSNL";
        if (consumerLabel) consumerLabel.textContent = "10-Digit Mobile Number";
        if (consumerInput) consumerInput.placeholder = "e.g. 9876543210";
        break;
      case "broadband":
        if (providerLabel) providerLabel.textContent = "Broadband / Internet Provider";
        if (providerInput) providerInput.placeholder = "e.g. Airtel Xstream, JioFiber, ACT Fibernet";
        if (consumerLabel) consumerLabel.textContent = "Broadband Account ID / Landline";
        if (consumerInput) consumerInput.placeholder = "e.g. 08023456789";
        break;
      case "gas":
        if (providerLabel) providerLabel.textContent = "Gas Provider";
        if (providerInput) providerInput.placeholder = "e.g. Indane, Bharat Gas, HP Gas, IGL";
        if (consumerLabel) consumerLabel.textContent = "LPG Consumer ID";
        if (consumerInput) consumerInput.placeholder = "e.g. 238479201";
        break;
      case "dth":
        if (providerLabel) providerLabel.textContent = "DTH Operator";
        if (providerInput) providerInput.placeholder = "e.g. Tata Play, Airtel Digital TV, Dish TV";
        if (consumerLabel) consumerLabel.textContent = "Subscriber ID / Smart Card No.";
        if (consumerInput) consumerInput.placeholder = "e.g. 1029384756";
        break;
    }
  }

  // Switch Navigation Views
  function switchView(viewName) {
    navItems.forEach(n => {
      if (n.getAttribute("data-view") === viewName) {
        n.classList.add("active");
      } else {
        n.classList.remove("active");
      }
    });

    viewSections.forEach(v => {
      if (v.id === `view-${viewName}`) {
        v.style.display = "block";
      } else {
        v.style.display = "none";
      }
    });

    // Refresh specific view data if needed
    if (viewName === "history") {
      fetchTransactions();
    } else if (viewName === "accounts") {
      fetchAccounts();
    } else if (viewName === "profile") {
      fetchProfile();
    }
  }

  // Helper: Open Modal
  window.openModal = function (modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.add("active");
  };

  // Helper: Close Modal
  window.closeModal = function (modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.remove("active");
  };

  // Helper: Show Receipt Modal
  window.showReceiptModal = function (txnId) {
    let txn = allTransactions.find(t => t.transactionId === txnId);
    if (!txn && pendingTransferData) {
      txn = {
        transactionId: txnId,
        date: new Date().toLocaleString(),
        transactionType: "Fund Transfer",
        senderAccount: pendingTransferData.sender,
        receiverAccount: pendingTransferData.receiver,
        amount: pendingTransferData.amount,
        status: "Successful"
      };
    }

    if (!txn) {
      txn = {
        transactionId: txnId,
        date: new Date().toLocaleString(),
        transactionType: "Online Banking Transaction",
        senderAccount: currentActiveAccount ? currentActiveAccount.accountNumber : "SJNA Bank",
        receiverAccount: "-",
        amount: 0,
        status: "Completed"
      };
    }

    document.getElementById("receipt-txn-id").textContent = txn.transactionId;
    document.getElementById("receipt-date").textContent = txn.date || new Date().toLocaleString();
    document.getElementById("receipt-type").textContent = txn.transactionType;
    document.getElementById("receipt-sender").textContent = txn.senderAccount || "-";
    document.getElementById("receipt-receiver").textContent = txn.receiverAccount || "-";
    document.getElementById("receipt-amount").textContent = `₹ ${formatINR(txn.amount)}`;
    document.getElementById("receipt-status").textContent = txn.status || "Successful";

    openModal("modal-receipt");
  };

  // Helper: Quick account switcher from accounts list
  window.selectAccount = function (accNo) {
    API.setActiveAccount(accNo);
    currentActiveAccount = accountsList.find(a => a.accountNumber === accNo) || null;
    if (accountSelect) accountSelect.value = accNo;
    updateActiveAccountCard();
    renderAccountsView();
    showToast(`Switched active account to ${accNo}`, "info");
  };

  // Helper: Open Create Account Modal
  window.openCreateAccountModal = function () {
    openModal("modal-create-account");
  };

  // Helper: Quick navigation switch
  window.navigateTo = function (viewName) {
    switchView(viewName);
  };

  // UI Toast Function
  function showToast(message, type = "info") {
    const container = document.getElementById("toast-container");
    if (!container) return;

    const toast = document.createElement("div");
    toast.className = `toast ${type}`;
    toast.innerHTML = `
      <span>${message}</span>
      <button style="background:none;border:none;cursor:pointer;color:inherit;font-size:1.1rem;" onclick="this.parentElement.remove()">&times;</button>
    `;

    container.appendChild(toast);

    setTimeout(() => {
      toast.remove();
    }, 4500);
  }

  // Utilities
  function formatINR(val) {
    const num = parseFloat(val);
    if (isNaN(num)) return "0.00";
    return num.toLocaleString("en-IN", {
      minimumFractionDigits: 2,
      maximumFractionDigits: 2
    });
  }

  function maskAccountNumber(accNo) {
    if (!accNo || accNo.length < 5) return accNo;
    const firstTwo = accNo.substring(0, 4);
    const lastFour = accNo.substring(accNo.length - 4);
    return `${firstTwo} •••• ${lastFour}`;
  }
});
