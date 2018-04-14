//
// Created by larnal on 15/03/18.
//

#include "Mapper.hpp"

#define OFF (-1)
#define IN_STATION true
#define IN_TRAIN false

void travel::Mapper::read_stations(const std::string &_filename) {

  std::ifstream ifs(_filename, std::ifstream::in); // fichier csv
  std::string line; // ligne d'une station
  std::string word; // élément de station
  std::deque<std::string> liste; // liste des éléments du csv

  // récupère chaque ligne du fichier, ne fait rien de la 1ère
  std::getline(ifs, line);

  while (std::getline(ifs, line)) {
    liste = hash_csv(line);
    if (!liste.empty()) {
      // création station et ajout hashmap
      travel::Station station = {liste.at(0), liste.at(2), liste.at(3), liste.at(4)};
      this->stations_hashmap[std::stoul(liste.at(1))] = station;
      liste.clear();
    }
  }
  ifs.close();
}

void travel::Mapper::read_connections(const std::string &_filename) {

  std::ifstream ifs(_filename, std::ifstream::in); // fichier csv
  std::string line; // ligne d'une station
  std::string word; // élément de station
  std::deque<std::string> liste;  // liste des éléments du csv

  std::getline(ifs, line);

  while (std::getline(ifs, line)) {
    liste = hash_csv(line);

    //remplit une station
    if (!liste.empty()) {
      this->connections_hashmap[std::stoul(liste.at(0))][std::stoul(liste.at(1))] = std::stoul(liste.at(2));
    }
  }

  ifs.close();
}

std::vector<std::pair<uint64_t, uint64_t> > travel::Mapper::compute_travel(const uint64_t _start, const uint64_t _end) {
/* Calcule le chemin le plus court entre _start et _end avec l'algorithme de Dijkstra.
 * _start = name_id station de départ
 * _end = name_id station d'arrivée
 *
 * return = vecteur de pair de chaque étape au format [station de départ] = temps à la prochaine station.
 */

  std::pair<uint64_t, uint64_t> mini;
  std::unordered_map<uint64_t, int64_t> tree; // arbre de Dijkstra
  std::vector<std::pair<uint64_t, uint64_t> > parents; // carte des couples [arrivée] = départ
  uint64_t noeud;
  path.clear();
  change = -1;

  if (_start == _end) {
    path.emplace_back(std::make_pair(_start, 0));
    return path;
  }

// Créer une map contenant tous les noeuds, met les distances à l'infini
  for (auto it : stations_hashmap) {
    tree[it.first] = 99999;
  }
  //Défini le noeud de départ, met sa distance à 0
  noeud = _start;
  tree[noeud] = 0;


  // Boucle algorithme Dijkstra
  while (noeud != _end) {

    // Met les distances aux voisins de noeud à leur nouvelle valeur
    for (auto it_n : connections_hashmap[noeud]) {
      if ((signed int) tree[noeud] + (signed int) it_n.second < (signed int) tree[it_n.first]) {
        tree[it_n.first] = tree[noeud] + it_n.second;
        parents.emplace_back(std::make_pair(it_n.first, noeud)); // enregistre < [arrivée] = départ > dans parents
      }
    }

    // Choisi le noeud avec la plus petite distance depuis _start
    tree[noeud] = OFF; // Ne prend pas en compte le noeud lui-même (élimination)
    mini = {-1, 99999}; // place-holder pour le noeud avec distance la plus petite

    for (auto it_tree : tree) {
      if (tree[it_tree.first] != OFF && (signed int) tree[it_tree.first] < (signed int) mini.second) {
        mini = it_tree;
      }
    }

    noeud = mini.first; // Attribution noeud le plus proche du départ
  } // Fin de Dijkstra

/* Parcours la liste de couples parents[arrivée] = depart et construit res[départ] = temps1  */
  uint64_t target = _end;
  for (auto it_p = parents.rbegin(); it_p != parents.rend(); it_p++) { //depuis _end vers _start
    if (it_p->first == target) { // Si key = arrivée segment
      path.insert(path.begin(), std::make_pair(it_p->second, connections_hashmap[it_p->second][it_p->first]));
      target = it_p->second; // le départ du segment devient l'arrivée du segment précédent
    }
  }
  /* MAJ des temps cumulé du vecteur */
  for (auto it_path = path.begin() + 1; it_path != path.end(); it_path++) {
    it_path->second += (it_path - 1)->second;
  }
  return path;
}

