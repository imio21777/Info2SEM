#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>

struct Product {
    int id;
    std::string name;
    std::string category;
    double price;
    int stock;
    bool isDeleted;
};

struct Transaction {
    int id;
    int productId;
    std::string type;
    std::string operator_name;
    std::string timestamp;
    int quantity;
};

class InventoryManager {
private:
    std::vector<Product> products;
    std::vector<Transaction> transactions;
    std::string productFile;
    std::string transactionFile;

    int nextProductId;
    int nextTransactionId;

    void loadData();
    void saveProducts();
    void saveTransactions();
    std::string getCurrentTime();
    bool isValidDate(const std::string& date);

    // 辅助函数：处理中文字符对齐
    size_t getDisplayWidth(const std::string& str);
    std::string padString(const std::string& str, size_t width, bool leftAlign = true);
    std::string centerString(const std::string& str, size_t width);

public:
    InventoryManager();
    ~InventoryManager();

    void displayProductCatalog();
    void addProduct(const std::string& name, const std::string& category, double price, int stock);
    bool deleteProduct(int productId);
    bool restockProduct(int productId, int quantity, const std::string& operator_name);
    bool sellProduct(int productId, int quantity, const std::string& operator_name);
    void browseByCategory(const std::string& category, bool sortByStock = false);
    void queryTransactions(int productId, const std::string& startDate = "", const std::string& endDate = "", const std::string& operator_name = "");
    void salesSummary(const std::string& startDate = "", const std::string& endDate = "", const std::string& category = "");
};

#endif