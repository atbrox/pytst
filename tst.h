/* 
 # $Id$
 # Copyright (C) 2004-2005 Nicolas Lehuen <nicolas@lehuen.com>
 #
 # This library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Lesser General Public
 # License as published by the Free Software Foundation; either
 # version 2.1 of the License, or (at your option) any later version.
 #
 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.
 #
 # You should have received a copy of the GNU Lesser General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __TST__H_INCLUDED__
#define __TST__H_INCLUDED__

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#ifdef __PYTHON__BUILD__
    #include "Python.h"
    #define tst_malloc PyMem_Malloc
    #define tst_realloc PyMem_Realloc
    #define tst_free PyMem_Free
#else
    #define tst_malloc malloc
    #define tst_realloc realloc
    #define tst_free free
#endif

#include "debug.h"

#define UNDEFINED_INDEX -1

#define TRACE(a) printf("%s:%d:%s\n",__FILE__,__LINE__,a)
#define TRACE2(a,b) printf("%s:%d:" a "\n",__FILE__,__LINE__,b)

#define TST_VERSION "0.64"

// Pour ajouter/supprimer les fonctions de scanning.
#define SCANNER

template<class S,class T> class tst_node {
public:
    S c;
    T data;
    int next;
    int right;
    int left;
#ifdef SCANNER
    int position;
    int backtrack;
    int backtrack_match_index;
#endif
};

template<class S,class T> class node_info {
public:
    int index;
    tst_node<S,T>* node;

    int height;
    int balance;
    int right_balance;
    int left_balance;
    int balance_performed;
};

template<class S,class T> class action {
public:
    action() {}
    virtual ~action() {}
    virtual void perform(S* string,int string_length,int remaining_distance,T data)=0;
    virtual T result()=0;
};

template<class S,class T> class filter {
public:
    filter() {}
    virtual ~filter() {}
    virtual T perform(S* string,int string_length,int remaining_distance,T data)=0;
};

template<class S,class T> class serializer {
public:
    serializer() {}
    virtual ~serializer() {}
    virtual void write(FILE* file,T data)=0;
    virtual T read(FILE* file)=0;
};

template<class S,class T> class tst {
public:
    tst(FILE* file,serializer<S,T>* reader);
    tst(int initial_size,T default_value);
    virtual ~tst() {
        clear_nodes();
    };

    void pack();
    T walk(filter<S,T>* filter,action<S,T>* to_perform);
    T almost(S* string,int string_length,int maximum_distance,filter<S,T>* filter,action<S,T>* to_perform);
    T common_prefix(S* string,int string_length,filter<S,T>* filter,action<S,T>* to_perform);

    T get(S* string,int string_length);
    T get_or_build(S* string,int string_length,filter<S,T>* factory);
    T put(S* string,int string_length,T data);
    void remove(S* string,int string_length);
    int get_maximum_key_length();
    size_t bytes_allocated();
    void write(FILE* file,serializer<S,T>* writer);
    void debug_print_root();

#ifdef SCANNER
    T scan(S* string,int string_length,action<S,T>* to_perform);
    T scan_with_stop_chars(S* string,int string_length,S* stop_chars,int stop_chars_length,action<S,T>* to_perform);
#endif

protected:
    tst_node<S,T>* array;
    T default_value;
    int root,next,size,empty,maximum_key_length;

    tst();
    void read(FILE* file,serializer<S,T>* serializer);

    void walk_recurse(tst_node<S,T>* current_node,S* current_key,int current_key_length,int current_key_limit,filter<S,T>* filter,action<S,T>* to_perform);
    void almost_recurse(tst_node<S,T>* current_node,S* current_key, int current_key_length, S* current_char,int current_index, int real_string_length, int string_length, int remaining_distance,filter<S,T>* filter,action<S,T>* to_perform,int current_key_limit);

    void create_node(node_info<S,T>* current_node_info);
    int build_node(node_info<S,T>* current_node,S* string,int string_length,int current_position);
    void remove_node(int* current_index,S* string,int string_length);
    tst_node<S,T>* find_node(int* current_index,int* best_node, S* string,int string_length);

    void balance_node(node_info<S,T>* bal);
    void ll(node_info<S,T>* bal);
    void rr(node_info<S,T>* bal);
    void lr(node_info<S,T>* bal);
    void rl(node_info<S,T>* bal);
    void compute_height_and_balance(node_info<S,T>* current_node_info);
    void clear_nodes();

    virtual T store_data(tst_node<S,T>* node,T data,int want_old_value);

#ifdef SCANNER
    void compute_backtrack(tst_node<S,T> *current_node,S* string,int si_match_start,int si_match_end);
#endif
};

template<class S,class T> void tst<S,T>::debug_print_root() {
    int next_index=UNDEFINED_INDEX;
    printf("Size: %i (%i bytes), next: %i (%i bytes)\n",size,size*sizeof(tst_node<S,T>),next,next*sizeof(tst_node<S,T>));
    if(root!=UNDEFINED_INDEX) {
        tst_node<S,T>* node=array+root;
        printf("root index: %i, next: %i, c: %c\n",root,node->next,node->c);
        next_index = node->next;
        if(next_index!=UNDEFINED_INDEX) {
            node=array+next_index;
            printf("root->next index: %i, next: %i, c: %c\n",next_index,node->next,node->c);
            next_index = node->next;
            if(next_index!=UNDEFINED_INDEX) {
                node=array+next_index;
                printf("root->next->next index: %i, next: %i, c: %c\n",next_index,node->next,node->c);
            }
        }
    }
}

template<class S,class T> T tst<S,T>::store_data(tst_node<S,T>* node,T data,int want_old_value) {
    T result=node->data;
    node->data=data;
    return result;
}

template<class S,class T> tst<S,T>::tst(int size,T default_value) {
    this->size=size;
    this->default_value=default_value;

    array=(tst_node<S,T>*)tst_malloc(size*sizeof(tst_node<S,T>));
    assert(array);
    if(array==NULL) {
        printf("Out of memory trying to allocate %d nodes of %d bytes, totalling %d Mb\n",size,sizeof(tst_node<S,T>),(size*sizeof(tst_node<S,T>))>>20);
        exit(1);
    }

    next=0;
    empty=UNDEFINED_INDEX;
    node_info<S,T> root_info;
    create_node(&root_info);
    root = root_info.index;
    maximum_key_length=0;
}

template<class S,class T> tst<S,T>::tst() {
}

template<class S,class T> tst<S,T>::tst(FILE* file, serializer<S,T>* reader) {
    tst<S,T>::read(file,reader);
}

template<class S,class T> void tst<S,T>::read(FILE* file, serializer<S,T>* reader) {
    fread(&size,sizeof(int),1,file);
    next=size;
    fread(&root,sizeof(int),1,file);
    fread(&maximum_key_length,sizeof(int),1,file);
    int has_data=fgetc(file);
    if(has_data) {
        default_value=reader->read(file);
    }
    else {
        default_value=NULL;
    }
    array=(tst_node<S,T>*)tst_malloc(size*sizeof(tst_node<S,T>));
    memset(array,0,size*sizeof(tst_node<S,T>));
    for(int i=0;i<size;i++) {
        tst_node<S,T>* node=array+i;

        node->c=fgetc(file);
        fread(&(node->left),sizeof(int),1,file);
        fread(&(node->next),sizeof(int),1,file);
        fread(&(node->right),sizeof(int),1,file);

        has_data=fgetc(file);
        if(has_data) {
            store_data(node,reader->read(file),0);
        }
        else {
            store_data(node,default_value,0);
        }
    }
}

template<class S,class T> void tst<S,T>::write(FILE* file, serializer<S,T>* writer) {
    fwrite(&next,sizeof(int),1,file);
    fwrite(&root,sizeof(int),1,file);
    fwrite(&maximum_key_length,sizeof(int),1,file);
    if(default_value) {
        putc('\1',file);
        writer->write(file,default_value);
    }
    else {
        putc('\0',file);
    }
    for(int i=0;i<next;i++) {
        tst_node<S,T>* node=array+i;
        fputc(node->c,file);
        fwrite(&(node->left),sizeof(int),1,file);
        fwrite(&(node->next),sizeof(int),1,file);
        fwrite(&(node->right),sizeof(int),1,file);
        T data=node->data;
        if(data!=default_value) {
            fputc('\1',file);
            writer->write(file,data);
        }
        else {
            fputc('\0',file);
        }
    }
}

template<class S,class T> void tst<S,T>::clear_nodes() {
    if(array) {
        for(int i=0;i<next;i++) {
            store_data(array+i,(T)NULL,0);
        }
        tst_free(array);
        array=NULL;
    }
}

template<class S,class T> void tst<S,T>::pack() {
    if(next<size) {
        size=next;
        array=(tst_node<S,T>*)tst_realloc(array,size*sizeof(tst_node<S,T>));
    }
}

template<class S,class T> T tst<S,T>::get(S* string,int string_length) {
    int current_index=root,best_node=UNDEFINED_INDEX;
    tst_node<S,T>* current_node=find_node(&current_index,&best_node,string,string_length);
    if(current_node) {
        return current_node->data;
    }
    else {
        return default_value;
    }
}

template<class S,class T> T tst<S,T>::put(S* string,int string_length,T data) {
    node_info<S,T> root_info;
    root_info.index=root;
    root_info.node=array+root;
    int node_index=build_node(&root_info,string,string_length,0);
    root = root_info.index;
    return store_data(array+node_index,data,1);
}

template<class S,class T> void tst<S,T>::remove(S* string,int string_length) {
    remove_node(&root,string,string_length);
    if(root==UNDEFINED_INDEX) {
        node_info<S,T> root_info;
        create_node(&root_info);
        root = root_info.index;
    }
}

template<class S,class T> T tst<S,T>::get_or_build(S* string,int string_length,filter<S,T>* factory) {
    node_info<S,T> root_info;
    root_info.index=root;
    root_info.node=array+root;
    int node_index=build_node(&root_info,string,string_length,0);
    root = root_info.index;

    tst_node<S,T>* current_node=array+node_index;
    T data=factory->perform(string,string_length,0,current_node->data);
    if(data!=current_node->data) {
        store_data(current_node,data,0);
    }
    return data;
}

template<class S,class T> T tst<S,T>::almost(S* string, int string_length, int maximum_distance,filter<S,T>* filter,action<S,T>* to_perform) {
    S* current_key=(S*)tst_malloc((string_length+maximum_distance+1)*sizeof(S));
    *current_key='\0';
    almost_recurse(array+root,current_key,0,string,0,string_length,string_length,maximum_distance,filter,to_perform,string_length+maximum_distance);
    tst_free(current_key);
    return to_perform->result();
}

template<class S,class T> T tst<S,T>::walk(filter<S,T>* filter,action<S,T>* to_perform) {
    S* key=(S*)tst_malloc((maximum_key_length+2)*sizeof(S));
    *key='\0';
    walk_recurse(array+root,key,0,maximum_key_length+1,filter,to_perform);
    tst_free(key);
    return to_perform->result();
}

template<class S,class T> void tst<S,T>::walk_recurse(tst_node<S,T>* current_node,S* current_key,int current_key_length,int current_key_limit,filter<S,T>* filter,action<S,T>* to_perform) {
    int other_index;

    other_index=current_node->left;
    if(other_index!=UNDEFINED_INDEX) {
        walk_recurse(array+other_index,current_key,current_key_length,current_key_limit,filter,to_perform);
    }

    assert(current_key_length < current_key_limit);
    current_key[current_key_length]=current_node->c;
    current_key_length++;

    T data = current_node->data;
    if(data!=default_value) {
        if(filter) {
            data = filter->perform(current_key,current_key_length,0,data);
        }
        if(to_perform) {
            to_perform->perform(current_key,current_key_length,0,data);
        }
    }

    other_index=current_node->next;
    if(other_index!=UNDEFINED_INDEX) {
        walk_recurse(array+other_index,current_key,current_key_length,current_key_limit,filter,to_perform);
    }

    current_key_length--;

    other_index=current_node->right;
    if(other_index!=UNDEFINED_INDEX) {
        walk_recurse(array+other_index,current_key,current_key_length,current_key_limit,filter,to_perform);
    }
}

template<class S,class T> void tst<S,T>::almost_recurse(tst_node<S,T>* current_node,S* current_key,int current_key_length,S* string, int current_index, int real_string_length, int string_length, int remaining_distance,filter<S,T>* filter, action<S,T>* to_perform,int current_key_limit) {
    int other_index;
    int diff,diff2;

    other_index=current_node->left;
    if (other_index!=UNDEFINED_INDEX) {
        almost_recurse(array+other_index,current_key,current_key_length,string,current_index,real_string_length,string_length,remaining_distance,filter,to_perform,current_key_limit);
    }

    diff=0;
    if(current_index<real_string_length) {
        if((string[current_index]-current_node->c)!=0) {
            diff=1;
        }
    }
    else {
        diff=1;
    }

    assert(current_key_length < current_key_limit);
    current_key[current_key_length]=current_node->c;
    current_key_length++;

    T data = current_node->data;
    if(data!=default_value) {
        diff2=(remaining_distance-abs(current_key_length-string_length))-diff;
        if(diff2>=0) {
            data = filter->perform(current_key,current_key_length,diff2,data);
            if(data!=default_value) {
                to_perform->perform(current_key,current_key_length,diff2,data);
            }
        }
    }

    other_index=current_node->next;
    if (other_index!=UNDEFINED_INDEX) {
        diff2 = remaining_distance - diff;
        if (diff2>=0) {
            almost_recurse(array+other_index,current_key,current_key_length,string,current_index+1,real_string_length,string_length,diff2,filter,to_perform,current_key_limit);
        }
    }

    current_key_length--;

    if(current_index<string_length && remaining_distance>0) {
        almost_recurse(current_node,current_key,current_key_length,string,current_index+1,real_string_length,string_length-1,remaining_distance-1,filter,to_perform,current_key_limit);
    }

    if(other_index!=UNDEFINED_INDEX && remaining_distance>0) {
        assert(current_key_length < current_key_limit);
        current_key[current_key_length]=current_node->c;
        current_key_length++;

        almost_recurse(array+other_index,current_key,current_key_length,string,current_index,real_string_length,string_length+1,remaining_distance-1,filter,to_perform,current_key_limit);
    
        current_key_length--;
    }

    other_index = current_node->right;
    if(other_index!=UNDEFINED_INDEX) {
        almost_recurse(array+other_index,current_key,current_key_length,string,current_index,real_string_length,string_length,remaining_distance,filter,to_perform,current_key_limit);
    }
}

template<class S,class T> T tst<S,T>::common_prefix(S* string,int string_length,filter<S,T>* filter,action<S,T>* to_perform) {
    S* current_key=(S*)tst_malloc((maximum_key_length+2)*sizeof(S));
    int current_key_length=0;

    T biggest=default_value;
    int current_index=root;
    tst_node<S,T>* current_node;
    int diff;
    S c;

    while(string_length>0) {
        c = *string;
        string++;
        string_length--;

        assert(current_key_length<=maximum_key_length);
        current_key[current_key_length]=c;
        current_key_length++;

        while(current_index!=UNDEFINED_INDEX) {
            current_node=array+current_index;
            diff=c-current_node->c;
            if(diff==0) {
                if(current_node->data!=default_value) {
                    biggest=current_node->data;
                    if(filter) {
                        biggest=filter->perform(current_key,current_key_length,0,biggest);
                    }
                }
                if(*string) {
                    current_index=current_node->next;
                    break;
                }
                else {
                    if(biggest!=default_value && to_perform) {
                        to_perform->perform(current_key,current_key_length,0,biggest);
                    }
                    current_index=current_node->next;
                    if(current_index!=UNDEFINED_INDEX) {
                        walk_recurse(array+current_index,current_key,current_key_length,maximum_key_length+2,filter,to_perform);
                    }
                    tst_free(current_key);
                    return to_perform->result();
                }
            }
            else if(diff>0) {
                current_index=current_node->right;
            }
            else {
                current_index=current_node->left;
            }
        }
        if(current_index==UNDEFINED_INDEX) {
            break;
        }
    }

    if(biggest!=default_value && to_perform) {
        to_perform->perform(current_key,current_key_length,0,biggest);
    }

    tst_free(current_key);
    return to_perform->result();
}

template<class S,class T> void tst<S,T>::create_node(node_info<S,T>* current_node_info) {
    int id;
    tst_node<S,T>* new_node;

    if(next==size) {
        int original_size = size;
        tst_node<S,T>* original_array = array;
        array = NULL;
        size=(int)(1.618*size)+1;
        
        // on utilise le nombre d'or pour l'accroissement du tableau
        while(array==NULL && size>original_size) {
            array=(tst_node<S,T>*)tst_realloc(original_array,size*sizeof(tst_node<S,T>));
            if(array==NULL) {
                printf("Out of memory trying to allocate %d nodes of %d bytes, totalling %d Mb",size,sizeof(tst_node<S,T>),(size*sizeof(tst_node<S,T>))>>20);
                size = (int)(size*0.9)-1;
                printf(" ...trying to allocate %d nodes, totalling %d Mb, instead\n",size,(size*sizeof(tst_node<S,T>))>>20);
            }
        }
        if(array==NULL) {
            printf("Out of memory trying to allocate %d nodes of %d bytes, totalling %d Mb\n",size,sizeof(tst_node<S,T>),(size*sizeof(tst_node<S,T>))>>20);
            exit(1);
        }
    }

    if(empty!=UNDEFINED_INDEX) {
        // si on a un noeud vide on l'utilise.
        id=empty;
        // on passe au noeud vide suivant.
        empty = (array+empty)->next;
    }
    else {
        id=next++;
    }

    new_node=array+id;
    new_node->c=0;
    new_node->next=UNDEFINED_INDEX;
    new_node->right=UNDEFINED_INDEX;
    new_node->left=UNDEFINED_INDEX;
    new_node->data=(T)NULL;
#ifdef SCANNER
    new_node->position=-1;
    new_node->backtrack=UNDEFINED_INDEX;
    new_node->backtrack_match_index=UNDEFINED_INDEX;
#endif
    store_data(new_node,default_value,0);

    current_node_info->index = id;
    current_node_info->node = new_node;
    current_node_info->height = 0;
    current_node_info->balance = 0;
    current_node_info->left_balance = 0;
    current_node_info->right_balance =0;
}

template<class S,class T> int tst<S,T>::build_node(node_info<S,T>* current_node_info,S* string,int string_length,int current_position) {
    int diff,result;

    if(current_node_info->node->c==0) {
        current_node_info->node->c=(*string);
#ifdef SCANNER
        current_node_info->node->position=current_position;
#endif
        diff=0;
    }
    else {
        diff=(*string)-(current_node_info->node->c);
    }

    current_node_info->height=-1;
    current_node_info->balance_performed=0;

    if(diff==0) {
        string++;
        current_position++;
        string_length--;

        compute_height_and_balance(current_node_info); // TODO : est-ce bien necessaire ?

        if(string_length>0) {
            if(current_position>maximum_key_length) {
                maximum_key_length=current_position;
            }

            node_info<S,T> next_node_info;
            next_node_info.index = current_node_info->node->next;
            if(next_node_info.index==UNDEFINED_INDEX) {
                /* attention cela doit FORCEMENT se faire en deux lignes
                car current_node peut �tre modifi� par tst_create_node. */
                create_node(&next_node_info);
                current_node_info->node = array + current_node_info->index;
                current_node_info->node->next=next_node_info.index;
            }
            next_node_info.node = array + next_node_info.index;
            result=build_node(&next_node_info,string,string_length,current_position);
            current_node_info->node = array + current_node_info->index;
            current_node_info->node->next = next_node_info.index;
            return result;
        }
        else {
            return current_node_info->index;
        }
    }
    else if(diff>0) {
        node_info<S,T> next_node_info;
        next_node_info.index = current_node_info->node->right;
        if(next_node_info.index==UNDEFINED_INDEX) {
            create_node(&next_node_info);
            current_node_info->node = array + current_node_info->index;
            current_node_info->node->right=next_node_info.index;
        }
        next_node_info.node = array + next_node_info.index;
        result=build_node(&next_node_info,string,string_length,current_position);
        current_node_info->node = array + current_node_info->index;
        current_node_info->node->right = next_node_info.index;

        if(next_node_info.balance_performed) {
            current_node_info->balance_performed=1;
        }
        else {
            if(current_node_info->node->left==UNDEFINED_INDEX) {
                current_node_info->height = next_node_info.height + 1;
                current_node_info->balance = next_node_info.height;
                current_node_info->left_balance = 0;
                current_node_info->right_balance = next_node_info.balance;
            }
            else {
                node_info<S,T> other_node_info;
                other_node_info.index = current_node_info->node->left;    
                other_node_info.node = array + other_node_info.index;
                compute_height_and_balance(&other_node_info);
                if(other_node_info.height>next_node_info.height) {
                    current_node_info->height=other_node_info.height+1;
                }
                else {
                    current_node_info->height=next_node_info.height+1;
                }
                current_node_info->balance = next_node_info.height-other_node_info.height;
                current_node_info->left_balance = other_node_info.balance;
                current_node_info->right_balance = next_node_info.balance;
            }
            balance_node(current_node_info);
        }

        return result;
    }
    else {
        node_info<S,T> next_node_info;
        next_node_info.index = current_node_info->node->left;
        if(next_node_info.index==UNDEFINED_INDEX) {
            create_node(&next_node_info);
            current_node_info->node = array + current_node_info->index;
            current_node_info->node->left=next_node_info.index;
        }
        next_node_info.node = array + next_node_info.index;
        result=build_node(&next_node_info,string,string_length,current_position);
        current_node_info->node = array + current_node_info->index;
        current_node_info->node->left = next_node_info.index;

        if(next_node_info.balance_performed) {
            current_node_info->balance_performed=1;
        }
        else {
            if(current_node_info->node->right==UNDEFINED_INDEX) {
                current_node_info->height = next_node_info.height + 1;
                current_node_info->balance = -next_node_info.height;
                current_node_info->right_balance = 0;
                current_node_info->left_balance = next_node_info.balance;
            }
            else {
                node_info<S,T> other_node_info;
                other_node_info.index = current_node_info->node->right;    
                other_node_info.node = array + other_node_info.index;
                compute_height_and_balance(&other_node_info);
                if(other_node_info.height>next_node_info.height) {
                    current_node_info->height=other_node_info.height+1;
                }
                else {
                    current_node_info->height=next_node_info.height+1;
                }
                current_node_info->balance = other_node_info.height-next_node_info.height;
                current_node_info->right_balance = other_node_info.balance;
                current_node_info->left_balance = next_node_info.balance;
            }

            balance_node(current_node_info);
        }

        return result;
    }
}

