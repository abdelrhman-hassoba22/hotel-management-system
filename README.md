# Hotel Management System - C++ & MySQL

A comprehensive hotel management solution featuring role-based access control (Admin, Staff, Guest), room/reservation management, and billing capabilities. Built with C++17 and MySQL for efficient data handling.

**Key Features**:
- 🔐 Secure authentication system
- 👔 Admin dashboard with employee management
- 🛏️ Room inventory and status tracking
- 📅 Reservation system with check-in/out

## Requirements

### Core Dependencies
- MySQL image [runing on docker] 
- **MySQL Connector/C++** (`libmysqlclient-dev`)
- **C++17** or newer
- **Linux environment** (for terminal functions)

### How to download docker and runing MYSQL image
1. Update your system:
```bash
sudo apt update
```
2. Download Docker's official installation script:
```bash
curl -fsSL https://get.docker.com -o get-docker.sh
```
3. Run the script to install Docker:
```bash
sudo sh get-docker.sh
```
4. Verify that Docker was installed successfully:
```bash
docker --version
```
5. Start the Docker service (if it’s not already running):
```bash
sudo systemctl start docker
```
6. Enable Docker to start at boot:
```bash
sudo systemctl enable docker
```
7. (Optional) Run Docker commands without using sudo: Add your current user to the docker group:
```bash
sudo usermod -aG docker $USER
```
8. Runing MYSQL container:
```bash
docker run --name mysql-container -e MYSQL_ROOT_PASSWORD=mypwd -e MYSQL_DATABASE=hoteldb \
-p 3306:3306 -d mysql:latest 
```

### how to install g++ (the GNU C++ compiler)
1. Update your package list:
```bash
sudo apt update
```
2. Install g++:
```bash
sudo apt install g++
```
3. Verify the installation:
```bash
g++ --version
```

### How to download MySQL Connector/C++
1. Update your system:
```bash
sudo apt update
```
2. Install the package that includes mysql.h:
```bash
sudo apt install libmysqlclient-dev
```

### Steps to Run  the project
1. Ensure the MySQL container is running
```bash
docker exec -it mysql-container mysql -u root -pmypwd -e "show databases"
```
2. Compile and run the create-entities.cpp file
```bash
g++ create-entities.cpp -o create-entities -lmysqlclient
```
3. Compile and run the main application
```bash
g++ hotel-management.cpp -o hotel -lmysqlclient
```

### Critical Issues in the Code (Must Fix Before Production)
1. 
SQL Injection Vulnerabilities
User inputs are directly concatenated into SQL queries, allowing attackers to execute malicious SQL commands
2. 
Plain Text Password Storage
Passwords are stored without hashing (violates GDPR and basic security standards)
3. 
Hardcoded Database Credentials
Production credentials exposed in source code (security risk)
4. 
No Input Validation
User inputs aren't sanitized or validated (risk of invalid data/crashes)
5. 
Insecure Password Handling
Passwords remain in memory as plaintext after use
6. 
Missing RBAC Enforcement
Staff can create admin accounts (privilege escalation risk)
7. 
Transaction Management Issues
Multi-step operations lack transaction safety (data inconsistency risk)
8. 
Date Validation Missing
No verification for date formats/logical ranges (e.g., check-out before check-in)
9. 
Error Over-Exposure
Raw database errors shown to users (information disclosure risk)
10. 
Linux-Only Dependencies
Uses Linux-specific headers (termios.h, unistd.h) - won't compile on Windows
11. 
No Password Policies
Allows weak passwords (e.g., "12345", "password")
12. 
Session Management Issues
No session timeout or activity tracking
 
Functional Limitations
13. 
No Audit Logging
No record of who performed critical actions
14. 
Lack of Pagination
Will crash when displaying large datasets
15. 
Missing Data Export
Cannot generate reports/backups
16. 
Flawed Room Availability Logic
Doesn't properly check date conflicts for reservations
 
