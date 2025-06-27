#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
    float priority;
} PriorityDebt;

// Globals
Transaction transactions[MAX_TRANSACTIONS];
Budget budgets[MAX_CATEGORIES];
Debt debts[MAX_DEBTS];
PriorityDebt debtQueue[MAX_DEBTS];
int transaction_count = 0, budget_count = 0, debt_count = 0;

// Function prototypes
void set_budget();
void add_transaction();

// Helpers
void normalize(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
}

void getCurrentDateTime(char *buffer) {
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

// File Handling
void save_transactions() {
    FILE *file = fopen("transactions.csv", "w");
    if (!file) {
        printf("Error saving transactions!\n");
        return;
    }
    for (int i = 0; i < transaction_count; i++) {
        Transaction t = transactions[i];
        fprintf(file, "%s,%.2f,%c,%s,%s\n", t.description, t.amount, t.type, t.category, t.date);
    }
    fclose(file);
}

void load_transactions() {
    FILE *file = fopen("transactions.csv", "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Transaction t;
        char *token = strtok(line, ",");
        if (!token) continue;
        strcpy(t.description, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        t.amount = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        t.type = token[0];

        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(t.category, token);

        token = strtok(NULL, "\n");
        if (!token) continue;
        strcpy(t.date, token);

        if (transaction_count < MAX_TRANSACTIONS)
            transactions[transaction_count++] = t;
    }
    fclose(file);
}

void save_budgets() {
    FILE *file = fopen("budgets.csv", "w");
    if (!file) {
        printf("Error saving budgets!\n");
        return;
    }
    for (int i = 0; i < budget_count; i++) {
        Budget b = budgets[i];
        fprintf(file, "%s,%.2f,%.2f\n", b.category, b.budget, b.spent);
    }
    fclose(file);
}

void load_budgets() {
    FILE *file = fopen("budgets.csv", "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Budget b;
        char *token = strtok(line, ",");
        if (!token) continue;
        strcpy(b.category, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        b.budget = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        b.spent = atof(token);

        if (budget_count < MAX_CATEGORIES)
            budgets[budget_count++] = b;
    }
    fclose(file);
}

void save_debts() {
    FILE *file = fopen("debts.csv", "w");
    if (!file) {
        printf("Error saving debts!\n");
        return;
    }
    for (int i = 0; i < debt_count; i++) {
        Debt d = debts[i];
        fprintf(file, "%s,%.2f,%d,%.2f,%.2f,%.2f\n", 
                d.name, d.principal, d.monthsRemaining, 
                d.interestRate, d.extraFees, d.paid);
    }
    fclose(file);
}

void load_debts() {
    FILE *file = fopen("debts.csv", "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Debt d;
        char *token = strtok(line, ",");
        if (!token) continue;
        strcpy(d.name, token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d.principal = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d.monthsRemaining = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d.interestRate = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d.extraFees = atof(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        d.paid = atof(token);

        if (debt_count < MAX_DEBTS) {
            debts[debt_count] = d;
            debtQueue[debt_count].index = debt_count;
            debtQueue[debt_count].priority = calculate_monthly_installment(d);
            debt_count++;
        }
    }
    fclose(file);
}

// Core functions
void add_transaction() {
    if (transaction_count >= MAX_TRANSACTIONS) {
        printf("Transaction limit reached!\n");
        return;
    }

    Transaction t;
    printf("Description: ");
    getchar(); fgets(t.description, sizeof(t.description), stdin);
    t.description[strcspn(t.description, "\n")] = '\0';

    printf("Amount: ");
    float amount;
    if (scanf("%f", &amount) != 1 || amount <= 0) {
        printf("Invalid amount. Must be positive.\n");
        while(getchar() != '\n'); // Clear input buffer
        return;
    }
    t.amount = amount;

    printf("Type (I For Income /E For Expense): ");
    char type;
    scanf(" %c", &type);
    type = toupper(type);
    if (type != 'I' && type != 'E') {
        printf("Invalid type. Must be I or E.\n");
        while(getchar() != '\n');
        return;
    }
    t.type = type;

    printf("Category: ");
    getchar(); fgets(t.category, sizeof(t.category), stdin);
    t.category[strcspn(t.category, "\n")] = '\0';
    if (strlen(t.category) == 0) {
        printf("Category cannot be empty.\n");
        return;
    }

    // Check if category exists in budgets for expenses
    if (t.type == 'E') {
        int found = 0;
        char normalizedCategory[30];
        strcpy(normalizedCategory, t.category);
        normalize(normalizedCategory);

        for (int i = 0; i < budget_count; i++) {
            char temp[30];
            strcpy(temp, budgets[i].category);
            normalize(temp);
            if (strcmp(temp, normalizedCategory) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Warning: No budget set for '%s'. Set one now? (Y/N): ", t.category);
            char choice;
            scanf(" %c", &choice);
            if (toupper(choice) == 'Y') {
                set_budget();
            }
        }
    }

    getCurrentDateTime(t.date);
    transactions[transaction_count++] = t;

    // Update budget spent
    char normalizedCategory[30];
    strcpy(normalizedCategory, t.category);
    normalize(normalizedCategory);
    for (int i = 0; i < budget_count; i++) {
        char temp[30];
        strcpy(temp, budgets[i].category);
        normalize(temp);
        if (strcmp(temp, normalizedCategory) == 0 && t.type == 'E') {
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
        printf("No transactions.\n");
        return;
    }

    // Sort by date descending
    Transaction sorted[MAX_TRANSACTIONS];
    memcpy(sorted, transactions, sizeof(Transaction) * transaction_count);
    for (int i = 0; i < transaction_count - 1; i++) {
        for (int j = i + 1; j < transaction_count; j++) {
            if (strcmp(sorted[i].date, sorted[j].date) < 0) {
                Transaction temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    for (int i = 0; i < transaction_count; i++)
        printf("%d. %s -> %c | %.2f | %s | %s\n",
               i + 1, sorted[i].description, sorted[i].type,
               sorted[i].amount, sorted[i].category, sorted[i].date);

    // Calculate totals
    float total_income = 0, total_expense = 0;
    for (int i = 0; i < transaction_count; i++) {
        if (transactions[i].type == 'I')
            total_income += transactions[i].amount;
        else
            total_expense += transactions[i].amount;
    }
    printf("\nTotal Income: Rs %.2f\nTotal Expenses: Rs %.2f\nNet Savings: Rs %.2f\n",
           total_income, total_expense, total_income - total_expense);
}

void set_budget() {
    if (budget_count >= MAX_CATEGORIES) {
        printf("Budget limit reached!\n");
        return;
    }

    Budget b;
    printf("Category: ");
    getchar(); fgets(b.category, sizeof(b.category), stdin);
    b.category[strcspn(b.category, "\n")] = '\0';
    normalize(b.category);

    // Check if category exists
    for (int i = 0; i < budget_count; i++) {
        char temp[30];
        strcpy(temp, budgets[i].category);
        normalize(temp);
        if (strcmp(temp, b.category) == 0) {
            printf("Budget for '%s' already exists. Update amount? (Y/N): ", budgets[i].category);
            char choice;
            scanf(" %c", &choice);
            if (toupper(choice) == 'Y') {
                printf("New budget amount: ");
                scanf("%f", &budgets[i].budget);
                budgets[i].spent = 0;
                printf("Budget updated.\n");
            }
            return;
        }
    }

    printf("Budget amount: ");
    scanf("%f", &b.budget);
    if (b.budget <= 0) {
        printf("Invalid budget amount.\n");
        return;
    }
    b.spent = 0;
    budgets[budget_count++] = b;
    printf("Budget set!\n");
}

void edit_budget() {
    if (budget_count == 0) {
        printf("No budgets to edit.\n");
        return;
    }

    char category[30];
    printf("Enter category to edit: ");
    getchar(); fgets(category, sizeof(category), stdin);
    category[strcspn(category, "\n")] = '\0';
    normalize(category);

    for (int i = 0; i < budget_count; i++) {
        char temp[30];
        strcpy(temp, budgets[i].category);
        normalize(temp);
        if (strcmp(temp, category) == 0) {
            printf("Current budget: Rs %.2f\n", budgets[i].budget);
            printf("Enter new budget amount: ");
            float newBudget;
            if (scanf("%f", &newBudget) != 1 || newBudget <= 0) {
                printf("Invalid amount.\n");
                while(getchar() != '\n');
                return;
            }
            budgets[i].budget = newBudget;
            budgets[i].spent = 0; // Reset spent
            printf("Budget updated.\n");
            return;
        }
    }
    printf("Category not found.\n");
}

void delete_budget() {
    if (budget_count == 0) {
        printf("No budgets to delete.\n");
        return;
    }

    char category[30];
    printf("Enter category to delete: ");
    getchar(); fgets(category, sizeof(category), stdin);
    category[strcspn(category, "\n")] = '\0';
    normalize(category);

    for (int i = 0; i < budget_count; i++) {
        char temp[30];
        strcpy(temp, budgets[i].category);
        normalize(temp);
        if (strcmp(temp, category) == 0) {
            for (int j = i; j < budget_count - 1; j++)
                budgets[j] = budgets[j + 1];
            budget_count--;
            printf("Budget deleted.\n");
            return;
        }
    }
    printf("Category not found.\n");
}

void display_budgets() {
    printf("\n===== BUDGETS =====\n");
    if (budget_count == 0) {
        printf("No budgets.\n");
        return;
    }
    for (int i = 0; i < budget_count; i++) {
        float rem = budgets[i].budget - budgets[i].spent;
        printf("%s | Budget: Rs %.2f | Spent: Rs %.2f | Remaining: Rs %.2f\n",
               budgets[i].category, budgets[i].budget, budgets[i].spent, rem);
    }
}

void add_debt() {
    if (debt_count >= MAX_DEBTS) {
        printf("Debt limit reached!\n");
        return;
    }

    Debt d;
    printf("Debt name: ");
    getchar(); fgets(d.name, sizeof(d.name), stdin);
    d.name[strcspn(d.name, "\n")] = '\0';

    printf("Principal amount: ");
    if (scanf("%f", &d.principal) != 1 || d.principal <= 0) {
        printf("Invalid principal.\n");
        while(getchar() != '\n');
        return;
    }

    printf("Months remaining: ");
    if (scanf("%d", &d.monthsRemaining) != 1 || d.monthsRemaining <= 0) {
        printf("Invalid months.\n");
        while(getchar() != '\n');
        return;
    }

    printf("Interest rate (%%): ");
    if (scanf("%f", &d.interestRate) != 1 || d.interestRate < 0) {
        printf("Invalid rate.\n");
        while(getchar() != '\n');
        return;
    }

    printf("Extra fees: ");
    if (scanf("%f", &d.extraFees) != 1 || d.extraFees < 0) {
        printf("Invalid fees.\n");
        while(getchar() != '\n');
        return;
    }

    d.paid = 0;
    debts[debt_count] = d;
    debtQueue[debt_count].index = debt_count;
    debtQueue[debt_count].priority = calculate_monthly_installment(d);
    debt_count++;
    printf("Debt added!\n");
}

void edit_debt() {
    if (debt_count == 0) {
        printf("No debts to edit.\n");
        return;
    }

    char name[30];
    printf("Enter debt name to edit: ");
    getchar(); fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    for (int i = 0; i < debt_count; i++) {
        if (strcmp(debts[i].name, name) == 0) {
            printf("Editing %s\n", name);
            printf("New principal (current Rs %.2f): ", debts[i].principal);
            if (scanf("%f", &debts[i].principal) != 1 || debts[i].principal <= 0) {
                printf("Invalid principal.\n");
                while(getchar() != '\n');
                return;
            }

            printf("New months remaining (current %d): ", debts[i].monthsRemaining);
            if (scanf("%d", &debts[i].monthsRemaining) != 1 || debts[i].monthsRemaining <= 0) {
                printf("Invalid months.\n");
                while(getchar() != '\n');
                return;
            }

            printf("New interest rate (current %.2f%%): ", debts[i].interestRate);
            if (scanf("%f", &debts[i].interestRate) != 1 || debts[i].interestRate < 0) {
                printf("Invalid rate.\n");
                while(getchar() != '\n');
                return;
            }

            printf("New extra fees (current $%.2f): ", debts[i].extraFees);
            if (scanf("%f", &debts[i].extraFees) != 1 || debts[i].extraFees < 0) {
                printf("Invalid fees.\n");
                while(getchar() != '\n');
                return;
            }

            // Recalculate priority
            debtQueue[i].priority = calculate_monthly_installment(debts[i]);
            printf("Debt updated.\n");
            return;
        }
    }
    printf("Debt not found.\n");
}

void delete_debt() {
    if (debt_count == 0) {
        printf("No debts to delete.\n");
        return;
    }

    char name[30];
    printf("Enter debt name to delete: ");
    getchar(); fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    for (int i = 0; i < debt_count; i++) {
        if (strcmp(debts[i].name, name) == 0) {
            for (int j = i; j < debt_count - 1; j++) {
                debts[j] = debts[j + 1];
                debtQueue[j] = debtQueue[j + 1];
                debtQueue[j].index = j;
            }
            debt_count--;
            printf("Debt deleted.\n");
            return;
        }
    }
    printf("Debt not found.\n");
}

void display_debts() {
    update_debt_payments();
    printf("\n===== DEBTS =====\n");
    if (debt_count == 0) {
        printf("No debts.\n");
        return;
    }

    for (int i = 0; i < debt_count; i++) {
        float total_due = debts[i].principal + 
                         (debts[i].principal * debts[i].interestRate / 100.0f) + 
                          debts[i].extraFees;
        float remaining = total_due - debts[i].paid;
        printf("%s | Principal: Rs %.2f | Installment: Rs %.2f/mo | Paid: Rs %.2f | Remaining: Rs %.2f | Months: %d\n",
               debts[i].name, debts[i].principal, 
               calculate_monthly_installment(debts[i]), 
               debts[i].paid, remaining, debts[i].monthsRemaining);
    }
}

void display_top_debts() {
    printf("\n=== PRIORITY DEBTS (Highest Installments First) ===\n");
    if (debt_count == 0) {
        printf("No debts.\n");
        return;
    }

    PriorityDebt sorted[MAX_DEBTS];
    memcpy(sorted, debtQueue, sizeof(PriorityDebt) * debt_count);

    for (int i = 0; i < debt_count - 1; i++) {
        for (int j = i + 1; j < debt_count; j++) {
            if (sorted[i].priority < sorted[j].priority) {
                PriorityDebt tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    for (int i = 0; i < debt_count; i++) {
        int idx = sorted[i].index;
        printf("%d. %s -> Rs %.2f/mo\n", i+1, debts[idx].name, sorted[i].priority);
    }
}

void menu() {
    int choice;
    do {
        printf("\n==== Personal Finance Dashboard ====\n");
        printf("1. Add Transaction\n");
        printf("2. View Transactions\n");
        printf("3. Set Budget\n");
        printf("4. Edit Budget\n");
        printf("5. Delete Budget\n");
        printf("6. View Budgets\n");
        printf("7. Add Debt\n");
        printf("8. Edit Debt\n");
        printf("9. Delete Debt\n");
        printf("10. View Debts\n");
        printf("11. View Priority Debts\n");
        printf("12. Save & Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: add_transaction(); break;
            case 2: display_transactions(); break;
            case 3: set_budget(); break;
            case 4: edit_budget(); break;
            case 5: delete_budget(); break;
            case 6: display_budgets(); break;
            case 7: add_debt(); break;
            case 8: edit_debt(); break;
            case 9: delete_debt(); break;
            case 10: display_debts(); break;
            case 11: display_top_debts(); break;
            case 12: printf("Saving data...\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 12);
}

int main() {
    load_transactions();
    load_budgets();
    load_debts();
    menu();
    save_transactions();
    save_budgets();
    save_debts();
    printf("Data saved. Exiting...\n");
    return 0;
}
