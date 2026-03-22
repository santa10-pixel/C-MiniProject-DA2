#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTS 100
#define FILE_NAME "inventory.dat"
#define LOW_STOCK_THRESHOLD 5

typedef struct {
  int productID;
  char name[50];
  float price;
  int quantity;
} Product;

Product inventory[MAX_PRODUCTS];
int productCount = 0;

EMSCRIPTEN_KEEPALIVE
void init() {
  FILE *file = fopen(FILE_NAME, "rb");
  if (file) {
    fread(&productCount, sizeof(int), 1, file);
    fread(inventory, sizeof(Product), productCount, file);
    fclose(file);
  }
}

void saveInventory() {
  FILE *file = fopen(FILE_NAME, "wb");
  if (file) {
    fwrite(&productCount, sizeof(int), 1, file);
    fwrite(inventory, sizeof(Product), productCount, file);
    fclose(file);
  }
}

EMSCRIPTEN_KEEPALIVE
const char *addProduct(int id, const char *name, float price, int quantity) {
  if (productCount >= MAX_PRODUCTS) {
    return "Error: Inventory is full.";
  }

  for (int i = 0; i < productCount; i++) {
    if (inventory[i].productID == id) {
      return "Error: Product ID already exists!";
    }
  }

  if (price < 0)
    return "Error: Invalid price.";
  if (quantity < 0)
    return "Error: Invalid quantity.";

  Product p;
  p.productID = id;
  strncpy(p.name, name, sizeof(p.name) - 1);
  p.name[sizeof(p.name) - 1] = '\0';
  p.price = price;
  p.quantity = quantity;

  inventory[productCount++] = p;
  saveInventory();
  return "Product added successfully!";
}

EMSCRIPTEN_KEEPALIVE
const char *updateStock(int id, int amount, int isPurchase) {
  for (int i = 0; i < productCount; i++) {
    if (inventory[i].productID == id) {
      if (isPurchase) {
        inventory[i].quantity += amount;
      } else {
        if (inventory[i].quantity < amount) {
          return "Error: Insufficient stock for sale.";
        }
        inventory[i].quantity -= amount;
      }
      saveInventory();
      return "Stock updated successfully!";
    }
  }
  return "Error: Product ID not found.";
}

EMSCRIPTEN_KEEPALIVE
const char *deleteProduct(int id) {
  for (int i = 0; i < productCount; i++) {
    if (inventory[i].productID == id) {
      for (int j = i; j < productCount - 1; j++) {
        inventory[j] = inventory[j + 1];
      }
      productCount--;
      saveInventory();
      return "Product deleted successfully.";
    }
  }
  return "Error: Product ID not found.";
}

EMSCRIPTEN_KEEPALIVE
int getProductCount() { return productCount; }

EMSCRIPTEN_KEEPALIVE
int getProductId(int index) {
  if (index >= 0 && index < productCount)
    return inventory[index].productID;
  return -1;
}

EMSCRIPTEN_KEEPALIVE
const char *getProductName(int index) {
  if (index >= 0 && index < productCount)
    return inventory[index].name;
  return "";
}

EMSCRIPTEN_KEEPALIVE
float getProductPrice(int index) {
  if (index >= 0 && index < productCount)
    return inventory[index].price;
  return -1.0f;
}

EMSCRIPTEN_KEEPALIVE
int getProductQuantity(int index) {
  if (index >= 0 && index < productCount)
    return inventory[index].quantity;
  return -1;
}

EMSCRIPTEN_KEEPALIVE
float getTotalValue() {
  float totalValue = 0.0;
  for (int i = 0; i < productCount; i++) {
    totalValue += (inventory[i].price * inventory[i].quantity);
  }
  return totalValue;
}
