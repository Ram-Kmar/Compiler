#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>

// Forward declarations from your AST
struct Expr;
struct Stmt;
struct Function;
struct LayerDecl;

// Represents a connection between nodes carrying data (a variable)
struct ComputeEdge {
    struct ComputeNode* target;
    std::string var_name;
};

// Represents a node in the compute graph (a Layer instantiation)
struct ComputeNode {
    std::string name; // Variable name of the layer instance
    std::string layer_type; // Type of the layer (e.g., "Linear", "Conv2d")
    int id; // Unique ID for topological sort/identification
    
    // Inputs to this node (nodes that feed into this one)
    std::vector<ComputeNode*> inputs; 
    
    // Outputs from this node (edges to other nodes)
    std::vector<ComputeEdge> outputs;
    
    // For visualization or debugging
    std::string debug_info;

    ComputeNode(std::string n, std::string t, int i) : name(std::move(n)), layer_type(std::move(t)), id(i) {}
};

// Manages the construction and tracking of the compute graph
class ComputeGraph {
public:
    ComputeGraph() = default;

    // Adds a new node to the graph when a layer is instantiated
    ComputeNode* add_node(const std::string& name, const std::string& layer_type);

    // Connects two nodes (from -> to) representing data flow via a variable
    void add_edge(ComputeNode* from, ComputeNode* to, const std::string& var_name);
    
    // Connects a node by name (looks up existing node)
    void add_edge(const std::string& from_name, const std::string& to_name, const std::string& var_name);

    // Returns all nodes in the graph
    const std::vector<std::unique_ptr<ComputeNode>>& get_nodes() const { return m_nodes; }

    // Topological sort for execution order
    std::vector<ComputeNode*> get_execution_order() const;

    // Print graph for debugging (DOT format for Graphviz?)
    void print_graph() const;

private:
    std::vector<std::unique_ptr<ComputeNode>> m_nodes;
    std::unordered_map<std::string, ComputeNode*> m_node_map; // Name -> Node pointer
    int m_next_id = 0;
};