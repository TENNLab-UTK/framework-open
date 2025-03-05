import sys
import json
import subprocess
import importlib

import neuro
import risp

SRE = RuntimeError

def print_commands(f):
    f.write("This is a processor tool program. The commands listed below are case-insensitive,\n");
    f.write("For commands that take a json either put a filename on the same line,\n");
    f.write("or the json can be multiple lines, starting on the next line.\n\n");


    f.write("Action commands --\n");

    f.write("MAKE/M proc_name proc_param_json    - Make a new processor with no network\n");
    f.write("LOAD/L network_json                 - Load a network on the processor\n");
    f.write("ML network_json                     - Make a new processor from the network & load the network.\n");

    f.write("AS node_id spike_time spike_val ... - Apply normalized spikes to the network (note: node_id, not input_id)\n");
    f.write("ASV node_id spike_time spike_val .. - Apply unnormalized spikes to the network (note: node_id, not input_id)\n");
    f.write("ASR node_id spike_raster_string     - Apply spike raster to the network (note: node_id, not input_id)\n");
    f.write("RUN simulation_time                 - Run the network for \"simulation_time\" cycles\n");
    f.write("RSC/RUN_SR_CH sim_time [node] [...] - Run, and then print spike raster and charge information in columns\n");
    f.write("CLEAR-A/CA                          - Clear the network's internal state \n");
    f.write("CLEAR/C                             - Remove the network from processor\n");
  

    f.write("\nOutput tracking info commands --\n");
    f.write("OLF [node_id] [...]                 - Output the last fire time for the given output or all outputs\n");
    f.write("OC [node_id] [...]                  - Output the spike count for the given output or all outputs\n");
    f.write("OT/OV [node_id] [...]               - Output the spike times for the given output or all outputs\n");
    f.write("TRACK_O [node_id] [...]             - Track output events for given outputs (empty=all)\n");
    f.write("UNTRACK_O [node_id] [...]           - Untrack output events for given output (empty=all)\n");
  

    f.write("\nNeuron / synapse tracking info commands --\n");
  
    f.write("NLF show_nonfiring(T/F)             - Last fire times for neurons.\n");
    f.write("NC show_nonfiring(T/F)              - Fire counts for neurons.\n");
    f.write("TNC                                 - Total fire count for all neurons.\n");
    f.write("TNA                                 - Total accumulates for all neurons.\n");
    f.write("NT/NV show_nonfiring(T/F)           - All firing times for tracked neurons.\n");
    f.write("GSR [T/F]] [node] [...]             - Print spike raster info for tracked neurons.\n");
    f.write("NCH [node_id] [...]                 - Print the charges (action potentials) of the neurons (empty=all).\n");
    f.write("NLFJ                                - Print the neuron last fire json\n");
    f.write("NCJ                                 - Print the neuron count json\n");
    f.write("NVJ type(V/S)                       - Print the neuron vector json\n");
    f.write("NCHJ                                - Print the neuron charge json\n");

    f.write("TRACK_N [node_id] [...]             - Track neuron events for specified neurons (empty=all)\n");
    f.write("UNTRACK_N [node_id] [...]           - Untrack neuron events for specified neurons (empty=all)\n");
    f.write("SW [from to]                        - Show synapse weights (or just one synapse).\n");
    f.write("PULL_NETWORK file                   - Pull the network off the processor and store in  file.\n");
    f.write("\n");

    f.write("Other info commands --\n");
    f.write("PARAMS [file]                       - Print the JSON that can recreate the processor\n");
    f.write("NP/PPACK                            - Print the PropertyPack that networks use with the processor.\n");
    f.write("NAME                                - Print the processor's name\n");
    f.write("EMPTYNET [file]                     - Create an empty network for this processor\n");
    f.write("PP                                  - Print processor's properties - (not a universal feature).\n");
    f.write("INFO                                - Print the network's node ids and output tracking info\n");
    f.write("PS                                  - Print the spikes we have applied\n");
    f.write("?                                   - Print commands\n");
    f.write("Q                                   - Quit\n");

def node_name(n):
    if n.name == "": return str(n.id)
    return f'{n.id}({n.name})'

