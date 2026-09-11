#include "../include/inventory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <regex>
#include <map>

InventoryManager::InventoryManager() : nextProductId(1), nextTransactionId(1) {
    productFile = "data/products.txt";
    transactionFile = "data/transactions.txt";
    loadData();
}

InventoryManager::~InventoryManager() {
    saveProducts();
    saveTransactions();
}

std::string InventoryManager::getCurrentTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    return std::string(buf);
}

bool InventoryManager::isValidDate(const std::string& date) {
    std::regex dateRegex(R"(\d{4}-\d{2}-\d{2})");
    return std::regex_match(date, dateRegex);
}

size_t InventoryManager::getDisplayWidth(const std::string& str) {
    size_t width = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c <= 0x7F) {
            // ASCII字符，占1个显示宽度
            width += 1;
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // UTF-8 2字节字符
            width += 2;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // UTF-8 3字节字符（中文），占2个显示宽度
            width += 2;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // UTF-8 4字节字符
            width += 2;
            i += 4;
        } else {
            // 其他情况，按1个字符处理
            width += 1;
            i += 1;
        }
    }
    return width;
}

std::string InventoryManager::padString(const std::string& str, size_t width, bool leftAlign) {
    size_t displayWidth = getDisplayWidth(str);
    if (displayWidth >= width) {
        return str;
    }

    size_t padding = width - displayWidth;
    std::string result;

    if (leftAlign) {
        result = str + std::string(padding, ' ');
    } else {
        result = std::string(padding, ' ') + str;
    }

    return result;
}

std::string InventoryManager::centerString(const std::string& str, size_t width) {
    size_t displayWidth = getDisplayWidth(str);
    if (displayWidth >= width) {
        return str;
    }

    size_t totalPadding = width - displayWidth;
    size_t leftPadding = totalPadding / 2;
    size_t rightPadding = totalPadding - leftPadding;

    return std::string(leftPadding, ' ') + str + std::string(rightPadding, ' ');
}

void InventoryManager::loadData() {
    std::ifstream productStream(productFile);
    if (productStream.is_open()) {
        std::string line;
        while (std::getline(productStream, line)) {
            std::stringstream ss(line);
            Product product;
            std::string deleted;

            ss >> product.id >> product.name >> product.category >> product.price >> product.stock >> deleted;
            product.isDeleted = (deleted == "1");

            products.push_back(product);
            if (product.id >= nextProductId) {
                nextProductId = product.id + 1;
            }
        }
        productStream.close();
    }

    std::ifstream transactionStream(transactionFile);
    if (transactionStream.is_open()) {
        std::string line;
        while (std::getline(transactionStream, line)) {
            std::stringstream ss(line);
            Transaction transaction;

            ss >> transaction.id >> transaction.productId >> transaction.type;
            ss >> transaction.operator_name >> transaction.timestamp >> transaction.quantity;

            transactions.push_back(transaction);
            if (transaction.id >= nextTransactionId) {
                nextTransactionId = transaction.id + 1;
            }
        }
        transactionStream.close();
    }
}

void InventoryManager::saveProducts() {
    std::ofstream file(productFile);
    if (file.is_open()) {
        for (const auto& product : products) {
            file << product.id << " " << product.name << " " << product.category << " "
                 << product.price << " " << product.stock << " " << (product.isDeleted ? 1 : 0) << "\n";
        }
        file.close();
    }
}

void InventoryManager::saveTransactions() {
    std::ofstream file(transactionFile);
    if (file.is_open()) {
        for (const auto& transaction : transactions) {
            file << transaction.id << " " << transaction.productId << " " << transaction.type << " "
                 << transaction.operator_name << " " << transaction.timestamp << " " << transaction.quantity << "\n";
        }
        file.close();
    }
}

void InventoryManager::displayProductCatalog() {
    std::cout << "\n╔══════════════════ 商品目录 ══════════════════╗\n";

    std::map<std::string, std::vector<Product*>> categoryMap;
    for (auto& product : products) {
        if (!product.isDeleted) {
            categoryMap[product.category].push_back(&product);
        }
    }

    for (auto& pair : categoryMap) {
        std::cout << "\n【" << pair.first << "】\n";
        std::cout << "┌────┬─────────────────┬─────────────┬──────────┐\n";
        std::cout << "│ ID │     商品名称    │    单价     │   库存   │\n";
        std::cout << "├────┼─────────────────┼─────────────┼──────────┤\n";

        for (auto* product : pair.second) {
            std::string idStr = std::to_string(product->id);
            std::string priceStr = std::to_string(product->price);
            priceStr = priceStr.substr(0, priceStr.find('.') + 3) + "元";
            std::string stockStr = std::to_string(product->stock) + "件";

            std::cout << "│ " << centerString(idStr, 2) << " │ "
                      << centerString(product->name, 15) << " │ "
                      << centerString(priceStr, 11) << " │ "
                      << centerString(stockStr, 8) << " │\n";
        }
        std::cout << "└────┴─────────────────┴─────────────┴──────────┘\n";
    }
}

