// BLAISE	p1103800
// DUMAS	p1100740

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <cassert>
#include <utility>
#include "../lib/tinyxml2.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

const unsigned int ASCII_A = 97;
const unsigned int ASCII_Z = ASCII_A + 26;
const bool         DEBUG = false;

typedef size_t                            etat_t;
typedef unsigned char                     symb_t;
typedef set< etat_t >                     etatset_t;
typedef vector< vector< etatset_t > >     trans_t;
typedef vector< etatset_t >               epsilon_t;
typedef map< etatset_t, etat_t >          map_t;


////////////////////////////////////////////////////////////////////////////////

struct sAutoNDE{
  // caractéristiques
  size_t nb_etats;
  size_t nb_symbs;
  size_t nb_finaux;

  etat_t initial;
  // état initial

  etatset_t finaux;
  // états finaux : finaux_t peut être un int*, un tableau dynamique comme vector<int>
  // ou une autre structure de donnée de votre choix.

  trans_t trans;
  // matrice de transition : trans_t peut être un int***, une structure dynamique 3D comme vector< vector< set<int> > >
  // ou une autre structure de donnée de votre choix.
  
  epsilon_t epsilon; 
  // transitions spontanées : epsilon_t peut être un int**, une structure dynamique 2D comme vector< set<int> >
  // ou une autre structure de donnée de votre choix.
};

////////////////////////////////////////////////////////////////////////////////

bool FromFileTxt(sAutoNDE& at, string path);
bool FromFileJff(sAutoNDE& at, string path);

bool FromFile(sAutoNDE& at, string path){

  string extension;
  if (path.find_last_of(".") != std::string::npos)
    extension = path.substr(path.find_last_of(".")+1);
  else
    extension = "";

  if (extension == "txt")
    return FromFileTxt(at,path);
  if (extension == "jff")
    return FromFileJff(at,path);

  cout << "extension de fichier non reconnue\n";
  return false;
}

bool FromFileTxt(sAutoNDE& at, string path){

  ifstream myfile(path.c_str(), ios::in); 
  //un flux d'entree obtenu à partir du nom du fichier
  string line;
  // un ligne lue dans le fichier avec getline(myfile,line);
  istringstream iss;
  // flux associé à la chaine, pour lire morceau par morceau avec >> (comme cin)
  etat_t s(0), t(0);
  // deux états temporaires
  symb_t a(0);
  // un symbole temporaire

  if (myfile.is_open()){
    // la première ligne donne 'nb_etats nb_symbs nb_finaux'
    do{ 
      getline(myfile,line);
    } while (line.empty() || line[0]=='#');
    // on autorise les lignes de commentaires : celles qui commencent par '#'
    iss.str(line);
    if((iss >> at.nb_etats).fail() || (iss >> at.nb_symbs).fail() || (iss >> at.nb_finaux).fail())
        return false;
    // la deuxième ligne donne l'état initial
    do{ 
      getline (myfile,line);
    } while (line.empty() || line[0]=='#');    
    iss.clear();
    iss.str(line);
    if((iss >> at.initial).fail())
      return -1;
    
    // les autres lignes donnent les états finaux
    for(size_t i = 0; i < at.nb_finaux; i++){
        do{ 
          getline (myfile,line);
        } while (line.empty() || line[0]=='#');
        iss.clear();
        iss.str(line);
         if((iss >> s).fail())
          continue;
//        cerr << "s= " << s << endl;
        at.finaux.insert(s);
    }     

    // on alloue les vectors à la taille connue à l'avance pour éviter les resize dynamiques
    at.epsilon.resize(at.nb_etats);
    at.trans.resize(at.nb_etats);
    for(size_t i=0;i<at.nb_etats;++i)
      at.trans[i].resize(at.nb_symbs);   

  // lecture de la relation de transition 
    while(myfile.good()){
      line.clear();
      getline (myfile,line);
      if (line.empty() && line[0]=='#')
        continue;
      iss.clear();
      iss.str(line); 

      // si une des trois lectures echoue, on passe à la suite
      if((iss >> s).fail() || (iss >> a).fail() || (iss >> t).fail() || (a< ASCII_A ) || (a> ASCII_Z ))
        continue; 
              
      //test espilon ou non
      if ((a-ASCII_A) >= at.nb_symbs){
        //cerr << "s=" << s<< ", (e), t=" << t << endl;
        at.epsilon[s].insert(t);
      }
      else{
        //cerr << "s=" << s<< ", a=" << a-ASCII_A << ", t=" << t << endl;
        at.trans[s][a-ASCII_A].insert(t);
      }
    }
    myfile.close();
    return true; 
 }
  return false;
  // on ne peut pas ouvrir le fichier
}


// -----------------------------------------------------------------------------
// Fonctions à compléter pour la première partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

