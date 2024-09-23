#include "framework.hpp"
#include "utils/json_helpers.hpp"

typedef std::runtime_error SRE;

namespace neuro
{
    using std::string;
    using nlohmann::json;

    static json property_specs = {
         { "min_value", "D" },
         { "max_value", "D" },
         { "size", "I" },
         { "index", "I" },
         { "type", "C" },
         { "name", "S" },
         { "Necessary", { "min_value", "max_value", "size", "index", "type", "name" } } };

    static json property_pack_specs = {
         { "node_properties", "A" },
         { "edge_properties", "A" },
         { "network_properties", "A" },
         { "Necessary", { "node_properties", "edge_properties", "network_properties" } } };

    /* Helper function */

    static void add_property(PropertyMap &props, 
                             int prop_id, 
                             const string& name, 
                             double dmin, 
                             double dmax, 
                             Property::Type type, 
                             int count);

    Property::Property(const string& dname, int idx, int len, double dmin, double dmax, Type dtype) :
        type(dtype),
        index(idx),
        size(len),
        min_value(dmin),
        max_value(dmax),
        name(dname) 
    { }

    Property::Property(const json& j)
    {
      from_json(j);
    }

    Property::Property(const Property& p) :
        type(p.type), 
        index(p.index),
        size(p.size),
        min_value(p.min_value),
        max_value(p.max_value),
        name(p.name)
    { }

    Property::Property(Property &&p) noexcept :
        type(p.type), 
        index(p.index),
        size(p.size),
        min_value(p.min_value), 
        max_value(p.max_value),
        name(std::move(p.name))
    { }

    bool operator==(const Property &lhs, const Property &rhs)
    {
        if(lhs.type != rhs.type) return false;
        if(lhs.index != rhs.index) return false;
        if(lhs.size != rhs.size) return false;
        if(lhs.min_value != rhs.min_value) return false;
        if(lhs.max_value != rhs.max_value) return false;
        if(lhs.name != rhs.name) return false;
        return true;
    }

    bool operator!=(const Property &lhs, const Property &rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const PropertyPack &lhs, const PropertyPack &rhs)
    {
        if(lhs.nodes != rhs.nodes) return false;
        if(lhs.edges != rhs.edges) return false;
        if(lhs.networks != rhs.networks) return false;
        return true;
    }

    bool operator!=(const PropertyPack &lhs, const PropertyPack &rhs)
    {
        return !(lhs == rhs);
    }

    /* JSP: This is pretty inefficient and clunky code, but I wanted to make sure that
       add_node_property(), add_edge_property() and add_network_property() all work,
       so I simply call them. */
   
    void PropertyPack::from_json(const json &j)
    {
      int index;
      size_t i, k;
      map <int, Property> by_index;
      map <int, Property>::iterator bit;
      Property *p;
      vector <string> ptypes;
      string json_key;

      Parameter_Check_Json_T(j, property_pack_specs);

      clear();
      
      ptypes.push_back("node");
      ptypes.push_back("edge");
      ptypes.push_back("network");

      for (k = 0; k < ptypes.size(); k++) {
        json_key = ptypes[k] + "_properties";
        by_index.clear();
        for (i = 0; i < j[json_key].size(); i++) {
          p = new Property (j[json_key][i]);
          by_index.insert(std::make_pair(p->index, *p));
          delete p;
        }
        for (bit = by_index.begin(); bit != by_index.end(); bit++) {
          switch(k) {
            case 0:
              index = add_node_property(bit->second.name,
                                        bit->second.min_value,
                                        bit->second.max_value,
                                        bit->second.type,
                                        bit->second.size);
              break;
            case 1:
              index = add_edge_property(bit->second.name,
                                        bit->second.min_value,
                                        bit->second.max_value,
                                        bit->second.type,
                                        bit->second.size);
              break;
            case 2:
              index = add_network_property(bit->second.name,
                                        bit->second.min_value,
                                        bit->second.max_value,
                                        bit->second.type,
                                        bit->second.size);
              break;
            default: throw std::logic_error("Switch in PropertyPack::from_json");
          }
          if (index != bit->second.index) {
            clear();
            throw SRE((string) "Property Pack: non-matching index in " + 
                               ptypes[k] + " json:\n" + bit->second.as_json().dump());
          }
        }
      }
    }

    void PropertyPack::clear()
    {
      nodes.clear();
      edges.clear();
      networks.clear();
      node_vec_size = 0;
      edge_vec_size = 0;
      net_vec_size = 0;
    }

    void PropertyPack::to_json(json &j) const
    {
      PropertyMap::const_iterator pit;

      j["node_properties"] = json::array(); 
      for (pit = nodes.begin(); pit != nodes.end(); pit++) {
        j["node_properties"].push_back(pit->second.as_json());
      }
      j["edge_properties"] = json::array(); 
      for (pit = edges.begin(); pit != edges.end(); pit++) {
        j["edge_properties"].push_back(pit->second.as_json());
      }
      j["network_properties"] = json::array();
      for (pit = networks.begin(); pit != networks.end(); pit++) {
        j["network_properties"].push_back(pit->second.as_json());
      }
    }