def max_node_name_len(net):
    max_name_len = 0
    for n in net.sorted_node_vector:
        max_name_len = max(max_name_len, int(len(node_name(n))))
    return max_name_len

def to_uppercase(s):
    rs = []
    for c in s:
        if 'a' <= c <= 'z':
            rs.append(chr(ord(c)+ord('A')-ord('a')))
        else:
            rs.append(c)
    return ''.join(rs)

def node_validation(n, node_id):
    try:
        nid = int(node_id)
    except:
        print(f'Bad node specification - {node_id}')
    if not n.is_node(nid): raise SRE(f'{node_id} is not a node in the network')
    return nid

def spike_validation(s, n, normalize):
    try:
        if normalize:
            if s.value < -1 or s.value > 1: raise SRE('spike val mst be >= -1 and <= 1')
        if s.time < 0: raise SRE('spike time must be > 0')
        node = n.get_node(s.id)
        if not node.is_input(): raise SRE(f'node {s.id} is not an input node')
        return True
    except Exception as e:
        print(e)
        return False

def output_node_id_validation(node_id, n):
    try:
        if node_id < 0: raise SRE("node_id msut be >0")
        node = n.get_node(node_id)
        if not node.is_output(): raise SRE(f'node {node_id} is not an output node')
        return True
    except Exception as e:
        print(e)
        return False

def network_processor_validation(n, p):
    success = (n is not None and p is not None)

    if not success: print("Processor or network is not loaded")
    return success

def read_json (sv, starting_field):
    if starting_field < len(sv):
        try:
            with open(sv[starting_field], 'r') as f:
                try:
                    rv = json.loads(f.read())
                    return rv
                except:
                    return None
        except Exception as e:
            sys.stderr.write(f'{e} {sv[starting_field]}\n')
    else:
        try:
            rv = json.loads(input().split()[0])
            return rv
        except:
            return None

def load_network(pp, network_json):
    net = neuro.Network()
    net.from_json(network_json)

    p = pp

    if p == None:
        proc_params = net.get_data("proc_params")
        proc_name = net.get_data("other")["proc_name"]

        p_abstract = importlib.import_module(proc_name)
        pt = p_abstract.Processor

        p = pt(proc_params)
        pp = p

    if p.get_network_properties().as_json() != net.get_properties().as_json():
        raise SRE("network and processor properties do not match.")

    if not p.load_network(net): raise SRE("load_network() failed")
    if not neuro.track_all_neuron_events(p, net): raise SRE("track_all_neuron_events() failed.")

    return net, p

def safe_exit(p, n):
    if p is not None: del p
    if n is not None: del n
    sys.exit(0)

