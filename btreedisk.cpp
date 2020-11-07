//#include "Page.h"
//#include "Record.h"

#include <iostream>
#include <list>
#include <fstream>
#include <string.h>
#include <vector>
#include <optional>
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
		leaf = true;
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
    	cout << key;
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
		TNodo<TRegistro,N> nodeZ;
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
	}
	void InsertNonFull(TNodo<TRegistro,N> &node, TRegistro &reg){
		int index = node.n - 1;
		if(node.leaf){
			while(index>=0 && node.keys[index]>reg){
				node.keys[index+1] = node.keys[index];
				--index;
			}
			cout << "2 hear"<<endl;
			node.keys[index+1] = reg;
			++(node.n);
			cout << node.n << endl;
			pm->Write(0, node);
		}
		else{
			while(index >= 0 && node.keys[index] > reg)
				--index;

			TNodo<TRegistro,N> next_node = pm->Read(node.children[index+1]);

			if(next_node.n == N){
				SplitChild(node,next_node, index+1);
				if(node.keys[index+1] < reg){
					++index;
				}
			}
			cout << "here"<<endl;
            TNodo<TRegistro,N> other_node = pm->Read(node.children[index+1]);
            InsertNonFull(other_node, reg);
		}
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
	void Insert(TRegistro reg, bool first = false){
		// inserta nodo tras nodo
		//TNodo<TRegistro,N> root = pm->Read(0);
		//TNodo<TRegistro,N> *nodo_a = new TNodo<TRegistro,N>();
		//r.Insert(reg);
		//pm->Write(0,r);
		//coupmt << pm->WriteNewNodo(*nodo_a)<<endl;

		TNodo<TRegistro,N> root = pm->Read(0);
		if(first){
			cout << "first";
			TNodo<TRegistro,N> r;
			r.children[0] = reg;
			++r.n;
			pm->Write(0,r);
		}
		else{
			if(root.n == N){
				TNodo<TRegistro,N> new_nodo;          
				TNodo<TRegistro,N> aux_r = root;

				//new_nodo.children[0] = 0;
				new_nodo.children[0] = pm->WriteNewNodo(aux_r);

				SplitChild(new_nodo,aux_r,0);

				int i=0;
				if(new_nodo.keys[0]< reg)
					++i;
				TNodo<TRegistro,N> aux =  pm->Read(new_nodo.children[i]);
				InsertNonFull(aux, reg);
				pm->Write(0,new_nodo);
			}
			else{
				InsertNonFull(root, reg);
				pm->Write(0, root);
			}
		}

	}
	TRegistro Search(Tkey k){
		TNodo<TRegistro,N> aux = pm->Read(k*sizeof(TNodo<TRegistro,N>));
		aux.print();
		return aux.keys[0];
	}
};

int main(){
	// key : data
	BTreeDisk<int,int> tree("btree.bin");
	// newnew solo se hace la primera vez bc aun no se que hacer cuando no hay ningun nodo
	tree.Insert(2,1);
	tree.Insert(10);
	tree.Insert(5);
	cout <<tree.Search(0)<<endl;
	//BTreeDisk<int,Record> tree("btree.bin");
/*	Record A(121,121);
	Record B(222,222);
	Record C(12,122);
	tree.Insert(A);
	tree.Search(0);
	tree.Insert(B);
	tree.Insert(C);
	tree.Search(0);
	*/
/*	my_data a(111,222), b(1010,2020), c(20,10);
	cout << tree.Insert(a)<<endl;

	cout << tree.Insert(b)<<endl;
	cout << tree.Insert(c)<<endl;
	(tree.Search(0))->print();
//	(tree.Search(1))->print();
*/
	return 0;
}