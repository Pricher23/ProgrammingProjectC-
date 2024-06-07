#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

void clearScreen() {
    system(CLEAR_COMMAND);
}

void showLogo(const std::string& userType) {
    std::cout << " APS - " << userType << "\n\n";
}


class User {
protected:
    std::string name;
    std::string surname;
    std::string password;
public:
    User(const std::string& name, const std::string& surname, const std::string& password)
        : name(name), surname(surname), password(password) {}

    std::string getName() const { return name; }
    std::string getSurname() const { return surname; }
    std::string getPassword() const { return password; }
};

class Customer : public User {
    int id;
    std::vector<std::pair<std::string, int>> cart; // item name and quantity

public:
    Customer(const std::string& name, const std::string& surname, const std::string& password, int id)
        : User(name, surname, password), id(id) {}

    int getId() const { return id; }

    void seeItems() {
        clearScreen();
        showLogo("Customer");
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open items file.\n";
            return;
        }
        std::string line;
        std::cout << "Available items:\n";
        while (getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    }

    void searchItems(const std::string& searchItem) {
        clearScreen();
        showLogo("Customer");

        std::vector<std::string> matchingItems;
        std::ifstream file("Stocks.txt");
        std::string line;
        while (getline(file, line)) {
            if (line.find(searchItem) != std::string::npos) {
                matchingItems.push_back(line);
            }
        }
        file.close();

        if (matchingItems.empty()) {
            std::cout << "No matching items found.\n";
            return;
        }

        std::cout << "Matching items:\n";
        for (size_t i = 0; i < matchingItems.size(); ++i) {
            std::cout << i + 1 << ". " << matchingItems[i] << "\n";
        }

        int choice;
        std::cout << "Enter the number of the item to add to cart: ";
        std::cin >> choice;

        if (choice < 1 || choice > matchingItems.size()) {
            std::cout << "Invalid choice.\n";
            return;
        }

        std::string selectedItem = matchingItems[choice - 1];
        std::stringstream ss(selectedItem);
        int sellerId, itemQuantity;
        std::string itemName;
        double itemValue;
        ss >> sellerId >> itemName >> itemQuantity >> itemValue;

        int quantity;
        std::cout << "Enter the quantity: ";
        std::cin >> quantity;

        if (quantity > itemQuantity) {
            std::cout << "Not enough stock available for " << itemName << ".\n";
            return;
        }

        addItemToCart(itemName, quantity, itemValue);
        updateStock(itemName, -quantity); // Decrease the item quantity in stock
    }

    void addItemToCart(const std::string& item, int quantity, double itemValue) {
        bool itemExists = false;
        for (auto& cartItem : cart) {
            if (cartItem.first == item) {
                cartItem.second += quantity;
                itemExists = true;
                break;
            }
        }
        if (!itemExists) {
            cart.push_back({item, quantity});
        }

        std::ofstream cartFile("Cart.txt", std::ios::app);
        if (cartFile.is_open()) {
            cartFile << id << " " << item << " " << quantity << " " << itemValue << "\n";
            cartFile.close();
        }
    }

    bool updateStock(const std::string& item, int quantityChange) {
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open stocks file.\n";
            return false;
        }

        std::ofstream temp("temp_stocks.txt");
        std::string line;
        bool itemUpdated = false;
        while (getline(file, line)) {
            std::stringstream ss(line);
            int sellerId, itemQuantity;
            std::string itemName;
            double itemValue;
            ss >> sellerId >> itemName >> itemQuantity >> itemValue;

            if (itemName == item) {
                if (itemQuantity + quantityChange < 0) {
                    std::cout << "Not enough stock for " << item << ".\n";
                    temp.close();
                    file.close();
                    remove("temp_stocks.txt");
                    return false;
                }
                itemQuantity += quantityChange;
                itemUpdated = true;
            }

            temp << sellerId << " " << itemName << " " << itemQuantity << " " << itemValue << "\n";
        }
        file.close();
        temp.close();
        remove("Stocks.txt");
        rename("temp_stocks.txt", "Stocks.txt");

