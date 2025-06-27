#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <vector>
#include <limits>
#include <termios.h>
#include <unistd.h>

using namespace std;

// function to hide password
int getch() {
    int ch;
    struct termios t_old, t_new;
    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

string getPassword() {
    string password;
    char ch;

    while ((ch = getch()) != '\n' ) {
        if (ch == 127) {
            if (!password.empty()) {
                cout << "\b \b";
                password.pop_back();
            }
        }else {
            cout << "*";
            password += ch;
        }
    }
    cout << endl;
    return password;
}

// function to clear screen
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}
// function wait for enter
void waitForEnter() {
    cout << "\nPress  Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    clearScreen();
}
// connect to database
MYSQL* connectToDatabase() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "mysql_init() failed" << endl;
        return NULL;
    }

    if (mysql_real_connect(conn, "127.0.0.1", "root", "mypwd", 
                           "hoteldb", 3306, NULL, 0) == NULL) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return NULL;
    }
    return conn;
}

// create new account
void createAccount(MYSQL *conn) {
    clearScreen();
    cout << "=== create a new account ===" << endl;
    
    string firstName, lastName, email, phone, password, position;
    
    cout << "first name: ";
    cin.ignore(); // تنظيف الـ buffer
    getline(cin, firstName);
    
    cout << "last name: ";
    getline(cin, lastName);
    
    cout << "email: ";
    getline(cin, email);
    
    cout << "phone:";
    getline(cin, phone);

    cout << "password: ";
    password = getPassword(); // استدعاء الدالة الخاصة بإدخال كلمة المرور
    
    cout << "position (manager/employee/customer): ";
    cin >> position;
    
    string query;
    if (position == "manager" || position == "employee") {
        query = "INSERT INTO employees (first_name, last_name, email, phone, password, position, hire_date, status) "
                "VALUES ('" + firstName + "', '" + lastName + "', '" + email + "', '" + phone + "', '" + password + "', '" + 
                position + "', CURDATE(), 'active')";
    } else {
        query = "INSERT INTO customers (first_name, last_name, email, phone, password, registration_date) "
                "VALUES ('" + firstName + "', '" + lastName + "', '" + email + "', '" + phone + "', '" + password + "', NOW())";
    }
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error creating account: " << mysql_error(conn) << endl;
        waitForEnter();
        return;
    }
    
    cout << "\naccount created successfully!" << endl;
    waitForEnter();
}
// login to account
int login(MYSQL *conn, string &userType, int &userId) {
    clearScreen();
    cout << "=== login ===" << endl;
    
    string email, password;
    cout << "email: ";
    cin >> email;
    cin.ignore();
    cout << "password: ";
    password = getPassword();
    cout << "account Type (manager/employee/customer): ";
    cin >> userType;
    
    string query;
    if (userType == "manager" || userType == "employee") {
        query = "SELECT employee_id, position FROM employees WHERE email = '" + email + 
                "' AND password = '" + password + "'";
    } else {
        query = "SELECT customer_id FROM customers WHERE email = '" + email + 
                "' AND password = '" + password + "'";
    }
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Error in login query: " << mysql_error(conn) << endl;
        waitForEnter();
        return 0;
    }
    
    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        cerr << "Error storing result: " << mysql_error(conn) << endl;
        waitForEnter();
        return 0;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        userId = (userType == "customer") ? stoi(row[0]) : stoi(row[0]);
        if (userType != "customer") userType = row[1];
        
        mysql_free_result(result);
        return 1;
    } else {
        cout << "\nemail or password is incorrect!" << endl;
        waitForEnter();
        mysql_free_result(result);
        return 0;
    }
}
// Display manager menu
void showManagerMenu(MYSQL *conn) {
    int choice;
    do {
        clearScreen();
        cout << "=== Manager Dashboard ===" << endl;
        cout << "1. View all rooms" << endl;
        cout << "2. View current bookings" << endl;
        cout << "3. View employees" << endl;
        cout << "4. View revenue" << endl;
        cout << "5. Add new room" << endl;
        cout << "6. Add new employee" << endl;
        cout << "0. Logout" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch(choice) {
            case 1: {
                clearScreen();
                cout << "=== Rooms List ===" << endl;
                if (mysql_query(conn, "SELECT r.room_number, rt.type_name, r.floor, r.status FROM rooms r JOIN room_types rt ON r.type_id = rt.type_id")) {
                    cerr << "Error querying rooms: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    cout << "Room No. | Room Type | Floor | Status" << endl;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " | " << row[1] << " | " << row[2] << " | " << row[3] << endl;
                    }
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }
            case 2: {
                clearScreen();
                cout << "=== Current Bookings ===" << endl;
                if (mysql_query(conn, "SELECT r.reservation_id, c.first_name, c.last_name, rm.room_number, r.check_in_date, r.check_out_date, r.status FROM reservations r JOIN customers c ON r.customer_id = c.customer_id JOIN rooms rm ON r.room_id = rm.room_id WHERE r.status IN ('confirmed', 'checked_in')")) {
                    cerr << "Error querying reservations: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    cout << "Booking ID | Customer Name | Room No. | Check-in | Check-out | Status" << endl;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " | " << row[1] << " " << row[2] << " | " << row[3] << " | " << row[4] << " | " << row[5] << " | " << row[6] << endl;
                    }
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }
            case 3: {
                clearScreen();
                cout << "=== Employees List ===" << endl;
                if (mysql_query(conn, "SELECT employee_id, first_name, last_name, position, hire_date, status FROM employees")) {
                    cerr << "Error querying employees: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    cout << "ID | Name | Position | Hire Date | Status" << endl;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " | " << row[1] << " " << row[2] << " | " << row[3] << " | " << row[4] << " | " << row[5] << endl;
                    }
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }
            case 4: {
                clearScreen();
                cout << "=== Revenue Report ===" << endl;
                if (mysql_query(conn, "SELECT SUM(total_amount) FROM reservations WHERE status = 'checked_out'")) {
                    cerr << "Error querying revenue: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row = mysql_fetch_row(result);
                    cout << "Total revenue from completed bookings: " << (row[0] ? row[0] : "0") << endl;
                    mysql_free_result(result);
                }
                
                if (mysql_query(conn, "SELECT SUM(amount) FROM payments WHERE status = 'completed'")) {
                    cerr << "Error querying payments: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row = mysql_fetch_row(result);
                    cout << "Total payments received: " << (row[0] ? row[0] : "0") << endl;
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }

            case 5: {
                clearScreen();
                cout << "=== Add New Room ===" << endl;
    
                // عرض معرفات وأسماء أنواع الغرف المتاحة فقط
                cout << "Available Room Types (ID - Name):" << endl;
                if (mysql_query(conn, "SELECT type_id, type_name FROM room_types")) {
                    cerr << "Error querying room types: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " - " << row[1] << endl;
                    }
                    mysql_free_result(result);
                    cout << "--------------------------" << endl;
                }
    
                string roomNumber, floor, typeId;
                cout << "Room number: ";
                cin >> roomNumber;
                cout << "Floor: ";
                cin >> floor;
                cout << "Room type ID: ";
                cin >> typeId;
    
                // التحقق الأساسي من صحة type_id
                string checkQuery = "SELECT 1 FROM room_types WHERE type_id = " + typeId;
                if (mysql_query(conn, checkQuery.c_str())) {
                    cerr << "Error checking room type: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *checkResult = mysql_store_result(conn);
                    if (mysql_num_rows(checkResult) == 0) {
                        cerr << "Invalid room type ID!" << endl;
                    } else {
                        string query = "INSERT INTO rooms (room_number, type_id, floor, status) VALUES ('" + 
                                      roomNumber + "', " + typeId + ", " + floor + ", 'available')";
            
                        if (mysql_query(conn, query.c_str())) {
                            cerr << "Error adding room: " << mysql_error(conn) << endl;
                        } else {
                            cout << "Room added successfully!" << endl;
                        }
                    }
                    mysql_free_result(checkResult);
                }
                waitForEnter();
                break;
            }
            case 6: {
                clearScreen();
                cout << "=== Add New Employee ===" << endl;
                string firstName, lastName, email, phone, password, position, salary;
                cout << "First name: ";
                cin >> firstName;
                cout << "Last name: ";
                cin >> lastName;
                cout << "Email: ";
                cin >> email;
                cout << "phone:";
                cin >> phone;
                cout << "Password: ";
                password = getPassword();
                cout << "Position (manager/employee): ";
                cin >> position;
                cout << "Salary: ";
                cin >> salary;
                
                string query = "INSERT INTO employees (first_name, last_name, email, phone, password, position, hire_date, salary, status) "
                              "VALUES ('" + firstName + "', '" + lastName + "', '" + email + "', '" + phone + "', '" + password + 
                              "', '" + position + "', CURDATE(), " + salary + ", 'active')";
                
                if (mysql_query(conn, query.c_str())) {
                    cerr << "Error adding employee: " << mysql_error(conn) << endl;
                } else {
                    cout << "Employee added successfully!" << endl;
                }
                waitForEnter();
                break;
            }
            case 0:
                break;
            default:
                cout << "Invalid choice!" << endl;
                waitForEnter();
        }
    } while (choice != 0);
}
// Show employee menu
void showEmployeeMenu(MYSQL *conn) {
    int choice;
    do {
        clearScreen();
        cout << "=== Employee Dashboard ===" << endl;
        cout << "1. View Available Rooms" << endl;
        cout << "2. Create New Reservation" << endl;
        cout << "3. View Current Reservations" << endl;
        cout << "4. Check-in Customer" << endl;
        cout << "5. Check-out Customer" << endl;
        cout << "0. Logout" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        
        switch(choice) {
            case 1: {
                clearScreen();
                cout << "=== Available Rooms ===" << endl;
                if (mysql_query(conn, "SELECT r.room_number, rt.type_name, rt.base_price, rt.capacity FROM rooms r JOIN room_types rt ON r.type_id = rt.type_id WHERE r.status = 'available'")) {
                    cerr << "Error querying available rooms: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    cout << "Room No | Type | Price | Capacity" << endl;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " | " << row[1] << " | " << row[2] << " | " << row[3] << endl;
                    }
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }
            case 2: {
                clearScreen();
                cout << "=== Create New Reservation ===" << endl;
                string customerId, roomId, checkInDate, checkOutDate, adults, children, totalAmount;
                cout << "Customer ID: ";
                cin >> customerId;
                cout << "Room ID: ";
                cin >> roomId;
                cout << "Check-in Date (YYYY-MM-DD): ";
                cin >> checkInDate;
                cout << "Check-out Date (YYYY-MM-DD): ";
                cin >> checkOutDate;
                cout << "Adults: ";
                cin >> adults;
                cout << "Children: ";
                cin >> children;
                cout << "Total Amount: ";
                cin >> totalAmount;
                
                string query = "INSERT INTO reservations (customer_id, room_id, check_in_date, check_out_date, adults, children, status, total_amount) "
                              "VALUES (" + customerId + ", " + roomId + ", '" + checkInDate + "', '" + checkOutDate + 
                              "', " + adults + ", " + children + ", 'confirmed', " + totalAmount + ")";
                
                if (mysql_query(conn, query.c_str())) {
                    cerr << "Error creating reservation: " << mysql_error(conn) << endl;
                } else {
                    cout << "Reservation created successfully!" << endl;
                    
                    // Update room status
                    string updateQuery = "UPDATE rooms SET status = 'occupied' WHERE room_id = " + roomId;
                    if (mysql_query(conn, updateQuery.c_str())) {
                        cerr << "Error updating room status: " << mysql_error(conn) << endl;
                    }
                }
                waitForEnter();
                break;
            }
            case 3: {
                clearScreen();
                cout << "=== Current Reservations ===" << endl;
                if (mysql_query(conn, "SELECT r.reservation_id, c.first_name, c.last_name, rm.room_number, r.check_in_date, r.check_out_date, r.status FROM reservations r JOIN customers c ON r.customer_id = c.customer_id JOIN rooms rm ON r.room_id = rm.room_id WHERE r.status IN ('confirmed', 'checked_in')")) {
                    cerr << "Error querying reservations: " << mysql_error(conn) << endl;
                } else {
                    MYSQL_RES *result = mysql_store_result(conn);
                    MYSQL_ROW row;
                    cout << "Res ID | Customer | Room No | Check-in | Check-out | Status" << endl;
                    while ((row = mysql_fetch_row(result))) {
                        cout << row[0] << " | " << row[1] << " " << row[2] << " | " << row[3] << " | " << row[4] << " | " << row[5] << " | " << row[6] << endl;
                    }
                    mysql_free_result(result);
                }
                waitForEnter();
                break;
            }
            case 4: {
                clearScreen();
                cout << "=== Customer Check-in ===" << endl;
                string reservationId;
                cout << "Reservation ID: ";
                cin >> reservationId;
                
                string query = "UPDATE reservations SET status = 'checked_in' WHERE reservation_id = " + reservationId;
                
                if (mysql_query(conn, query.c_str())) {
                    cerr << "Error checking in: " << mysql_error(conn) << endl;
                } else {
                    cout << "Check-in successful!" << endl;
                }
                waitForEnter();
                break;
            }
            case 5: {
                clearScreen();
                cout << "=== Customer Check-out ===" << endl;
                string reservationId;
                cout << "Reservation ID: ";
                cin >> reservationId;
                
                // Get room ID first
                string roomQuery = "SELECT room_id FROM reservations WHERE reservation_id = " + reservationId;
                if (mysql_query(conn, roomQuery.c_str())) {
                    cerr << "Error getting room ID: " << mysql_error(conn) << endl;
                    waitForEnter();
                    break;
                }
                
                MYSQL_RES *result = mysql_store_result(conn);
                if (!result || mysql_num_rows(result) == 0) {
                    cerr << "Reservation not found" << endl;
                    mysql_free_result(result);
                    waitForEnter();
                    break;
                }
                
                MYSQL_ROW row = mysql_fetch_row(result);
                string roomId = row[0];
                mysql_free_result(result);
                
                // Update reservation status
                string updateReservation = "UPDATE reservations SET status = 'checked_out' WHERE reservation_id = " + reservationId;
                if (mysql_query(conn, updateReservation.c_str())) {
                    cerr << "Error checking out: " << mysql_error(conn) << endl;
                    waitForEnter();
                    break;
                }
                
                // Update room status
                string updateRoom = "UPDATE rooms SET status = 'available' WHERE room_id = " + roomId;
                if (mysql_query(conn, updateRoom.c_str())) {
                    cerr << "Error updating room status: " << mysql_error(conn) << endl;
                } else {
                    cout << "Check-out successful!" << endl;
                }
                waitForEnter();
                break;
            }
            case 0:
                break;
            default:
                cout << "Invalid choice!" << endl;
                waitForEnter();
        }
    } while (choice != 0);
}
// Main menu
void mainMenu(MYSQL *conn) {
    int choice;
    do {
        clearScreen();
        cout << "=== Hotel Management System ===" << endl;
        cout << "1. Login" << endl;
        cout << "2. Create New Account" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        
        switch(choice) {
            case 1: {
                string userType;
                int userId;
                if (login(conn, userType, userId)) {
                    if (userType == "manager") {
                        showManagerMenu(conn);
                    } else if (userType == "employee") {
                        showEmployeeMenu(conn);
                    } else {
                        cout << "Welcome, Customer!" << endl;
                        waitForEnter();
                    }
                }
                break;
            }
            case 2:
                createAccount(conn);
                break;
            case 0:
                cout << "Thank you for using the Hotel Management System!" << endl;
                break;
            default:
                cout << "Invalid choice!" << endl;
                waitForEnter();
        }
    } while (choice != 0);
}

// Main function
int main() {
    MYSQL *conn = connectToDatabase();
    if (conn == NULL) {
        return 1;
    }
    
    mainMenu(conn);
    
    mysql_close(conn);
    return 0;
}
