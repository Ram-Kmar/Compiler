#include "compute_graph.h"
#include <iostream>
#include <algorithm>
#include <memory>
#include <queue>
#include <map>

ComputeNode* ComputeGraph::add_node(const std::string& name, const std::string& layer_type) {
    auto node = std::make_unique<ComputeNode>(name, layer_type, m_next_id++);
    ComputeNode* ptr = node.get();
    m_nodes.push_back(std::move(node));
        m_node_map[name] = ptr;
        return ptr;
    }
    
    void ComputeGraph::add_edge(ComputeNode* from, ComputeNode* to, const std::string& var_name) {
        if (from && to) {
            // Check if edge already exists to avoid duplicates
            bool exists = false;
            for (auto* n : to->inputs) {
                if (n == from) { exists = true; break; }
            }
            if (!exists) {
                to->inputs.push_back(from);
                from->outputs.push_back({to, var_name});
                std::cout << "[Graph Debug] Added connection: " << from->name << " -> " << to->name << " (via " << var_name << ")" << std::endl;
            }
        }
    }
    
    void ComputeGraph::add_edge(const std::string& from_name, const std::string& to_name, const std::string& var_name) {
        if (m_node_map.count(from_name) && m_node_map.count(to_name)) {
            add_edge(m_node_map[from_name], m_node_map[to_name], var_name);
        } else {
            std::cerr << "Warning: Could not connect " << from_name << " to " << to_name << " (Node not found)" << std::endl;
        }
    }
    
    std::vector<ComputeNode*> ComputeGraph::get_execution_order() const {
        // Kahn's Algorithm for Topological Sort
        std::vector<ComputeNode*> result;
        std::map<ComputeNode*, int> in_degree;
        std::queue<ComputeNode*> zero_in_degree_queue;
    
        // Initialize in-degrees
        for (const auto& node : m_nodes) {
            in_degree[node.get()] = 0;
        }
        for (const auto& node : m_nodes) {
            for (const auto& edge : node->outputs) {
                in_degree[edge.target]++;
            }
        }
    
        // Find starting nodes
        for (const auto& node : m_nodes) {
            if (in_degree[node.get()] == 0) {
                zero_in_degree_queue.push(node.get());
            }
        }
    
        // Process
        while (!zero_in_degree_queue.empty()) {
            ComputeNode* u = zero_in_degree_queue.front();
            zero_in_degree_queue.pop();
            result.push_back(u);
    
            for (const auto& edge : u->outputs) {
                ComputeNode* v = edge.target;
                in_degree[v]--;
                if (in_degree[v] == 0) {
                    zero_in_degree_queue.push(v);
                }
            }
        }
    
        if (result.size() != m_nodes.size()) {
            std::cerr << "Error: Cycle detected in compute graph!" << std::endl;
            return {}; 
        }
    
        return result;
    }
    
    void ComputeGraph::print_graph() const {
        std::cout << "--- Compute Graph (DOT format) ---" << std::endl;
        std::cout << "digraph G {" << std::endl;
        std::cout << "  rankdir=LR;" << std::endl; // Left-to-Right layout
        std::cout << "  node [shape=box];" << std::endl;
        for (const auto& node : m_nodes) {
            std::cout << "  " << node->name << " [label=\"" << node->name << "\\n(" << node->layer_type << ")\"];" << std::endl;
            for (const auto& edge : node->outputs) {
                std::cout << "  " << node->name << " -> " << edge.target->name << " [label=\"" << edge.var_name << "\"];" << std::endl;
            }
        }
        std::cout << "}" << std::endl;
        std::cout << "----------------------------------" << std::endl;
    }