std::vector<std::pair<uint64_t, uint64_t> > travel::Mapper::compute_and_display_travel(const uint64_t _start,
                                                                                       const uint64_t _end) {
/* Affiche dans la console les étapes du trajet calculé. Renvoie un vecteur contenant chaque étape et le temps de chaque étape.
 * _start = name_id station de départ
 * _end = name_id station d'arrivée
 *
 * return = vecteur de paire contenant chaque étape du trajet et le temps de chaque étape :
 *      Une étape = un trajet de train ou un trajet dans la station.
 */
  compute_travel(_start, _end);
  travel_data t;
  t.from = _start;

  // Affichage du trajet demandé
  std::cout << "Best way from " << stations_hashmap[_start].name << " (line " << stations_hashmap[_start].line_id
            << ") to " << stations_hashmap[_end].name << " (line " << stations_hashmap[_start].line_id << ") is:"
            << std::endl;

  auto it = path.begin() + 1; // iterateur qui parcourt le vecteur. Si un seul élément, saute la boucle for

  if (path.size() > 1) { // Si plus d'une étape, vérification du type de transport à la 1ere étape
    t.location = are_same_station(_start, it->first);
  } else { // sinon in_station par défaut
    t.location = IN_STATION;
  }

  for (it = path.begin() + 1; it != path.end(); ++it) { // Parcours du vecteur de stations

    if (are_same_station((it - 1)->first, it->first) && t.location == IN_TRAIN) { // Changement IN_TRAIN > IN_STATION
      t = travel_change_state(t, it - 1);
    } else if (!are_same_station((it - 1)->first, it->first)) {
      t.stations++;
      if (t.location == IN_STATION) { // CHangement IN_STATION > IN_TRAIN
        t = travel_change_state(t, it - 1);
      }
    }
  }

// Affichage pour la dernière ligne
  if (are_same_station((it - 1)->first, _end)) {
    if (t.location == IN_TRAIN) {
      t = travel_change_state(t, it - 1);
    }
    t.to = _end;
    display_travel_message(t, (it - 1)->second - t.time);

  } else {
    t.stations++;
    if (t.location == IN_STATION) {
      t = travel_change_state(t, it - 1);
    }
    t.to = _end;
    display_travel_message(t, (it - 1)->second - t.time);
  }

  //Affichage temps total + nombre de changement
  if (t.change < 0) { t.change = 0; }
  std::cout << "After " << time_converter((it - 1)->second) << " and " << t.change
            << (t.change > 1 ? " changes," : " change,")
            << " you have reached your destination!" << std::endl << std::endl;

  change = t.change;
  return path;
}

void travel::Mapper::display_travel(std::string &_start, std::string &_end) {
  /* Trouve le plus court chemin à partir du nom (string) des stations de départ et d'arrivée*/

  uint64_t start;
  uint64_t end;
  try {
    start = get_id(_start);
    end = get_id(_end);
  }

  catch (std::out_of_range &error) {
    throw;
  }

  compute_and_display_travel(start, end);
}

std::deque<std::string> travel::Mapper::hash_csv(const std::string &_line) {
/* découpe le csv en mot séparé par une virgule et renvoie une queue des mots
 * _line = ligne de csv
 *
 * return = deque des mots.
 */
  std::string word; // élément de station
  std::deque<std::string> liste;  // liste des éléments du csv

  // découpe la ligne pour chaque délimiteur ','
  std::stringstream str(_line);
  // boucle pour chaque élément de la ligne
  while (std::getline(str, word, ',')) {
    //remplit la liste ave les élements
    liste.push_back(word);
  }
  return liste;
}