        return itemUpdated;
    }

    void removeItemFromCart(const std::string& item, int quantity) {
        for (auto it = cart.begin(); it != cart.end(); ++it) {
            if (it->first == item) {
                if (it->second > quantity) {
                    it->second -= quantity;
                } else {
                    cart.erase(it);
                }
                break;
            }
        }
        std::ofstream temp("temp_cart.txt");
        std::ifstream cartFile("Cart.txt");
        std::string line;
        while (getline(cartFile, line)) {
            if (line.find(item) == std::string::npos || line.find(std::to_string(id)) == std::string::npos) {
                temp << line << "\n";
            }
        }
        cartFile.close();
        temp.close();
        remove("Cart.txt");
        rename("temp_cart.txt", "Cart.txt");

        updateStock(item, quantity); // Increase the item quantity in stock
    }

    void viewCart() {
        clearScreen();
        showLogo("Customer");
        std::cout << "Your cart contains:\n";
        double totalValue = 0;
        for (const auto& cartItem : cart) {
            std::ifstream file("Stocks.txt");
            if (!file.is_open()) {
                std::cout << "Unable to open items file.\n";
                return;
            }
            std::string line;
            while (getline(file, line)) {
                if (line.find(cartItem.first) != std::string::npos) {
                    std::stringstream ss(line);
                    int sellerId, itemQuantity;
                    std::string itemName;
                    double itemValue;
                    ss >> sellerId >> itemName >> itemQuantity >> itemValue;
                    totalValue += cartItem.second * itemValue;
                    std::cout << cartItem.first << " - Quantity: " << cartItem.second << " - Total: " << cartItem.second * itemValue << " RON\n";
                    break;
                }
            }
            file.close();
        }
        std::cout << "Total value: " << totalValue << " RON\n";
    }

    void continueToPurchase() {
        std::ofstream historyFile("History.txt", std::ios::app);
        if (historyFile.is_open()) {
            double totalValue = 0;
            for (const auto& cartItem : cart) {
                historyFile << id << " " << cartItem.first << " / " << cartItem.second << "\n";
                std::ifstream file("Stocks.txt");
                if (file.is_open()) {
                    std::string line;
                    while (getline(file, line)) {
                        if (line.find(cartItem.first) != std::string::npos) {
                            std::stringstream ss(line);
                            int sellerId, itemQuantity;
                            std::string itemName;
                            double itemValue;
                            ss >> sellerId >> itemName >> itemQuantity >> itemValue;
                            totalValue += cartItem.second * itemValue;
                            updateSellerAndStoreFunds(sellerId, itemValue * cartItem.second);
                            break;
                        }
                    }
                    file.close();
                }
            }
            historyFile << "Total value: " << totalValue << " RON\n";
            historyFile.close();
        }
        cart.clear();
        std::ofstream cartFile("Cart.txt", std::ios::trunc); // Clear the cart file
        cartFile.close();
    }

    void updateSellerAndStoreFunds(int sellerId, double totalValue) {
        double storeShare = totalValue * 0.15;
        double sellerShare = totalValue * 0.85;

        // Update store funds
        std::ifstream storeFile("Store_funds.txt");
        std::ofstream tempStoreFile("temp_store_funds.txt");
        std::string line;
        bool storeUpdated = false;
        while (getline(storeFile, line)) {
            std::stringstream ss(line);
            std::string storeName;
            double currentFunds;
            ss >> storeName >> currentFunds;
            if (storeName == "Store") {
                currentFunds += storeShare;
                storeUpdated = true;
            }
            tempStoreFile << storeName << " " << currentFunds << "\n";
        }
        if (!storeUpdated) {
            tempStoreFile << "Store " << storeShare << "\n";
        }
        storeFile.close();
        tempStoreFile.close();
        remove("Store_funds.txt");
        rename("temp_store_funds.txt", "Store_funds.txt");

        // Update seller funds
        std::ifstream sellerFile("Sellers_file.txt");
        std::ofstream tempSellerFile("temp_sellers_file.txt");
        while (getline(sellerFile, line)) {
            std::stringstream ss(line);
            int id;
            std::string name, surname, password;
            double currentFunds;
            ss >> id >> name >> surname >> password >> currentFunds;
            if (id == sellerId) {
                currentFunds += sellerShare;
            }
            tempSellerFile << id << " " << name << " " << surname << " " << password << " " << currentFunds << "\n";
        }
        sellerFile.close();
        tempSellerFile.close();
        remove("Sellers_file.txt");
        rename("temp_sellers_file.txt", "Sellers_file.txt");
    }

    void viewHistory() {
    clearScreen();
    showLogo("Customer");
    std::ifstream historyFile("History.txt");
    if (!historyFile.is_open()) {
        std::cout << "Unable to open history file.\n";
        return;
    }
    std::string line;
    while (getline(historyFile, line)) {
        std::stringstream ss(line);
        int customerId;
        ss >> customerId;
        if (customerId == id) {
            std::cout << line << "\n";
        }
    }
    historyFile.close();
}

};