template<class S,class T> void tst<S,T>::remove_node(int* current_index,S* string,int string_length) {
    tst_node<S,T>* current_node = array + *current_index;
    int diff,*next_index;

    if(current_node->c==0) {
        return;
    }
    else {
        diff=(*string)-(current_node->c);
    }

    if(diff==0) {
        string++;
        string_length--;
        if(string_length>0) {
            next_index = &(current_node->next);
            if(*next_index!=UNDEFINED_INDEX) {
                remove_node(next_index,string,string_length);
            }
        }
        else {
            store_data(current_node,default_value,0);
        }
    }
    else if(diff>0) {
        next_index = &(current_node->right);
        if(*next_index!=UNDEFINED_INDEX) {
            remove_node(next_index,string,string_length);
        }
    }
    else {
        next_index = &(current_node->left);
        if(*next_index!=UNDEFINED_INDEX) {
            remove_node(next_index,string,string_length);
        }
    }

    current_node=array+*current_index;

    if(   current_node->data==default_value
        && current_node->next==UNDEFINED_INDEX
        && current_node->right==UNDEFINED_INDEX
        && current_node->left==UNDEFINED_INDEX) {
            // on ajoute le noeud � la liste des noeuds vides
            current_node->next=empty;
            empty = *current_index;
            *current_index=UNDEFINED_INDEX;
        }
    else {
        node_info<S,T> current_node_info;
        current_node_info.index=*current_index;
        current_node_info.node=current_node;
        current_node_info.height=-1;
        balance_node(&current_node_info);
        *current_index = current_node_info.index;
    }
}

