#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;

class Product {
public:
    int id;
    string name;
    double price;
    int stock;
    int salesCount;

    Product(int id, string name, double price, int stock, int salesCount = 0)
        : id(id), name(name), price(price), stock(stock), salesCount(salesCount) {}

    bool operator<(const Product& other) const {
        return id < other.id;
    }
};

class CartItem {
public:
    Product* product;
    int quantity;

    CartItem(Product* p, int q) : product(p), quantity(q) {}
};

class Customer {
public:
    int id;
    string name;
    vector<CartItem> cart;

    Customer(int id, string name) : id(id), name(name) {}
};

vector<Product> products;
vector<Customer> customers;
vector<vector<Customer*>> cashier_queues(3);
double totalSales = 0;
double totalDiscounts = 0;

void saveProductsToFile() {
    ofstream file("products.txt");
    if (!file.is_open()) {
        cout << "Error opening file for saving products!" << endl;
        return;
    }

    for (const auto& product : products) {
        file << product.id << ","
             << product.name << ","
             << fixed << setprecision(2) << product.price << ","
             << product.stock << ","
             << product.salesCount << endl;
    }

    file.close();
}

void saveProductsTableForViewing() {
    ofstream out("products_table.txt");
    if (!out.is_open()) {
        cout << "Error creating viewable product table!" << endl;
        return;
    }

    out << "+----+----------------+---------+--------+------------+\n";
    out << "| ID | Name           | Price   | Stock  | Sold Count |\n";
    out << "+----+----------------+---------+--------+------------+\n";

    for (auto& p : products) {
        out << "| " << setw(2) << right << p.id << " "
            << "| " << setw(14) << left << p.name
            << "| " << setw(7) << fixed << setprecision(2) << right << p.price << " "
            << "| " << setw(6) << right << p.stock << " "
            << "| " << setw(10) << right << p.salesCount << " |\n";
    }

    out << "+----+----------------+---------+--------+------------+\n";
    out.close();
}


void loadProducts() {
    ifstream in("products.txt");
    if (!in) return;
    string line;

    while (getline(in, line)) {
        stringstream ss(line);
        string temp, name;
        int id, stock, salesCount = 0;
        double price;

        try {
            getline(ss, temp, ',');
            if (temp.empty()) continue;
            id = stoi(temp);

            getline(ss, name, ',');
            if (name.empty()) continue;

            getline(ss, temp, ',');
            if (temp.empty()) continue;
            price = stod(temp);

            getline(ss, temp, ',');
            if (temp.empty()) continue;
            stock = stoi(temp);

            getline(ss, temp);
            if (!temp.empty()) salesCount = stoi(temp);

            products.emplace_back(id, name, price, stock, salesCount);
        } catch (...) {
            cerr << "Invalid product entry skipped: " << line << "\n";
        }
    }

    sort(products.begin(), products.end());
}
void saveCustomersTableForViewing() {
    ofstream out("customers_table.txt");
    if (!out.is_open()) {
        cout << "Error creating customer table file!" << endl;
        return;
    }

    out << "+------------+--------------------------+\n";
    out << "| CustomerID | Name                     |\n";
    out << "+------------+--------------------------+\n";

    for (const auto& c : customers) {
        out << "| " << setw(10) << right << c.id << " "
            << "| " << setw(24) << left << c.name << " |\n";
    }

    out << "+------------+--------------------------+\n";
    out.close();
    //cout << "Customer table saved to 'customers_table.txt'\n";
}


void loadCustomers() {
    ifstream in("customers.txt");
    if (!in) return;
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        string temp, name;
        int id;
        getline(ss, temp, ',');
        id = stoi(temp);
        getline(ss, name);
        customers.emplace_back(id, name);
        int index = customers.size() - 1;
        cashier_queues[index % 3].push_back(&customers.back());
    }
}

Product* binarySearchProduct(int pid, int left, int right) {
    if (left > right) return nullptr;
    int mid = (left + right) / 2;
    if (products[mid].id == pid) return &products[mid];
    else if (pid < products[mid].id) return binarySearchProduct(pid, left, mid - 1);
    else return binarySearchProduct(pid, mid + 1, right);
}

Customer* findCustomer(int id) {
    for (auto& c : customers)
        if (c.id == id) return &c;
    return nullptr;
}

void showProductCart() {
    cout << "\nAvailable Products:\n";
    cout << "-------------------------------------------------------------\n";
    cout << setw(10) << "ID" << setw(20) << "Name"
         << setw(10) << "Price" << setw(10) << "Stock" << setw(10) << "Sold\n";
    cout << "-------------------------------------------------------------\n";
    for (auto& p : products) {

        cout << setw(10) << p.id << setw(20) << p.name
             << setw(10) << p.price << setw(10) << p.stock << setw(10) << p.salesCount << "\n";
        cout << "-------------------------------------------------------------\n";
    }

}

