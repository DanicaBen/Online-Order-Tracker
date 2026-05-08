#include "crow.h"
#include <queue>
#include <stack>
#include <unordered_map>
#include <vector>
#include <string>

struct Order {
    std::string id;
    std::string item;
    double price;
    std::string city;
    int status; // 0:pending, 1:processing, 2:shipped, 3:delivered, 4:cancelled
};

// Global Data Structures
std::unordered_map<std::string, Order> orderMap;
std::queue<std::string> orderQueue;
std::stack<std::pair<std::string, int>> undoStack; // id and previous status
int nextId = 1;

int main() {
    crow::SimpleApp app;

    // API: Place a New Order (Enqueue & Map Insert)
    CROW_ROUTE(app, "/api/order").methods("POST"_method)([](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);

        std::string id = "ORD-" + std::to_string(nextId++);
        Order newOrder{id, x["item"].s(), x["price"].d(), x["city"].s(), 0};
        
        // Actual C++ Data Structure Operations
        orderMap[id] = newOrder;
        orderQueue.push(id);
        undoStack.push({id, -1}); // -1 means "new order"

        crow::json::wvalue res;
        res["id"] = id;
        return crow::response(res);
    });

    // API: Process Next Order (Dequeue)
    CROW_ROUTE(app, "/api/process").methods("POST"_method)([]() {
        if (orderQueue.empty()) return crow::response(400, "Queue Empty");

        std::string id = orderQueue.front();
        orderQueue.pop();
        
        undoStack.push({id, orderMap[id].status});
        orderMap[id].status = 1; // Move to processing

        return crow::response(200, "Processed " + id);
    });

    // API: Get All Data (For UI Sync)
    CROW_ROUTE(app, "/api/data")([]() {
        crow::json::wvalue res;
        int i = 0;
        for (auto const& [id, order] : orderMap) {
            res["orders"][i]["id"] = order.id;
            res["orders"][i]["item"] = order.item;
            res["orders"][i]["price"] = order.price;
            res["orders"][i]["status"] = order.status;
            res["orders"][i]["city"] = order.city;
            i++;
        }
        return res;
    });

    app.port(18080).multithreaded().run();
}
