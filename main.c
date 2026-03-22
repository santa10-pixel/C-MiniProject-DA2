#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTS 100
#define FILE_NAME "inventory_data.txt"
#define LOW_STOCK_THRESHOLD 5

typedef struct {
    int productID;
    char name[50];
    float price;
    int quantity;
} Product;

typedef struct {
    Product products[MAX_PRODUCTS];
    int productCount;
} ShopInventory;

void loadProducts(ShopInventory* inventory) {
    inventory->productCount = 0;
    FILE *file = fopen(FILE_NAME, "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (inventory->productCount >= MAX_PRODUCTS) break;
        
        char *token = strtok(line, ",");
        if (token) {
            int id = atoi(token);
            token = strtok(NULL, ",");
            if (token) {
                char name[50];
                strncpy(name, token, 49);
                name[49] = '\0';
                
                token = strtok(NULL, ",");
                if (token) {
                    float price = atof(token);
                    token = strtok(NULL, "\n");
                    if (token) {
                        int quantity = atoi(token);
                        Product *p = &inventory->products[inventory->productCount];
                        p->productID = id;
                        strcpy(p->name, name);
                        p->price = price;
                        p->quantity = quantity;
                        inventory->productCount++;
                    }
                }
            }
        }
    }
    fclose(file);
}

void saveProducts(ShopInventory* inventory) {
    FILE *file = fopen(FILE_NAME, "w");
    if (!file) {
        printf("Error saving to file.\n");
        return;
    }
    for (int i = 0; i < inventory->productCount; i++) {
        Product *p = &inventory->products[i];
        fprintf(file, "%d,%s,%.2f,%d\n", p->productID, p->name, p->price, p->quantity);
    }
    fclose(file);
}

void initInventory(ShopInventory* inventory) {
    loadProducts(inventory);
}

