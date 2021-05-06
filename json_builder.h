#pragma once

#include "json.h"
#include <memory>
#include <stack>

namespace json {


	class Builder {
	private:
		/////BUILDER's AUX.CLASSES AREA////////////////////////////////////////////////////
		class DictValue;
		class DictItem;
		class ArrayItem;

		class Context {
		public:
			Context(Builder& b)
				:builder_(b) {
			}

			DictValue Key(const std::string& key); // for DictItem
			
			DictItem StartDict(); // for ArrayItem and DictValue
			Builder& EndDict(); // for DictItem

			ArrayItem StartArray(); // for Arrayitem and DictValue
			Builder& EndArray(); // for Arrayitem

		protected: // ContValue shouldn't be available to call via Child interface. Only delegation is allowed
			Builder& ContValue(const NodeValue& value); // Problem with 2 different types of Value (for Map and Array). 
														// It's just some king of wrapper to delegate Builder's Value method to inherited classes
		private:
			Builder& builder_;
		};


		class ArrayItem : public Context { // Aux. class for Array. Allows to add Value, start and end Array
		public:
			ArrayItem(Builder& b) // Delegate creation to parent class
				:Context(b) {
			}

			ArrayItem Value(const NodeValue& value) {
				return ArrayItem(ContValue(value)); // Delegate adding value into a node to parent (Context)
			}
			DictValue Key(const std::string& key) = delete; // N/A for an Array
			Builder& EndDict() = delete; // N/A for an Array
		};

		class DictItem : public Context { // Aux. class for Array. Allows to add Value, start and end Array
		public:
			DictItem(Builder& b) // Delegate creation to parent class
				:Context(b) {
			}
			
			DictItem StartDict() = delete; // Cannot create new Map creation definition instead of Key definition
			ArrayItem StartArray() = delete; // Array can be created only in Map's Value
			Builder& EndArray() = delete; // Only ArrayItem class has right to end an Array
		};

		class DictValue : public Context {
		public:
			DictValue(Builder& b) // Delegate creation to parent class
				:Context(b) {
			}

			DictItem Value(const NodeValue& value) { 
				return DictItem(ContValue(value)); // Delegate adding value into a node to parent (Context)
			}

			DictValue Key(const std::string& key) = delete; // Key cannot be right after Key
			Builder& EndDict() = delete; // N/A for Map Value
			Builder& EndArray() = delete; // N/A for Map Value
		};
		/////BUILDER METHOD AREA////////////////////////////////////////////////////
	public:
		Builder() = default;

		Node Build();

		DictValue Key(std::string key);

		Builder& Value(NodeValue value);

		DictItem StartDict();
		Builder& EndDict();

		ArrayItem StartArray();
		Builder& EndArray();
		
	private:
		//Node root_; // is it really needed? probably it can be used to avoid some copy		
		bool node_ended_ = false;
		std::vector<std::unique_ptr<Node>> nodes_stack_; // Vector as a stack? Dynamic memory is prefered of stack would be better?
	};
}