    json PropertyPack::as_json() const
    {
        json j;

        j = json::object();
        to_json(j);
        return j;
    }

    /* JSP: This is really inefficient, but if you're calling pretty_json(), I doubt
       you're going to be worried about efficiency. */

    string PropertyPack::pretty_json() const
    {
        vector <string> ptypes;
        json j;
        string s, key;
        size_t i, k;
        Property *p;

        ptypes.push_back("node");
        ptypes.push_back("edge");
        ptypes.push_back("network");

        to_json(j);
        s = "";
        for (i = 0; i < ptypes.size(); i++) {
          key = ptypes[i] + "_properties";
          s += ((i == 0) ? "{ " : "  ");
          s += "\"";
          s += key;
          s += "\": ";
          if (j[key].size() == 0) {
            s += j[key].dump();
          } else {
            for (k = 0; k < j[key].size(); k++) {
              s += ((k != 0) ? "\n    " : "[\n    ");
              p = new Property(j[key][k]);
              s += p->pretty_json();
              delete p;
              if (k+1 != j[key].size()) s += ",";
            }
            s += "]";
          }
          if (i+1 != ptypes.size()) {
            s += ",\n";
          } else {
            s += " }";
          }
        }
        return s;
    }
    int PropertyPack::add_node_property(const string& name, double dmin, double dmax, Property::Type type, int cnt)
    {
        // starting index
        int start_idx = node_vec_size;
        if(cnt < 1) throw std::runtime_error("Count must be > 0.\n");

        // increment vector size
        node_vec_size += cnt;

        // insert property to PropertyMap
        add_property(nodes, start_idx, name, dmin, dmax, type, cnt);
        
        // return starting index for the property
        return start_idx;
    }

    int PropertyPack::add_edge_property(const string& name, double dmin, double dmax, Property::Type type, int cnt)
    {
        // starting index
        int start_idx = edge_vec_size;
        if(cnt < 1) throw std::runtime_error("Count must be > 0.\n");

        // increment vector size
        edge_vec_size += cnt;

        // insert property to PropertyMap
        add_property(edges, start_idx, name, dmin, dmax, type, cnt);
        
        // return starting index for the property
        return start_idx;
    }

    int PropertyPack::add_network_property(const string& name, double dmin, double dmax, Property::Type type, int cnt)
    {
        // starting index
        int start_idx = net_vec_size;
        if(cnt < 1) throw std::runtime_error("Count must be > 0.\n");

        // increment vector size
        net_vec_size += cnt;

        // insert property to PropertyMap
        add_property(networks, start_idx, name, dmin, dmax, type, cnt);
        
        // return starting index for the property
        return start_idx;
    }

    void add_property(PropertyMap &props, int prop_id, const string& name, double dmin, double dmax, Property::Type type, int count)
    {
        if(count <= 0) throw std::invalid_argument("Property count must be > 0.");
        if(name.empty()) throw std::invalid_argument("Property name must not be empty.");
        if(props.find(name) != props.end()) {
          throw std::invalid_argument((string) "Property name " + name + " already exists.");
        }

        props.insert({name, Property(name, prop_id, count, dmin, dmax, type)});
    }

    Property& Property::operator= (const Property &p)
    {
        type = p.type;
        index = p.index;
        size = p.size;
        min_value = p.min_value;
        max_value = p.max_value;
        name = p.name;
        return *this;
    }

    Property& Property::operator= (Property &&p)
    {
        type = p.type;
        index = p.index;
        size = p.size;
        min_value = p.min_value;
        max_value = p.max_value;
        name = std::move(p.name);
        return *this;
    }

    void Property::from_json(const json &j)
    {
        string e;
        char buf[100];
        Property::Type t;

        e = Parameter_Check_Json(j, property_specs);
        if (e != "") throw std::runtime_error(e);
        t = j["type"]; 
        if (t != Property::Type::BOOLEAN && 
            t != Property::Type::INTEGER && 
            t != Property::Type::DOUBLE) {
          snprintf(buf, 100, "Bad Property JSON - Type must be D (%d), I (%d) or B (%d).\n",
                       'D', 'I', 'B');
          throw SRE((string) buf + j.dump());
        }
        type = t;
        index = j["index"]; 
        size = j["size"]; 
        min_value = j["min_value"]; 
        max_value = j["max_value"]; 
        name = j["name"]; 
    }

    void Property::to_json(json &j) const
    {
        j["type"] = type;
        j["index"] = index;
        j["size"] = size;
        j["min_value"] = min_value;
        j["max_value"] = max_value;
        j["name"] = name;
    }

    json Property::as_json() const
    {
        json j;

        j = json::object();
        to_json(j);
        return j;
    }

    string Property::pretty_json() const
    {
      string s, key;
      json keys = { "name", "type", "index", "size", "min_value", "max_value" };
      json j;
      size_t i;
 
      j = as_json();
      s = "{";
      for (i = 0; i < keys.size(); i++) {
        s += " ";
        s += keys[i].dump();
        key = keys[i];
        s += ":";
        s += j[key].dump();
        if (i+1 != keys.size()) s += ",";
      }
      s += " }";
      return s;
    }
}