class Seller : public User {
    int id;
    double money;
    std::vector<std::string> itemList;  // Declare itemList here

public:
    Seller(const std::string& name, const std::string& surname, const std::string& password, int id)
        : User(name, surname, password), id(id), money(0.0) {}

    int getId() const { return id; }
    double getMoney() const { return money; }
    void setMoney(double amount) { money = amount; }

    void seeStock() const {
        clearScreen();
        showLogo("Seller");
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open stocks file.\n";
            return;
        }
        std::string line;
        while (getline(file, line)) {
            std::stringstream ss(line);
            int sellerId;
            ss >> sellerId;
            if (sellerId == id) {
                std::cout << line << "\n";
            }
        }
        file.close();
    }

    void searchAndChangeItem() {
        clearScreen();
        showLogo("Seller");

        std::string searchItem;
        std::cout << "Enter item name to search: ";
        std::cin.ignore();
        std::getline(std::cin, searchItem);

        std::vector<std::string> matchingItems;
        std::ifstream file("Stocks.txt");
        std::string line;
        while (getline(file, line)) {
            std::stringstream ss(line);
            int itemId;
            std::string itemName;
            ss >> itemId >> itemName;
            if (itemId == id && itemName.find(searchItem) != std::string::npos) {
                matchingItems.push_back(line);
            }
        }
        file.close();

        if (matchingItems.empty()) {
            std::cout << "No matching items found.\n";
            return;
        }

        std::cout << "Matching items:\n";
        for (size_t i = 0; i < matchingItems.size(); ++i) {
            std::cout << i + 1 << ". " << matchingItems[i] << "\n";
        }

        int choice;
        std::cout << "Enter the number of the item to change: ";
        std::cin >> choice;

        if (choice < 1 || choice > matchingItems.size()) {
            std::cout << "Invalid choice.\n";
            return;
        }

        std::string oldItem = matchingItems[choice - 1];

        std::string newItem;
        double newValue;
        int newQuantity;
        std::cout << "Enter new item name: ";
        std::cin.ignore();
        std::getline(std::cin, newItem);
        std::cout << "Enter new value: ";
        std::cin >> newValue;
        std::cout << "Enter new quantity: ";
        std::cin >> newQuantity;

        changeItem(oldItem, newItem, newValue, newQuantity);
    }

    void addItem(const std::string& item, double value, int quantity) {
        itemList.push_back(item);
        std::ofstream file("Stocks.txt", std::ios::app);
        if (file.is_open()) {
            file << id << " " << item << " " << quantity << " " << value << "\n";
            file.close();
        }
    }

    void removeItem() {
        clearScreen();
        showLogo("Seller");

        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open stocks file.\n";
            return;
        }

        std::vector<std::string> sellerItems;
        std::string line;
        while (getline(file, line)) {
            std::stringstream ss(line);
            int sellerId;
            std::string itemName;
            ss >> sellerId >> itemName;
            if (sellerId == id) {
                sellerItems.push_back(line);
            }
        }
        file.close();

        if (sellerItems.empty()) {
            std::cout << "No items found for this seller.\n";
            return;
        }

        std::cout << "Items for this seller:\n";
        for (size_t i = 0; i < sellerItems.size(); ++i) {
            std::cout << i + 1 << ". " << sellerItems[i] << "\n";
        }

        int choice;
        std::cout << "Enter the number of the item to remove: ";
        std::cin >> choice;

        if (choice < 1 || choice > sellerItems.size()) {
            std::cout << "Invalid choice.\n";
            return;
        }

        std::string itemToRemove = sellerItems[choice - 1];
        std::stringstream ss(itemToRemove);
        int sellerId;
        std::string itemName;
        ss >> sellerId >> itemName;

        // Remove item from the stock
        std::ofstream temp("temp_stocks.txt");
        std::ifstream inFile("Stocks.txt");
        while (getline(inFile, line)) {
            if (line != itemToRemove) {
                temp << line << "\n";
            }
        }
        inFile.close();
        temp.close();
        remove("Stocks.txt");
        rename("temp_stocks.txt", "Stocks.txt");

        // Remove item from the itemList
        itemList.erase(std::remove(itemList.begin(), itemList.end(), itemName), itemList.end());

        std::cout << "Item removed successfully.\n";
    }

    void changeItem(const std::string& oldItem, const std::string& newItem, double newValue, int newQuantity) {
        std::ofstream temp("temp_stocks.txt");
        std::ifstream file("Stocks.txt");
        std::string line;
        bool itemUpdated = false;

        while (getline(file, line)) {
            std::stringstream ss(line);
            int sellerId, itemQuantity;
            std::string itemName;
            double itemValue;
            ss >> sellerId >> itemName >> itemQuantity >> itemValue;

            if (line == oldItem && sellerId == id) {
                temp << sellerId << " " << newItem << " " << newQuantity << " " << newValue << "\n";
                itemUpdated = true;
            } else {
                temp << line << "\n";
            }
        }
        if (!itemUpdated) {
            temp << id << " " << newItem << " " << newQuantity << " " << newValue << "\n";
        }
        file.close();
        temp.close();
        remove("Stocks.txt");
        rename("temp_stocks.txt", "Stocks.txt");
    }

    void viewStatistics() const {
        clearScreen();
        showLogo("Seller");
        std::cout << "Total money made: " << money << "\n";
        std::cout << "Items sold: " << itemList.size() << "\n";
    }
};

