#pragma once
#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <vector>
#include <map>
#include <memory>
#include <exception>
#include <stdexcept>

#include "robinhood/robin_map.h"
#include "nlohmann/json.hpp"
#include "utils/MOA.hpp"

/** 
 * The neuro namespace contains all of the classes defined by and used by the TENNLab
 * open source neuromorphic computing framework.   In general, the documentation for 
 * each component is in the markdown directory:
 *
 * Networks, Nodes and Edges:   markdown/framework_network.md
 * Processors:                  markdown/framework_processor.md
 * Properties and PropertyPack: markdown/framework_properties.md
 * Associated Data in Networks: markdown/framework_network_json_format.md
 */

namespace neuro
{

using std::map;
using std::string;
using std::vector;
using std::pair;
using std::tuple;
using std::unique_ptr;
using nlohmann::json;

class Property; 
class PropertyPack;
class Node;
class Edge;
class Network;
class Processor;
struct Spike;

typedef pair<int,int> Coords;

/**
 * This first group of definitions facilitates using a hash table for nodes and edges.
 */

class int_hash
{
public:
    size_t operator() (const uint32_t val) const
    {
        // From MurmurHash3
        size_t h = val;
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        return h;
    }
};

/**
 * A functor class for hashing pair values.
 */
class coord_hash
{
public:
    /* See: https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes */
    size_t operator() (const Coords &p) const
    {
        size_t l = int_hash{}(p.first);
        size_t r = int_hash{}(p.second);
        l ^= r + 0x9e3779b9 + (l << 6) + (l >> 2);
        return l;
    }
};

/* Use Hash Tables for storing a sparse collection of nodes / edges */
typedef tsl::robin_map<uint32_t, unique_ptr<Node>, int_hash> NodeMap;
typedef tsl::robin_map<Coords, unique_ptr<Edge>, coord_hash> EdgeMap;

/* TMP + Perfect Forwarding for C++11 // built into C++14 */
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/**
 * The Property class represents a specific configurable property within a node, edge, or network.
 * Please read the documentation in `markdown/framework_properties.md` for a description
 * of both the `Property` and `Property_Pack` classes.
 */

class Property
{
public:

    /** * Each property has an associated data type chosen from this enumeration. */
    enum Type : signed char
    {
        INTEGER = 'I', /**< Numerical, integer value */
        DOUBLE  = 'D', /**< Numerical, double precision (64 bits) floating point value */
        BOOLEAN = 'B', /**< Boolean value */
    };

    /* Four constructors, including copy and move constructors */

    Property(const string& dname, int idx, int len, double dmin, double dmax, Type dtype);
    Property(const json& j);
    Property(const Property& p);
    Property(Property &&p) noexcept;

    /* Assignment overloads work. */

    Property& operator= (const Property &p);
    Property& operator= (Property &&p);

    ~Property() = default;

    /* Json methods */

    void to_json(json &j) const;     /**< Add keys/vals to existing json */
    json as_json() const;            /**< Return json representation */
    void from_json(const json &j);   /**< Create from json */
    string pretty_json() const;      /**< Create a json string that's better than dump(). */

    /* The data -- again, please read the markdown for descriptions. */

    Type type;                       /**< Integer/Boolean/Double */
    int index;                       /**< Starting index in the values vector */
    int size;                        /**< Number of vector entries */
    double min_value;                /**< Minimum value that it can attain */
    double max_value;                /**< Maximum value that it can attain */
    string name;                     /**< Name of the property */
};

/* Property equality in case you need it. */

bool operator==(const Property &lhs, const Property &rhs);
bool operator!=(const Property &lhs, const Property &rhs);

/**
 * A PropertyPack encompasses a grouping of properties for a node, edge, and network 
 * all within the same structure. This ensures you have a coherent set of properties which 
 * make sense for your desired processor(s).  Please see `markdown/framework_properties.md`
 * for documentation.
 */

/* Properties are stored internally using a standard map. */

typedef std::map<string, Property> PropertyMap;

class PropertyPack
{
public:

    void to_json(json &j) const;     /**< Add keys/vals to existing json */
    json as_json() const;            /**< Return json representation */
    void from_json(const json &j);   /**< Create from json */
    string pretty_json() const;      /**< Create a json string that's better than dump(). */
    void clear();                    /**< Reset state to empty. */