std::string travel::Mapper::time_converter(const uint64_t _time) const {
  /* Affiche l'heure au format h, min, s.
   * time = heure en seconde
   *
   * output = string de l'heure.
   */

  auto hour = (int) (_time / 3600);
  auto min = _time % 3600;

  min = (min / 60);
  auto sec = min % 60;
  return (hour > 0 ? std::to_string(hour) + " h, " : "") + (min > 0 ? std::to_string(min) + " min, " : "")
      + std::to_string(sec) + " sec";

}

bool travel::Mapper::are_same_station(const uint64_t _a, const uint64_t _b) {
  /* Renvoie true si les 2 stations sont dans la même station réelle.
   * input = name.id des stations.
   *
   * output = bool.
   */
  return stations_hashmap[_a].name == stations_hashmap[_b].name;
}

travel::Mapper::travel_data &travel::Mapper::travel_change_state(struct travel_data &t,
                                                                 const std::vector<std::pair<uint64_t,
                                                                                             uint64_t> >::iterator &it) {
/* Affiche le message de trajet, en train ou dans la station, met à jour les données de voyage.
 * travel_data = données de trajet
 * it = iterateur de la fonction display_and_compute_travel
 * location = dernier type de trajet (en station ou en train)
 *
 * output = travel_data
 */

  t.to = it->first;
  display_travel_message(t, it->second - t.time);

  if (t.location == IN_TRAIN) {
    t.change++;
    t.stations = 0;
  }

  t.from = it->first;
  t.time = it->second;
  t.location = !t.location;

  return t;
}

void travel::Mapper::display_travel_message(travel_data &t, const uint64_t &it) {

  if (it > 0) {
    if (t.location == IN_STATION) {
      std::cout << "Walk to " << stations_hashmap[t.to].name << ", line " << stations_hashmap[t.to].line_id << " ("
                << time_converter(it) << ")" << std::endl;

    } else {
      std::cout << "    <Take line " << stations_hashmap[t.from].line_id << "> " << stations_hashmap[t.from].line_name
                << std::endl;
      std::cout << "        From " << stations_hashmap[t.from].name << " to " << stations_hashmap[t.to].name << " ["
                << t.stations << (t.stations > 1 ? " stations] (" : " station] (") << time_converter(it) << ")" << std::endl;
    }
  }
}

uint64_t travel::Mapper::get_id(std::string name) {

  name = str_simplify(name);
  for (auto it : stations_hashmap) {
    if (name.at(0) == tolower(it.second.name.at(0))) { // Premier charactere identique
      std::string station = str_simplify(it.second.name);

      if (name.compare(0, name.length() - 1, station, 0, name.length() - 1) == 0) {
        return it.first;
      }
    }
  }
  throw std::out_of_range("Station \"" + name + "\" not found.");

}

std::string &travel::Mapper::str_simplify(std::string &str) {
  std::unordered_map<char, char> special_char
      ({{'é', 'e'}, {'è', 'e'}, {'ê', 'e'}, {'ë', 'e'}, {'à', 'a'}, {'â', 'a'}, {'ä', 'a'}, {'ô', 'o'}, {'ö', 'o'},
        {'ù', 'u'}, {'û', 'u'}});

  for (std::string::iterator c = str.begin(); c != str.end();) {
    *c = (char) tolower(*c);
    if (!(*c >= 'a' && *c <= 'z')) {
      auto it = special_char.find(*c);
      if (it != special_char.end()) {
        *c = it->second;
      } else {
        str.erase(std::remove(str.begin(), str.end(), *c), str.end());
        c = str.begin();
      }
    } else {
      c++;
    }
  }
  if (str.compare(0, 5, "saint") == 0) {
    str.erase(1, 3);
  }
  return str;
}
uint64_t travel::Mapper::get_time() const {
  if (path.empty()) {
    throw std::out_of_range(
        "path not defined. Please compute a path using Mapper::display_travel() or Mapper::compute_and_display_travel() methods.");
  } else {
    return path.back().second;
  }
}

int travel::Mapper::get_change() const {
  if (change == -1) {
    throw std::out_of_range(
        "change not defined. Please compute a path using Mapper::display_travel() or Mapper::compute_and_display_travel() methods.");
  } else {
    return change;
  }

}
