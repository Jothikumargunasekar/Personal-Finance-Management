// Full code provided below
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TRANSACTIONS 100
#define MAX_CATEGORIES 10
#define MAX_DEBTS 10

typedef struct {
    char description[100];
    float amount;
    char type; // 'I' for income, 'E' for expense
    char category[30];
    char date[20];
} Transaction;

typedef struct {
    char category[30];
    float budget;
    float spent;
} Budget;

typedef struct {
    char name[30];
    float principal;
    int monthsRemaining;
    float interestRate;
    float extraFees;
    float paid;
} Debt;

typedef struct {
    int index;
    float priority; // Monthly installment
} PriorityDebt;

// Globals
Transaction transactions[MAX_TRANSACTIONS];
Budget budgets[MAX_CATEGORIES];
Debt debts[MAX_DEBTS];
PriorityDebt debtQueue[MAX_DEBTS];
int transaction_count = 0, budget_count = 0, debt_count = 0;

// Helpers
void normalize(char* str) {
    for (int i = 0; str[i]; i++)
        if (str[i] >= 'A' && str[i] <= 'Z') str[i] += 32;
}

void getCurrentDateTime(char* buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, 20, "%d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

float calculate_monthly_installment(Debt d) {
    float total = d.principal + (d.principal * d.interestRate / 100.0f) + d.extraFees;
    return total / d.monthsRemaining;
}

void update_debt_payments() {
    for (int i = 0; i < debt_count; i++) {
        debts[i].paid = 0;
        char debtName[30];
        strcpy(debtName, debts[i].name);
        normalize(debtName);

        for (int j = 0; j < transaction_count; j++) {
            char category[30];
            strcpy(category, transactions[j].category);
            normalize(category);

            if (strcmp(category, debtName) == 0 && transactions[j].type == 'E') {
                debts[i].paid += transactions[j].amount;
            }
        }
    }
}

// Core functions
void add_transaction() {
    if (transaction_count >= MAX_TRANSACTIONS) {
        printf("Limit reached!\n");
        return;
    }

    Transaction t;
    printf("Description: ");
    getchar(); fgets(t.description, sizeof(t.description), stdin);
    t.description[strcspn(t.description, "\n")] = '\0';

    printf("Amount: "); scanf("%f", &t.amount);
    printf("Type (I/E): "); getchar(); scanf("%c", &t.type);
    printf("Category: "); getchar(); fgets(t.category, sizeof(t.category), stdin);
    t.category[strcspn(t.category, "\n")] = '\0';
    normalize(t.category);

    getCurrentDateTime(t.date);
    transactions[transaction_count++] = t;

    for (int i = 0; i < budget_count; i++) {
        char temp[30]; strcpy(temp, budgets[i].category); normalize(temp);
        if (strcmp(temp, t.category) == 0 && t.type == 'E') {
            budgets[i].spent += t.amount;
            if (budgets[i].spent > budgets[i].budget)
                printf("⚠️  Budget exceeded for '%s'!\n", budgets[i].category);
        }
    }

    printf("Transaction added!\n");
}

void display_transactions() {
    printf("\n===== TRANSACTIONS =====\n");
    if (transaction_count == 0) {
        printf("No transactions.\n"); return;
    }
    for (int i = 0; i < transaction_count; i++)
        printf("%d. %s -> %c | %.2f | %s | %s\n",
            i+1, transactions[i].description, transactions[i].type,
            transactions[i].amount, transactions[i].category, transactions[i].date);
}

void set_budget() {
    if (budget_count >= MAX_CATEGORIES) {
        printf("Limit reached!\n"); return;
    }

    Budget b;
    printf("Category: "); getchar(); fgets(b.category, sizeof(b.category), stdin);
    b.category[strcspn(b.category, "\n")] = '\0';

    printf("Budget amount: "); scanf("%f", &b.budget);
    b.spent = 0;
    budgets[budget_count++] = b;
    printf("Budget set!\n");
}

void delete_budget() {
    char cat[30];
    printf("Category to delete: "); getchar(); fgets(cat, sizeof(cat), stdin);
    cat[strcspn(cat, "\n")] = '\0'; normalize(cat);

    for (int i = 0; i < budget_count; i++) {
        char temp[30]; strcpy(temp, budgets[i].category); normalize(temp);
        if (strcmp(temp, cat) == 0) {
            for (int j = i; j < budget_count - 1; j++)
                budgets[j] = budgets[j + 1];
            budget_count--;
            printf("Deleted.\n");
            return;
        }
    }
    printf("Not found.\n");
}

void display_budgets() {
    printf("\n===== BUDGETS =====\n");
    if (budget_count == 0) {
        printf("No budgets.\n"); return;
    }
    for (int i = 0; i < budget_count; i++) {
        float rem = budgets[i].budget - budgets[i].spent;
        if (budgets[i].spent > budgets[i].budget)
            printf("⚠️  ");
        printf("%s | Total: %.2f | Spent: %.2f | Remaining: %.2f\n",
            budgets[i].category, budgets[i].budget, budgets[i].spent, rem);
    }
}

void add_debt() {
    if (debt_count >= MAX_DEBTS) {
        printf("Limit reached!\n"); return;
    }

    Debt d;
    printf("Debt name: "); getchar(); fgets(d.name, sizeof(d.name), stdin);
    d.name[strcspn(d.name, "\n")] = '\0';

    printf("Principal amount: "); scanf("%f", &d.principal);
    printf("Months remaining: "); scanf("%d", &d.monthsRemaining);
    printf("Interest rate (%%): "); scanf("%f", &d.interestRate);
    printf("Extra fees: "); scanf("%f", &d.extraFees);
    d.paid = 0;

    debts[debt_count] = d;
    debtQueue[debt_count].index = debt_count;
    debtQueue[debt_count].priority = calculate_monthly_installment(d);
    debt_count++;

    printf("Debt added!\n");
}

void display_debts() {
    update_debt_payments();
    printf("\n===== DEBTS =====\n");
    if (debt_count == 0) {
        printf("No debts.\n"); return;
    }

    // Sort debts by monthly installment descending
    for (int i = 0; i < debt_count - 1; i++)
        for (int j = i + 1; j < debt_count; j++)
            if (debtQueue[i].priority < debtQueue[j].priority) {
                PriorityDebt temp = debtQueue[i];
                debtQueue[i] = debtQueue[j];
                debtQueue[j] = temp;
            }

    for (int i = 0; i < debt_count; i++) {
        int idx = debtQueue[i].index;
        float emi = calculate_monthly_installment(debts[idx]);
        printf("%s | Total: %.2f | Paid: %.2f | Months: %d | Interest: %.2f%% | Fees: %.2f | EMI: %.2f\n",
            debts[idx].name, debts[idx].principal, debts[idx].paid,
            debts[idx].monthsRemaining, debts[idx].interestRate,
            debts[idx].extraFees, emi);
    }
}

void display_graph() {
    printf("\n===== EXPENSE GRAPH =====\n");
    if (transaction_count == 0) {
        printf("No transactions.\n"); return;
    }
    for (int i = 0; i < transaction_count; i++) {
        printf("%s -> %c | %.2f\n", transactions[i].description, transactions[i].type, transactions[i].amount);
    }
}

void delete_transaction() {
    if (transaction_count == 0) {
        printf("No transactions.\n"); return;
    }

    display_transactions();
    int index;
    printf("Transaction number to delete: "); scanf("%d", &index);

    if (index < 1 || index > transaction_count) {
        printf("Invalid.\n"); return;
    }

    Transaction t = transactions[index - 1];
    if (t.type == 'E') {
        for (int i = 0; i < budget_count; i++) {
            char temp[30]; strcpy(temp, budgets[i].category); normalize(temp);
            if (strcmp(temp, t.category) == 0) {
                budgets[i].spent -= t.amount; break;
            }
        }
    }

    for (int i = index - 1; i < transaction_count - 1; i++)
        transactions[i] = transactions[i + 1];
    transaction_count--;

    printf("Deleted.\n");
}

int main() {
    int choice;
    while (1) {
        printf("\nMenu:\n");
        printf("1. Add Transaction\n2. Display Transactions\n3. Set Budget\n");
        printf("4. Display Budgets\n5. Delete Budget\n6. Add Debt\n");
        printf("7. Display Debts\n8. Show Graph\n9. Delete Transaction\n10. Exit\n");
        printf("Choice: "); scanf("%d", &choice);

        switch (choice) {
            case 1: add_transaction(); break;
            case 2: display_transactions(); break;
            case 3: set_budget(); break;
            case 4: display_budgets(); break;
            case 5: delete_budget(); break;
            case 6: add_debt(); break;
            case 7: display_debts(); break;
            case 8: display_graph(); break;
            case 9: delete_transaction(); break;
            case 10: exit(0);
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}