    PropertyMap nodes;               /**< Map of properties for Node objects */
    PropertyMap edges;               /**< Map of properties for Edge objects */
    PropertyMap networks;            /**< Map of properties for Network objects */

    size_t node_vec_size = 0;        /**< How long the node data vector should be. */
    size_t edge_vec_size = 0;        /**< How long the edge data vector should be. */
    size_t net_vec_size = 0;         /**< How long the network data vector should be. */

    /* Should allow the class to be "tie"-able */

    /**
     * This allows a PropertyPack to work like a native tuple. A developer can call 
     * `std::tie` to quickly dump the node, edge, and network property maps 
     * in a single line of code.
     */
    operator tuple<PropertyMap&, PropertyMap&, PropertyMap&>()
    {
        return tuple<PropertyMap&, PropertyMap&, PropertyMap&>{nodes, edges, networks};
    }

    /* Adding properties to a property pack. */

    int add_node_property(const string& name, double dmin, double dmax, Property::Type type, int cnt = 1);
    int add_edge_property(const string& name, double dmin, double dmax, Property::Type type, int cnt = 1);
    int add_network_property(const string& name, double dmin, double dmax, Property::Type type, int cnt = 1);

};

bool operator==(const PropertyPack &lhs, const PropertyPack &rhs);
bool operator!=(const PropertyPack &lhs, const PropertyPack &rhs);

/**
 * The Node class represents a single node in a graph/network. 
 * Typically, for our use, this means it serves as a neuron.
 * Please read the documentation in `markdown/framework_network.md for infomation
 * about this class.
 */
class Node
{
public:

    /* Constructors, assignment overleads, etc. */

    Node(uint32_t idx, Network* n = nullptr) : id(idx), net(n) {};
    Node(const Node &n) = delete;
    Node(Node &&n) = delete;
    Node& operator=(const Node &n) = delete;
    Node& operator=(Node &&n) = delete;
    ~Node() = default;

    /* Fields */

    uint32_t id = 0;                /**< Node ID */
    int input_id = -1;              /**< If it's an input node, the input id number. */
    int output_id = -1;             /**< If it's an output node, the output id number. */
    const Network *net;             /**< Pointer to the network that contains the node. */
    vector<double> values;          /**< Values defined by the PropertyPack */
    vector<Edge*> incoming;         /**< Incoming edges */
    vector<Edge*> outgoing;         /**< Outgoing edges */
    vector <double> coordinates;    /**< Optional -- useful if you are writing or using a visualization */
    string name;                    /**< Optional -- can be useful for viz's or hand-tooling networks */

    /* Getting / Setting values */

    void set(int idx, double val);              /**< Set a value by its index in values */
    void set(const string& name, double val);   /**< Set a value by its name in the PropertyPack */
    double get(int idx);                        /**< Get a value by index. */
    double get(const string& name);             /**< Get a value by name. */

    /* Hidden / Input / Output */

    inline bool is_hidden() const               /**< Is the node hidden? */
        { return !is_input() && !is_output(); }

    inline bool is_input() const                /**< Is the node an input node? */
        { return input_id >= 0; }

    inline bool is_output() const               /**< Is the node an output node? */
        { return output_id >= 0; }
};

/**
 * The Edge class serves as a directed edge between two nodes in a network. 
 * Typically, this means it will serve as a synapse between two nodes.
 * Please read the documentation in `markdown/framework_network.md for infomation
 * about this class.
 */
class Edge
{
public:

    /* Constructors, assignment overleads, etc. */

    Edge(Node *f, Node *t, Network *n = nullptr) : from(f), to(t), net(n) {}
    Edge(const Edge &e) = delete;
    Edge(Edge &&e) = delete;
    Edge& operator=(const Edge &n) = delete;
    Edge& operator=(Edge &&n) = delete;
    ~Edge() = default;
    
   /* Fields */

    Node* from;                         /**< The node that the edge is coming from. */
    Node* to;                           /**< The node that the edge is going to. */
    const Network *net;                 /**< Pointer to the network that contains the node. */
    vector<double> values;              /**< Values defined by the PropertyPack */
    json as_json() const;               /**< Turn it into a json object */
    vector <double> control_point;      /**< Optional. Bezier control point(s) for displaying. */

