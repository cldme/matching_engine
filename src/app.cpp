#include "order_book.h"
#include <CLI/CLI.hpp>
#include <replxx.hxx>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iostream>

int main()
{
    OrderBook mOrderBook;

    // list of supported commands
    std::vector<std::string> commands =
    {
        "add_order", "modify_order", "delete_order", "help", "exit", "quit"
    };

    // setup replxx
    replxx::Replxx repl;
    std::string prompt = ">> ";
    repl.history_load("app_history");
    repl.set_completion_callback([&commands](std::string const& input, int& contextLen) -> std::vector<replxx::Replxx::Completion>
    {
        std::vector<replxx::Replxx::Completion> results;
        for (const auto& cmd : commands)
        {
            if (cmd.find(input) == 0)
            {
                results.emplace_back(cmd);
            }
        }
        contextLen = input.size();
        return results;
    });

    repl.set_hint_callback([&commands](std::string const& input, int& contextLen, replxx::Replxx::Color& color) -> replxx::Replxx::hints_t
    {
        replxx::Replxx::hints_t hints;
        for (const auto& cmd : commands) {
            if (cmd.find(input) == 0) {
                hints.emplace_back(cmd);
            }
        }
        if (!hints.empty()) {
            color = replxx::Replxx::Color::GRAY;
            contextLen = input.size();
        }
        return hints;
    });

    repl.set_highlighter_callback([&commands](std::string const& input, replxx::Replxx::colors_t& colors)
    {
        std::istringstream iss(input);
        std::string word;
        size_t pos = 0;
    
        while (iss >> word) {
            size_t found = input.find(word, pos);
            for (const auto& cmd : commands) {
                if (word == cmd) {
                    for (size_t i = 0; i < word.size(); ++i) {
                        if (found + i < colors.size()) {
                            colors[found + i] = replxx::Replxx::Color::GREEN;
                        }
                    }
                }
            }
            pos = found + word.length();
        }
    });

    // setup CLI11
    std::string line;
    CLI::App cli{"Order Book CLI"};

    // setup command for add order
    std::string side;
    Volume volume;
    Price price;
    auto addCmd = cli.add_subcommand("add_order", "Add a new order");
    addCmd->add_option("side", side, "order side can be 'buy' or 'sell'")->required();
    addCmd->add_option("volume", volume, "order volume")->required();
    addCmd->add_option("price", price, "order price")->required();
    addCmd->callback([&]()
    {
        if (side != "buy" && side != "sell")
        {
            std::cerr << "Invalid side: must be 'buy' or 'sell'\n";
            return;
        }
        Side s = (side == "buy") ? Side::Bid : Side::Ask;
        mOrderBook.AddOrder(s, price, volume);
    });

    // setup command for modify order
    Id modId;
    Volume newVolume;
    Price newPrice;
    auto modCmd = cli.add_subcommand("modify_order", "Modify an order");
    modCmd->add_option("id", modId, "order id")->required();
    modCmd->add_option("volume", newVolume, "order new volume")->required();
    modCmd->add_option("price", newPrice, "order new price")->required();
    modCmd->callback([&]()
    {
        mOrderBook.ModifyOrder(modId, newPrice, newVolume);
    });

    // setup command for delete order
    Id delId;
    auto delCmd = cli.add_subcommand("delete_order", "Delete an order");
    delCmd->add_option("id", delId, "order id")->required();
    delCmd->callback([&]()
    {
        mOrderBook.DeleteOrder(delId);
    });

    std::cout << "Welcome to the order book CLI. Type -h or --help for help or 'quit' to exit.\n";
    while (true)
    {
        const char* input = repl.input(prompt.c_str());
        if (input == nullptr)
            break;

        std::string line(input);
        if (line == "exit" || line == "quit")
            break;
        if (line.empty())
            continue;

        // history support
        repl.history_add(line);

        // tokenize input like argv
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
            tokens.push_back(token);

        // help support
        if (tokens.size() == 1 && (tokens[0] == "-h" || tokens[0] == "--help"))
        {
            std::cout << cli.help() << "\n";
            continue;
        }

        std::vector<char*> argv;
        argv.push_back((char*)"app");
        for (auto& s : tokens)
            argv.push_back(&s[0]);

        try
        {
            cli.clear();  // reset CLI11
            cli.parse(static_cast<int>(argv.size()), argv.data());
        }
        catch (const CLI::ParseError& e)
        {
            cli.exit(e);
        }
    }

    repl.history_save("app_history");
    return 0;
}