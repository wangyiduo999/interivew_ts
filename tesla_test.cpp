#include <cmath>
#include <climits>
#include <vector>
#include <iostream>     // std::cout
#include <algorithm>    // std::max
#include <string>
#include <fstream>
#include <unordered_map>
#include <locale>         // std::locale, std::isdigit
#include <sstream>        // std::stringstream
#include <queue>


using namespace std;


class DirectedGraphNode {
public:
  string variable;
  int right_sum;
  int in_degree = 0;
  int val;
  vector<DirectedGraphNode *> neighbors;
  vector<DirectedGraphNode *> dependencies;
  DirectedGraphNode(string vr) {
    this->variable = vr;
  }
};

/*It is lazy way to do it. because variables are all alphabet as
  mentioned in the question*/
bool is_number(string& s)
{
  if (s.empty())
    return false;

  if (s[0] >= '0' && s[0] <= '9')
    return true;
  return false;

}

bool is_valid_ops_char(char c) {

  return ((c >= '0' && c <= '9') ||
          (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z')) ||
         (c == '+') ||
         (c == '=');

}

void remove_invalid_char(string &s) {
  string ret;
  for (int i = 0; i < s.size(); i++) {
    char c = s[i];
    if (is_valid_ops_char(c)) {
      ret += c;
    }

  }
  s = ret;
}

vector<string> split(string& equation, string& pattern) {
  string::size_type pos;
  vector<string> items;

  equation += pattern;
  int size = equation.size();

  for (int i = 0; i < size; i++)
  {
    pos = equation.find(pattern, i);
    if (pos < size)
    {
      string s = equation.substr(i, pos - i);
      if (s != " " && (!s.empty())) {
        remove_invalid_char(s);
        items.push_back(s);
      }

      i = pos + pattern.size() - 1;
    }
  }
  return items;
}

void parse_line_and_build_graph(vector<string>& variables,
                                unordered_map<string, DirectedGraphNode*>& variable_graph_map,
                                string& equation) {
  string pattern = " ";
  vector<string> items;

  //get each item
  items = split(equation, pattern);
  if (items.empty())
    return;

  // get this variable from first item
  string variable = items[0];
  variables.push_back(variable);

  // build this items's graph node
  DirectedGraphNode* new_node;
  if (variable_graph_map.find(variable) == variable_graph_map.end()) {
    new_node = new DirectedGraphNode(variable);
    variable_graph_map[variable] = new_node;
  } else {
    new_node = variable_graph_map[variable];
  }

  //build this node's neighbors
  int right_sum = 0;
  int in_degree = 0;
  // skip "="
  for (int i = 2; i < items.size(); i++) {
    if (items[i] == "+")
      continue;

    if (is_number(items[i])) {
      int temp_val;
      stringstream(items[i]) >> temp_val;
      right_sum += temp_val;
    } else {
      in_degree++;
      string neighbor = items[i];
      DirectedGraphNode* new_neighbor_node;
      if (variable_graph_map.find(neighbor) == variable_graph_map.end()) {
        new_neighbor_node = new DirectedGraphNode(neighbor);
        variable_graph_map[neighbor] = new_neighbor_node;
      } else {
        new_neighbor_node = variable_graph_map[neighbor];
      }
      new_neighbor_node->neighbors.push_back(new_node);
      new_node->dependencies.push_back(new_neighbor_node);
    }
  }

  //update the known right sum and in degree of each node
  new_node->right_sum = right_sum;
  new_node->in_degree = in_degree;

}

int topological_sorting(vector<string>& variables,
                        unordered_map<string, DirectedGraphNode*>& variable_graph_map) {

  queue<DirectedGraphNode*> q;

  for (int i = 0; i < variables.size(); i++) {
    string variable = variables[i];
    DirectedGraphNode* variable_node = variable_graph_map[variable];
    if (variable_node->in_degree == 0)
      q.push(variable_node);
  }

  while (!q.empty()) {
    DirectedGraphNode* variable_node = q.front();
    q.pop();

    //update this node's val
    int sum = variable_node->right_sum;

    for (int i = 0; i < variable_node->dependencies.size(); i++) {
      DirectedGraphNode* dependency_node = variable_node->dependencies[i];
      sum += dependency_node->val;
    }
    variable_node->val = sum;

    // decrease one of indegree for each neighbors
    for (int i = 0; i < variable_node->neighbors.size(); i++) {
      DirectedGraphNode* neighbor_node = variable_node->neighbors[i];
      neighbor_node->in_degree--;
      if (neighbor_node->in_degree == 0)
        q.push(neighbor_node);
    }
  }

  /*This is bonus operation because we already assume there is one solution*/
  for (int i = 0; i < variables.size(); i++) {
    string variable = variables[i];
    DirectedGraphNode* variable_node = variable_graph_map[variable];
    if (variable_node->in_degree != 0) {
      cout << variable_node->variable << " in degree is " << variable_node->in_degree << endl;
      return -1;
    }
  }

  return 0;

}


int main(int argc, char *argv[]) {
  string buffer;
  ifstream input_file;
  vector<string> equations;
  vector<string> variables;
  unordered_map<string, DirectedGraphNode*> variable_graph_map;
  string file_name;

  if (argc == 2) {
    file_name = argv[1];
  } else {
    cout << "Please give one valid parameter: input file name" << endl;
    return 0;
  }

  input_file.open(file_name);
  if (! input_file.is_open()) {
    cout << "Error opening file";
    exit (1);
  }
  while (!input_file.eof() )
  {
    getline(input_file, buffer);
    equations.push_back(buffer);
  }

  // show original equations
  cout << "ORIGINAL EQUATIONS:" << endl;
  for (int i = 0; i < equations.size(); i++) {
    cout << equations[i] << endl;
  }


  //build graph
  for (int i = 0; i < equations.size(); i++) {
    parse_line_and_build_graph(variables, variable_graph_map, equations[i]);
  }

  cout << endl;
  cout << "ANSWER:" << endl;

  //do topological sort
  int ret;
  ret = topological_sorting(variables, variable_graph_map);
  if (ret < 0) {
    cout << "No valid answer\n" << endl;
  } else {

    // sort in ascending order
    sort(variables.begin(), variables.end());
    for (int i = 0; i < variables.size(); i++) {
      cout << variables[i] << " = " <<  variable_graph_map[variables[i]]->val << endl;;
    }
  }

  /*there should be graph clean up. But in our case, we exsit diretly,
   *there is no memory leak after return.
   */
  return 0;

}