    /* Getting / Setting values */

    void set(int idx, double val);              /**< Set a value by its index in values */
    void set(const string& name, double val);   /**< Set a value by its name in the PropertyPack */
    double get(int idx);                        /**< Get a value by index. */
    double get(const string& name);             /**< Get a value by name. */
};

/**
 * The Network class contains a directed graph of nodes and edges along with the 
 * associated properties to describe the characteristics of each component.
 * Please read the documentation in `markdown/framework_network.md for infomation
 * about this class.
 */

class Network 
{
public:

    /* Constructors, assignment overloads, etc. */

    Network() = default;
    Network(const Network &net);
    Network(Network &&net);
    Network& operator=(const Network &net);
    Network& operator=(Network &&net);
    ~Network() noexcept;

    bool operator==(const Network &rhs) const;

    void clear(bool include_properties); /**< Clear network, optionally clear properties. */

    /* JSON methods */

    void to_json(json &j) const;       /**< Add keys/vals to existing json */
    json as_json() const;              /**< Return json representation */
    void from_json(const json &j);     /**< Create from json */

    string pretty_json() const;        /**< Create a json string that's better than dump(). */
    string pretty_nodes() const;       /**< Create a nice json string of the nodes. */
    string pretty_edges() const;       /**< Create a nice json string of the edges. */

    /* Properties / PropertyPack */

    void set_properties(const PropertyPack& properties);  /**< Set the PropertyPack */
    PropertyPack get_properties() const;                  /**< Get the PropertyPack */

    bool is_node_property(const string& name) const;    /**< Query the node properties for name. */
    bool is_edge_property(const string& name) const;    /**< Query the edge properties for name. */
    bool is_network_property(const string& name) const; /**< Query the network properties for name. */

    const Property* get_node_property(const string& name) const;    /**< Get the node property. */
    const Property* get_edge_property(const string& name) const;    /**< Get the edge property. */
    const Property* get_network_property(const string& name) const; /**< Get the network property. */
 
    /* Adding / modifying / deleting nodes and edges */

    Node* add_node(uint32_t idx);                        /**< Create node with id */
    bool is_node(uint32_t idx) const;                    /**< Does node with id exist? */
    Node* get_node(uint32_t idx) const;                  /**< Return node with id */
    Node* add_or_get_node(uint32_t idx);                 /**< Return node with id , or create it */
    void remove_node(uint32_t idx, bool force = false);  /**< Delete node - if force=false, error on IO nodes. */
    void rename_node(uint32_t old_name, uint32_t new_name); /**< Change node's id */

    Edge* add_edge(uint32_t fr, uint32_t to);            /**< Analogous to add_node() */
    bool is_edge(uint32_t fr, uint32_t to) const;        /**< Analogous to is_node() */
    Edge* get_edge(uint32_t fr, uint32_t to) const;      /**< Analogous to get_node() */
    Edge* add_or_get_edge(uint32_t fr, uint32_t to);     /**< Analogous to add_or_get_node() */
    void remove_edge(uint32_t fr, uint32_t to);          /**< Analogous to remove_node() */

    /* Input and output nodes */

    int add_input(uint32_t idx);                    /**< Add the next input */
    void set_input(int input_id, uint32_t nid);     /**< Set the given input node  */
    Node* get_input(int input_id) const;            /**< Return a pointer to the given input */
    int num_inputs() const;                         /**< The number of input nodes */

    int add_output(uint32_t idx);                    /**< Add the next output */
    void set_output(int output_id, uint32_t nid);    /**< Set the given output node  */
    Node* get_output(int output_id) const;           /**< Return a pointer to the given output */
    int num_outputs() const;                         /**< The number of output nodes */

    /* The Associated Data JSON */

    void set_data(const string& name, const json& data);   /**< Add/Set key/val pair to JSON */
    json get_data(const string& name) const;               /**< Get json for the given key */
    vector<string> data_keys() const;                      /**< Get all of the keys */

    /* Methods to help do randomization */

    Node* get_random_node(MOA &moa) const;        /**< Get a random node */
    Edge* get_random_edge(MOA &moa) const;        /**< Get a random edge */
    Node* get_random_input(MOA &moa) const;       /**< Get a random input node */
    Node* get_random_output(MOA &moa) const;      /**< Get a random output node */
    
