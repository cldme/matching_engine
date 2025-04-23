# Matching Engine

A simple matching engine implemented in C++.

## 🚀 Building the Project

To build and run the application:

```bash
# Clone the repository
git clone https://github.com/cldme/matching_engine.git
cd matching_engine

# Create a build directory
mkdir build
cd build

# Run CMake and build the project
cmake ..
make

# Navigate to the output binary and run the app
cd src
./app
```

## 📘 App Usage
```bash
cldme@MSI:~/projects/matching_engine/build/src$ ./app
Welcome to the order book CLI. Type -h or --help for help or 'quit' to exit.
>> --help
Order Book CLI

Usage: [OPTIONS] [SUBCOMMAND]

OPTIONS:
  -h,     --help              Print this help message and exit

SUBCOMMANDS:
  add_order                   Add a new order
  modify_order                Modify an order
  delete_order                Delete an order

>> add_order buy 10 100
[UPDATE] Added order=(id=0 side=BID volume=10 price=100 levelIndex=0 isActive=1)
>> add_order sell 5 100
[UPDATE] Added order=(id=1 side=ASK volume=5 price=100 levelIndex=0 isActive=1)
[TRADE] volume=5 bid order=(id=0 side=BID volume=10 price=100 levelIndex=0 isActive=1) ask order=(id=1 side=ASK volume=5 price=100 levelIndex=0 isActive=1)
>> add_order sell 5 100
[UPDATE] Added order=(id=2 side=ASK volume=5 price=100 levelIndex=0 isActive=1)
[TRADE] volume=5 bid order=(id=0 side=BID volume=5 price=100 levelIndex=0 isActive=1) ask order=(id=2 side=ASK volume=5 price=100 levelIndex=0 isActive=1)
>> add_order sell 5 100
[UPDATE] Added order=(id=3 side=ASK volume=5 price=100 levelIndex=0 isActive=1)
>> delete_order 3
[UPDATE] Deleted order=(id=3 side=ASK volume=5 price=100 levelIndex=0 isActive=0)
>> add_order buy 10 100
[UPDATE] Added order=(id=4 side=BID volume=10 price=100 levelIndex=0 isActive=1)
>> modify_order 4 5 50
[UPDATE] Deleted order=(id=4 side=BID volume=10 price=100 levelIndex=0 isActive=0)
[UPDATE] Added order=(id=5 side=BID volume=5 price=50 levelIndex=0 isActive=1)
>> add_order sell 5 50
[UPDATE] Added order=(id=6 side=ASK volume=5 price=50 levelIndex=0 isActive=1)
[TRADE] volume=5 bid order=(id=5 side=BID volume=5 price=50 levelIndex=0 isActive=1) ask order=(id=6 side=ASK volume=5 price=50 levelIndex=0 isActive=1)
>>
```