bool FromFileJff(sAutoNDE& at, string path){
  at.nb_etats = 0;
  at.nb_finaux = 0;
  at.nb_symbs = 0;
  using namespace tinyxml2;
  XMLDocument doc;
  if(doc.LoadFile(path.c_str()) != 0)
  {
  	return false;
  }
  XMLElement* e = doc.RootElement()->FirstChildElement("state");
  
  while(e != NULL)
  {
  	at.nb_etats++;
  	XMLElement* state = e->FirstChildElement("initial");
  	int id;
  	e->QueryIntAttribute("id", &id);
  	if(state!=NULL)
  	{
  		at.initial = id;
  	}
  	state = e->FirstChildElement("final");
  	if(state!=NULL)
  	{
  		at.nb_finaux++;
  		at.finaux.insert(id);
  	}
  	e = e->NextSiblingElement("state");
  }
  at.epsilon.resize(at.nb_etats);
  at.trans.resize(at.nb_etats);
  
  
  e = doc.RootElement()->FirstChildElement("transition");
  while(e != NULL)
  {
  	XMLElement* trans;
  	int i, j;
  	const char* c;
  	trans = e->FirstChildElement("from");
  	if(trans!=NULL) trans->QueryIntText(&i);
  	trans = e->FirstChildElement("to");
  	if(trans!=NULL) trans->QueryIntText(&j);
  	trans = e->FirstChildElement("read");
  	if(trans!=NULL) c = trans->GetText();
  	if(c == NULL)
  	{
  		at.epsilon[i].insert(j);
  	}
  	else
  	{
  		char car = c[0];
  		if(car-ASCII_A+1 > at.nb_symbs)
  		{
  			at.nb_symbs = car-ASCII_A+1;
  			for(size_t i=0;i<at.nb_etats;++i)
      				at.trans[i].resize(at.nb_symbs);
  		}
  		at.trans[i][car-ASCII_A].insert(j);
  	}
  	e = e->NextSiblingElement("transition");
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ContientFinal(const sAutoNDE& at,const etatset_t& e){
	
	auto d = e.cbegin();
	auto f = e.cend();
	
	auto fat = at.finaux.cend();
	
	while (d != f) {
		
		auto dat = at.finaux.cbegin();
		while (dat != fat) {
			if(*dat == *d ){
				return true;
			}
			dat++;
		}
	  d++;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool EstDeterministe(const sAutoNDE& at){
  //Y a-t-il des ε transitions?
  for(unsigned int i = 0; i < at.epsilon.size(); i++)
  {
  	if(!at.epsilon[i].empty()) return false;
  }
  //Pour chaque etat, il existe une et une seule transition par lettre de l'alphabet
  for(unsigned int i = 0; i < at.trans.size(); i++)
  {
  	for(unsigned int j = 0; j < at.trans[i].size(); j++)
  	{
  		if(at.trans[i][j].size() != 1) return false;
  	}
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void Fermeture(const sAutoNDE& at, etatset_t& e){
  // Cette fonction clot l'ensemble d'états E={e_0, e_1, ... ,e_n} passé en
  // paramètre avec les epsilon transitions
  bool changement = true;
  while(changement)
  {
  	changement = false;
  	for(auto it = e.begin(); it != e.end(); ++it)
  	{
  		for(auto it2 = at.epsilon[*it].begin(); it2 != at.epsilon[*it].end(); ++it2)
  		{
  			if(e.count(*it2) == 0)
  			{
  				e.insert(*it2);
	  			if(*it2<*it) changement = true;
  			}
	  	}
 	 }
  }
}

////////////////////////////////////////////////////////////////////////////////

etatset_t Delta(const sAutoNDE& at, const etatset_t& e, symb_t c){
  //sur la base de celle pour le cas sans transitions spontanées,
  // définir cette fonction en utilisant Fermeture
	etatset_t f = e;
	etatset_t g;
	Fermeture(at, f);
	for(auto it = f.begin(); it != f.end(); ++it)
	{
		for(auto it2 = at.trans[*it][c-ASCII_A].begin(); it2 != at.trans[*it][c-ASCII_A].end(); ++it2)
		{
			g.insert(*it2);
		}
	}
	Fermeture(at, g);
  	return g;
}

////////////////////////////////////////////////////////////////////////////////

bool Accept(const sAutoNDE& at, string str){
  etatset_t et;
  et.insert(at.initial);
  for(unsigned int i = 0; i < str.size(); i++)
  {
  	et = Delta(at, et, str[i]);
  }
  Fermeture(at, et);
  return ContientFinal(at, et);
}

////////////////////////////////////////////////////////////////////////////////

bool operator==(etatset_t a, etatset_t b)
{
	if(a.size() != b.size())
	{
		return false;
	}
	else
	{
		auto it1 = a.begin();
		auto it2 = b.begin();
		while(it1 != a.end())
		{
			if(*it1 != *it2)
			{
				return false;
			} else {
				it1++;
				it2++;
			}
		}
	}
	return true;
}

bool Contient(vector<etatset_t> v, etatset_t e)
{
	for(unsigned int i = 0; i < v.size(); i++)
	{
		if(v[i] == e) return true;
	}
	return false;
}

int Find(vector<etatset_t> v, etatset_t e)
{
	for(unsigned int i = 0; i < v.size(); i++)
	{
		if(v[i] == e) return i;
	}
	return -1;
}

sAutoNDE Determinize(const sAutoNDE& at){
  sAutoNDE r;
  r.nb_symbs = at.nb_symbs;
  r.nb_etats = 1;
  r.nb_finaux = 0;
  r.initial = at.initial;
  vector<etatset_t> etats;
  etatset_t etatC;
  etatC.insert(at.initial);
  Fermeture(at, etatC);
  if(ContientFinal(at, etatC))
  {
  	r.nb_finaux++;
  	r.finaux.insert(0);
  }
  etats.push_back(etatC);
  r.trans.resize(r.nb_etats);
  r.trans[0].resize(r.nb_symbs);
  for(unsigned int i = 0; i < etats.size(); i++)
  {
  	for(unsigned int j = 0; j < r.nb_symbs; j++)
  	{
  		//char c = j+ASCII_A;
  		etatC = Delta(at, etats[i], j+ASCII_A);
  		if(!Contient(etats, etatC))
  		{
  			etats.push_back(etatC);
  			r.nb_etats++;
  			if(ContientFinal(at, etatC))
  			{
  				r.nb_finaux++;
  				r.finaux.insert(etats.size()-1);	
  			}
  			r.trans.resize(r.nb_etats);
  			r.trans[r.nb_etats-1].resize(r.nb_symbs);
  			r.trans[i][j].insert(r.nb_etats-1);
  		} else {
  			r.trans[i][j].insert(Find(etats, etatC));
  		}
  	}
  }
  r.epsilon.resize(r.nb_etats);
  return r;
}


// -----------------------------------------------------------------------------
// Fonctions à compléter pour la seconde partie du projet
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& out, const sAutoNDE& at){
  if(EstDeterministe(at) == true)
  {
  	out << "#Cet automate est deterministe" << endl;
  }
  else
  {
  	out << "#Cet automate n'est pas deterministe" << endl;
  }
  out << at.nb_etats << " " << at.nb_symbs << " " << at.nb_finaux << endl;
  for(auto i = at.finaux.begin(); i != at.finaux.end(); ++i)
  {
  	out << *i << " ";
  }
  out << endl;
  for(unsigned int i = 0; i < at.trans.size(); i++)
  {
  	for(unsigned int j = 0; j < at.trans[i].size(); j++)
  	{
  		for(auto it = at.trans[i][j].begin(); it != at.trans[i][j].end(); ++it)
  		{
  			char c = j+ASCII_A;
  			out << i << " " << c << " " << *it << endl;
  		}
  	}
  }
  for(unsigned int i = 0; i < at.epsilon.size(); i++)
  {
  	for(auto it = at.epsilon[i].begin(); it != at.epsilon[i].end(); ++it)
  	{
  		out << i << " e " << *it << endl;;
  	}
  }
  
  return out;
}


////////////////////////////////////////////////////////////////////////////////

bool ToGraph(sAutoNDE& at, string path){
	int t = path.size();
  if(path[t-3]  != '.' && path[t-2] != 'g' && path[t-1] != 'v') path+=".gv";
  ofstream fichier(path, ios::out | ios::app);
  if(fichier)
  {
  	fichier << "digraph finite_state_machine {" << endl;
  	fichier << "rankdir = LR;" << endl;
  	fichier  << "size=\"10,10\"" << endl;
  	
  	fichier  << "node [shape = doublecircle];";
  	for(auto i = at.finaux.begin(); i != at.finaux.end(); i++)
  	{
  		fichier << " " << *i;
  	}
  	fichier << ";" << endl;
  	fichier  << "node [shape = point]; q;" << endl;
  	fichier  << "node [shape = circle];" << endl;
  	
  	fichier  << "q -> " << at.initial << ";" << endl;
  	for(unsigned int i = 0; i < at.nb_etats; i++)
  	{
  		for(unsigned int j = 0; j < at.nb_symbs; j++)
  		{
  			char c = j+ASCII_A;
  			for(auto it = at.trans[i][j].begin(); it != at.trans[i][j].end(); it++)
  			{
  				fichier  << i << " -> " << *it << " [label = \"" << c << "\"];" << endl;
  			}
  		}
  	}
  	
  	for(unsigned int i = 0; i < at.epsilon.size(); i++)
  	{
  		for(auto it = at.epsilon[i].begin(); it != at.epsilon[i].end(); it++)
  		{
  			fichier  << i << " -> " << *it << " [label =\"ε\"];" << endl;
  		}
  	}
  	fichier << "}" << endl;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool ToJflap(sAutoNDE& at, string path){
	using namespace tinyxml2;
	
	
	
	XMLDocument doc;
	XMLNode * structure = doc.NewElement("structure");
	doc.InsertFirstChild(structure);
	
	XMLElement * type = doc.NewElement( "type" );
	type->SetText("fa");
	structure->InsertEndChild(type);
	
	
	//~ Etats
	for(unsigned int i = 0;i<at.nb_etats;i++){
		XMLElement* state = doc.NewElement("state");
		state->SetAttribute("id", i);
		if(at.initial == i){
			XMLElement* ini = doc.NewElement("initial");
			state->InsertEndChild(ini);
		}
		
		for(auto it = at.finaux.begin();it != at.finaux.end();it++){
			if(i==*it){
				XMLElement* final_ = doc.NewElement("final");
				state->InsertEndChild(final_);
			}
		}
		structure->InsertEndChild(state);
	}
	
	//~ Trans
	for(unsigned int et_ini = 0;et_ini < at.nb_etats;et_ini++){
		for(unsigned int char_lu = 0;char_lu < at.nb_symbs;char_lu++){
			for(auto it = at.trans[et_ini][char_lu].begin();it != at.trans[et_ini][char_lu].end();it++){
				cout << et_ini << " " << char_lu << " " << *it << endl;
				XMLElement* trans = doc.NewElement("transition");
				XMLElement* from = doc.NewElement("from");
				XMLElement* to = doc.NewElement("to");
				XMLElement* read = doc.NewElement("read");
				
				char* c = (char*)malloc(2*sizeof(char));
				c[0] = (ASCII_A + char_lu);
				c[1] = '\0';
				from->SetText(et_ini);
				read->SetText(c);
				to->SetText(*it);
				
				trans->InsertEndChild(from);
				trans->InsertEndChild(to);
				trans->InsertEndChild(read);
				structure->InsertEndChild(trans);
			}
		}
	}
	
	//~ Epsilon-trans
	for(unsigned int et_ini = 0;et_ini < at.nb_etats;et_ini++){
		for(auto it = at.epsilon[et_ini].begin();it != at.epsilon[et_ini].end();it++){
			XMLElement* epsilont = doc.NewElement("transition");
			XMLElement* from = doc.NewElement("from");
			XMLElement* to = doc.NewElement("to");
			
			from->SetText(et_ini);
			to->SetText(*it);
			
			epsilont->InsertEndChild(from);
			epsilont->InsertEndChild(to);
			structure->InsertEndChild(epsilont);
		}
	}
	
	
	char* buffer = (char*)malloc((path.size()+1) * sizeof(char));
	size_t length = path.copy(buffer, path.size());
	buffer[length]='\0';
	
	
	
	doc.SaveFile(buffer);
	
	
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// fonction outil : on garde x, et on "ajoute" trans et epsilon de y
// en renommant ses états, id et en décallant les indices des états de y
// de x.nb_etats 
sAutoNDE Append(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction
  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r;
  r.nb_etats = (x.nb_etats + y.nb_etats);
  r.nb_symbs = x.nb_symbs;
  r.nb_finaux = (x.nb_finaux + y.nb_finaux);
  for(auto it = x.finaux.begin();it!=x.finaux.end();it++){
	  r.finaux.insert(*it);
  }
  
  for(auto it = y.finaux.begin();it!=y.finaux.end();it++){
	  r.finaux.insert((*it) + x.nb_etats);
  }
  /*
   * Redimensionner les transitions de r
   */
  r.trans.resize(r.nb_etats);
  for(unsigned int i = 0;i<r.trans.size();i++){
	  r.trans[i].resize(r.nb_symbs);
  }
  /*
   * Insertion des transitions de x et y
   */
  for(unsigned int i = 0; i < x.nb_etats; i++){
	  for(unsigned int j = 0; j < x.nb_symbs; j++){
		  for(auto it = x.trans[i][j].begin(); it != x.trans[i][j].end(); it++){
			  r.trans[i][j].insert(*it);
		  }
	  }
  }
  for(unsigned int i = 0; i < y.nb_etats; i++){
	  for(unsigned int j = 0; j < y.nb_symbs; j++){
		  for(auto it = y.trans[i][j].begin(); it != y.trans[i][j].end(); it++){
			  r.trans[i + x.nb_etats][j].insert((*it) + x.nb_etats);
		  }
	  }
  }
  //~ r.epsilon
  r.epsilon.resize(x.nb_etats + y.nb_etats);
  
  for(unsigned int i = 0;i<x.nb_etats;i++){
	  for(auto it = x.epsilon[i].begin();it != x.epsilon[i].end(); it++){
		  r.epsilon[i].insert(*it);
	  }
  }
  
  for(unsigned int i = 0;i<y.nb_etats;i++){
	  for(auto it = y.epsilon[i].begin();it != y.epsilon[i].end(); it++){
		  r.epsilon[i + x.nb_etats].insert((*it) + x.nb_etats);
	  }
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Union(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r = Append(x, y);
  r.nb_etats++;
  r.trans.resize(r.nb_etats);
  r.trans[r.nb_etats-1].resize(r.nb_symbs);
  r.epsilon.resize(r.nb_etats);
  r.epsilon[r.nb_etats-1].insert(x.initial);
  r.epsilon[r.nb_etats-1].insert(y.initial + x.nb_etats);
  r.initial = r.nb_etats-1;
  return r;
}

////////////////////////////////////////////////////////////////////////////////


sAutoNDE Concat(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction

  assert(x.nb_symbs == y.nb_symbs);
  sAutoNDE r = Append(x, y);
  r.initial = x.initial;
  r.finaux.clear();
  for(auto it = y.finaux.begin(); it != y.finaux.end(); it++)
  {
  	r.finaux.insert((*it) + x.nb_etats);
  }
  r.nb_finaux = y.nb_finaux;
  for(auto it = x.finaux.begin(); it != x.finaux.end(); ++it)
  {
  	r.epsilon[*it].insert(y.initial + x.nb_etats);
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////

sAutoNDE Complement(const sAutoNDE& x){
  //TODO définir cette fonction
  //etatset_t finaux;
  sAutoNDE y;
  y.nb_finaux = 0;
  y.nb_etats = 0;
  y.nb_symbs = x.nb_symbs;
  y = Append(x, y);
  y.initial = x.initial;
  for(etat_t i = 0; i < x.nb_etats; i++)
  {
  	if(x.finaux.count(i) == 0)
  	{
  		y.finaux.insert(i);
  	} else {
  		y.finaux.erase(i);
  	}
  }
  return y;
}


////////////////////////////////////////////////////////////////////////////////

sAutoNDE Kleene(const sAutoNDE& x){
  //TODO définir cette fonction
  sAutoNDE y;
  y.nb_finaux = 0;
  y.nb_etats = 0;
  y.nb_symbs = x.nb_symbs;
  y = Append(x, y);
  y.nb_etats++;
  y.trans.resize(y.nb_etats);
  for(unsigned int i = 0; i < y.nb_etats; i++)
  {
  	y.trans[i].resize(y.nb_symbs);
  }
  y.epsilon.resize(y.nb_etats);
  y.initial = y.nb_etats-1;
  for(auto it = x.finaux.begin(); it != x.finaux.end(); ++it)
  {
  	y.epsilon[*it].insert(y.initial);
  }
  y.epsilon[y.initial].insert(x.initial);
  return y;
}

////////////////////////////////////////////////////////////////////////////////

// Intersection avec la loi de De Morgan
sAutoNDE Intersection(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction
  return Complement(Union(Complement(x), Complement(y)));
}

////////////////////////////////////////////////////////////////////////////////

struct Pair{
	unsigned int first;
	unsigned int second;
};

bool operator==(Pair p1, Pair p2)
{
	return (p1.first == p2.first && p1.second == p2.second);
}

int Find(vector<Pair> v, Pair e)
{
	for(unsigned int i = 0; i < v.size(); i++)
	{
		if(v[i] == e) return i;
	}
	return -1;
}

// Intersection avec l'automate produit
sAutoNDE Produit(const sAutoNDE& x, const sAutoNDE& y){
  //TODO définir cette fonction
  sAutoNDE r;
  r.nb_etats = x.nb_etats * y.nb_etats;
  r.nb_symbs = x.nb_symbs;
  r.nb_finaux = x.nb_finaux * y.nb_finaux;
  r.trans.resize(r.nb_etats);
  for(unsigned int i = 0; i < r.nb_etats; i++)
  {
  	r.trans[i].resize(r.nb_symbs);
  }
  Pair p1, p2;
  vector<Pair> etats;
  for(unsigned int i = 0; i < x.nb_etats; i++)
  {
  	for(unsigned int j = 0; j < y.nb_etats; j++)
  	{
  		p1.first = i;
  		p1.second = j;
  		etats.push_back(p1);
  	}
  }
  for(unsigned int i = 0; i < etats.size(); i++)
  {
  	p1.first = etats[i].first;
  	p1.second = etats[i].second;
  	if(p1.first == x.initial && p1.second == y.initial)
  	{
  		r.initial = i;
  	}
  	if(x.finaux.count(p1.first) && y.finaux.count(p1.second))
  	{
  		r.finaux.insert(i);
  	}
  	for(unsigned int c = 0; c < r.nb_symbs; c++)
  	{
  		if(!x.trans[p1.first][c].empty() && !y.trans[p1.second][c].empty())
  		{
  			for(auto it1 = x.trans[p1.first][c].begin(); it1 != x.trans[p1.first][c].end(); it1++)
  			{
  				for(auto it2 = y.trans[p1.second][c].begin(); it2 != y.trans[p1.second][c].end(); it2++)
  				{
  					p2.first = *it1;
  					p2.second = *it2;
  					r.trans[i][c].insert(Find(etats, p2));
  				}
  				
  			}
  			
  			
  			
  		}
  		
  	}
  	for(auto it1 = x.epsilon[p1.first].begin(); it1 != x.epsilon[p1.first].end(); it1++)
  	{
  		for(auto it2 = y.epsilon[p1.second].begin(); it2 != y.epsilon[p1.second].end(); it2++)
  		{
  			p2.first = *it1;
  			p2.second = *it2;
  			r.epsilon[i].insert((unsigned int)Find(etats, p2));
  		}
  	}
  }
  

  return r;
}

////////////////////////////////////////////////////////////////////////////////

vector<string> GenererMots(int alpha, vector<string> v)
{
	vector<string> v2;
	for(unsigned int i = 0; i < v.size(); i++)
	{
		for(int c = 0; c < alpha; c++)
		{
			char car = c+ASCII_A;
			v2.push_back(v[i]+car);
		}
	}
	return v2;
}

int Destination(etat_t origin, string mot, const sAutoNDE& at)
{
	etatset_t etat;
	etat.insert(origin);
	for(unsigned int i = 0; i < mot.size(); i++)
	{
		etat = Delta(at, etat, mot[i]);
	}
	auto it = etat.begin();
	return *it;
}

int ClasseDepuisEtat(int etat, vector<etatset_t>& classes)
{
	for(unsigned int i = 0; i < classes.size(); i++)
	{
		if(classes[i].count(etat) == 1) return i;
	}
	return -1;
}

int GroupeDepuisClasse(int c, vector<int>& groupes)
{
	for(unsigned int i = 0; i < groupes.size(); i++)
	{
		if(groupes[i] == c) return i;
	}
	return -1;
}

sAutoNDE Minimize(const sAutoNDE& at){
  //TODO définir cette fonction
  assert(EstDeterministe(at));
  //classes contiendra a la fin de la boucle autant de case que de classes. Chaque etat dans une meme case appartiennent a la meme classe
  vector<etatset_t> classes;
  classes.resize(2);
  //Initialise en deux classes: les etats finaux et non finaux
  for(unsigned int i = 0; i < at.nb_etats; i++)
  {
  	if(at.finaux.count(i) == 1)
  	{
  		classes[0].insert(i);
  	}
  	else
  	{
  		classes[1].insert(i);
  	}
  }
  bool changement = true;
  //Ce vector contiendra tout les mots de taille n pour le nème tour de boucle
  vector<string> mots;
  mots.push_back("");
  //On s'arrete si il n'y a plus de groupe a splitter
  while(changement)
  {
  	changement = false;
  	//1ere etape: on genere des mots de taille n+1
  	mots = GenererMots(at.nb_symbs, mots);
  	for(unsigned int i = 0; i < classes.size(); i++)
  	{
  		for(unsigned int j = 0; j < mots.size(); j++)
  		{
			vector<int> groupes(classes.size(), -1);
  			for(auto it = classes[i].begin(); it != classes[i].end(); it++)
  			{//Pour chaque classe, chaque etat de cette classe et chaque mot de taille n, regarder dans quels classes ils atterissent (on les appelera groupe)
  				if(groupes[i] == -1)
  				{//Si on trouve -1, je suis le premier du groupe. Donc j'appartiens au groupe de la classe où j'atterit
  					groupes[i] = ClasseDepuisEtat(Destination(*it, mots[j], at), classes);
  				}
  				else
  				{//Sinon, ça se complique
  					int g1 = ClasseDepuisEtat(Destination(*it, mots[j], at), classes);
  					if(groupes[i] != g1)
  					{
  						//Si groupe different, spliter le groupe
  						changement = true;
  						etat_t e = *it;
  						it--;
  						classes[i].erase(e);
  						int g2 = GroupeDepuisClasse(g1, groupes);
  						if(g2 == -1)
  						{//Soit j'atterit dans un nouveau groupe
  							groupes.push_back(g1);
  							etatset_t cl;
  							cl.insert(e);
  							classes.push_back(cl);
  						}
  						else
  						{//Soit j'atterit dans un groupe deja existent
  							classes[g2].insert(e);
  						}
  						
  					}
  				}
  			}
  		}
  	}
  	
  }
  //Maintenant, on peut remplir notre nouvel automate
  sAutoNDE r;
  r.nb_symbs = at.nb_symbs;
  r.nb_etats = classes.size();
  r.nb_finaux = 0;
  r.trans.resize(r.nb_etats);
  for(unsigned int i = 0; i < r.nb_etats; i++)
  {
  	r.trans[i].resize(r.nb_symbs);
  }
  r.epsilon.resize(r.nb_etats);
  for(unsigned int i = 0; i < classes.size(); i++)
  {
  	if(classes[i].count(at.initial))
  	{
  		r.initial = i;
  	}
  	if(ContientFinal(at, classes[i]))
  	{
  		r.finaux.insert(i);
  		r.nb_finaux++;
  	}
  	auto it1 = classes[i].begin();
  	for(unsigned int c = 0; c < r.nb_symbs; c++)
  	{
  		auto it2 = at.trans[*it1][c].begin();
  		r.trans[i][c].insert(ClasseDepuisEtat(*it2, classes));
  	}
  	
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////

// détermine la pseudo équivalence par comparaison de tous les mots de Sigma* de longueur < à word_size_max
bool PseudoEquivalent(const sAutoNDE& a1, const sAutoNDE& a2, unsigned int word_size_max) {
  //TODO définir cette fonction
  vector<string> v;
  v.push_back("");
  if(a1.finaux.count(a1.initial) != a2.finaux.count(a2.initial)) return false;
  for(unsigned int i = 1; i <= word_size_max; i++)
  {
  	v = GenererMots(a1.nb_symbs, v);
  	for(unsigned int j = 0; j < v.size(); j++)
  	{
  		bool b1 = Accept(a1, v[j]);
  		bool b2 = Accept(a2, v[j]);
  		if(b1 != b2) return false;
  	}
  }
  return true;
}

//~ Détermine si les deux automates donnés sont égaux :
	//~	- même nombre d'états
	//~	- même nombre de symboles
	//~ - même état initial
	//~ - même états finaux
	//~ - mêmes transitions
bool SontEgaux(const sAutoNDE& a1, const sAutoNDE& a2){
	//   - même nombre d'états
	if(a1.nb_etats != a2.nb_etats){
		return false;
	}
	
	//   - même nombre de symboles
	if(a1.nb_symbs != a2.nb_symbs){
		return false;
	}
	
	//~ - même état initial
	if(a1.initial != a2.initial){
		return false;
	}
	
	
	//~ - même états finaux
	for(auto it1 = a1.finaux.begin(), it2 = a2.finaux.begin(); it1 != a1.finaux.end() && it2 != a2.finaux.end();it1++, it2++){
		if(*it1 != *it2){
			return false;
		}
	}
	
	//~ - mêmes transitions
	//~ 	* Epsilon-transitions
	for(unsigned int i = 0;i<a1.nb_etats;i++){
		for(auto it1 = a1.epsilon[i].begin(), it2 = a2.epsilon[i].begin(); it1 != a1.epsilon[i].end() && it2 != a2.epsilon[i].end();it1++, it2++){
			if(*it1 != *it2){
				return false;
			}
		}
	}
	//~ 	* Transitions
	for(unsigned int i = 0;i<a1.nb_etats;i++){
		for(unsigned int j = 0;j<a1.nb_symbs;j++){
			for(auto it1 = a1.trans[i][j].begin(), it2 = a2.trans[i][j].begin(); it1 != a1.trans[i][j].end() && it2 != a2.trans[i][j].end();it1++, it2++){
				if(*it1 != *it2){
					return false;
				}
			}
		}
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// détermine l'équivalence par "égalité" des automates :
//   - même nombre d'états
//   - même état initial
//   - mêmes états finaux
//   - même table de transition
// à un renommage des états près
bool Equivalent(const sAutoNDE& a1, const sAutoNDE& a2) {
	//   - même nombre d'états
	if(a1.nb_etats != a2.nb_etats){
		return false;
	}
	
	//   - même nombre de symboles
	if(a1.nb_symbs != a2.nb_symbs){
		return false;
	}
	
	// On déterminise les deux automates
	sAutoNDE a1_det_min = Determinize(a1);
	sAutoNDE a2_det_min = Determinize(a2);
	
	
	a1_det_min = Minimize(a1_det_min);
	a2_det_min = Minimize(a2_det_min);
	
	if(a1_det_min.initial != a2_det_min.initial)
	{
		SwitchEtats(a2_det_min, a1_det_min.initial, a2_det_min.initial);
	}
	for(unsigned int i = 0; i < a1_det_min.nb_etats; i++)
	{
		for(unsigned int j = 0; j < a1_det_min.nb_symbs; j++)
		{
			auto it1 = a1_det_min.trans[i][j].begin();
			auto it2 = a2_det_min.trans[i][j].begin();
			if(*it1 != *it2)
			{
				SwitchEtats(a2_det_min, *it1, *it2);
			}
		}
	}

  return SontEgaux(a1_det_min, a2_det_min);
}

////////////////////////////////////////////////////////////////////////////////


void Help(ostream& out, char *s){
  out << "Utilisation du programme " << s << " :" << endl ;
  out << "-acc ou -accept Input Word:\n\t détermine si le mot Word est accepté" << endl;
  out << "-det ou -determinize Input :\n\t déterminise Input" << endl;
  out << "-cup ou -union Input1 Input2 :\n\t calcule l'union" << endl;
  out << "-cat ou -concat Input1 Input2 :\n\t calcul la concaténation" << endl;
  out << "-star ou -kleene Input :\n\t calcul de A*" << endl;
  out << "-bar ou -complement Input :\n\t calcul du complément" << endl;
  out << "-cap ou -intersection Input1 Input2 :\n\t calcul de l'intersection par la loi de De Morgan" << endl;
  out << "-prod ou -produit Input1 Input2 :\n\t calcul de l'intersection par construction de l'automate produit" << endl;
/*
  out << "-expr2aut ou expressionrationnelle2automate ExpressionRationnelle :\n\t calcul de l'automate correspondant à l'expression rationnelle" << endl;
*/
  out << "-min ou -minimisation Input :\n\t construit l'automate standard correspondant à Input" << endl;
  out << "-pequ ou -pseudo_equivalent Input1 Input2 size :\n\t équivalence d'automates par comparaison mot par mot de longueur < à size" << endl;
  out << "-equ ou -equivalent Input1 Input2 :\n\t équivalence d'automates par minimisation et comparaison des tables de transition" << endl;
  out << "-nop ou -no_operation Input :\n\t ne rien faire de particulier" << endl;

  out << "-o ou -output Output :\n\t écrire le résultat dans le fichier Output, afficher sur STDOUT si non spécifié" << endl;
  out << "-g ou -graphe :\n\t l'output est au format dot/graphiz" << endl  << endl;
  
  out << "Exemple '" << s << " -determinize auto.txt -output determin -g'" << endl;
  out << "Exemple '" << s << " -minimisation test.jff -output min -j'" << endl;
}




////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[] ){
  if(argc < 3){
    Help(cout, argv[0]);
    return EXIT_FAILURE;
  }
  
  int pos;
  int act=-1;                 // pos et act pour savoir quelle action effectuer
  int nb_files = 0;           // nombre de fichiers en entrée
  string str, in1, in2, out, acc, expr;
  unsigned int word_size_max; // pour la pseudo équivalence
  // chaines pour (resp.) tampon; fichier d'entrée Input1; fichier d'entrée Input2;
  // fichier de sortie et chaine dont l'acceptation est à tester 
  bool toFile=false, graphMode=false, jflapMode=false;     // sortie STDOUT ou fichier ? Si fichier, format graphviz ? Jflap ?

  // options acceptées
  const size_t NBOPT = 16;
  string aLN[] = {"accept", "determinize", "union", "concat", "kleene", "complement", "intersection", "produit", "expressionrationnelle2automate", "minimisation", "pseudo_equivalent", "equivalent", "no_operation", "output", "graph", "jflap"};
  string aSN[] = {"acc", "det", "cup", "cat", "star", "bar", "cap", "prod", "expr2aut", "min", "pequ", "equ", "nop", "o", "g", "j"};
  
  // on essaie de "parser" chaque option de la ligne de commande
  for(int i=1; i<argc; ++i){
    if (DEBUG) cerr << "argv[" << i << "] = '" << argv[i] << "'" << endl;
    str = argv[i];
    pos = -1;
    string* pL = find(aLN, aLN+NBOPT, str.substr(1));
    string* pS = find(aSN, aSN+NBOPT, str.substr(1));
    
    if(pL!=aLN+NBOPT)
      pos = pL - aLN;
    if(pS!=aSN+NBOPT)
      pos = pS - aSN;   
      
    if(pos != -1){
      // (pos != -1) <=> on a trouvé une option longue ou courte
      if (DEBUG) cerr << "Key found (" << pos << ") : " << str << endl;
      switch (pos) {
        case 0: //acc
          in1 = argv[++i];
          acc = argv[++i];
	  nb_files = 1;
          break;
        case 1: //det
          in1 = argv[++i];
	  nb_files = 1;
          break;
        case 2: //cup
          in1 = argv[++i];
          in2 = argv[++i];
	  nb_files = 2;
          break;
        case 3: //cat
          in1 = argv[++i];
          in2 = argv[++i];
	  nb_files = 2;
          break;
        case 4: //star
          in1 = argv[++i];
	  nb_files = 1;
          break;
        case 5: //bar
          in1 = argv[++i];
	  nb_files = 1;
          break;
        case 6: //cap
          in1 = argv[++i];
          in2 = argv[++i];
	  nb_files = 2;
          break;
        case 7: //prod
          in1 = argv[++i];
          in2 = argv[++i];
	  nb_files = 2;
          break;
        case 8: //expr2aut
          expr = argv[++i];
	  nb_files = 0;
          break;
        case 9: //min
          in1 = argv[++i];
	  nb_files = 1;
          break;
        case 10: //pequ
          in1 = argv[++i];
          in2 = argv[++i];
	  word_size_max = atoi(argv[++i]);
	  nb_files = 2;
          break;
        case 11: //equ
          in1 = argv[++i];
          in2 = argv[++i];
	  nb_files = 2;
          break;
        case 12: //nop
          in1 = argv[++i];
	  nb_files = 1;
          break;          
        case 13: //o
          toFile = true;
          out = argv[++i];
          break;
        case 14: //g
          graphMode = true;
          break;
        case 15: //j
          jflapMode = true;
          break;
        default:
          return EXIT_FAILURE;
        }
    }
    else{
      cerr << "Option inconnue "<< str << endl;
      return EXIT_FAILURE;
    }
    
    if(pos<13){
      if(act > -1){
        cerr << "Plusieurs actions spécififées"<< endl;
        return EXIT_FAILURE;
      }
      else
        act = pos;
    }    
  }
  
  if (act == -1){
    cerr << "Pas d'action spécififée"<< endl;
    return EXIT_FAILURE;
  }  

/* Les options sont OK, on va essayer de lire le(s) automate(s) at1 (et at2)
et effectuer l'action spécifiée. Atr stockera le résultat*/

  sAutoNDE at1, at2, atr;
  
  if ((nb_files == 1 or nb_files == 2) and !FromFile(at1, in1)){
    cerr << "Erreur de lecture " << in1 << endl;
    return EXIT_FAILURE;
  }  
  if (nb_files ==2 and !FromFile(at2, in2)){
    cerr << "Erreur de lecture " << in2 << endl;
    return EXIT_FAILURE;
  }  
  
  switch(act) {
  case 0: //acc
    cout << "'" << acc << "' est accepté: " << Accept(at1, acc) << endl;
    atr = at1;
    break;
  case 1: //det
    atr = Determinize(at1);
    break;
  case 2: //cup
    atr =  Union(at1, at2); 
    break;
  case 3: //cat
    atr =  Concat(at1, at2); 
    break;
  case 4: //star
    atr =  Kleene(at1);
    break;
  case 5: //bar
    atr =  Complement(at1);
    break;
  case 6: //cap
    atr =  Intersection(at1, at2);
    break;
  case 7: //prod
    atr =  Produit(at1, at2);
    break;
  case 8: //expr2aut
/*
    atr =  ExpressionRationnelle2Automate(expr);
*/
    break;
  case 9: //minimisation
    atr =  Minimize(at1);
    break;
  case 10: //pseudo équivalence
    cout << "Après comparaison de tous les mots de longeur < à " << word_size_max << ", les deux automates sont pseudo-équivalents : ";
    cout << PseudoEquivalent(at1,at2,word_size_max) << endl;
    atr = at1;
    break;
  case 11: //équivalence
    cout << "Les deux automates sont équivalents : " << Equivalent(at1,at2) << endl;
    atr = at1;
    break;
  case 12: //nop
    atr = at1;
    break;
  default:
    return EXIT_FAILURE;
  }

  // on affiche le résultat ou on l'écrit dans un fichier
  if(!toFile)
    cout << atr;
  else{
    if(graphMode){
      ToGraph(atr, out + ".gv");
      system(("dot -Tpng " + out + ".gv -o " + out + ".png").c_str());
    }
    if(jflapMode){
      ToJflap(atr, out + ".jff");
    }
    ofstream f((out + ".txt").c_str(), ios::trunc); 
    if(f.fail())
      return EXIT_FAILURE;
    f << atr;    
  }
  
  return EXIT_SUCCESS;
}