    void randomize_properties(MOA &moa);          /**< Randomize network values */
    void randomize_properties(MOA &moa, Node *n); /**< Randomize node values */
    void randomize_properties(MOA &moa, Edge *e); /**< Randomize edge values */

    void randomize_property(MOA& moa, const string& pname);          /**< Randomize single value */
    void randomize_property(MOA& moa, Node *n, const string& pname); /**< Randomize single value */
    void randomize_property(MOA& moa, Edge *n, const string& pname); /**< Randomize single value */

    void randomize_property(MOA& moa,                /**< Randomize a single value in the vector. */
                            const Property& p,  
                            vector<double>& values); 

    void randomize(const json& params);            /**< Generate a random network (unimplemented) */

    /* Pruning and Sorting */

    void prune();                      /**< Prune nodes and edges not on an I/O path */
    void make_sorted_node_vector();       /**< Sort the nodes by id, nothing if already sorted. */
    vector <Node *> sorted_node_vector;   /**< The sorted nodes */

    /* Iterators and Metadata */

    NodeMap::iterator begin();         /**< Beginning of the nodes in the node hash table. */
    NodeMap::iterator end();           /**< End of the nodes in the node hash table. */

    EdgeMap::iterator edges_begin();       /**< Beginning of the edges in the edge hash table. */
    EdgeMap::iterator edges_end();         /**< End of the edges in the edge hash table. */
    
    size_t num_nodes() const;          /**< Number of nodes. */
    size_t num_edges() const;          /**< Number of edges. */

    /* The values defined by the PropertyPack */

    vector<double> values;

protected:
    /* helpers for move/copy operations */
    void copy_from(const Network& net);
    void move_from(Network&& net);

    /* return a random value appropriate for the given property */
    double random_value(MOA &moa, const Property &p);

    /* random network -- edges with probability p */
    void randomize_p(const json& params);

    /* random network -- input -> hidden -> output */
    void randomize_h(const json& params);

    /* inputs/outputs -- index = input/output id, value = node id */
    vector<int> m_inputs;
    vector<int> m_outputs;

    /* Nodes and Edges for the network stored centrally */

    NodeMap m_nodes;
    EdgeMap m_edges;

    PropertyPack m_properties;

    /* dictionary of associated data (e.g. encoder params, app params, etc.) */
    json m_associated_data = {};

    friend class Node;
    friend class Edge;
};

/**
 * A Spike is a simple data structure to represent a tuple of id, time, and value.
 */
struct Spike
{
    int id; /**< Represents the input id of the destination neuron. */
    double time; /**< Represents the timing of when the spike should arrive. */
    double value; /**< On platforms which support variable values, this is the charge to accumulate. */

    Spike(int id_, double time_, double value_) : 
        id(id_), time(time_), value(value_) {}
};

/**
 * The Processor class is an interface for neuromorphic simulators and hardware.
 * This interface specifies the necessary methods to interact seamlessly with 
 * the rest of the framework.  In other words, when you want to implement a 
 * simulator for a processor (or implement the systems code to connect a physical
 * processor to the framework), then these are the methods that you need to
 * implement.  Please read the documentation in markdown/framework_processor (relative
 * to the main framework directory) for information about these methods and implementing
 * a processor to match this interface.
 */
class Processor
{
public:
    virtual ~Processor() {}

    static Processor *make(const string &name, json &params);

    /* Load or clear a network. */

    virtual bool load_network(Network* n, int network_id = 0) = 0;
    virtual bool load_networks(std::vector<Network*> &n) = 0;
    virtual void clear(int network_id = 0) = 0;
    
    /* Queue spike(s) as input to a network or to multiple networks.
       Spike values should be between 0 and 1, and then the processor can
       convert its value to the proper range. */

    virtual void apply_spike(const Spike& s, int network_id = 0) = 0;
    virtual void apply_spike(const Spike& s, const vector<int>& network_ids) = 0;

    virtual void apply_spikes(const vector<Spike>& s, int network_id = 0) = 0;
    virtual void apply_spikes(const vector<Spike>& s, const vector<int>& network_ids) = 0;

    /* Run the network(s) for the desired time with queued input(s) */

    virtual void run(double duration, int network_id = 0) = 0;
    virtual void run(double duration, const vector<int>& network_ids) = 0;

