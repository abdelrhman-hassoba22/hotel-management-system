#include <iostream>
#include <mysql/mysql.h>
#include <vector>
#include <string>
#include <ctime>

using namespace std;

// دالة لتنفيذ استعلامات SQL مع معالجة الأخطاء
void execute_query(MYSQL* conn, const string& query) {
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        cerr << "Failed query: " << query << endl;
        mysql_close(conn);
        exit(1);
    }
}

int main() {
    MYSQL* conn;
    
    // Initialize connection
    conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "mysql_init() failed" << endl;
        return 1;
    }

    // Connect to database
    if (mysql_real_connect(conn, "127.0.0.1", "root", "mypwd", 
                           "hoteldb", 3306, NULL, 0) == NULL) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return 1;
    }

    // Table creation queries
    vector<string> create_table_queries = {
        // Room types table
        "CREATE TABLE IF NOT EXISTS room_types ("
        "type_id INT AUTO_INCREMENT PRIMARY KEY, "
        "type_name VARCHAR(50) NOT NULL UNIQUE, "  // Added UNIQUE constraint
        "description TEXT, "
        "base_price DECIMAL(10,2) NOT NULL, "
        "capacity INT NOT NULL)",

        // Rooms table
        "CREATE TABLE IF NOT EXISTS rooms ("
        "room_id INT AUTO_INCREMENT PRIMARY KEY, "
        "room_number VARCHAR(10) NOT NULL UNIQUE, "
        "type_id INT NOT NULL, "
        "floor INT NOT NULL, "
        "status ENUM('available', 'occupied', 'maintenance') DEFAULT 'available', "
        "FOREIGN KEY (type_id) REFERENCES room_types(type_id) ON DELETE RESTRICT)",

        // Customers table
        "CREATE TABLE IF NOT EXISTS customers ("
        "customer_id INT AUTO_INCREMENT PRIMARY KEY, "
        "first_name VARCHAR(50) NOT NULL, "
        "last_name VARCHAR(50) NOT NULL, "
        "email VARCHAR(100) UNIQUE, "
        "phone VARCHAR(20) NOT NULL, "
        "id_number VARCHAR(50), "
        "address TEXT, "
        "password VARCHAR(255) NOT NULL, "
        "registration_date DATETIME DEFAULT CURRENT_TIMESTAMP)",

        // Reservations table
        "CREATE TABLE IF NOT EXISTS reservations ("
        "reservation_id INT AUTO_INCREMENT PRIMARY KEY, "
        "customer_id INT NOT NULL, "
        "room_id INT NOT NULL, "
        "check_in_date DATE NOT NULL, "
        "check_out_date DATE NOT NULL, "
        "adults INT NOT NULL DEFAULT 1, "
        "children INT NOT NULL DEFAULT 0, "
        "status ENUM('confirmed', 'cancelled', 'checked_in', 'checked_out') DEFAULT 'confirmed', "
        "total_amount DECIMAL(10,2) NOT NULL, "
        "notes TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE CASCADE, "
        "FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE CASCADE)",

        // Employees table
        "CREATE TABLE IF NOT EXISTS employees ("
        "employee_id INT AUTO_INCREMENT PRIMARY KEY, "
        "first_name VARCHAR(50) NOT NULL, "
        "last_name VARCHAR(50) NOT NULL, "
        "position VARCHAR(50) NOT NULL, "
        "email VARCHAR(100) UNIQUE, "
        "phone VARCHAR(20) NOT NULL, "
        "hire_date DATE NOT NULL, "
        "salary DECIMAL(10,2), "
        "password VARCHAR(255) NOT NULL, "
        "status ENUM('active', 'inactive', 'on_leave') DEFAULT 'active')",

        // Services table
        "CREATE TABLE IF NOT EXISTS services ("
        "service_id INT AUTO_INCREMENT PRIMARY KEY, "
        "service_name VARCHAR(100) NOT NULL UNIQUE, "  // Added UNIQUE constraint
        "description TEXT, "
        "price DECIMAL(10,2) NOT NULL, "
        "category ENUM('room_service', 'restaurant', 'laundry', 'transportation') NOT NULL)",

        // Service bills table
        "CREATE TABLE IF NOT EXISTS service_bills ("
        "bill_id INT AUTO_INCREMENT PRIMARY KEY, "
        "reservation_id INT NOT NULL, "
        "service_id INT NOT NULL, "
        "quantity INT NOT NULL DEFAULT 1, "
        "request_time DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "status ENUM('pending', 'delivered', 'cancelled') DEFAULT 'pending', "
        "FOREIGN KEY (reservation_id) REFERENCES reservations(reservation_id) ON DELETE CASCADE, "
        "FOREIGN KEY (service_id) REFERENCES services(service_id) ON DELETE CASCADE)",

        // Payments table
        "CREATE TABLE IF NOT EXISTS payments ("
        "payment_id INT AUTO_INCREMENT PRIMARY KEY, "
        "reservation_id INT NOT NULL, "
        "amount DECIMAL(10,2) NOT NULL, "
        "payment_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "payment_method ENUM('cash', 'credit_card', 'debit_card', 'bank_transfer') NOT NULL, "
        "status ENUM('pending', 'completed', 'refunded') DEFAULT 'completed', "
        "transaction_reference VARCHAR(100), "
        "notes TEXT, "
        "FOREIGN KEY (reservation_id) REFERENCES reservations(reservation_id) ON DELETE CASCADE)"
    };

    // Execute table creation queries
    for (const auto& query : create_table_queries) {
        execute_query(conn, query);
    }

    // Insert default room types
    vector<string> room_type_queries = {
        "INSERT IGNORE INTO room_types (type_name, description, base_price, capacity) VALUES "
        "('Single', 'Single room with private bathroom', 150.00, 1)",
        
        "INSERT IGNORE INTO room_types (type_name, description, base_price, capacity) VALUES "
        "('Double', 'Double room with one double bed', 250.00, 2)",
        
        "INSERT IGNORE INTO room_types (type_name, description, base_price, capacity) VALUES "
        "('Suite', 'Luxury suite with living area', 500.00, 4)"
    };

    for (const auto& query : room_type_queries) {
        execute_query(conn, query);
    }

    // Insert rooms (3 floors × 10 rooms per floor)
    for (int floor = 1; floor <= 3; ++floor) {
        for (int room_num = 1; room_num <= 10; ++room_num) {
            string room_number = to_string(floor) + (room_num < 10 ? "0" : "") + to_string(room_num);
            string type;
            
            if (room_num <= 4) type = "Single";
            else if (room_num <= 8) type = "Double";
            else type = "Suite";

            string query = 
                "INSERT IGNORE INTO rooms (room_number, type_id, floor) "
                "SELECT '" + room_number + "', type_id, " + to_string(floor) + " "
                "FROM room_types WHERE type_name = '" + type + "'";
            
            execute_query(conn, query);
        }
    }

    // Insert default services
    vector<string> service_queries = {
        "INSERT IGNORE INTO services (service_name, description, price, category) VALUES "
        "('Breakfast', 'Continental breakfast', 15.00, 'restaurant')",
        
        "INSERT IGNORE INTO services (service_name, description, price, category) VALUES "
        "('Laundry', 'Express laundry service', 10.00, 'laundry')",
        
        "INSERT IGNORE INTO services (service_name, description, price, category) VALUES "
        "('Airport Transfer', 'Private car transfer', 30.00, 'transportation')"
    };

    for (const auto& query : service_queries) {
        execute_query(conn, query);
    }

    // إضافة عملاء افتراضيين لحل مشكلة المفتاح الخارجي
    vector<string> customer_queries = {
        "INSERT IGNORE INTO customers (first_name, last_name, email, phone, password) VALUES "
        "('Ahmed', 'Mohamed', 'ahmed@example.com', '01012345678', 'pass123')",
        
        "INSERT IGNORE INTO customers (first_name, last_name, email, phone, password) VALUES "
        "('Fatima', 'Ali', 'fatima@example.com', '01123456789', 'pass456')",
        
        "INSERT IGNORE INTO customers (first_name, last_name, email, phone, password) VALUES "
        "('Omar', 'Khalid', 'omar@example.com', '01234567890', 'pass789')"
    };

    for (const auto& query : customer_queries) {
        execute_query(conn, query);
    }

    // إضافة حجوزات تجريبية مرتبطة بالعملاء الافتراضيين
    vector<string> reservation_queries = {
        "INSERT IGNORE INTO reservations (customer_id, room_id, check_in_date, check_out_date, total_amount) "
        "SELECT c.customer_id, r.room_id, '2023-12-01', '2023-12-05', 750.00 "
        "FROM customers c, rooms r "
        "WHERE c.email = 'ahmed@example.com' AND r.room_number = '101'",
        
        "INSERT IGNORE INTO reservations (customer_id, room_id, check_in_date, check_out_date, total_amount) "
        "SELECT c.customer_id, r.room_id, '2023-12-10', '2023-12-15', 1250.00 "
        "FROM customers c, rooms r "
        "WHERE c.email = 'fatima@example.com' AND r.room_number = '205'",
        
        "INSERT IGNORE INTO reservations (customer_id, room_id, check_in_date, check_out_date, total_amount) "
        "SELECT c.customer_id, r.room_id, '2023-12-20', '2023-12-25', 3000.00 "
        "FROM customers c, rooms r "
        "WHERE c.email = 'omar@example.com' AND r.room_number = '310'"
    };

    for (const auto& query : reservation_queries) {
        execute_query(conn, query);
    }

    cout << "Database tables, default data, and sample reservations created successfully!" << endl;

    // Release resources
    mysql_close(conn);
    return 0;
}
