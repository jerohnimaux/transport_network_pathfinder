#include <cstdint>
#include <stdint-gcc.h>
#include "Mapper.hpp"

using namespace std;
int main() {
  travel::Mapper parser;
  parser.read_stations("./data/s.csv");
  parser.read_connections("./data/c.csv");

  string a;
  string b;

  while (true) {
    cout << "D'où venez-vous ?" << endl;
    getline(cin, a);
    if (a == "quit" || a == "exit") break;

    cout << "Où allez-vous ?" << endl;
    getline(cin, b);
    if (b == "quit" || b == "exit") break;

    try {
      parser.display_travel(a, b);
    }

    catch (out_of_range &error) {
      cout << "error" << endl;
    }
  }

  return 0;
}