void addProduct() {
    int id, stock;
    string name;
    double price;
    cout << "Enter Product ID: "; cin >> id;

    if (binarySearchProduct(id, 0, products.size() - 1)) {
        cout << "Product ID already exists!\n";
        return;
    }

    cin.ignore();
    cout << "Enter Product Name: "; getline(cin, name);
    cout << "Enter Price: "; cin >> price;
    cout << "Enter Stock: "; cin >> stock;

    products.emplace_back(id, name, price, stock);
    sort(products.begin(), products.end());
    saveProductsToFile();
    cout << "Product added successfully.\n";
}

void editProduct() {
    int id;
    cout << "Enter Product ID to edit: ";
    cin >> id;
    Product* p = binarySearchProduct(id, 0, products.size() - 1);
    if (!p) {
        cout << "Product not found.\n";
        return;
    }

    cin.ignore();
    string newName;
    cout << "Enter new name (or '.' to keep): ";
    getline(cin, newName);
    if (newName != ".") p->name = newName;

    double price;
    cout << "Enter new price (-1 to keep): "; cin >> price;
    if (price != -1) p->price = price;

    int stock;
    cout << "Enter new stock (-1 to keep): "; cin >> stock;
    if (stock != -1) p->stock = stock;

    saveProductsToFile();
    cout << "Product updated.\n";
}

void deleteProduct() {
    int id;
    cout << "Enter Product ID to delete: ";
    cin >> id;
    auto it = remove_if(products.begin(), products.end(),
                        [id](Product& p) { return p.id == id; });
    if (it != products.end()) {
        products.erase(it, products.end());
        saveProductsToFile();
        cout << "Product deleted.\n";
    } else {
        cout << "Product not found.\n";
    }
}
bool customerExists(int id) {
    for (auto& c : customers) {
        if (c.id == id) return true;
    }
    return false;
}

void registerCustomer() {
    int id;
    string name;
    cout << "Enter Customer ID: "; cin >> id;
    cin.ignore();
    cout << "Enter Name: "; getline(cin, name);

    if (customerExists(id)) {
        cout << " Customer with ID " << id << " already exists.\n";
        return;
    }

    customers.emplace_back(id, name);

    // Save to file (overwrite whole file, not append)
    ofstream out("customers.txt");
    for (const auto& c : customers) {
        out << c.id << "," << c.name << "\n";
    }

    // Save for table viewing
    saveCustomersTableForViewing();

    cashier_queues[customers.size() % 3].push_back(&customers.back());
    cout << "Customer registered.\n";
}

void addToCart(Customer& cust) {
    while (true) {
        int pid;
        cout << "Enter Product ID to add (0 to stop): ";
        cin >> pid;
        if (pid == 0) break;

        Product* p = binarySearchProduct(pid, 0, products.size() - 1);
        if (!p) {
            cout << "Product not found.\n";
            continue;
        }

        int qty;
        cout << "Enter Quantity (Stock: " << p->stock << "): ";
        cin >> qty;
        if (qty > p->stock) {
            cout << "Insufficient stock.\n";
            continue;
        }

        p->stock -= qty;
        cust.cart.emplace_back(p, qty);
        cout << "Added to cart.\n";
    }
    saveProductsToFile();
}

void viewInvoice(int id) {
    string filename = "invoice_" + to_string(id) + ".txt";
    ifstream in(filename);
    if (!in) {
        cout << "Invoice not found.\n";
        return;
    }
    string line;
    while (getline(in, line)) cout << line << "\n";

}
string getCurrentTimestamp() {
    time_t now = time(0);                 // current time
    tm* local = localtime(&now);          // convert to local time

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", local); // format time
    return string(buffer);                // return as std::string
}


void saveSalesReportToFile() {
    string timestamp = getCurrentTimestamp();
    string filename = "sales_report_" + timestamp + ".txt";

    ofstream out(filename);
    if (!out.is_open()) {
        cout << "Error creating sales report file!" << endl;
        return;
    }

    out << "====================== Sales Report ======================\n";
    out << "Total Customers Served : " << customers.size() << "\n";
    out << "Total Sales            : " << fixed << setprecision(2) << totalSales << " Taka\n";
    out << "Total Discounts Given  : " << fixed << setprecision(2) << totalDiscounts << " Taka\n";
    out << "==========================================================\n";

    out.close();
    cout << "Sales report saved to '" << filename << "'\n";
}




void salesReport() {
    cout << "\n--- Sales Report ---\n";
    cout << "Total Customers: " << customers.size() << "\n";
    cout << "Total Sales: " << totalSales << " Taka\n";
    cout << "Total Discounts: " << totalDiscounts << " Taka\n";
     saveSalesReportToFile();
}

// DP-based discount calculation from multiple slabs
double calculateBestDiscount(double total) {
    // Discount slabs as pairs: min amount, discount percent
    vector<pair<double, double>> slabs = {
        {3000, 0.20},
        {2000, 0.15},
        {1000, 0.10}
    };

    double bestDiscount = 0;

    for (auto& slab : slabs) {
        if (total >= slab.first) {
            double disc = total * slab.second;
            if (disc > bestDiscount) bestDiscount = disc;
        }
    }
    return bestDiscount;
}