    /* Get processor time based on specified network */
    virtual double get_time(int network_id = 0) = 0;

    /* Output tracking.  See the markdown for a detailed description of these.  */

    virtual bool track_output_events(int output_id, bool track = true, int network_id = 0) = 0;
    virtual bool track_neuron_events(uint32_t node_id, bool track = true, int network_id = 0) = 0;

    /* Access output spike data */

    virtual double output_last_fire(int output_id, int network_id = 0) = 0;
    virtual vector <double> output_last_fires(int network_id = 0) = 0;

    virtual int output_count(int output_id, int network_id = 0) = 0;
    virtual vector <int> output_counts(int network_id = 0) = 0;

    virtual vector <double> output_vector(int output_id, int network_id = 0) = 0;
    virtual vector < vector <double> > output_vectors(int network_id = 0) = 0;

    /* Spike data from all neurons. */

    virtual long long total_neuron_counts(int network_id = 0) = 0;
    virtual long long total_neuron_accumulates(int network_id = 0) = 0;

    virtual vector <int> neuron_counts(int network_id = 0) = 0;
    virtual vector <double> neuron_last_fires(int network_id = 0) = 0;
    virtual vector < vector <double> > neuron_vectors(int network_id = 0) = 0;

    /* Charge data from all neurons. */

    virtual vector <double> neuron_charges(int network_id = 0) = 0;

    /* Synapse data from all synapses. */

    virtual void synapse_weights (vector <uint32_t> &pres, 
                                  vector <uint32_t> &posts,
                                  vector <double> &vals,
                                  int network_id = 0) = 0;

    /* Remove state, keep network loaded */
    virtual void clear_activity(int network_id = 0) = 0;

    /* Network and Processor Properties.  The network properties correspond to the Data
       field in the network, nodes and edges.  The processor properties are so that
       applications may query the processor for various properties (e.g. input scaling,
       fire-on-threshold vs fire-over-threshold). */

    virtual PropertyPack get_network_properties() const = 0;
    virtual json get_processor_properties() const = 0;

    /* get_params() returns the json that you can use to recreate the processor. 
       It doesn't have to be the same json as was used to create the processor; just
       json to create the same processor. */

    virtual json get_params() const = 0;

    /* get_name() returns the name of the processor. */

    virtual string get_name() const = 0;
};

/**
 * Helper procedures for applications using processors.
 */

/* This calls track_output_events for all of the output neurons in the given network. 
   If it fails, it will undo the track_output_events() calls that it made previously. */
    
bool track_all_output_events(Processor *p, Network *n, int network_id = 0);
bool track_all_neuron_events(Processor *p, Network *n, int network_id = 0);

/* If node n is the ith smallest node by node id, and the jth smallest node
   in the ordering of the NodeMap then rv[i].second = j, and rv[i].first = the
   node's id.  This helps deal with the return values to neuron_counts(),
   neuron_last_fires(), neuron_vectors() and neuron_charges(), and also with the fact that
   nodes are stored in a hash table.  */

json neuron_last_fires_to_json(const vector <double> &last_fires, Network *n);
json neuron_counts_to_json(const vector <int> &counts, Network *n);
json neuron_charges_to_json(const vector <double> &charges, Network *n);

json neuron_vectors_to_json(const vector < vector <double> > &events, 
                            const string &type,
                            Network *n);

/* This is a heavyweight procedure.  It assumes that *n* has been loaded on *p*.  It
   will call run(1) duration times, and then return a json with two keys.  Each val
   is a vector of vectors.  The outer vector has an entry for each timestep.

    "spike_raster": The inner vector is 0 or 1 for each neuron. The values are presented in
                    the same order as sorted_node_vector.
    "charges": The inner vector contains the charge of each neuron (doubles).  The values
                    are presented in the same order as sorted_node_vector.
 */

json run_and_track(int duration, Processor *p, int network_id = 0);

/* This copies n, but sets the synapse weights from the network that is on the processor. */

Network *pull_network(Processor *p, Network *n, int network_id = 0);

/* Use the spike raster to emit the appropriate apply_spikes() calls. */

void apply_spike_raster(Processor *p, int in_neuron, const vector <char> &sr, int network_id = 0);


}   // End of neuro namespace.

#endif