class Admin {
    std::string nickname;
    std::string password;
public:
    Admin(const std::string& nickname, const std::string& password)
        : nickname(nickname), password(password) {}

    std::string getNickname() const { return nickname; }
    std::string getPassword() const { return password; }

    void seeInventory() const {
        clearScreen();
        showLogo("Admin");
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open inventory file.\n";
            return;
        }
        std::string line;
        std::cout << "Available items:\n";
        while (getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    }

    void updateInventory() {
        clearScreen();
        showLogo("Admin");
        seeInventory(); // Show all items before updating the inventory
        int choice;
        std::string itemName;
        double price;
        int quantity;

        std::cout << "Update Inventory:\n";
        std::cout << "1. Delete item\n";
        std::cout << "2. Change item\n";
        std::cout << "3. Add item\n";
        std::cout << "4. Back\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter item name to delete: ";
                std::cin.ignore();
                std::getline(std::cin, itemName);
                deleteItemFromInventory(itemName);
                break;
            case 2:
                changeItemInInventory();
                break;
            case 3:
                std::cout << "Enter item name to add: ";
                std::cin.ignore();
                std::getline(std::cin, itemName);
                std::cout << "Enter price: ";
                std::cin >> price;
                std::cout << "Enter quantity: ";
                std::cin >> quantity;
                addItemToInventory(itemName, price, quantity);
                break;
            case 4:
                return;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }

