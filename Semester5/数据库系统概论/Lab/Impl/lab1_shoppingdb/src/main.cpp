#include "../include/inventory.h"
#include <iostream>
#include <string>
#include <limits>
#include <sstream>

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void showMenu() {
    std::cout << "\n===== 商城库存管理系统 =====\n";
    std::cout << "1. 查看商品目录\n";
    std::cout << "2. 添加商品\n";
    std::cout << "3. 删除商品\n";
    std::cout << "4. 进货\n";
    std::cout << "5. 销售\n";
    std::cout << "6. 按类别浏览\n";
    std::cout << "7. 查询进销记录\n";
    std::cout << "8. 销量汇总\n";
    std::cout << "0. 退出\n";
    std::cout << "\n提示: 在任何功能中输入 'q' 可返回主菜单\n";
    std::cout << "请选择操作: ";
}

bool checkForQuit(const std::string& input) {
    return input == "q" || input == "Q";
}

template<typename T>
bool safeInput(T& value, const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::cin >> input;

    if (checkForQuit(input)) {
        std::cout << "返回主菜单...\n";
        return false;
    }

    std::stringstream ss(input);
    if (!(ss >> value) || !ss.eof()) {
        std::cout << "输入格式错误\n";
        return false;
    }

    return true;
}

int main() {
    InventoryManager manager;
    int choice;

    while (true) {
        showMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            clearInput();
            std::cout << "无效输入，请输入数字\n";
            continue;
        }

        switch (choice) {
            case 1:
                manager.displayProductCatalog();
                break;

            case 2: {
                std::string name, category;
                double price;
                int stock;

                std::cout << "商品名称 (输入q返回主菜单): ";
                std::cin >> name;
                if (checkForQuit(name)) break;
                if (name.empty()) {
                    std::cout << "错误: 商品名称不能为空\n";
                    break;
                }

                std::cout << "商品类别 (输入q返回主菜单): ";
                std::cin >> category;
                if (checkForQuit(category)) break;
                if (category.empty()) {
                    std::cout << "错误: 商品类别不能为空\n";
                    break;
                }

                while (true) {
                    if (!safeInput(price, "商品单价 (输入q返回主菜单): ")) break;
                    if (price <= 0) {
                        std::cout << "错误: 商品单价必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }
                if (checkForQuit(std::to_string(price))) break;

                while (true) {
                    if (!safeInput(stock, "初始库存 (输入q返回主菜单): ")) break;
                    if (stock < 0) {
                        std::cout << "错误: 初始库存不能为负数，请重新输入\n";
                        continue;
                    }
                    break;
                }
                if (checkForQuit(std::to_string(stock))) break;

                manager.addProduct(name, category, price, stock);
                break;
            }

            case 3: {
                int productId;
                while (true) {
                    if (!safeInput(productId, "请输入要删除的商品ID (输入q返回主菜单): ")) goto case3_exit;
                    if (productId <= 0) {
                        std::cout << "错误: 商品ID必须大于0，请重新输入\n";
                        continue;
                    }
                    bool success = manager.deleteProduct(productId);
                    if (!success) {
                        std::cout << "请检查商品ID是否正确，是否重新输入? (y/n): ";
                        char retry;
                        std::cin >> retry;
                        if (retry != 'y' && retry != 'Y') break;
                        continue;
                    }
                    break;
                }
                case3_exit:
                break;
            }

            case 4: {
                int productId, quantity;
                std::string operator_name;
                bool success;

                while (true) {
                    if (!safeInput(productId, "商品ID (输入q返回主菜单): ")) goto case4_exit;
                    if (productId <= 0) {
                        std::cout << "错误: 商品ID必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }

                while (true) {
                    if (!safeInput(quantity, "进货数量 (输入q返回主菜单): ")) goto case4_exit;
                    if (quantity <= 0) {
                        std::cout << "错误: 进货数量必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }

                while (true) {
                    std::cout << "操作人 (输入q返回主菜单): ";
                    std::cin >> operator_name;
                    if (checkForQuit(operator_name)) goto case4_exit;
                    if (operator_name.empty()) {
                        std::cout << "错误: 操作人姓名不能为空，请重新输入\n";
                        continue;
                    }
                    break;
                }

                success = manager.restockProduct(productId, quantity, operator_name);
                if (!success) {
                    std::cout << "进货失败，请检查商品ID是否正确\n";
                }
                case4_exit:
                break;
            }

            case 5: {
                int productId, quantity;
                std::string operator_name;
                bool success;

                while (true) {
                    if (!safeInput(productId, "商品ID (输入q返回主菜单): ")) goto case5_exit;
                    if (productId <= 0) {
                        std::cout << "错误: 商品ID必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }

                while (true) {
                    if (!safeInput(quantity, "销售数量 (输入q返回主菜单): ")) goto case5_exit;
                    if (quantity <= 0) {
                        std::cout << "错误: 销售数量必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }

                while (true) {
                    std::cout << "操作人 (输入q返回主菜单): ";
                    std::cin >> operator_name;
                    if (checkForQuit(operator_name)) goto case5_exit;
                    if (operator_name.empty()) {
                        std::cout << "错误: 操作人姓名不能为空，请重新输入\n";
                        continue;
                    }
                    break;
                }

                success = manager.sellProduct(productId, quantity, operator_name);
                if (!success) {
                    std::cout << "销售失败，请检查商品ID和库存情况\n";
                }
                case5_exit:
                break;
            }

            case 6: {
                std::string category;
                char sortChoice;

                std::cout << "请输入类别名称 (输入q返回主菜单): ";
                std::cin >> category;
                if (checkForQuit(category)) break;

                std::cout << "是否按库存排序? (y/n, 输入q返回主菜单): ";
                std::string sortInput;
                std::cin >> sortInput;
                if (checkForQuit(sortInput)) break;

                sortChoice = sortInput[0];
                bool sortByStock = (sortChoice == 'y' || sortChoice == 'Y');
                manager.browseByCategory(category, sortByStock);
                break;
            }

            case 7: {
                int productId;
                std::string startDate, endDate, operator_name;
                std::string dateInput, operatorInput;

                while (true) {
                    if (!safeInput(productId, "请输入商品ID (输入q返回主菜单): ")) goto case7_exit;
                    if (productId <= 0) {
                        std::cout << "错误: 商品ID必须大于0，请重新输入\n";
                        continue;
                    }
                    break;
                }

                std::cout << "是否按时间范围查询? (y/n, 输入q返回主菜单): ";
                std::cin >> dateInput;
                if (checkForQuit(dateInput)) break;

                if (dateInput == "y" || dateInput == "Y") {
                    std::cout << "开始日期 (YYYY-MM-DD, 输入q返回主菜单): ";
                    std::cin >> startDate;
                    if (checkForQuit(startDate)) break;

                    std::cout << "结束日期 (YYYY-MM-DD, 输入q返回主菜单): ";
                    std::cin >> endDate;
                    if (checkForQuit(endDate)) break;
                }

                std::cout << "是否按操作人查询? (y/n, 输入q返回主菜单): ";
                std::cin >> operatorInput;
                if (checkForQuit(operatorInput)) break;

                if (operatorInput == "y" || operatorInput == "Y") {
                    std::cout << "操作人姓名 (输入q返回主菜单): ";
                    std::cin >> operator_name;
                    if (checkForQuit(operator_name)) break;
                }

                manager.queryTransactions(productId, startDate, endDate, operator_name);
                case7_exit:
                break;
            }

            case 8: {
                std::string startDate, endDate, category;
                std::string dateInput, categoryInput;

                std::cout << "是否按时间范围汇总? (y/n, 输入q返回主菜单): ";
                std::cin >> dateInput;
                if (checkForQuit(dateInput)) break;

                if (dateInput == "y" || dateInput == "Y") {
                    std::cout << "开始日期 (YYYY-MM-DD, 输入q返回主菜单): ";
                    std::cin >> startDate;
                    if (checkForQuit(startDate)) break;

                    std::cout << "结束日期 (YYYY-MM-DD, 输入q返回主菜单): ";
                    std::cin >> endDate;
                    if (checkForQuit(endDate)) break;
                }

                std::cout << "是否按类别汇总? (y/n, 输入q返回主菜单): ";
                std::cin >> categoryInput;
                if (checkForQuit(categoryInput)) break;

                if (categoryInput == "y" || categoryInput == "Y") {
                    std::cout << "类别名称 (输入q返回主菜单): ";
                    std::cin >> category;
                    if (checkForQuit(category)) break;
                }

                manager.salesSummary(startDate, endDate, category);
                break;
            }

            case 0:
                std::cout << "谢谢使用！\n";
                return 0;

            default:
                std::cout << "无效选择，请重新输入\n";
                break;
        }

        clearInput();
    }

    return 0;
}