template<class S,class T> tst_node<S,T>* tst<S,T>::find_node(int* current_index,int* best_node,S* string,int string_length) {
    tst_node<S,T>* _array=array;
    tst_node<S,T>* current_node;
    int diff;

    while(*current_index!=UNDEFINED_INDEX) {
        current_node=_array+*current_index;

        if(current_node->c==0) {
            *current_index=UNDEFINED_INDEX;
            return NULL;
        }
        else {
            diff=(*string)-(current_node->c);
        }

        if(diff==0) {
            if(current_node->data!=default_value) {
                *best_node=*current_index;
            }

            string++;
            string_length--;
            if(string_length>0) {
                *current_index = current_node->next;
            }
            else {
                return current_node;
            }
        }
        else if(diff>0) {
            *current_index=current_node->right;
        }
        else {
            *current_index=current_node->left;
        }
    }

    return NULL;
}

#ifdef SCANNER
template<class S,class T> void tst<S,T>::compute_backtrack(tst_node<S,T> *current_node,S* string, int si_match_start, int si_match_end) {
    if(current_node->backtrack==UNDEFINED_INDEX) {
        while(si_match_start<si_match_end) {
            current_node->backtrack=root;
            current_node->backtrack_match_index=UNDEFINED_INDEX;
            find_node(&(current_node->backtrack),&(current_node->backtrack_match_index),string+si_match_start,si_match_end-si_match_start);
            if(current_node->backtrack==UNDEFINED_INDEX) {
                si_match_start++;
            }
            else {
                current_node->backtrack=(array+current_node->backtrack)->next;
                break;
            }
        }
        if(current_node->backtrack==UNDEFINED_INDEX) {
            current_node->backtrack=root;
            current_node->backtrack_match_index=UNDEFINED_INDEX;
        }
    }
}