    void addItemToInventory(const std::string& itemName, double price, int quantity) {
        std::ofstream file("Stocks.txt", std::ios::app);
        if (file.is_open()) {
            file << "Store " << itemName << " " << price << " " << quantity << "\n";
            file.close();
        } else {
            std::cout << "Unable to open inventory file.\n";
        }
    }

    void deleteItemFromInventory(const std::string& itemName) {
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open inventory file.\n";
            return;
        }

        std::ofstream temp("temp.txt");
        std::string line;
        while (getline(file, line)) {
            if (line.find(itemName) == std::string::npos) {
                temp << line << "\n";
            }
        }
        file.close();
        temp.close();
        remove("Stocks.txt");
        rename("temp.txt", "Stocks.txt");
    }

    void changeItemInInventory() {
        clearScreen();
        showLogo("Admin");

        std::string searchItem;
        std::cout << "Enter item name to search: ";
        std::cin.ignore();
        std::getline(std::cin, searchItem);

        std::vector<std::string> matchingItems;
        std::ifstream file("Stocks.txt");
        std::string line;
        while (getline(file, line)) {
            if (line.find(searchItem) != std::string::npos) {
                matchingItems.push_back(line);
            }
        }
        file.close();

        if (matchingItems.empty()) {
            std::cout << "No matching items found.\n";
            return;
        }

        std::cout << "Matching items:\n";
        for (size_t i = 0; i < matchingItems.size(); ++i) {
            std::cout << i + 1 << ". " << matchingItems[i] << "\n";
        }

        int choice;
        std::cout << "Enter the number of the item to change: ";
        std::cin >> choice;

        if (choice < 1 || choice > matchingItems.size()) {
            std::cout << "Invalid choice.\n";
            return;
        }

        std::string oldItem = matchingItems[choice - 1];
        std::stringstream ss(oldItem);
        int storeId;
        std::string itemName;
        double itemValue;
        int itemQuantity;
        ss >> storeId >> itemName >> itemQuantity >> itemValue;

        std::string newItemName;
        double newItemValue;
        int newItemQuantity;
        std::cout << "Enter new item name: ";
        std::cin.ignore();
        std::getline(std::cin, newItemName);
        std::cout << "Enter new value: ";
        std::cin >> newItemValue;
        std::cout << "Enter new quantity: ";
        std::cin >> newItemQuantity;

        std::vector<std::string> lines;
        file.open("Stocks.txt");
        while (getline(file, line)) {
            if (line != oldItem) {
                lines.push_back(line);
            }
        }
        file.close();

        std::ofstream temp("Stocks.txt");
        for (const auto& l : lines) {
            temp << l << "\n";
        }
        temp << storeId << " " << newItemName << " " << newItemQuantity << " " << newItemValue << "\n";
        temp.close();
    }

    void seeSellers() const {
        clearScreen();
        showLogo("Admin");
        std::ifstream file("Sellers_file.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open sellers file.\n";
            return;
        }
        std::string line;
        while (getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    }

    void removeSeller(const std::string& sellerName) {
        std::ifstream file("Sellers_file.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open sellers file.\n";
            return;
        }

        std::ofstream temp("temp.txt");
        std::string line;
        while (getline(file, line)) {
            if (line.find(sellerName) == std::string::npos) {
                temp << line << "\n";
            }
        }
        file.close();
        temp.close();
        remove("Sellers_file.txt");
        rename("temp.txt", "Sellers_file.txt");
    }

    void seeItemsPerSeller(const std::string& sellerName) const {
        clearScreen();
        showLogo("Admin");
        std::ifstream file("Stocks.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open items file.\n";
            return;
        }
        std::string line;
        while (getline(file, line)) {
            if (line.find(sellerName) != std::string::npos) {
                std::cout << line << "\n";
            }
        }
        file.close();
    }

    void seeCustomersHistory() const {
        clearScreen();
        showLogo("Admin");
        std::ifstream file("History.txt");
        if (!file.is_open()) {
            std::cout << "Unable to open history file.\n";
            return;
        }
        std::string line;
        while (getline(file, line)) {
            std::cout << line << "\n";
        }
        file.close();
    }

    void seeStatistics() const {
        clearScreen();
        showLogo("Admin");

        // Number of customers
        std::ifstream customersFile("Customers_file.txt");
        int customerCount = 0;
        std::string line;
        while (getline(customersFile, line)) {
            customerCount++;
        }
        customersFile.close();

        // Products sold and money made
        std::ifstream historyFile("History.txt");
        std::vector<std::string> productTypes;
        double totalStoreEarnings = 0.0;
        while (getline(historyFile, line)) {
            if (line[0] == 'T') {
                std::stringstream ss(line);
                std::string temp;
                double value;
                ss >> temp >> value;
                totalStoreEarnings += value;
            } else {
                std::stringstream ss(line);
                int customerId;
                std::string product;
                ss >> customerId >> product;
                if (std::find(productTypes.begin(), productTypes.end(), product) == productTypes.end()) {
                    productTypes.push_back(product);
                }
            }
        }
        historyFile.close();

        std::cout << "Statistics:\n";
        std::cout << "Total number of customers: " << customerCount << "\n";
        std::cout << "Total products sold: " << productTypes.size() << "\n";
        std::cout << "Total store earnings: " << totalStoreEarnings << " RON\n";
    }
};

