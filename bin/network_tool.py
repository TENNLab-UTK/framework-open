import sys
import json
import subprocess

import neuro
import risp

SRE = RuntimeError

def print_commands(f):
    f.write("This is a network tool program. The commands listed below are case-insensitive,\n")
    f.write("For commands that take a json either put a filename on the same line,\n")
    f.write("or the json can be multiple lines, starting on the next line.\n")

    f.write("\nCreate/Clear Network Commands\n")
    f.write("FJ json                    - Read a network.\n")
    f.write("TJ [file]                  - Create JSON from the network.\n")
    f.write("COPY_FROM                  - Make a copy of yourself using copy_from.  Print & delete.\n")
    f.write("DESTROY                    - Delete network, create empty network.\n")
    f.write("CLEAR                      - Clear network\n")
    f.write("CLEAR_KP                   - Clear network but keep the property pack intact\n")     
    
    f.write("\nAccess Network Info Commands\n")
    f.write("INFO                       - Print some info about the network.\n")
    f.write("NODES [node_id] [...]      - Print the nodes using a kind of user-friendly JSON format.\n")
    f.write("EDGES [from] [to] [...]    - Print the edges using a kind of user-friendly JSON format.\n")
    f.write("PROPERTIES/P               - Print the network's property pack.\n")
    f.write("ASSOC_DATA [key]           - Print the network's associated_data, or just that key.\n")
    f.write("TYPE node_id  ...          - Print the node's type\n")
    f.write("NM                         - Print node names map\n")
    
    f.write("\nNetwork Operation Commands\n")

    f.write("PRUNE                      - Prune the network - remove nodes/edges not on an I/O path.\n")
    f.write("AN node_id ...             - Add nodes\n")
    f.write("AI node_id ...             - Add inputs\n")
    f.write("AO node_id ...             - Add outputs\n")
    f.write("AE from to ...             - Add edges \n")
    f.write("RN node_id ... [IOE(T|F)]  - Remove nodes. IOE (def:F) flags an error if Input/Output\n")
    f.write("RE from to ...             - Remove edges\n")
    f.write("RENAME from to             - Rename nodes\n")
    f.write("SETNAME node_id name       - Set the names of nodes. A name of \"-\" clears the name\n")
    f.write("SETCOORDS node_id [x] [y]  - Set the coordinates of a node. This is used by viz\n")
    f.write("SET_CP node_id [x] [y]     - Set the control points of a node. This is used by viz\n")
    f.write("CLEAR_VIZ                  - Get rid of all coordinates and control points in the network\n")

    f.write("SNP node_id ... name value - Set the node's named property to value.\n")
    f.write("SEP from to ... name value - Set the edge's named property to value.\n")
    f.write("SNP_ALL name value         - Set named property to value for all nodes.\n")
    f.write("SEP_ALL name value         - Set named property to value for all edges.\n")
    f.write("RNP node_id ... [pname]    - Randomize the node's properties or named property\n")
    f.write("REP from to ... [pname]    - Randomize the edge's properties or named property\n")

    f.write("SEED val                   - Seed the RNG\n")
    f.write("SHOW_SEED                  - Show the RNG seed\n")
    f.write("RE_SEED                    - Re-seed the RNG based on the current time & show seed\n")
    f.write("SPROPERTIES/SP json        - Set the network's property pack\n")
    f.write("SET_ASSOC key json         - Set the key/val in the network's associated data.\n")
    f.write("SORT/SORTED [Q]            - Sort the network and print the sorted node id's. Q = no output\n")
 
    f.write("\nHelper Commands\n")
    f.write("RUN                        - Run the app.\n")
    f.write("?                          - Print commands.\n")
    f.write("Q                          - Quit.\n")

def is_number(string):
    for c in string:
        if not c.isdigit(): return False
    return True

def to_uppercase(s):
    rs = []
    for c in s:
        if 'a' <= c <= 'z':
            rs.append(chr(ord(c)+ord('A')-ord('a')))
        else:
            rs.append(c)
    return ''.join(rs)