template<class S,class T> T tst<S,T>::scan(S* string,int string_length,action<S,T>* to_perform) {
    // Le premier caract�re de la chaine ne correspondant pas � un match
    int si_non_match_start=0;
    // Le noeud pour lequel on a enregistr� un match (noeud avec un objet associ�)
    int ni_best_match=UNDEFINED_INDEX;
    // Le premier caract�re de la chaine correspondant � un match
    int si_match_start=UNDEFINED_INDEX;
    // La position actuelle dans la cha�ne (index)
    int si_current=0;
    // Le num�ro du noeud actuel de l'arbre
    int ni_current=root;
    // Le noeud actuel de l'arbre
    tst_node<S,T> *n_current;
    // Boucle principale
    while(1) {
        n_current = array+ni_current;

        // On avance dans l'arbre d'un cran
        if(si_current<string_length) {
            int diff=string[si_current]-n_current->c;
            if(diff>0) {
                ni_current=n_current->right;
            }
            else if(diff<0) {
                ni_current=n_current->left;
            }
            else {
                // ok, le caract�re courant est accept�
                if(si_match_start==UNDEFINED_INDEX) {
                    // Si on n'a pas encore enregistr� un d�but de match
                    // on le fait
                    si_match_start=si_current;
                }
                
                if(n_current->data!=default_value) {
                    // Si le noeud en cours contient une valeur
                    // on l'enregistre
                    ni_best_match=ni_current;
                }

                si_current++;
                if(si_current<string_length) {
                    // Si on peut avancer, on avance
                    ni_current=n_current->next;
                }
                else {
                    // Fin de chaine ==> pas de match possible
                    ni_current=UNDEFINED_INDEX;
                }
            }
        }
        else {
            // On est toujours en fin de chaine ==> pas de match possible
            ni_current=UNDEFINED_INDEX;
        }

        if(ni_current==UNDEFINED_INDEX) {
            // Le caract�re courant n'est pas accept�
            if(ni_best_match!=UNDEFINED_INDEX) {
                // Si on a un match r�ussi en cours

                // Si le match en cours d�marre apr�s la zone de non-match
                // C'est qu'on a bien une zone de non-match
                int non_match_length = si_match_start-si_non_match_start;
                if(non_match_length>0) {
                    // On l'envoie.
                    to_perform->perform(string+si_non_match_start,non_match_length,-non_match_length,default_value);
                }

                // On envoie maintenant le match
                // On connait sa longueur gr�ce � la position dans l'arbre
                // TODO: voir si on ne pourrait pas de passer de �a
                tst_node<S,T> *match_node=array+ni_best_match;
                int match_length = match_node->position+1;
                // On envoie le match
                to_perform->perform(string+si_match_start,match_length,match_length,match_node->data);
                // On repositionne la zone de non-match juste apr�s la fin du match
                si_non_match_start=si_match_start+match_length;

                // Annulation du match
                ni_best_match=UNDEFINED_INDEX;

                // On backtracke
                compute_backtrack(n_current,string,si_non_match_start,si_current);
                // Le backtrack nous donne l� o� on se trouve dans le noeud juste apr�s le match...
                ni_current = n_current->backtrack;
                // ... et l� o� on avait notre meilleur match
                ni_best_match = n_current->backtrack_match_index;

                if(ni_current==root) {
                    // quand on a un retour � la racine, on n'a aucun match en cours
                    si_match_start=UNDEFINED_INDEX;
                }
                else {
                    // sinon c'est qu'un match est en cours, on va recalculer son point de d�marrage
                    // gr�ce � la position du noeud
                    n_current=array+ni_current;
                    si_match_start=si_current-n_current->position;
                }
            }
            else if(si_match_start!=UNDEFINED_INDEX) {
                // Si le caract�re courant n'est pas accept�, qu'on avait commenc�
                // un match mais que celui-ci n'avait pas r�ussi, on va backtracker.
                compute_backtrack(n_current,string,si_match_start+1,si_current);
                ni_current = n_current->backtrack;
                ni_best_match=n_current->backtrack_match_index;

                if(ni_current==root) {
                    // Quand on a un retour � la racine, on n'a aucun match en cours
                    si_match_start=UNDEFINED_INDEX;
                }
                else {
                    // sinon c'est qu'un match est en cours, on va recalculer son point de d�marrage
                    // gr�ce � la position du noeud
                    n_current=array+ni_current;
                    si_match_start=si_current-n_current->position;
                }
            }
            else {
                // le caract�re courant n'est pas accept� et on n'avait pas de match en cours
                if(si_current<string_length) {
                    // si on peut avancer d'un caract�re on le fait
                    si_current++;
                    // on revient � la racine
                    ni_current = root;
                }
                else {
                    // si on est � la fin de la cha�ne on sort de la boucle pour la fin.
                    break;
                }
            }
        }
        else {
            // on avance tranquillement dans l'arbre...
        }
    }

    // on n'arrive ici que si on est arriv� � la fin de la chaine en entr�e
    string_length = si_current - si_non_match_start;
    if(string_length>0) {
        // s'il y avait un non-match en cours
        // on l'envoie
        to_perform->perform(string+si_non_match_start,string_length,-string_length,default_value);
    }

    return to_perform->result();
}