bool isValidPassword(const std::string& password) {
    return password.length() >= 8;
}

int generateCustomerId() {
    std::ifstream file("Customers_file.txt");
    std::string line;
    int lastId = 0;

    if (file.is_open()) {
        while (getline(file, line)) {
            std::stringstream ss(line);
            int currentId;
            ss >> currentId;
            if (currentId > lastId) {
                lastId = currentId;
            }
        }
        file.close();
    }
    return lastId + 1;
}



int generateSellerId() {
    static int sellerId = 0;
    return ++sellerId;
}

int verifyCustomer(const std::string& name, const std::string& surname, const std::string& password) {
    std::ifstream file("Customers_file.txt");
    std::string fileName, fileSurname, filePassword;
    int fileId;
    if (file.is_open()) {
        while (file >> fileId >> fileName >> fileSurname >> filePassword) {
            if (fileName == name && fileSurname == surname && filePassword == password) {
                return fileId;
            }
        }
        file.close();
    }
    return -1;
}

bool verifySeller(const std::string& name, const std::string& surname, const std::string& password, Seller& seller) {
    std::ifstream file("Sellers_file.txt");
    std::string fileName, fileSurname, filePassword;
    int fileId;
    double fileMoney;
    if (file.is_open()) {
        while (file >> fileId >> fileName >> fileSurname >> filePassword >> fileMoney) {
            if (fileName == name && fileSurname == surname && filePassword == password) {
                seller = Seller(fileName, fileSurname, filePassword, fileId);  // Initialize the seller object
                seller.setMoney(fileMoney);  // Set the seller's money
                return true;
            }
        }
        file.close();
    }
    return false;
}




bool verifyAdmin(const std::string& nickname, const std::string& password, Admin& admin) {
    std::ifstream file("admins_file.txt");
    std::string fileNickname, filePassword;
    if (file.is_open()) {
        while (file >> fileNickname >> filePassword) {
            if (fileNickname == nickname && filePassword == password) {
                admin = Admin(fileNickname, filePassword);
                return true;
            }
        }
        file.close();
    }
    return false;
}

void saveCustomer(const Customer& customer) {
    std::ofstream file("Customers_file.txt", std::ios::app);
    if (file.is_open()) {
        file << customer.getId() << " " << customer.getName() << " " << customer.getSurname() << " " << customer.getPassword() << "\n";
        file.close();
    }
}