def main():

    sv = []
    spikes_array = []
    output_times = []
    all_output_times = []
    neuron_times = []
    spike_strings = []
    v = []
    event_counts = []
    pres = []
    posts = []
    weights = []
    charges = []
    sr = []
    
    gsr_nodes = []

    if len(sys.argv) > 2 or (len(sys.argv) == 2 and sys.argv[1] == "--help"):
        sys.stderr.write("usage: processor_tool [prompt]\n\n")
        print_commands(sys.stderr)
        sys.exit(1)

    if len(sys.argv) == 2:
        prompt = sys.argv[1] + " "
    else: prompt = ""

    p = None
    net = None
    max_name_len = 0

    while 1:
        try:
            if prompt != "": print(f'{prompt}', end='')
            try:
                sv = input().split()
            except:
                safe_exit(p, net)
            if len(sv) != 0: sv[0] = to_uppercase(sv[0])

            size = len(sv)
            
            if len(sv) == 0 or sv[0][0] == "#": pass
            elif sv[0] == "?": print_commands(sys.stdout)
            elif sv[0] == "Q": safe_exit(p, net)
            elif sv[0] == "ML":
                network_json = read_json(sv, 1)
                if network_json == None:
                    print("usage: ML network_json. Bad json")
                    continue
                else:
                    try:
                        if p is not None:
                            del p
                            p = None
                        if net is not None:
                            del net
                            net = None
                        
                        net, p = load_network(p, network_json)
                        max_name_len = max_node_name_len(net)
                    except Exception as e:
                        print(e)
                        if net is not None:
                            del net
                            net = None
                        if p is not None:
                            del p
                            p = None
                        net = None
                        p = None
                    except:
                        print("Unknon error when making processor")
                        if net is not None:
                            del net
                            net = None
                        if p is not None:
                            del p
                            p = None
                        net = None
                        p = None
            elif sv[0] == "MAKE" or sv[0] == "M":
                if size < 2:
                    print("usage: MAKE proc_name processor_params_json")
                else:
                    if p is not None:
                        del p
                        p = None
                    proc_name = sv[1]
                    proc_params = read_json(sv, 2)
                    if proc_params == None:
                        print("usage: MAKE proc_name processor_params_json. Bad json")
                    else:
                        try:
                            p_abstract = importlib.import_module(sv[1])
                            pt = p_abstract.Processor
                            p = pt(proc_params)
                        except Exception as e:
                            print(e)
                        except:
                            print("Unknown error when making processor")
            elif sv[0] == "LOAD" or sv[0] == "L":
                if p is None:
                    print("Must make a processor first")
                else:
                    network_json = read_json(sv, 1)
                    if network_json == None:
                        print("usage: LOAD/L network_json. Bad json")
                    else:
                        if net is not None:
                            del net
                            net = None
                        try:
                            net, p = load_network(p, network_json)
                            max_name_len = max_node_name_len(net)
                        except Exception as e:
                            print(e)
                        except:
                            print("Unknown error when loading a network")
            elif sv[0] == "AS" or sv[0] == "ASV":
                if network_processor_validation(net, p):
                    if size < 2 or (size - 1) % 3 != 0:
                        print(f'usage {sv[0]} node_id spike_time spike_value node_id1 spike_time1 spike_value1 ...')
                    else:
                        normalize = (len(sv[0]) == 2)
                        for i in range(int((size-1)/3)):
                            try:
                                try:
                                    spike_id = int(sv[i*3+1])
                                    spike_time = float(sv[i*3+2])
                                    spike_val = float(sv[i*3+3])
                                except:
                                    print(f'Invalid Spike [{sv[i*3+1]},{sv[i*3+2]},{sv[i*3+3]}]')
                                    continue
                                valid = spike_validation(neuro.Spike(spike_id, spike_time, spike_val), net, normalize)
                                if not valid: continue

                                p.apply_spike(neuro.Spike(net.get_node(spike_id).input_id, spike_time, spike_val), normalize)
                                spikes_array.append(neuro.Spike(spike_id, spike_time, spike_val))
                            except Exception as e:
                                print(e)
            elif sv[0] == "ASR":
                if network_processor_validation(net, p):
                    if size != 3:
                        print("usage: ASR node_id spike_raster_string")
                    else:
                        try:
                            sr.clear()
  
                            for i in range(len(sv[2])):
                                if sv[2][i] != '0' and sv[2][i] != '1':
                                    raise SRE("ASR -- Spike raster string must be only 0's and 1's.")
                                if sv[2][i] == '0':
                                    sr.append(chr(0x00))
                                else:
                                    sr.append(chr(0x01))
                            try:
                                spike_id = int(sv[1])
                            except:
                                print(f'Bad Neuron id: {sv[1]}')
                                continue
                            valid = spike_validation(neuro.Spike(spike_id, 0, 0), net, True)
                            if not valid:
                                continue

                            neuro.apply_spike_raster(p, net.get_node(spike_id).input_id, sr)

                        except Exception as e:
                            print(e)
            elif sv[0] == "PS":
                for i in range(len(spikes_array)):
                    print(f'Spike: [{spikes_array[i].id},{spikes_array[i].time},{spikes_array[i].value}]')
            elif sv[0] == "RUN":
                if network_processor_validation(net, p):
                    if size != 2:
                        print("usage: RUN sim_time. sim_time >= 0.")
                    else:
                        try:
                            sim_time = float(sv[1])
                        except:
                            print("usage: RUN sim_time. sim_time >= 0.")
                            continue
                        p.run(sim_time)
                        spikes_array.clear()
            elif sv[0] == "RUN_SR_CH" or sv[0] == "RSC":
                if network_processor_validation(net, p):
                    if size == 1:
                        print("usage: RSC/RUN_SR_CH sim_time [node] [...]")
                    else:
                        try:
                            sim_time = int(sv[1])
                        except:
                            print("usage: RSC/RUN_SR_CH sim_time [node] [...]")
                        spikes_array.clear()
                        net.make_sorted_node_vector()
                        gsr_nodes.clear()
                        for i in range(2,size):
                            gsr_nodes.append(int(sv[i]))
                        if not gsr_nodes:
                            for i in range(len(net.sorted_node_vector)):
                                gsr_nodes.append(net.sorted_node_vector[i].id)

                        j1 = neuro.run_and_track(sim_time, p)

                        print("Time",end='')
                        for n in net.sorted_node_vector:
                            if n.id in gsr_nodes:
                                print(f' {node_name(n):>{max_name_len}}', end='')
                        print(" |", end='')
                        for n in net.sorted_node_vector:
                            if n.id in gsr_nodes:
                                print(f' {node_name(n):>{max_name_len}}', end='')
                        print()

                        for i in range(len(j1["spike_raster"])):
                            print(f'{int(i):4}',end='')
                            for j in range(len(j1["spike_raster"][i])):
                                n = net.sorted_node_vector[j]
                                if n.id in gsr_nodes:
                                    k = j1["spike_raster"][i][j]
                                    print(f' {"*" if k == 1 else "-":>{max_name_len}}',end='')
                            print(" |", end='')
                            for j in range(len(j1["charges"][i])):
                                n = net.sorted_node_vector[j]
                                if n.id in gsr_nodes:
                                    if j1["charges"][i][j].is_integer(): val = int(j1["charges"][i][j])
                                    else: val = float(j1["charges"][i][j])
                                    print(f' {float(val):>{max_name_len}}', end='')
                            print()
            elif sv[0] == "GT":
                if network_processor_validation(net, p):
                    print(f'time: {p.get_time():.1f}')
            elif sv[0] == "NLF":
                if size == 1 or (sv[1] != 'T' and sv[1] != 'F'):
                    print("usage: NLF show_nonfiring - Last fire times for all neurons. show_nonfiring=T/F")
                elif network_processor_validation(net, p):
                    net.make_sorted_node_vector()
                    output_times = p.neuron_last_fires()
                    if len(output_times) == 0:
                        print(f' Recording last fire times for neurons is not implimented by {p.get_name()}')
                    
                    for i in range(len(output_times)):
                        node = net.sorted_node_vector[i]
                        if output_times[i] != -1.0 or sv[1][0] == 'T':
                            print(f'Node {node_name(node):>{max_name_len}} last fire: {output_times[i]:>.1f}')
            elif sv[0] == "TNC":
                if size != 1:
                    print("usage: TNC - Total fire counts for all neurons.")
                elif network_processor_validation(net, p):
                    print(f'{p.total_neuron_counts()}')
            elif sv[0] == "TNA":
                if size != 1:
                    print("usage: TNA - Total accumulates for all neurons.")
                elif network_processor_validation(net, p):
                    print(f'{p.total_neuron_accumulates()}')
            elif sv[0] == "NC":
                if size == 1 or(sv[1] != 'T' and sv[1] != 'F'):
                    print("usage: NC show_nonfiring - Fire counts for all neurons. show_nonfiring = T/F")
                elif network_processor_validation(net, p):
                    net.make_sorted_node_vector()
                    event_counts = p.neuron_counts()
                    if len(event_counts) == 0:
                        print(f'Recording event counts for neurons is not implemented by {p.get_name()}')
                    for i in range(len(event_counts)):
                        node = net.sorted_node_vector[i]
                        if event_counts[i] == 0 or sv[1][0] == 'T':
                            print(f'Node {node_name(node):{max_name_len}} fire count: {event_counts[i]}')
            elif sv[0] == "NCH":
                if network_processor_validation(net, p):
                    net.make_sorted_node_vector()
                    charges = p.neuron_charges()
                    if len(charges) == 0:
                        print(f'Recording charges for neurons is not implemented by {p.get_name()}')
                    v.clear()
                    for i in range(1, size):
                        v.append(node_validation(net, sv[i]))
                    v.sort()
                    j = 0
                    for i in range(len(charges)):
                        node = net.sorted_node_vector[i]
                        node_id = node.id
                        if len(v) == 0 or (j < len(v) and v[j] == node_id):
                            if charges[i].is_integer():
                                cg = int(charges[i])
                                print(f'Node {node_name(node):>{max_name_len}} charge: {cg}')
                            else:
                                cg = float(charges[i])
                                print(f'Node {node_name(node):>{max_name_len}} charge: {cg:.6}')
                            j += 1
            elif sv[0] == "SW":
                if size != 1 and size != 3:
                    print("usgae: SW [from to] - show weights for all synapses, or just one synapse")
                elif network_processor_validation(net, p):
                    if size == 3:
                        fro = int(sv[1])
                        to = int(sv[2])
                    else:
                        fro = -1
                    pres, posts, weights = p.synapse_weights()

                    for i in range(len(pres)):
                        if fro == -1 or (fro == int(pres[i]) and to == int(posts[i])):
                            print(f'  {pres[i]:4} -> {posts[i]:4} : {weights[i]:7.4f}')
            elif sv[0] == "PULL_NETWORK":
                if size != 2:
                    print("usage: PULL_NETWORK file - pull network from the processor to the file.")
                elif network_processor_validation(net, p):
                    try:
                        with open(sv[1], "w+") as f:
                            pulled = neuro.pull_network(p, net)
                            f.write(str(pulled.as_json()))
                            f.write('\n')
                            del pulled
                    except Exception as e:
                        print(e)
            elif sv[0] == "NV" or sv[0] == "NT":
                if size == 1 or (sv[1] != 'T' and sv[1] != 'F'):
                    print("usage: NV show_nonfiring - Fire counts for all neurons. show_nonfiring=T/F")
                elif network_processor_validation(net, p):
                    net.make_sorted_node_vector()
                    try:
                        neuron_times = p.neuron_vectors()
                        if len(neuron_times) == 0:
                            print(f' Recording events for neurons is not implimented by {p.get_name()}')
                        else:
                            for i in range(len(net.sorted_node_vector)):
                                node_id = net.sorted_node_vector[i].id
                                if len(neuron_times[i]) > 0 or sv[1][0] == 'T':
                                    print(f' Node {node_id:2} fire times:', end='')

                                    for j in range(len(neuron_times[i])):
                                        print(f' {neuron_times[i][j]:.1f}',end='')
                                    print()
                    except SRE as e:
                        print(e)
                    except Exception as e:
                        print("Unknown error")
            elif sv[0] == "OLF":
                if network_processor_validation(net, p):
                    if size == 1:
                        output_times = p.output_last_fires()
                        for i in range(net.num_outputs()):
                            node = net.get_output(i)
                            print(f' node {node_name(node)} last fire time {output_times[i]:.1f}')
                    else:
                        for i in range(1, size):
                            try:
                                try:
                                    node_id = int(sv[i])
                                except Exception:
                                    print(f'{sv[i]} is not a valid node id')
                                    continue
                                if not output_node_id_validation(node_id, net):
                                    continue
                                output_id = net.get_node(node_id).output_id
                                node = net.get_node(node_id)
                                print(f' node {node_name(node)} last fire time {p.output_last_fire(output_id):.1f}')
                            except SRE as e:
                                print(e)
            elif sv[0] == "OC":
                if network_processor_validation(net, p):
                    if size == 1:
                        event_counts = p.output_counts()
                        for i in range(net.num_outputs()):
                            node = net.get_output(i)
                            print(f'node {node_name(node)} spike counts: {event_counts[i]}')
                    else:
                        for i in range(1, size):
                            try:
                                try:
                                    node_id = int(sv[i])
                                except Exception:
                                    print(f'{sv[i]} is not a valid node id')

                                if not output_node_id_validation(node_id, net):
                                    continue
                                output_id = net.get_node(node_id).output_id
                                node = net.get_node(node_id)
                                print(f'node {node_name(node)} spike counts: {p.output_count(output_id)}')
                            except SRE as e:
                                print(e)

            elif sv[0] == "TRACK_O" or sv[0] == "UNTRACK_O":
                if network_processor_validation(net, p):
                    if size == 1:
                        if sv[0][0] == 'T':
                            neuro.track_all_output_events(p, net)
                        else:
                            for i in range(net.num_outputs()): p.track_output_events(i, False)
                    else:
                        for i in range(1, size):
                            try:
                                try:
                                    node_id = int(sv[i])
                                except Exception:
                                    print(f'{sv[i]} is not a valid node id')
                                    continue
                                if not output_node_id_validation(node_id, net): continue

                                output_id = net.get_node(node_id).output_id
                                if not p.track_output_events(output_id, (sv[0][0] =='T')):
                                    buf = f'{output_id}, {sv[0][0] == "T"}) failed.'
                                    raise SRE("track_output_events("+buf)
                            except SRE as e:
                                print(e)
            elif sv[0] == "TRACK_N" or sv[0] == "UNTRACK_N":
                if network_processor_validation(net, p):
                    if size == 1:
                        if sv[0][0] == 'T':
                            neuro.track_all_neuron_events(p, net)
                        else:
                            node_map = net.get_node_map()
                            for node in node_map.values():
                                p.track_neuron_events(node.id, False)
                    else:
                        for i in range(1, size):
                            try:
                                try:
                                    node_id = int(sv[i])
                                except:
                                    print(f'{sv[i]} is not a valid node id')
                                    continue
                                if node_id < 0:
                                    print(f'{sv[i]} is not a valid node id')
                                    continue
                                if not p.track_neuron_events(node_id, (sv[0][0] == 'T')):
                                    buf = f'{node_id}, {(sv[0][0] == "T")}) failed.'
                                    raise SRE("track_neuron_events(" + buf)
                            except SRE as e:
                                print(e)
            elif sv[0] == "OT" or sv[0] == "OV":
                if network_processor_validation(net, p):
                    if size == 1:
                        try:
                            all_output_times = p.output_vectors()
                            if len(all_output_times) == 0:
                                raise SRE("Processor error -- p.output_vectors returned a vector of size zero")
                            for i in range(net.num_outputs()):
                                node = net.get_output(i)
                                print(f'node {node_name(node)} spike times:', end='')
                                for j in range(len(all_output_times[i])):
                                    print(f' {all_output_times[i][j]:.1f}',end='')
                                print()
                        except SRE as e:
                            print(e)
                        except Exception:
                            print("Unknown error")
                    else:
                        for i in range(1, size):
                            try:
                                try:
                                    node_id = int(sv[i])
                                except:
                                    print(f'{sv[i]} is not a valid node id')
                                    continue
                                if not output_node_id_validation(node_id, net):
                                    continue
                                output_id = net.get_node(node_id).output_id
                                output_times = p.output_vector(output_id)
                                node = net.get_node(node_id)
                                print(f'node {node_name(node)} spike times: ', end='')
                                for j in range(len(output_times)):
                                    print(f'{output_times[j]:.1f} ', end='')
                                print()
                            except SRE as e:
                                print(e)
                            except Exception as e:
                                print("Unknown error")
            elif sv[0] == "CA" or sv[0] == "CLEAR-A":
                if network_processor_validation(net, p):
                     p.clear_activity()
            elif sv[0] == "CLEAR" or sv[0] == "C":
                if network_processor_validation(net, p):
                    p.clear()
                    del net
                    net = None
            elif sv[0] == "PP":
                if p == None:
                    print("Must make a processor first")
                else:
                    print(p.get_processor_properties())
            elif sv[0] == "NP" or sv[0] == "PPACK":
                if p == None:
                    print("Must make a processor first")
                else:
                    print(p.get_network_properties().pretty_json())
            elif sv[0] == "NAME":
                if p == None:
                    print("Must make a processor first.")
                else:
                    print(p.get_name())
            elif sv[0] == "EMPTYNET":
                if p == None:
                    print("Must make a processor first")
                else:
                    if size > 1:
                        try:
                            with open(sv[1], "w+") as f:
                                f.write(sv[1])
                        except Exception as e:
                            print(e)
                    if net is not None:
                        del net
                        net = None
                    net = neuro.Network()
                    net.set_properties(p.get_network_properties())
                    j1 = neuro.json()
                    j1["proc_name"] = p.get_name()
                    net.set_data("other", j1)
                    net.set_data("proc_params", p.get_params())
                    if size == 1:
                        print(net.pretty_json())
                    else:
                        try:
                            with open(sv[1], "w+") as f:
                                f.write(str(net.as_json()))
                        except Exception as e:
                            print(e)
            elif sv[0] == "PARAMS":
                if size != 1 and size != 2:
                    print("usage: PARAMS [file]")
                elif p is None:
                    print("Must make a processor first")
                elif size == 2:
                    try:
                        with open(sv[1], "w+") as f:
                            f.write(p.get_params())
                    except Exception as e:
                        print(e)
                else:
                    print(p.get_params().dump(2))
            elif sv[0] == "GSR":
                gsr_nodes.clear()
                gsr_hidden_nodes = True
                if size > 1:
                    if sv[1] == 'T' or sv[1] == 'F':
                        gsr_hidden_nodes = (sv[1] == 'T')
                        i = 2
                    else:
                        i = 1
                    while i < size:
                        gsr_nodes.insert(int(sv[i]))
                        i += 1

                if network_processor_validation(net, p):
                    net.make_sorted_node_vector()
                    neuron_times = p.neuron_vectors()
                    spike_raster = neuro.neuron_vectors_to_json(neuron_times, "S", net)
                    spike_strings.clear()
                    # spike_strings = list(spike_raster["Spikes"])
                    for i in range(len(spike_raster["Spikes"])):
                        spike_strings.append(spike_raster["Spikes"][i])

                    for i in range(len(net.sorted_node_vector)):
                        node = net.sorted_node_vector[i]
                        if len(gsr_nodes) == 0 or node.id not in gsr_nodes:
                            if gsr_hidden_nodes or not node.is_hidden():
                                if node.is_input():
                                    nid = "INPUT "
                                elif node.is_output():
                                    nid = "OUTPUT"
                                else:
                                    nid = "HIDDEN"
                                print(f'{node_name(node):<{max_name_len}} {nid} : {spike_strings[i]}')
            elif sv[0] == "INFO":
                if network_processor_validation(net, p):
                    print("Input nodes:  ",end='')
                    for i in range(net.num_inputs()):
                        print(f'{node_name(net.get_input(i))} ',end='')
                    print()
                    print("Hidden nodes: ",end='')
                    node_map = net.get_node_map()
                    for node in node_map.values():
                        if node.is_hidden():
                            print(f'{node_name(node)} ',end='')
                    print()
                    print("Output nodes: ",end='')
                    for i in range(net.num_outputs()):
                        print(f'{node_name(net.get_output(i))} ',end='')
                    print('\n')
            elif sv[0] == "NCJ":
                if network_processor_validation(net, p):
                    print(neuro.neuron_counts_to_json(p.neuron_counts(), net))
            elif sv[0] == "NCHJ":
                if network_processor_validation(net, p):
                    print(neuro.neuron_charges_to_json(p.neuron_charges(), net))
            elif sv[0] == "NVJ":
                if size != 2 or (sv[1] != 'V' and sv[1] != 'S'):
                    sys.stderr.write("NVJ type=V/S - Print the neuron vector json\n")
                    sys.stderr.write('  V means "Events" key with vector of vector values.\n')
                    sys.stderr.write('  S means "Spikes" key with vector of spikes strings.\n')
                else:
                    try:
                        print(neuro.neuron_vectors_to_json(p.neuron_vectors(), sv[1], net))
                    except SRE as e:
                        print(e)
                    except:
                        print("Unknown Error")
            elif sv[0] == "NLFJ":
                if network_processor_validation(net, p):
                    print(neuro.neuron_last_fires_to_json(p.neuron_last_fires(), net))
            else:
                print(f'Invalid command {sv[0]}. Use \'?\' to print a list of commands')

        except Exception as e:
            print(e)
            sys.exit(0)

if __name__ == "__main__":
    main()