void InventoryManager::addProduct(const std::string& name, const std::string& category, double price, int stock) {
    if (price < 0 || stock < 0) {
        std::cout << "错误：价格和库存不能为负数\n";
        return;
    }

    Product product;
    product.id = nextProductId++;
    product.name = name;
    product.category = category;
    product.price = price;
    product.stock = stock;
    product.isDeleted = false;

    products.push_back(product);
    std::cout << "商品添加成功，ID: " << product.id << "\n";
}

bool InventoryManager::deleteProduct(int productId) {
    auto it = std::find_if(products.begin(), products.end(),
                          [productId](const Product& p) { return p.id == productId; });

    if (it != products.end() && !it->isDeleted) {
        it->isDeleted = true;
        std::cout << "商品 " << it->name << " 已删除\n";
        return true;
    } else {
        std::cout << "未找到指定商品或商品已被删除\n";
        return false;
    }
}

bool InventoryManager::restockProduct(int productId, int quantity, const std::string& operator_name) {
    if (quantity <= 0) {
        std::cout << "错误：进货数量必须大于0\n";
        return false;
    }

    auto it = std::find_if(products.begin(), products.end(),
                          [productId](const Product& p) { return p.id == productId && !p.isDeleted; });

    if (it != products.end()) {
        it->stock += quantity;

        Transaction transaction;
        transaction.id = nextTransactionId++;
        transaction.productId = productId;
        transaction.type = "进货";
        transaction.operator_name = operator_name;
        transaction.timestamp = getCurrentTime();
        transaction.quantity = quantity;

        transactions.push_back(transaction);
        std::cout << "进货成功，商品: " << it->name << "，数量: " << quantity << "，当前库存: " << it->stock << "\n";
        return true;
    } else {
        std::cout << "未找到指定商品（ID: " << productId << "）或商品已被删除\n";
        return false;
    }
}

bool InventoryManager::sellProduct(int productId, int quantity, const std::string& operator_name) {
    if (quantity <= 0) {
        std::cout << "错误：销售数量必须大于0\n";
        return false;
    }

    auto it = std::find_if(products.begin(), products.end(),
                          [productId](const Product& p) { return p.id == productId && !p.isDeleted; });

    if (it != products.end()) {
        if (it->stock >= quantity) {
            it->stock -= quantity;

            Transaction transaction;
            transaction.id = nextTransactionId++;
            transaction.productId = productId;
            transaction.type = "销售";
            transaction.operator_name = operator_name;
            transaction.timestamp = getCurrentTime();
            transaction.quantity = quantity;

            transactions.push_back(transaction);
            std::cout << "销售成功，商品: " << it->name << "，数量: " << quantity << "，剩余库存: " << it->stock << "\n";
            return true;
        } else {
            std::cout << "库存不足！当前库存: " << it->stock << "，请求销售: " << quantity << "\n";
            return false;
        }
    } else {
        std::cout << "未找到指定商品（ID: " << productId << "）或商品已被删除\n";
        return false;
    }
}

void InventoryManager::browseByCategory(const std::string& category, bool sortByStock) {
    std::vector<Product> categoryProducts;
    for (const auto& product : products) {
        if (!product.isDeleted && product.category == category) {
            categoryProducts.push_back(product);
        }
    }

    if (categoryProducts.empty()) {
        std::cout << "该类别下没有商品\n";
        return;
    }

    if (sortByStock) {
        std::sort(categoryProducts.begin(), categoryProducts.end(),
                 [](const Product& a, const Product& b) { return a.stock > b.stock; });
    }

    std::cout << "\n╔══════════════ 类别: " << category << " ══════════════╗\n";
    std::cout << "┌────┬─────────────────┬─────────────┬──────────┐\n";
    std::cout << "│ ID │     商品名称    │    单价     │   库存   │\n";
    std::cout << "├────┼─────────────────┼─────────────┼──────────┤\n";

    for (const auto& product : categoryProducts) {
        std::string idStr = std::to_string(product.id);
        std::string priceStr = std::to_string(product.price);
        priceStr = priceStr.substr(0, priceStr.find('.') + 3) + "元";
        std::string stockStr = std::to_string(product.stock) + "件";

        std::cout << "│ " << centerString(idStr, 2) << " │ "
                  << centerString(product.name, 15) << " │ "
                  << centerString(priceStr, 11) << " │ "
                  << centerString(stockStr, 8) << " │\n";
    }
    std::cout << "└────┴─────────────────┴─────────────┴──────────┘\n";
}