// Greedy restocking by score = (salesCount + 1) / (stock + 1)
void restockProductsGreedy() {
    double budget;
    cout << "Enter total budget for smart restocking: ";
    cin >> budget;

    // Create vector of products with score for sorting
    vector<pair<Product*, double>> restockList;
    for (auto& p : products) {
        double score = (p.salesCount + 1.0) / (p.stock + 1.0);
        restockList.emplace_back(&p, score);
    }

    // Sort descending by score (greedy priority)
    sort(restockList.begin(), restockList.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });

    for (auto& entry : restockList) {
        Product* p = entry.first;
        if (budget <= 0) break;

        // Restock max units possible within budget, max 10 units at once
        int maxUnits = min(10, (int)(budget / p->price));
        if (maxUnits > 0) {
            p->stock += maxUnits;
            budget -= maxUnits * p->price;
            cout << "Restocked " << maxUnits << " of " << p->name << " (Score: " << fixed << setprecision(2) << entry.second << ")\n";
        }
    }

    saveProductsToFile();
    cout << "Remaining Budget: " << budget << "\n";
}

void checkout(Customer& cust) {
    double total = 0, discount = 0;
    string filename = "invoice_" + to_string(cust.id) + ".txt";
    ofstream out(filename);
    stringstream invoiceStream;
    invoiceStream << fixed << setprecision(2);
    invoiceStream << "\n========================= Invoice ===========================\n";
    invoiceStream << "Customer: " << cust.name << "\n";
    invoiceStream << "--------------------------------------------------------------\n";
    invoiceStream << left << setw(20) << "Product"
                  << right << setw(10) << "Qty"
                  << setw(10) << "Price"
                  << setw(10) << "Total\n";
    invoiceStream <<"--------------------------------------------------------------\n";

    for (auto& item : cust.cart) {
        double sub = item.product->price * item.quantity;
        total += sub;
        item.product->salesCount += item.quantity;

        invoiceStream << left << setw(20) << item.product->name
                      << right << setw(10) << item.quantity
                      << setw(10) << item.product->price
                      << setw(10) << sub << "\n";
    }

    if (total >= 1000) {
        discount = total * 0.10;
        invoiceStream << "\nOffer Applied: 10% Discount\n";
    }

    totalSales += total - discount;
    totalDiscounts += discount;

    invoiceStream << "--------------------------------------------------------------\n";
    invoiceStream << left << setw(30) << "Total" << right << setw(10) << total << " Taka\n";
    invoiceStream << left << setw(30) << "Discount" << right << setw(10) << discount << " Taka\n";
    invoiceStream << left << setw(30) << "Amount Payable" << right << setw(10) << (total - discount) << " Taka\n";
    invoiceStream << "==============================================================\n";

    out << invoiceStream.str();
    out.close();
    cout << invoiceStream.str();

    cust.cart.clear();
    saveProductsToFile();
    cout << "Checkout complete. Invoice saved to " << filename << "\n";
}


bool adminLogin() {
    string u, p;
    cout << "Admin Login\nUsername: "; cin >> u;
    cout << "Password: "; cin >> p;
    return u == "admin" && p == "1234";
}

void menu() {
    if (!adminLogin()) {
        cout << "Login failed.\n";
        return;
    }

    int ch;
    while (true) {
        cout << "\n========== Supershop Menu ==========\n";
        cout << "1. Add Product\n";
        cout << "2. Register Customer\n";
        cout << "3. Add to Cart\n";
        cout << "4. Checkout\n";
        cout << "5. View Products\n";
        cout << "6. View Invoice\n";
        cout << "7. Sales Report\n";
        cout << "8. Edit Product\n";
        cout << "9. Delete Product\n";
        cout << "10. Restock Products\n";
        cout << "11. Exit\n";
        cout << "====================================\n";
        cout << "Enter your choice: ";
        cin >> ch;

        switch (ch) {
             case 1: {
                char more;
                do {
                    addProduct();
                    cout << "Add another product? (y/n): ";
                    cin >> more;
                } while (tolower(more) == 'y');
                break;
            }
            case 2: {
                char more;
                do {
                    registerCustomer();
                    cout << "Register another customer? (y/n): ";
                    cin >> more;
                } while (tolower(more) == 'y');
                break;
            }
            case 3: {
                int id;
                cout << "Enter Customer ID: "; cin >> id;
                Customer* c = findCustomer(id);
                if (c) addToCart(*c);
                else cout << "Customer not found.\n";
                break;
            }
            case 4: {
                int id;
                cout << "Enter Customer ID: "; cin >> id;
                Customer* c = findCustomer(id);
                if (c) checkout(*c);
                else cout << "Customer not found.\n";
                break;
            }
            case 5: showProductCart(); break;
            case 6: {
                int id;
                cout << "Enter Customer ID: "; cin >> id;
                viewInvoice(id);
                break;
            }
            case 7: salesReport(); break;
            case 8: editProduct(); break;
            case 9: deleteProduct(); break;
            case 10: restockProductsGreedy(); break;
            case 11:
                cout << "Exiting Supershop System. Goodbye!\n";
                return;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    }
}

int main() {
    loadProducts();
    loadCustomers();
     saveProductsTableForViewing();
     saveCustomersTableForViewing();
    menu();
    return 0;
}

