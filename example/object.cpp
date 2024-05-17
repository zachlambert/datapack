#include <datapack/object.hpp>
#include <stack>
#include <iostream>


int main() {
    using namespace datapack;

    Object object;
    object.set_root(ObjectMap());

    auto root = object.root();
    root.insert("a", 1.0);
    root.insert("b", 2.0);

    auto c = root.insert("c", ObjectList());
    c.append("hello");
    c.append("world");

    root.insert("d", true);
    auto e = root.insert("e", ObjectMap());
    e.insert("foo", "foo");
    e.insert("bar", "bar");
    e.insert("and", std::nullopt);

    {
        std::stack<ObjectConstPointer> nodes;
        nodes.push(ObjectConstPointer(object.root()));

        int depth = 0;
        while (!nodes.empty()) {
            auto node = nodes.top();
            nodes.pop();
            if (!node) {
                depth--;
                continue;
            }
            nodes.push(node.next());

            for (int i = 0; i < depth; i++) {
                std::cout << "    ";
            }

            if (!node.key().empty()) {
                std::cout << node.key() << ": ";
            }

            if (node.get_if<ObjectMap>()) {
                std::cout << "object:\n";
                nodes.push(node.child());
                depth++;
                continue;
            }
            else if (node.get_if<ObjectList>()) {
                std::cout << "list:\n";
                depth++;
                nodes.push(node.child());
                continue;
            }

            if (auto value = node.get_if<double>()) {
                std::cout << *value << "\n";
            }
            else if (auto value = node.get_if<std::string>()) {
                std::cout << *value << "\n";
            }
            else if (auto value = node.get_if<bool>()) {
                std::cout << (*value ? "true" : "false") << "\n";
            }
            else if (node.get_if<std::nullopt_t>()) {
                std::cout << "null\n";
            }
        }
    }

    return 0;
}