void InventoryManager::queryTransactions(int productId, const std::string& startDate, const std::string& endDate, const std::string& operator_name) {
    auto productIt = std::find_if(products.begin(), products.end(),
                                 [productId](const Product& p) { return p.id == productId; });

    if (productIt == products.end()) {
        std::cout << "未找到指定商品\n";
        return;
    }

    std::cout << "\n╔══════════════════ 商品 [" << productIt->name << "] 的进销记录 ══════════════════╗\n";
    std::cout << "┌──────┬──────────┬──────────┬─────────────────────────┬────────┐\n";
    std::cout << "│记录ID│ 操作类型 │  操作人  │        操作时间         │  数量  │\n";
    std::cout << "├──────┼──────────┼──────────┼─────────────────────────┼────────┤\n";

    bool found = false;
    for (const auto& transaction : transactions) {
        if (transaction.productId == productId) {
            bool matchDate = true;
            bool matchOperator = true;

            if (!startDate.empty() && !endDate.empty()) {
                std::string transDate = transaction.timestamp.substr(0, 10);
                matchDate = (transDate >= startDate && transDate <= endDate);
            }

            if (!operator_name.empty()) {
                matchOperator = (transaction.operator_name == operator_name);
            }

            if (matchDate && matchOperator) {
                std::string idStr = std::to_string(transaction.id);
                std::string quantityStr = std::to_string(transaction.quantity) + "件";

                std::cout << "│ " << centerString(idStr, 4) << " │ "
                          << centerString(transaction.type, 8) << " │ "
                          << centerString(transaction.operator_name, 8) << " │ "
                          << centerString(transaction.timestamp, 23) << " │ "
                          << centerString(quantityStr, 6) << " │\n";
                found = true;
            }
        }
    }

    if (!found) {
        std::cout << "│                      没有找到符合条件的记录                      │\n";
    }
    std::cout << "└──────┴──────────┴──────────┴─────────────────────────┴────────┘\n";
}

void InventoryManager::salesSummary(const std::string& startDate, const std::string& endDate, const std::string& category) {
    std::cout << "\n===== 销量汇总 =====\n";

    if (!startDate.empty() && !endDate.empty()) {
        std::cout << "时间范围: " << startDate << " 至 " << endDate << "\n";
    }
    if (!category.empty()) {
        std::cout << "类别: " << category << "\n";
    }

    std::map<int, int> salesMap;

    for (const auto& transaction : transactions) {
        if (transaction.type == "销售") {
            bool matchDate = true;
            bool matchCategory = true;

            if (!startDate.empty() && !endDate.empty()) {
                std::string transDate = transaction.timestamp.substr(0, 10);
                matchDate = (transDate >= startDate && transDate <= endDate);
            }

            if (!category.empty()) {
                auto productIt = std::find_if(products.begin(), products.end(),
                                            [transaction](const Product& p) { return p.id == transaction.productId; });
                if (productIt != products.end()) {
                    matchCategory = (productIt->category == category);
                }
            }

            if (matchDate && matchCategory) {
                salesMap[transaction.productId] += transaction.quantity;
            }
        }
    }

    std::cout << "┌─────┬─────────────────┬─────────────┬───────────┐\n";
    std::cout << "│ ID  │     商品名称    │    类别     │   销量    │\n";
    std::cout << "├─────┼─────────────────┼─────────────┼───────────┤\n";

    int totalSales = 0;
    for (const auto& pair : salesMap) {
        auto productIt = std::find_if(products.begin(), products.end(),
                                    [pair](const Product& p) { return p.id == pair.first; });
        if (productIt != products.end()) {
            std::string idStr = std::to_string(pair.first);
            std::string salesStr = std::to_string(pair.second) + "件";

            std::cout << "│ " << centerString(idStr, 3) << " │ "
                      << centerString(productIt->name, 15) << " │ "
                      << centerString(productIt->category, 11) << " │ "
                      << centerString(salesStr, 9) << " │\n";
            totalSales += pair.second;
        }
    }

    std::cout << "├─────┴─────────────────┴─────────────┼───────────┤\n";
    std::string totalSalesStr = std::to_string(totalSales) + "件";
    std::cout << "│" << centerString("总销量", 37) << "│ " << centerString(totalSalesStr, 9) << " │\n";
    std::cout << "└─────┴─────────────────┴─────────────┴───────────┘\n";
}