void addProduct(ShopInventory* inventory) {
    if (inventory->productCount >= MAX_PRODUCTS) {
        printf("Error: Inventory is full.\n");
        return;
    }

    int id, qty;
    char name[50];
    float price;
    char buffer[256];

    printf("Enter Product ID: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    id = atoi(buffer);
    if (id <= 0) {
        printf("Invalid ID. Enter a positive number.\n");
        return;
    }

    for (int i = 0; i < inventory->productCount; i++) {
        if (inventory->products[i].productID == id) {
            printf("Error: Product ID already exists!\n");
            return;
        }
    }

    printf("Enter Product Name: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    buffer[strcspn(buffer, "\n")] = 0;
    strncpy(name, buffer, 49);
    name[49] = '\0';

    printf("Enter Price: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    price = atof(buffer);
    if (price < 0) {
        printf("Invalid price. Enter a non-negative number.\n");
        return;
    }

    printf("Enter Quantity: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    qty = atoi(buffer);
    if (qty < 0) {
        printf("Invalid quantity. Enter a non-negative number.\n");
        return;
    }

    Product *p = &inventory->products[inventory->productCount];
    p->productID = id;
    strcpy(p->name, name);
    p->price = price;
    p->quantity = qty;
    inventory->productCount++;

    saveProducts(inventory);
    printf("Product added successfully.\n");
}

void updateStock(ShopInventory* inventory) {
    int id, option, amount;
    char buffer[256];

    printf("Enter Product ID to update: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    id = atoi(buffer);

    for (int i = 0; i < inventory->productCount; i++) {
        if (inventory->products[i].productID == id) {
            Product *p = &inventory->products[i];
            printf("\nProduct Found: %s | Current Quantity: %d\n", p->name, p->quantity);
            printf("1. Purchase (Add Stock)\n");
            printf("2. Sale (Reduce Stock)\n");
            printf("Choose Option: ");

            if (!fgets(buffer, sizeof(buffer), stdin)) return;
            option = atoi(buffer);
            if (option != 1 && option != 2) {
                printf("Invalid option.\n");
                return;
            }

            printf("Enter Amount: ");
            if (!fgets(buffer, sizeof(buffer), stdin)) return;
            amount = atoi(buffer);
            if (amount <= 0) {
                printf("Invalid amount.\n");
                return;
            }

            if (option == 1) {
                p->quantity += amount;
                printf("Stock added. New Quantity: %d\n", p->quantity);
            } else {
                if (p->quantity < amount) {
                    printf("Error: Insufficient stock for this sale.\n");
                } else {
                    p->quantity -= amount;
                    printf("Sale processed. New Quantity: %d\n", p->quantity);
                }
            }
            saveProducts(inventory);
            return;
        }
    }
    printf("Product ID not found.\n");
}

void deleteProduct(ShopInventory* inventory) {
    int id;
    char buffer[256];

    printf("Enter Product ID to delete: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    id = atoi(buffer);

    for (int i = 0; i < inventory->productCount; i++) {
        if (inventory->products[i].productID == id) {
            for (int j = i; j < inventory->productCount - 1; j++) {
                inventory->products[j] = inventory->products[j + 1];
            }
            inventory->productCount--;
            saveProducts(inventory);
            printf("Product deleted successfully.\n");
            return;
        }
    }
    printf("Product ID not found.\n");
}

void displayProduct(const Product* p) {
    printf("%-10d%-30sRs.%-14.2f%-10d\n", p->productID, p->name, p->price, p->quantity);
}

void searchProduct(const ShopInventory* inventory) {
    int id;
    char buffer[256];

    printf("Enter Product ID to search: ");
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    id = atoi(buffer);

    for (int i = 0; i < inventory->productCount; i++) {
        if (inventory->products[i].productID == id) {
            printf("\nProduct Found:\n");
            printf("%-10s%-30s%-17s%-10s\n", "ID", "Name", "Price", "Quantity");
            printf("-----------------------------------------------------------------\n");
            displayProduct(&inventory->products[i]);
            return;
        }
    }
    printf("Product ID not found.\n");
}

void lowStockReport(const ShopInventory* inventory) {
    int found = 0;
    printf("\n--- Low Stock Report (Threshold <= %d) ---\n", LOW_STOCK_THRESHOLD);
    printf("%-10s%-30s%-17s%-10s\n", "ID", "Name", "Price", "Quantity");
    printf("-----------------------------------------------------------------\n");

    for (int i = 0; i < inventory->productCount; i++) {
        if (inventory->products[i].quantity <= LOW_STOCK_THRESHOLD) {
            displayProduct(&inventory->products[i]);
            found = 1;
        }
    }
    if (!found) {
        printf("No products are running low on stock.\n");
    }
}

void showTotalValue(const ShopInventory* inventory) {
    float totalValue = 0.0;
    for (int i = 0; i < inventory->productCount; i++) {
        totalValue += (inventory->products[i].price * inventory->products[i].quantity);
    }
    printf("\nTotal Inventory Value: Rs.%.2f\n", totalValue);
}

int main() {
    ShopInventory inventory;
    initInventory(&inventory);
    int choice;
    char buffer[256];

    while (1) {
        printf("\n=== Inventory Management System ===\n");
        printf("1. Add Product\n");
        printf("2. Update Stock (Purchase/Sale)\n");
        printf("3. Delete Product\n");
        printf("4. Search by Product ID\n");
        printf("5. Low-Stock Report\n");
        printf("6. Show Total Inventory Value\n");
        printf("7. Exit & Save\n");
        printf("Enter your choice: ");

        if (!fgets(buffer, sizeof(buffer), stdin)) continue;
        choice = atoi(buffer);

        switch (choice) {
            case 1: addProduct(&inventory); break;
            case 2: updateStock(&inventory); break;
            case 3: deleteProduct(&inventory); break;
            case 4: searchProduct(&inventory); break;
            case 5: lowStockReport(&inventory); break;
            case 6: showTotalValue(&inventory); break;
            case 7: 
                printf("Exiting system. Data automatically saved.\n");
                return 0;
            default:
                printf("Invalid choice! Please select 1-7.\n");
        }
    }
    return 0;
}