void saveSeller(const Seller& seller) {
    std::ofstream file("Sellers_file.txt", std::ios::app);
    if (file.is_open()) {
        file << seller.getId() << " " << seller.getName() << " " << seller.getSurname() << " " << seller.getPassword() << " " << seller.getMoney() << "\n";
        file.close();
    }
}

void registerUser() {
    clearScreen();
    showLogo("Register");
    int choice;
    std::cout << "Register:\n";
    std::cout << "1. Customer\n";
    std::cout << "2. Seller\n";
    std::cout << "3. Back\n";
    std::cin >> choice;

    std::string name, surname, password;
    switch (choice) {
        case 1: {
            std::cout << "Enter name: ";
            std::cin >> name;
            std::cout << "Enter surname: ";
            std::cin >> surname;
            do {
                std::cout << "Enter password: ";
                std::cin >> password;
                if (!isValidPassword(password)) {
                    std::cout << "Password must be at least 8 characters long.\n";
                }
            } while (!isValidPassword(password));
            if (verifyCustomer(name, surname, password) == -1) {
                Customer customer(name, surname, password, generateCustomerId());
                saveCustomer(customer);
            } else {
                std::cout << "Customer already registered. Please login.\n";
            }
            break;
        }
        case 2: {
            std::cout << "Enter name: ";
            std::cin >> name;
            std::cout << "Enter surname: ";
            std::cin >> surname;
            do {
                std::cout << "Enter password: ";
                std::cin >> password;
                if (!isValidPassword(password)) {
                    std::cout << "Password must be at least 8 characters long.\n";
                }
            } while (!isValidPassword(password));
            Seller tempSeller(name, surname, password, 0);  // Temporary seller object
            if (!verifySeller(name, surname, password, tempSeller)) {
                Seller seller(name, surname, password, generateCustomerId());
                saveSeller(seller);
            } else {
                std::cout << "Seller already registered. Please login.\n";
            }
            break;
        }
        case 3:
            return;
        default:
            std::cout << "Invalid choice.\n";
            break;
    }
}

void showCustomerMenu(Customer& customer) {
    int choice;
    do {
        clearScreen();
        showLogo("Customer");
        std::cout << "Customer Menu:\n";
        std::cout << "1. See/Buy Items\n";
        std::cout << "2. View Cart\n";
        std::cout << "3. View History\n";
        std::cout << "4. Continue to Purchase\n";
        std::cout << "5. Logout\n";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string searchItem;
                std::cout << "Enter item name to search: ";
                std::cin.ignore();
                std::getline(std::cin, searchItem);
                customer.searchItems(searchItem);
                break;
            }
            case 2:
                customer.viewCart();
                break;
            case 3:
                customer.viewHistory();
                break;
            case 4:
                customer.continueToPurchase();
                break;
            case 5:
                std::cout << "Logging out...\n";
                return;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
        std::cout << "\nPress any key to continue...";
        std::cin.ignore().get(); // Pause the screen
    } while (choice != 5);
}

void showSellerMenu(Seller& seller) {
    int choice;
    do {
        clearScreen();
        showLogo("Seller");
        std::cout << "Seller Menu:\n";
        std::cout << "1. See Stock\n";
        std::cout << "2. Add Item\n";
        std::cout << "3. Remove Item\n";
        std::cout << "4. Change Item\n";
        std::cout << "5. View Statistics\n";
        std::cout << "6. Logout\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                seller.seeStock();
                break;
            case 2: {
                std::string item;
                double value;
                int quantity;
                std::cout << "Enter item to add: ";
                std::cin.ignore();
                std::getline(std::cin, item);
                std::cout << "Enter value of item: ";
                std::cin >> value;
                std::cout << "Enter quantity: ";
                std::cin >> quantity;
                seller.addItem(item, value, quantity);
                break;
            }
            case 3:
                seller.removeItem(); // Call removeItem without arguments
                break;
            case 4:
                seller.searchAndChangeItem();
                break;
            case 5:
                seller.viewStatistics();
                break;
            case 6:
                std::cout << "Logging out...\n";
                return;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
        std::cout << "\nPress any key to continue...";
        std::cin.ignore().get(); // Pause the screen
    } while (choice != 6);
}