template<class S> inline int is_in(S c,S* stop_chars,int stop_chars_length) {
    for(int i=0;i<stop_chars_length;i++,stop_chars++) {
        if(c==*stop_chars) {
            return 1;
        }
    }
    return 0;
}

template<class S,class T> T tst<S,T>::scan_with_stop_chars(S* string,int string_length,S* stop_chars,int stop_chars_length,action<S,T>* to_perform) {
    // Le premier caract�re de la chaine ne correspondant pas � un match
    int si_non_match_start=0;
    // Le noeud pour lequel on a enregistr� un match (noeud avec un objet associ�)
    int ni_best_match=UNDEFINED_INDEX;
    // Le premier caract�re de la chaine correspondant � un match
    int si_match_start=UNDEFINED_INDEX;
    // La position actuelle dans la cha�ne (index)
    int si_current=0;
    // Le num�ro du noeud actuel de l'arbre
    int ni_current=root;
    // Le noeud actuel de l'arbre
    tst_node<S,T> *n_current;
    // Boucle principale

    while(1) {
        n_current=array+ni_current;

        // On avance dans l'arbre d'un cran
        if(si_current<string_length) {
            int diff = string[si_current]-n_current->c;
            if(diff>0) {
                ni_current = n_current->right;
            }
            else if(diff<0) {
                ni_current = n_current->left;
            }
            else {
                // ok, le caract�re courant est accept�
                if(si_match_start==UNDEFINED_INDEX) {
                    // On teste si le d�but de match est le d�but de chaine ou apr�s un stop_char
                    if(si_current==0 || is_in(string[si_current-1],stop_chars,stop_chars_length)) {
                        // oui, on enregistre le d�but
                        si_match_start=si_current;
                    }
                    else {
                        // non, on passe � la suite
                        si_current++;
                    }
                }
                
                if(si_match_start!=UNDEFINED_INDEX) {
                    // si le d�marrage a bien pris

                    if(n_current->data!=default_value && (si_current==(string_length-1) || is_in(string[si_current+1],stop_chars,stop_chars_length))) {
                        // on a une donn�e dans le noeud courant
                        // et on est en fin de cha�ne ou le caract�re pr�c�dent est un stop_char
                        ni_best_match = ni_current;
                    }

                    si_current++;
                    if(si_current<string_length) {
                        // Si on peut avancer, on avance
                        ni_current = n_current->next;
                    }
                    else {
                        // Fin de chaine ==> pas de match possible
                        ni_current = UNDEFINED_INDEX;
                    }
                }
            }
        }
        else {
            // On est toujours en fin de chaine ==> pas de match possible
            ni_current = UNDEFINED_INDEX;
        }

        if(ni_current==UNDEFINED_INDEX) {
            // Le caract�re courant n'est pas accept�
            if(ni_best_match!=UNDEFINED_INDEX) {
                // Si on a un match r�ussi en cours

                // Si le match en cours d�marre apr�s la zone de non-match
                // C'est qu'on a bien une zone de non-match
                int non_match_length = si_match_start-si_non_match_start;
                if(non_match_length>0) {
                    // On l'envoie.
                    to_perform->perform(string+si_non_match_start,non_match_length,-non_match_length,default_value);
                }

                // On envoie maintenant le match
                // On connait sa longueur gr�ce � la position dans l'arbre
                // TODO: voir si on ne pourrait pas de passer de �a
                tst_node<S,T> *match_node=array+ni_best_match;
                int match_length = match_node->position+1;
                // On envoie le match
                to_perform->perform(string+si_match_start,match_length,match_length,match_node->data);
                // On repositionne la zone de non-match juste apr�s la fin du match
                si_non_match_start=si_match_start+match_length;

                // Annulation du match
                ni_best_match=UNDEFINED_INDEX;

                // On backtracke
                // backtrack de naze parce qu'on ne peut pas utiliser
                // les infos de backtrack � cause des s�parateurs.
                si_current = si_non_match_start;
                ni_current = root;
                ni_best_match=UNDEFINED_INDEX;
                si_match_start=UNDEFINED_INDEX;
            }
            else if(si_match_start!=UNDEFINED_INDEX) {
                // Si le caract�re courant n'est pas accept�, qu'on avait commenc�
                // un match mais que celui-ci n'avait pas r�ussi, on va backtracker.
                // backtrack de naze parce qu'on ne peut pas utiliser
                // les infos de backtrack � cause des s�parateurs.
                si_current=si_match_start+1;
                ni_current = root;
                ni_best_match=UNDEFINED_INDEX;
                si_match_start=UNDEFINED_INDEX;
            }
            else {
                // le caract�re courant n'est pas accept� et on n'avait pas de match en cours
                if(si_current<string_length) {
                    // si on peut avancer d'un caract�re on le fait
                    si_current++;
                    // on revient � la racine
                    ni_current = root;
                }
                else {
                    // si on est � la fin de la cha�ne on sort de la boucle pour la fin.
                    break;
                }
            }
        }
        else {
            // on avance tranquillement dans l'arbre...
        }
    }

    // on n'arrive ici que si on est arriv� � la fin de la chaine en entr�e
    string_length = si_current - si_non_match_start;
    if(string_length>0) {
        // s'il y avait un non-match en cours
        // on l'envoie
        to_perform->perform(string+si_non_match_start,string_length,-string_length,default_value);
    }

    return to_perform->result();
}
#endif