def read_json (sv, starting_field):
    if starting_field < len(sv):
        try:
            with open(sv[starting_field], 'r') as f:
                try:
                    rv = json.loads(f.readline().split()[0])
                    return rv
                except:
                    return None
        except Exception as e:
            sys.stderr.write(f'{e} {sv[starting_field]}')
    else:
        try:
            rv = json.loads(input().split()[0])
            return rv
        except:
            return None

def get_node_id_by_name (name, node_names):
    if name in node_names: return node_names[name]
    return -1

def get_node_name(n):
    node_id = str(n.id)
    if (n.name == ""): return node_id
    return f'{node_id}({n.name})'

def main():
    sv = []
    keys = []
    dv = []
    node_names = {}
    tmp_map = {}
    j1 = {}

    rng = neuro.MOA()

    d = 0

    if len(sys.argv) > 2 or (len(sys.argv) == 2 and sys.argv[1] == "--help"):
        sys.stderr.write("usage: network_tool [prompt]\n\n")
        print_commands(sys.stderr)
        sys.exit(1)

    seed = rng.Seed_From_Time()
    rng.Seed(seed, "network_tool")

    if len(sys.argv) == 2:
        prompt = sys.argv[1] + " "
    else:
        prompt = ""

    n = neuro.Network()
    n2 = neuro.Network()
    e = None
    lowest_free_id = 0
    pp = neuro.PropertyPack()

    while 1:
        if prompt != "": print(prompt, end='')

        sv = input().split()

        size = len(sv)
        if size != 0: sv[0] = to_uppercase(sv[0])

        if size == 0:
            pass
        elif sv[0][0] == '#':
            pass
        elif sv[0] == '?':
            print_commands(sys.stdout)
        elif sv[0] == "Q":
            exit(0)

        elif sv[0] == "FJ":
            j1 = read_json(sv, 1)
            if j1 == None:
                print("Bad json.")
            else:
                try:
                    node_names.clear()
                    lowest_free_id = 0

                    n.from_json(j1)
                    NodeMap = n.get_node_map()

                    for node in NodeMap.values():
                        node_names[str(node.id)] = node.id
                except SRE as e:
                    print(e)
        elif sv[0] == "SEED":
            try:
                if len(sv) != 2:
                    print("usage: SEED val")
                else:
                    seed = int(sv[1])
                    if seed < 0:
                        raise SRE
                    rng.Seed(seed, "network_tool")
            except:
                print("usage: SEED val")
        elif sv[0] == "SHOW_SEED":
            print(seed)
        elif sv[0] == "RE_SEED":
            seed = rng.Seed_From_Time()
            rng.Seed(seed, "network_tool")
            print(seed)
        elif sv[0] == "TJ":
            try:
                if size > 1:
                    with open(sv[1], "w+") as f:
                        f.write(n.pretty_json())
                else:
                    print(n.pretty_json())
            except Exception as e:
                print(e)
        elif sv[0] == "PRUNE":
            try:
                if size != 1:
                    print("usage: PRUNE")
                    n.prune()

                tmp_map.clear()
                for key in node_names:
                    if n.is_node(int(node_names[key])): tmp_map[key] = node_names[key]
                    
                node_names = tmp_map
                lowest_free_id = 0

            except Exception as e:
                print(e)
        elif sv[0] == "COPY_FROM":
            n2 = n
            j1 = n2.as_json()
            print(j1.dump(2))
        elif sv[0] == "DESTROY":
            del n
            n = neuro.Network()
            node_names.clear()
            lowest_free_id = 0
        elif sv[0] == "NODES":
            if size == 1:
                print(n.pretty_nodes())
            else:
                for i in range(1, size):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                        node = n.get_node(nid)
                        j1 = neuro.json()
                        j1["id"] = node.id,
                        j1["values"] = node.values
                        if len(node.coordinates) != 0:
                            j1["coords"] = node.coordinates
                        print(j1.dump)
                    except Exception as e:
                        print(e)
        elif sv[0] == "EDGES":
            if size == 1:
                print(n.pretty_edges())
            elif (size-1) % 2 != 0:
                print("usage: EDGES [from1] [to1] [from2] [to2] ...")
            else:
                for i in range(0, int((size-1)/2)):
                    try:
                        fro = get_node_id_by_name(sv[1+i*2], node_names)
                        to = get_node_id_by_name(sv[2+i*2], node_names)
                        if fro == -1 or to == -1:
                            raise SRE(f'{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')
                        e = n.get_edge(fro, to)
                        print(''.join(str(e.as_json()).split()))
                    except Exception as e:
                        print(e)
        elif sv[0] == "SORTED" or sv[0] == "SORT":
            if size > 2 or (size == 2 and sv[1] != "Q"):
                print("usage: SORT/SORTED [Q] - Sort the network and print the sorted node id's.", end='')
                print(" Q = no output")
            n.make_sorted_node_vector()
            if size == 1:
                for i in range(len(n.sorted_node_vector)):
                    if i != 0: print('',end=' ')
                    print(f'{n.sorted_node_vector[i].id}', end='')
                print()
        elif sv[0] == "PROPERTIES" or sv[0] == "P":
            print(f'{n.get_properties().pretty_json()}')
        elif sv[0] == "SPROPERTIES" or sv[0] == "SP":
            j1 = read_json(sv, 1)
            if j1 == None:
                print("SPROPERTIES: Bad json.")
            else:
                try:
                    print(j1)
                    pp.from_json(j1)
                    n.set_properties(pp)
                except Exception as e:
                    print(e)
        elif sv[0] == "INFO":
            print(f'Nodes:    {n.num_nodes()}')
            print(f'Edges:    {n.num_edges()}')
            print(f'Inputs:   {n.num_inputs()}')
            print(f'Outputs:  {n.num_outputs()}', end='\n\n')

            print("Input nodes:  ")
            for i in range(n.num_inputs()):
                node = n.get_input(i)
                print(get_node_name(node), end=' ')
            print()

            print("Hidden nodes:  ")
            hnode_map = n.get_node_map()
            for node in hnode_map.values():
                if node.is_hidden(): print(get_node_name(node), end=' ')
            print()

            print("Output nodes  ")
            for i in range(n.num_outputs()):
                node = n.get_output(i)
                print(get_node_name(node), end=' ')
            print()
        elif sv[0] == "CLEAR-KP" or sv[0] == "CLEAR_KP":
            n.clear(False)
            node_names.clear()
            lowest_free_id = 0
        elif sv[0] == "CLEAR":
            n.clear(True)
            node_names.clear()
            lowest_free_id = 0
        elif sv[0] == "NM":
            if size == 1: print(node_names)
            else:
                for i in range(1, size):
                    try:
                        if get_node_id_by_name(sv[i], node_names) == -1:
                            raise SRE(f'Node {sv[i]} does not exist')
                        else:
                            print(f'"{sv[i]}": {node_names[sv[i]]}')
                    except Exception as e:
                        print(e)
        elif sv[0] == "AE":
            if (size-1) % 2 != 0 or size < 3:
                print("usage: AE from1 to1 from2 to2 ...")
            else:
                for i in range(int((size-1)/2)):
                    try:
                        fro = get_node_id_by_name(sv[1+i*2], node_names)
                        to = get_node_id_by_name(sv[2+i*2], node_names)
                        if fro == -1 or to == -1:
                            raise SRE(f'{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')
                        
                        n.add_edge(fro, to)
                    except Exception as e:
                        print(e)
        elif sv[0] == "SEP":
            if size < 5 or (size -3) % 2 != 0:
                print("usage: SEP from1 to1 from2 to2 ... name value")
            else:
                try: 
                    d = float(sv[size-1])
                except:
                    print(f'{sv[size-1]} is not a valid property value.')
                    continue
                if (not n.is_edge_property(sv[size-2])):
                        print('Edge property "{sv[size-2]}" doesn\'t exist.')
                else:
                    for i in range(int((size-3)/2)):
                        try:
                            fro = get_node_id_by_name(sv[1+i*2], node_names)
                            to = get_node_id_by_name(sv[2+i*2], node_names)

                            if fro == -1 or to == -1:
                                raise SRE(f'{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')

                            e = n.get_edge(fro, to)
                            e.set(sv[size-2], d)
                        except Exception as e:
                            print(e)
        elif sv[0] == "SEP_ALL":
            if size != 3:
                print("usage: SEP_ALL name value")
            else:
                try:
                    d = float(sv[size-1])
                except:
                    print(f'{sv[size-1]} is not a valid property value.')
                    continue
                if not n.is_edge_property(sv[size-2]):
                    print(f'edge property "{sv[size-2]}" doesn\'t exist')
                else:
                    prop = n.get_edge_property(sv[size-2])
                    edges_map = n.get_edge_map()
                    for edge in edges_map.values():
                        edge.set(prop.index, d)
        elif sv[0] == "SNP":
            if size < 4:
                print("usage: SNP node_id1 node_id2 ... name value")
            else:
                try:
                    d = float(sv[size-1])
                except:
                    print(f'{sv[size-1]} is not a valid property value')
                    continue
                if not n.is_node_property(sv[size-2]):
                    print(f'node property "{sv[size-2]}" doesn\'t exist')
                else:
                    for i in range(1, size-2):
                        try:
                            nid = get_node_id_by_name(sv[i], node_names)
                            if nid == -1: raise SRE(f'{sv[i]} is not a valid node')

                            node = n.get_node(nid)
                            node.set(sv[size-2], d)
                        except Exception as e:
                            print(e)
        elif sv[0] == "SNP_ALL":
            if size != 3:
                print("usage: SNP_ALL name value")
            else:
                try:
                    d = float(sv[size-1])
                except:
                    print(f'{sv[size-1]} is not a valid property value')
                    continue
                if not n.is_node_property(sv[size-2]):
                    print(f'node property "{sv[size-2]}" doesn\'t exist')
                else:
                    prop = n.get_node_property(sv[size-2])
                    node_map = n.get_node_map()
                    for node in node_map.values():
                        node.set(prop.index, d)
        elif sv[0] == "AN":
            if size < 2:
                print("usage: AN node_id1 node_id2")
            else:
                for i in range (1, size):
                    try:
                        if get_node_id_by_name(sv[i], node_names) != -1:
                            raise SRE(f'Node {sv[i]} already exists')

                        if is_number(sv[i]):
                            nid = int(sv[i])
                            if n.is_node(nid): raise SRE(f'Node {sv[i]} already exists')
                            n.add_node(nid)
                            node_names[sv[i]] = nid
                        else:
                            while n.is_node(lowest_free_id): lowest_free_id += 1
                            node_names[sv[i]] = lowest_free_id
                            node = n.add_node(lowest_free_id)
                            node.name = sv[i]
                            lowest_free_id += 1
                    except Exception as e:
                        print(e)
        elif sv[0] == "AI":
            if size < 2:
                print("usage: AI node_id1 node_id2")
            else:
                for i in range(1, size):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                        n.add_input(nid)
                    except Exception as e:
                        print(e)
        elif sv[0] == "AO":
            if size < 2:
                print("usage: AO node_id1 node_id2")
            else:
                for i in range(1, size):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                        n.add_output(nid)
                    except Exception as e:
                        print(e)
        elif sv[0] == "RN":
            if size < 2:
                print("usage: RN node_id1 node_id2 ... [T|F]")
            else:
                if sv[size-1] != "T" and sv[size-1] != "F":
                    sv.append("F")
                    size += 1
                for i in range(1, size-1):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                        n.remove_node(nid, sv[size-1] == "F")
                    except Exception as e:
                        print(e)
        elif sv[0] == "RE":
            if size < 3 or (size-1) % 2 != 0:
                print("usage: RE fromt to1 from2 to2 ...")
            else:
                for i in range(int((size-1)/2)):
                    try:
                        fro = get_node_id_by_name(sv[1+i*2], node_names)
                        to = get_node_id_by_name(sv[2+i*2], node_names)
                        if fro == -1 or to == -1:
                            raise SRE(f'{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')
                        n.remove_edge(fro, to)
                    except Exception as e:
                        print(e)
        elif sv[0] == "RENAME":
            if size != 3:
                print("usage: RENAME from to")
            else:
                try:
                    fro = get_node_id_by_name(sv[1], node_names)
                    to = get_node_id_by_name(sv[2], node_names)
                    if fro == -1 or to == -1:
                        raise SRE(f'{sv[1]} -> {sv[2]} is not a valid edge')
                    n.rename_node(fro, to)
                except Exception as e:
                    print(e)
        elif sv[0] == "RNP":
            if size < 2:
                print("usage: RNP node_id1 node_id2 ... [pname]")
            elif get_node_id_by_name(sv[size-1], node_names) == -1:
                if not n.is_node_property(sv[size-1]):
                    print(f'There is neither a node nor a property name {sv[size-1]}')
                else:
                    for i in range(1, size-1):
                        try:
                            nid = get_node_id_by_name(sv[i], node_names)
                            if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                            node = n.get_node(nid)
                            n.randomize_property(rng, node, sv[size-1])
                        except Exception as e:
                            print(e)
            else:
                for i in range(i, size):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')
                        node = n.get_node(nid)
                        n.randomize_properties(rng, node)
                    except Exception as e:
                        print(e)
        elif sv[0] == "REP":
            if size < 3:
                print("usage: REP from1 to1 from2 to2 ... [pname]")
            elif get_node_id_by_name(sv[size-1], node_names) == -1:
                if not n.is_edge_property(sv[size-1]):
                    print(f'edge property "{sv[size-1]}" doesn\'t exist')
                elif (size-2) % 2 != 0:
                    print("usage: REP from1 to1 from2 to2 ... [pname]")
                else:
                    for i in range(int((size-2)/2)):
                        try:
                            fro = get_node_id_by_name(sv[1+i*2], node_names)
                            to = get_node_id_by_name(sv[2+i*2], node_names)
                            if fro == -1 or to == -1: 
                                raise SRE(f'{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')
                            e = n.get_edge(fro, to)
                            n.randomize_property(rng, e, sv[size-1])
                        except Exception as e:
                            print(e)
            else:
                if (size -1) % 2 != 0:
                    print("usage: REP from1 to1 from2 to2 ... [pname]")
                else:
                    for i in range(int((size-1)/2)):
                        try:
                            fro = get_node_id_by_name(sv[1+i*2], node_names)
                            to = get_node_id_by_name(sv[2+i*2], node_names)
                            if fro == -1 or to == -1: 
                                raise SRE('f{sv[1+i*2]} -> {sv[2+i*2]} is not a valid edge')
                            e = n.get_edge(fro, to)
                            n.randomize_properties(rng, e)
                        except Exception as e:
                            print(e)
        elif sv[0] == "TYPE":
            if size < 2:
                print("usage: TYPE node_id1 node_id2 ...")
            else:
                for i in range(1, size):
                    try:
                        nid = get_node_id_by_name(sv[i], node_names)
                        if nid == -1: raise SRE(f'{sv[i]} is not a valid node')

                        node = n.get_node(nid)
                        if node.is_input() and node.is_output():
                            print(f'{nid} is an input and output node')
                        elif node.is_input():
                            print(f'{nid} is an input node')
                        elif node.is_output():
                            print(f'{nid} is an output node')
                        else:
                            print(f'{nid} is a hidden node')
                    except Exception as e:
                        print(e)
        elif sv[0] == "ASSOC_DATA":
            if size > 2:
                print("usage: ASSOC_DATA [key] - Print the network's associated_data, or just that key")
            elif size == 1:
                keys = n.data_keys()
                for key in keys:
                    j1 = n.get_data(key)
                    print(f' Key {key} : {neuro.pretty_json_helper(j1)}')
            else:
                keys = n.data_keys()
                if sv[1] not in keys: print(f'"{sv[1]}" is not a valid key')
                else:
                    j1 = n.get_data(sv[1])
                    print(j1)
        elif sv[0] == "SET_ASSOC":
            if size < 2:
                print("usage: SET_ASSOC key json - json is filename or json that start on the next line")
            else:
                j1 = read_json(sv, 2)
                if j1 is not None:
                    n.set_data(sv[1], j1)
        elif sv[0] == "SETNAME":
            if size != 3:
                print("usage: SETNAME node_id name - Set the names of node. A name of \"-\" clears the name")
            else:
                try:
                    nid = get_node_id_by_name(sv[1], node_names)
                    if nid == -1: raise SRE(f'{sv[1]} is not a valid node')
                    node = n.get_node(nid)

                    if sv[2] == '-': node.name = ""
                    else: node.name = sv[2]
                except Exception as e:
                    print(e)
        elif sv[0] == "SETCOORDS":
            if size < 2:
                print("usage: SETCOORDS node_id [x] [y] - Set the cordinates of a node. This is used by viz")
            else:
                try:
                    nid = get_node_id_by_name(sv[1], node_names)
                    if nid == -1: raise SRE(f'{sv[1]} is not a valid node')
                    node = n.get_node(nid)

                    dv.clear()
                    for i in range(2, min(4, size)):
                        dv.append(float(sv[i]))
                    node.coordinates = dv
                except Exception as e:
                    print(e)
        elif sv[0] == "CLEAR_VIZ":
            nodes = n.get_node_map()
            for node in nodes.values():
                node.coordinates.clear()
                for outgoing in node.outgoing:
                    outgoing.control_point.clear()
        elif sv[0] == "SET_CP":
            if size < 3:
                print("usage: SET_CP from to [x] [y] - Set the control points of an edge. This is used by viz")
            else:
                try:
                    fro = get_node_id_by_name(sv[1], node_names)
                    to = get_node_id_by_name(sv[2], node_names)
                    if fro == -1 or to == -1: raise SRE(f'{sv[1]} -> {sv[2]} is not a valid edge')

                    e = n.get_edge(fro, to)
                    dv.clear()
                    for i in range(3, min(5, size)):
                        dv.append(float(sv[i]))
                    e.control_point = dv
                except Exception as e:
                    print(e)
        elif sv[0] == "RUN":
            j1 = n.as_json()
            assoc_data = json.loads(str(j1))["Associated_Data"]

            if "app_name" not in assoc_data["other"]:
                print("app name is not specified in the network")
            elif "proc_name" not in assoc_data["other"]:
                print("proc name is not specified in the network")
            else:
                app_name = assoc_data["other"]["app_name"]
                proc_name = assoc_data["other"]["proc_name"]
                cmd = f'cpp-aps/bin/{app_name}_{proc_name} -a test -n NETWORK_TOOL_NETWORK_TEMP.JSON'

                try:
                    with open("NETWORK_TOOL_NETWORK_TEMP.JSON", "w+") as f:
                        f.write(str(j1))
                except:
                    print("Couldn't write to NETWORK_TOOL_NETWORK_TEMP.JSON file for app to run")
                    continue
                print(f'running {app_name}_{proc_name}')

                try:
                    subprocess.run(cmd, shell=True, text=True)
                except:
                    print(f'Failed to run app. Check if the cpp-apps/bin/{app_name}_{proc_name} exists')

                i = subprocess.run("rm -f NETWORK_TOOL_NETWORK_TEMP.JSON", shell=True, text=True)
        else:
            print(f'Invalid command {sv[0]}. Use \'?\' to print a list of commands.')





                


if __name__ == "__main__":
    main()
