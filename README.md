# Secure File Transfer Using AES & Sockets

A simple **client-server** application in C that allows secure file transfer using **AES-256-CBC encryption** over **TCP sockets**.

## Features
- **TCP Socket Communication**: Transfers files securely between client and server.
- **AES-256 Encryption**: Uses **AES-256-CBC** to encrypt files before transmission.
- **Initialization Vector (IV) Security**: Generates a **random IV** for each transmission to ensure encryption security.
- **Cross-Platform Compatibility**: Works on **Linux** (tested on Fedora).

---

## **Requirements**
Make sure you have the following installed:
- **GCC Compiler**
- **OpenSSL (for encryption)**

### **Linux (Fedora/Debian) Installation**
```sh
# Fedora
sudo dnf install openssl-devel

# Ubuntu/Debian
sudo apt install libssl-dev
