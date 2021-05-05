#include "json_builder.h"

namespace json {
    /////BUILDER CLASS AREA///////////////////////////////////////////////////////// 
    Builder::DictValue Builder::Key(std::string key) {
        using namespace std;
        
        if (nodes_stack_.empty()) { // Key can be added only into initialized Map
            throw std::logic_error("Key() failed. Builder State at EMPTY"s);
        }
        else if (!node_ended_) { // Node is an Array or Map.        
            if (nodes_stack_.back()->IsMap()) {
                nodes_stack_.emplace_back(std::make_unique<Node>(key)); // Add not paired Key in stack. It will wait for Value and after that will be added in Map
            }
            else {
                throw std::logic_error((nodes_stack_.size() > 1) && nodes_stack_.back()->IsString() ? "Key() after Key() (wrong input)"s // stack consist of Map node and Key
                    : "Key() not in a dict node"s); // attempt to add Key into anoter nodevalue (not a Map)
            }           
        }
        else { // node_ended = T        
            throw std::logic_error("Key() failed. Node was already ended"s);
        }
        return DictValue(*this); // return value with aux.class type which allows only to add a Value after Key
    }

    Builder& Builder::Value(NodeValue value) {
        using namespace std;
        
        if (nodes_stack_.empty()) { // Node was not created. Just add value and finish Node creation
            nodes_stack_.emplace_back(std::make_unique<Node>(value));
            node_ended_ = true;
        }
        else if (!node_ended_) { // Node is an Array or Map.
            if (nodes_stack_.back()->IsArray()) { // Array?
                json::Array tmp = nodes_stack_.back()->AsArray();
                tmp.emplace_back(value);
                *nodes_stack_.back() = Node(std::move(tmp));
            }
            else if ((nodes_stack_.size() > 1) && nodes_stack_.back()->IsString()) { // Map? if last element is Key, than create pair Key - Value 
                std::string key = std::move(nodes_stack_.back()->AsString()); // Key
                nodes_stack_.pop_back(); // remove Key from storage
                json::Dict dict = std::move(nodes_stack_.back()->AsMap()); // tmp Map creation
                dict.insert({ key, value }); // add new Key - Value pair
                *nodes_stack_.back() = Node(std::move(dict)); // overwrite extended Map
            }
            else {
                throw std::logic_error("Value() failed. Dict without a key"s); // Not an Array or Dict without not paired Key 
            }
        } else { // node_ended = T              
            throw std::logic_error("Value() failed. Builder State at ENDED value"s);        
        }
        return *this; // Method Chaining
    }
    Builder::DictItem Builder::StartDict() {
        using namespace std;
        if (nodes_stack_.empty()) { // Node was not created
            nodes_stack_.emplace_back(std::make_unique<Node>(Dict())); // Add empty map in stack
        }
        else if (!node_ended_) { // Array or Map added in stack
            if (nodes_stack_.back()->IsMap()) { // Start new Map right after Map initialization. (Key is expected)
                throw std::logic_error("StartDict() failed. New Dict inside of another Dict"s);
            }
            else { // Condition is allow to add Map in node
                nodes_stack_.emplace_back(std::make_unique<Node>(Dict()));
            }
        }
        else { // node_ended = T 
            throw std::logic_error("StartDict() failed. Builder State at ENDED value"s);         
        }
        return DictItem(*this); // return value with aux.class type which allows only to a Key or end Map
    }
    Builder& Builder::EndDict() {
        using namespace std;
        if (nodes_stack_.empty()) { // Cannot end Map because it was not created before
            throw std::logic_error("EndDict() failed. Dict was not initialized before"s);
        }
        else if (!node_ended_) { // Handle Map
            if (nodes_stack_.back()->IsMap()) {
                if (nodes_stack_.size() == 1) { // Map is ready to be ended. All Key-Value pairs were added in Map before
                    node_ended_ = true;
                }
                else { // stack consist of 2 or more value. Sub Map was created in an Array or main Map  
                    Dict value = nodes_stack_.back()->AsMap();
                    nodes_stack_.pop_back();
                    this->Value(value); // it has to be added as a Value in node
                }
            }
            else {
                throw std::logic_error(nodes_stack_.back()->IsString() ? "EndDict() failed. Dict value expected"s // Map has last Key without a Value
                    : "EndDict() failed. Node is not a Dict"s); // Node is not a Map
            }
        }
        else { // node_ended = T 
            throw std::logic_error("EndDict() failed. Dict has already been ended"s);
        }        
        return *this;
    }
    Builder::ArrayItem Builder::StartArray() {
        using namespace std;
        if (nodes_stack_.empty()) { // Node was not created
            nodes_stack_.emplace_back(std::make_unique<Node>(Array()));
        }           
        else if (!node_ended_) {
            if (nodes_stack_.back()->IsMap()) { // Start new Array right after Map initialization. (Key is expected)
                throw std::logic_error("StartArray() failed. Builder has started Dict"s);
            }
            nodes_stack_.emplace_back(std::make_unique<Node>(Array())); // Array can be added as an Value of another Array or Map
        }
        else { // node_ended = T 
            throw std::logic_error("StartArray() failed. Builder State at ENDED value"s);           
        }
        return ArrayItem(*this);;
    }
    Builder& Builder::EndArray() {
        using namespace std;
        if (nodes_stack_.empty()) { // Cannot end an Array because it was not created before
            throw std::logic_error("EndArray() failded. Array was not initialized before"s);
        }
        else if (!node_ended_) {
            if (nodes_stack_.back()->IsArray()) {
                if (nodes_stack_.size() == 1) { // check if Array is ready to be ended
                    node_ended_ = true;
                }
                else { // End of Sub Array and adding it in main container 
                    json::Array value = nodes_stack_.back()->AsArray();
                    nodes_stack_.pop_back();
                    this->Value(value);
                }
            }
            else {
                throw std::logic_error("EndArray() failded. Builder hasn't started Array before"s);
            }
        }
        else { // node_ended = T
            throw std::logic_error("EndArray() failded. Builder State at ENDED value"s);
        }
        return *this;
    }

    Node Builder::Build() {
        using namespace std;
        if (node_ended_) { // return Node if it's ended
            return *nodes_stack_.back();
        }
        else {
            throw std::logic_error("Build() is failed. Node is not ended"s);
        }
    }

    /////AUXILIARY CLASSES AREA/////////////////////////////////////////////////////////  
    Builder::DictValue Builder::Context::Key(const std::string& key) {
        return b_.Key(key);
    }
    Builder& Builder::Context::ContValue(const NodeValue& value) {
        return b_.Value(value);
    }

    Builder::DictItem Builder::Context::StartDict() {
        return b_.StartDict();
    }
    Builder& Builder::Context::EndDict() {
        return b_.EndDict();
    }

    Builder::ArrayItem Builder::Context::StartArray() {
        return b_.StartArray();
    }
    Builder& Builder::Context::EndArray() {
        return b_.EndArray();
    }
}