void showAdminMenu(Admin& admin) {
    int choice;
    do {
        clearScreen();
        showLogo("Admin");
        std::cout << "Admin Menu:\n";
        std::cout << "1. See Inventory\n";
        std::cout << "2. Update Inventory\n";
        std::cout << "3. See Sellers\n";
        std::cout << "4. See Customers History\n";
        std::cout << "5. See Statistics\n";
        std::cout << "6. Logout\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                admin.seeInventory();
                break;
            case 2:
                admin.updateInventory();
                break;
            case 3:
                admin.seeSellers();
                break;
            case 4:
                admin.seeCustomersHistory();
                break;
            case 5:
                admin.seeStatistics();
                break;
            case 6:
                std::cout << "Logging out...\n";
                return;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
        std::cout << "\nPress any key to continue...";
        std::cin.ignore().get(); // Pause the screen
    } while (choice != 6);
}

void loginUser() {
    clearScreen();
    showLogo("Login");
    int choice;
    std::cout << "Login:\n";
    std::cout << "1. Customer\n";
    std::cout << "2. Seller\n";
    std::cout << "3. Admin\n";
    std::cout << "4. Back\n";
    std::cin >> choice;

    std::string name, surname, password, nickname;
    switch (choice) {
        case 1: {
            std::cout << "Enter name: ";
            std::cin >> name;
            std::cout << "Enter surname: ";
            std::cin >> surname;
            std::cout << "Enter password: ";
            std::cin >> password;
            int customerId = verifyCustomer(name, surname, password);
            if (customerId != -1) {
                Customer customer(name, surname, password, customerId);
                showCustomerMenu(customer);
            } else {
                std::cout << "Invalid credentials.\n";
                std::cout << "Do you want to register? (y/n): ";
                char regChoice;
                std::cin >> regChoice;
                if (regChoice == 'y') {
                    registerUser();
                }
            }
            break;
        }
        case 2: {
            std::cout << "Enter name: ";
            std::cin >> name;
            std::cout << "Enter surname: ";
            std::cin >> surname;
            std::cout << "Enter password: ";
            std::cin >> password;
            Seller seller(name, surname, password, 0);  // Temporary seller object
            if (verifySeller(name, surname, password, seller)) {
                showSellerMenu(seller);
            } else {
                std::cout << "Invalid credentials.\n";
                std::cout << "Do you want to register? (y/n): ";
                char regChoice;
                std::cin >> regChoice;
                if (regChoice == 'y') {
                    registerUser();
                }
            }
            break;
        }
        case 3: {
            std::cout << "Enter nickname: ";
            std::cin >> nickname;
            std::cout << "Enter password: ";
            std::cin >> password;
            Admin admin(nickname, password);  // Initialize an Admin object
            if (verifyAdmin(nickname, password, admin)) {
                showAdminMenu(admin);  // Pass the Admin object to showAdminMenu
            } else {
                std::cout << "Invalid credentials.\n";
            }
            break;
        }
        case 4:
            return;
        default:
            std::cout << "Invalid choice.\n";
            break;
    }
}

void showMainMenu() {
    int choice;
    do {
        clearScreen();
        showLogo("Main Menu");
        std::cout << "Main Menu:\n";
        std::cout << "1. Register\n";
        std::cout << "2. Login\n";
        std::cout << "3. Exit\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                loginUser();
                break;
            case 3:
                std::cout << "Exiting...\n";
                return;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    } while (choice != 3);
}

int main() {
    showMainMenu();
    return 0;
}


for this code create a readme file such that it will help people use this program