template<class S,class T> void tst<S,T>::balance_node(node_info<S,T>* bal) {
    if(bal->height==-1) {
        compute_height_and_balance(bal);
    }
    bal->balance_performed=0;
    if(bal->balance>1) {
        if(bal->right_balance>0) {
            rr(bal);
        }
        else {
            rl(bal);
        }
        bal->balance_performed=1;
    }
    else if(bal->balance<-1) {
        if(bal->left_balance<0) {
            ll(bal);
        }
        else {
            lr(bal);
        }
        bal->balance_performed=1;
    }
    
    assert(abs(bal->balance)<2);
    assert(abs(bal->right_balance)<2);
    assert(abs(bal->left_balance)<2);
}

template<class S,class T> void tst<S,T>::ll(node_info<S,T>* bal) {
    int left_index=bal->node->left;
    tst_node<S,T>* left_node=array+left_index;
    int left_right_index=left_node->right;
    bal->node->left=left_right_index;
    left_node->right=bal->index;
    
    bal->index=left_index;
    bal->node=array+left_index;
    bal->height=bal->height-1;
    bal->balance=0;
    bal->right_balance=0;
}

template<class S,class T> void tst<S,T>::rr(node_info<S,T>* bal) {
    int right_index=bal->node->right;
    tst_node<S,T>* right_node=array+right_index;
    int right_left_index=right_node->left;
    bal->node->right=right_left_index;
    right_node->left=bal->index;
    
    bal->index=right_index;
    bal->node=array+right_index;
    bal->height=bal->height-1;
    bal->balance=0;
    bal->left_balance=0;
}

