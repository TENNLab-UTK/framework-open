#include <string>
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include "framework.hpp"
using namespace std;
using namespace neuro;

int main(int argc, char **argv)
{
  string tblr;
  json jin;
  Network net;
  vector <Node *> nodes;
  Node *n;
  Edge *e;
  int i, c, j, windex, to;
  int nn;
  vector <double> x, y, angles;
  double a;
  double pi;
  double clm;       /* 1 for counterclockwise; -1 for clockwise */
  double incr, radius, rfactor;
  double nx, ny, px, py;
  const Property *p;

  unordered_map <int, int> id_to_index;
  unordered_map <int, int>::iterator iit;

  if (argc != 2) {
    fprintf(stderr, "usage: bin/network_to_jgraph tb|lr - network on stdin.\n");
    exit(1);
  }

  tblr = argv[1];
  if (tblr != "tb" && tblr != "lr") {
    fprintf(stderr, "usage: bin/network_to_jgraph tb|lr - network on stdin.\n");
    exit(1);
  }
 
  if (!(cin >> jin)) { fprintf(stderr, "bad JSON\n"); exit(1); }
  net.from_json(jin);
 
  net.make_sorted_node_vector();


  /* Find the inputs and put them into nodes in their proper place.  They should
     be centered around node 0. */

  nodes.resize(net.sorted_node_vector.size(), NULL);


  nn = net.num_inputs();
  nn = -nn/2;
  if (nn < 0) nn += net.sorted_node_vector.size();

  for (i = 0; i < (int) net.sorted_node_vector.size(); i++) {
    n = net.sorted_node_vector[i];
    if (n->is_input()) {
      nodes[nn] = n;
      nn = (nn + 1) % nodes.size();
    }
  }
  
  /* Now do the same with the outputs.  They should be centered around nodes.size()/2. 
     If an output == an input, it's going to look wrong, but it won't be a bug. */

  i = net.num_outputs();
  if (i + net.num_inputs() > (int) nodes.size()) i = nodes.size() - (int) net.num_inputs();
  nn = (int) nodes.size()/2 - i/2;

  /* This shouldn't go into an infinite loop, since i + net.num_inputs() <= nodes.size() */

  while (nodes[nn] != NULL) nn = (nn + 1) % nodes.size();
  while (nodes[(nn+i-1)%nodes.size()] != NULL) nn = (nn + nodes.size() - 1) % nodes.size();
  if (nodes[nn] != NULL) {
    fprintf(stderr, "Couldn't find room for the output nodes.\n");
    exit(1);
  }

  for (i = 0; i < (int) net.sorted_node_vector.size(); i++) {
    n = net.sorted_node_vector[i];
    if (!n->is_input() && n->is_output()) {
      nodes[nn] = n;
      nn = (nn + 1) % nodes.size();
    }
  }
  

  /* Fill in the rest of the nodes. */

  nn = 0;
  for (i = 0; i < (int) net.sorted_node_vector.size(); i++) {
    n = net.sorted_node_vector[i];
    if (n->is_hidden()) {
      while (nodes[nn] != NULL) nn++;
      nodes[nn] = n;
    }
  }

  /* Double-check and make sure that all of the nodes are non-NULL.
     While you're at it, set id_to_index. */

  for (i = 0; i < (int) nodes.size(); i++) {
    if (nodes[i] == NULL) {
      fprintf(stderr, "Internal error -- nodes[%d] is NULL.\n", i);
      exit(1);
    }
    id_to_index[nodes[i]->id] = i;
  }

  /* Now, set up the x/y coordinates of each node.  When it's LR, we'll go counter-clockwise,
     starting at 180 degrees.  When it's TB, we'll go clockwise, starting at 90%.   We'll
     also center depending on the number of inputs.  Odd = node 0 goes at 90/180.  Even =
     node 0 gets adjusted.  */

  pi = acos(-1);

  if (tblr == "lr") {
    a = pi;
    clm = 1;
  } else {
    a = pi/2;
    clm = -1;
  }

  incr = pi * 2 / (double) nodes.size() * clm;
  radius = sin(incr)*0.45;
  if (radius > 0.1) radius = 0.1;
  if (radius < -0.1) radius = -0.1;
  if (radius < 0) radius = -radius;
  rfactor = 1-radius;

  if (net.num_inputs() % 2 == 0) {
    a += (incr/2);
  }

  for (i = 0; i < (int) nodes.size(); i++) {
    angles.push_back(a + incr * (double) i);
    x.push_back(cos(a + incr * (double) i));
    y.push_back(sin(a + incr * (double) i));
  }
 
  printf("newgraph\n");
  printf("xaxis min %lg max %lg size 3 nodraw\n", -1-radius*1.1, 1+radius*1.1);
  printf("yaxis min %lg max %lg size 3 nodraw\n", -1-radius*1.1, 1+radius*1.1);

  printf("\n(* NODE AND LABEL DEFINITIONS *)\n\n");
  
  printf("(* HIDDEN_NODE_DEF *) curve 0 marktype ellipse fill 1 marksize %lg %lg\n", 
          2*radius, 2*radius);
  printf("(* INPUT_NODE_DEF  *) copycurve cfill 1 1 0\n");
  printf("(* OUTPUT_NODE_DEF *) copycurve cfill 1 .5 .5\n");

  printf("(* NODE_LABEL_DEF  *) newstring hjc vjc fontsize 9\n");

  printf("\n(* EDGES *)\n\n");
  p = net.get_edge_property("Weight");
  if (p == NULL) {
    fprintf(stderr, "Error -- no synapse property \"Weight\"\n");
    exit(1);
  }
  windex = p->index;

  for (i = 0; i < (int) nodes.size(); i++) {
    n = nodes[i];
    for (j = 0; j < (int) n->outgoing.size(); j++) {
      e = n->outgoing[j];
      to = id_to_index[e->to->id];
      printf("newline bezier rarrow color %s acfill %s pts ", 
           (e->values[windex] >= 0) ? "0 0 0" : "1 0 0",
           (e->values[windex] >= 0) ? "0 0 0" : "1 0 0");
      if (to == i) {
        nx = cos(angles[i]+0.3);
        ny = sin(angles[i]+0.3);
        px = cos(angles[i]-0.3);
        py = sin(angles[i]-0.3);
        printf(" %lg %lg %lg %lg %lg %lg %lg %lg\n",
               x[i]*rfactor, y[i]*rfactor, nx/2.0, ny/2.0, 
               px/2.0, py/2.0, x[i]*rfactor, y[i]*rfactor);
      } else {
        printf(" %lg %lg %lg %lg %lg %lg %lg %lg\n",
               x[i], y[i], x[i]/2.0, y[i]/2.0, 
               x[to]/2.0, y[to]/2.0, x[to]*rfactor, y[to]*rfactor);
      }
    }
  }

  printf("\n(* NODES *)\n\n");

  for (i = 0; i < (int) x.size(); i++) {
    n = nodes[i];
    if (n->is_input()) {
      c = 1;
    } else if (n->is_output()) {
      c = 2;
    } else {
      c = 0;
    }
    printf("copycurve %d pts %lg %lg\n", c, x[i], y[i]);
  }

  printf("\n(* NODE_LABELS *)\n\n");
  for (i = 0; i < (int) x.size(); i++) {
    n = nodes[i];
    printf("(* LABEL *) copystring 0 x %lg y %lg : %d\n", x[i], y[i], (int) n->id);
  }

  return 0;
}  
