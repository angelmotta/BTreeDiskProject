//#include "Page.h"
//#include "Record.h"

#include <iostream>
#include <list>
#include <fstream>
#include <string.h>
#include <vector>
#include <optional>
#include <set>
#include <queue>
using namespace std;


//#define NodoN Nodo<N>

template<typename TRegistro, unsigned N=3>
class TNodo{
public:
	long id;
	int t = (N+1)/2;    // grado minimo
	TRegistro keys[N+1];
	unsigned int children[N+2];
	int n;
	bool leaf;
	TNodo(){
		leaf = false;
		n = 0;
	}
	bool is_leaf(){
		return leaf;
	}

	void InOrder(){
	    for(int i = 0; i < this->n; i++){

	    }
	}
	
	void print(){
		for(int i=0; i<n; ++i){
			cout << keys[i]<<"\t";
		}
		cout << endl;
	}
	
};

class Record
{
    //char key[50];
    int key;
//    long pdir /* [6] */;
    long offset/* [6] */; 

public:
	Record(){};
    Record(int k, long off){
    	key =k;
    	offset =off;
    }
    void print(){
    	cout << key <<endl;
    }
};

template<typename TPage>
class PageManager{
private:
	char file_name[50];
public:
	PageManager(char * name){
		strcpy(file_name,name);
	}
	TPage Read(uint64_t dir){
		TPage new_nodo;
		ifstream file(file_name, ios::binary);
		file.seekg(dir*sizeof(TPage));
		file.read((char*)&new_nodo, sizeof(TPage));
		return new_nodo;
	}
	void Write(uint64_t dir, TPage &nodo){
		ofstream file(file_name, ios::binary);
		file.seekp(dir*sizeof(TPage));
		file.write((char*)&nodo, sizeof(TPage));
		file.close();
	}
	uint64_t WriteNewNodo(TPage nodo){
		ofstream file(file_name, ios::binary | ios::app);
		uint64_t aux = (file.tellp()/sizeof(TPage));
		file.write((char*) &nodo, sizeof(TPage));
		file.close();
		return aux;
	}
};



// TRegistro es el tipo de data que trabajo
template<typename Tkey, typename TRegistro, unsigned N=3>
class BTreeDisk{
	PageManager<TNodo<TRegistro,N>> *pm;

	void SplitChild(TNodo<TRegistro,N> & p,TNodo<TRegistro,N> &node, int idx){
		cout << "SplitChild "<<endl;
		TNodo<TRegistro,N> nodeZ;
		nodeZ.leaf = p.leaf;
		nodeZ.n = p.t-1;

		for(int i = 0; i < p.t - 1; i++){
            nodeZ.keys[i] = node.keys[i+p.t];
        }
		if(!node.leaf){
            for(int i = 0; i < p.t; i++){
                nodeZ.children[i] = node.children[i+p.t];
            }
        }
		node.n = p.t - 1;
        for(int i = p.n; i >= idx + 1; i--){
            p.children[i + 1] = p.children[i];
        }
		p.children[idx + 1] = pm->WriteNewNodo(nodeZ);
		for(int i = p.n - 1; i >= idx; i--){
            p.keys[i + 1] = p.keys[i];
        }
		p.keys[idx] = node.keys[p.t-1];
        p.n++;
        pm->Write(p.children[idx], node);
	}

	void InsertNonFull(uint64_t n_node, TRegistro &reg){
		TNodo<TRegistro,N> node = pm->Read(n_node);
		int index = node.n-1;
//		if(node.n != 0){		
			if(node.leaf){
				cout << "leaf"<<endl;
				while(index>=0 && node.keys[index]>reg){
					node.keys[index+1] = node.keys[index];
					--index;
				}
				node.keys[index+1] = reg;
				node.n = node.n + 1;
				pm->Write(n_node,node);
			}
			else{
				while(index >= 0 && node.keys[index] > reg)
					--index;

				TNodo<TRegistro,N> next_node = pm->Read(node.children[index+1]);
				if(next_node.n == N){
					SplitChild(node, next_node, index+1);
					if(node.keys[index+1] < reg){
						++index;
					}
				}
				pm->Write(n_node, node);
	            InsertNonFull(node.children[index+1], reg);
			}
	//	}
	}
public:
	BTreeDisk(string s){
		char aux[50];
		strcpy(aux, s.c_str());
		pm = new PageManager<TNodo<TRegistro,N>>(aux);
	}

	void newnew(){
		// crea el primer nodo// eliminar mas tarde
		TNodo<TRegistro,N> nn;
		nn.leaf = true;
		pm->WriteNewNodo(nn);
	}

    void inOrder() {
        auto temp = pm->read(0);
        temp.print();

    }

	void Insert(TRegistro reg){
		TNodo<TRegistro,N> root = pm->Read(0);
		if(root.n == 0){
			cout << "first" <<endl;
			TNodo<TRegistro,N> r;
			r.keys[0] = reg;
			r.n = 1;
			r.leaf = true;
			pm->WriteNewNodo(r);
			cout << r.n <<endl;
		}
		else{
			if(root.n == N){
				cout << "insert full"<<endl;
				TNodo<TRegistro,N> new_root, new_nodo;
				new_nodo = root;
				new_root.children[0] = pm->WriteNewNodo(new_nodo);
				SplitChild(new_root, new_nodo, 0);
				int idx = 0;
				if(reg > new_root.keys[0]){
					++idx;
				}
				pm->Write(0, new_root);
				InsertNonFull(new_root.children[idx],reg);
			}
			else{
				InsertNonFull(0, reg);
			}
		}

	}

	TRegistro Search(Tkey k){
		TNodo<TRegistro,N> aux = pm->Read(k*sizeof(TNodo<TRegistro,N>));
		aux.print();
		return aux.keys[0];
	}

	void Print(){
		set<uint64_t> visitados;
		queue<uint64_t> q;
		visitados.insert(0);
		q.push(0);
		TNodo<TRegistro,N> node;
		queue<long> espacios;
		long espacios_i = 0;
		espacios.push(0);
		while(q.size() != 0){
			node = pm->Read(q.front());
			node.print();
			q.pop();
			for(int i=0; i<=node.n; ++i){
				auto p = visitados.find(node.children[i]);
				if(p == visitados.end()){
					visitados.insert(node.children[i]);
					q.push(node.children[i]);
				}
			}
			espacios.push(espacios.front() + node.n + 1);
			if(espacios_i == espacios.front()){
				cout << '\n';
				espacios.pop();
			}
			else{
				cout << ' - ';
			}
			++espacios_i;
		}
		cout << '\n';
	}
};


int main(){
	// key : data
	BTreeDisk<int,int> tree("btree.bin");

	tree.Insert(2);
	tree.Insert(10);
	tree.Insert(5);
	tree.Search(0);
	tree.Insert(23);
	tree.Search(0);
	tree.Insert(55);
	tree.Search(0);
	tree.Insert(12);
	tree.Search(0);
	cout << endl;
	tree.Print();
	//BTreeDisk<int,Record> tree("btree.bin");

	return 0;
}