template<class S,class T> void tst<S,T>::lr(node_info<S,T>* bal) {
    node_info<S,T> left;
    left.index = bal->node->left;
    left.node = array + left.index;
    rr(&left);
    bal->node->left=left.index;
    ll(bal);
}

template<class S,class T> void tst<S,T>::rl(node_info<S,T>* bal) {
    node_info<S,T> right;
    right.index = bal->node->right;
    right.node = array + right.index;
    ll(&right);
    bal->node->right=right.index;
    rr(bal);
}

template<class S,class T> void tst<S,T>::compute_height_and_balance(node_info<S,T>* current_node_info) {
    int left = current_node_info->node->left;
    int right = current_node_info->node->right;

    if(right!=UNDEFINED_INDEX) {
        node_info<S,T> right_balance;
        right_balance.index=right;
        right_balance.node=array+right;
        compute_height_and_balance(&right_balance);

        if(left!=UNDEFINED_INDEX) {
            node_info<S,T> left_balance;
            left_balance.index=left;
            left_balance.node=array+left;
            compute_height_and_balance(&left_balance);

            if(left_balance.height > right_balance.height) {
                current_node_info->height = left_balance.height+1;
            }
            else {
                current_node_info->height = right_balance.height+1;
            }
            current_node_info->balance = right_balance.height-left_balance.height;
            current_node_info->right_balance = right_balance.balance;
            current_node_info->left_balance = left_balance.balance;
        }
        else {
            current_node_info->height = right_balance.height + 1;
            current_node_info->balance = right_balance.height;
            current_node_info->right_balance = right_balance.balance;
            current_node_info->left_balance = 0;
        }
    }
    else {
        if(left!=UNDEFINED_INDEX) {
            node_info<S,T> left_balance;
            left_balance.index=left;
            left_balance.node=array+left;
            compute_height_and_balance(&left_balance);

            current_node_info->height = left_balance.height + 1;
            current_node_info->balance = -left_balance.height;
            current_node_info->right_balance = 0;
            current_node_info->left_balance = left_balance.balance;
        }
        else {
            current_node_info->height = 0;
            current_node_info->balance = 0;
            current_node_info->right_balance = 0;
            current_node_info->left_balance = 0;
        }
    }
}

template<class S,class T> size_t tst<S,T>::bytes_allocated() {
    return sizeof(tst)+size*sizeof(tst_node<S,T>);
}

template<class S,class T> int tst<S,T>::get_maximum_key_length() {
    return maximum_key_length;
}

#endif
