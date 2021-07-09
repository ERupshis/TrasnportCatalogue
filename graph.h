#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>



namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;
    };

    using IncidenceList = std::vector<EdgeId>;

    template <typename Weight>
    class DirectedWeightedGraph {          
    public:        
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

        DirectedWeightedGraph() = default;
        explicit DirectedWeightedGraph(size_t vertex_count);
        EdgeId AddEdge(const Edge<Weight>& edge);

        void ResizeIncidenceLists(size_t vertex_count);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;

        const Edge<Weight>& GetEdge(EdgeId edge_id) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

        std::vector<Edge<Weight>>& ModifyEdges(); //SPRINT 14
        std::vector<IncidenceList>& ModifyIncidenceLists(); //SPRINT 14

    private:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
        : incidence_lists_(vertex_count) {
    }

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        if (incidence_lists_.size() <= edge.from) {
            incidence_lists_.resize(2 * edge.from + 1);
        }
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template <typename Weight>
    void DirectedWeightedGraph<Weight>::ResizeIncidenceLists(size_t vertex_count) {
        incidence_lists_.resize(vertex_count);
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
        DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }
    template <typename Weight>
    std::vector<Edge<Weight>>& DirectedWeightedGraph<Weight>::ModifyEdges() { //SPRINT 14
        return edges_;
    }
    template <typename Weight>
    std::vector<IncidenceList>& DirectedWeightedGraph<Weight>::ModifyIncidenceLists() { //SPRINT 14
        return incidence_lists_;
    }
}  // namespace graph