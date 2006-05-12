# -*- coding: iso-8859-1 -*-
from array import array
import sys

# Tant qu'� faire vu qu'ici le nombre de caract�res par noeud est compl�tement
# dynamique, on peut se l�cher, mettre 1024 caract�re, ce qui revient � une
# impl�mentation de Patricia avec en plus le partage des caract�res de pr�fixe
# commun
CHARS_PER_NODE = 1024

class tst_node(object):
    """ classe repr�sentant un noeud du TST """

    # __slots__ est une optimisation permettant de cr�er des objets Python
    # non dynamiques, ce qui utilise moins de m�moire
    __slots__ = ['chars','data','next','left','right']

    instances = 0

    def __init__(self):
        tst_node.instances += 1
        self.chars = array('c')
        self.data = None
        self.next = None
        self.left = None
        self.right = None
    
    def __repr__(self):
        return "node(%s,data=%s,%i,%i,%i)"%(
            self.chars,
            self.data,
            self.left is not None and 1 or 0,
            self.next is not None and 1 or 0,
            self.right is not None and 1 or 0,
        )

class compact_tst(object):
    """ Impl�mentation d'un TST compact """

    def __init__(self):
        self.root = None

    def __getitem__(self,string):
        """ Lit dans l'arbre selon la syntaxe tst[string] """
        node = self.root
        index = 0
        
        while node is not None:
            local_index = 0
    
            # On avance tant qu'il y a �galit�
            diff = 0
            while local_index < len(node.chars) and index < len(string):
                diff = cmp(string[index],node.chars[local_index]) 
                if diff == 0:
                    local_index += 1
                    index += 1
                else:
                    break
            
            if local_index < len(node.chars) - 1:
                # on s'est arr�t� avant le dernier caract�re du noeud,
                # il n'y a donc pas de match possible (sinon il y aurait eu
                # split � l'insertion)
                return None
            
            elif local_index == len(node.chars) - 1:
                # diff�rence au dernier caract�re du noeud
                # on va donc aller soit � droite, soit � gauche
                if diff>0:
                    node = node.left
                elif diff<0:
                    node = node.right
                else:
                    node = node.next
            else:
                # tous les caract�res du noeud correspondent � ceux de la cha�ne
                if index == len(string):
                    # si on est en fin de cha�ne, on retourne la donn�e stock�e
                    # dans le noeud, si elle existe
                    return node.data
                else:
                    # sinon, on passe au noeud suivant
                    node = node.next
        
        # node is None ==> pas de match
        return None

    
    def __setitem__(self,string,value):
        """ Ecrit dans l'arbre selon la syntaxe tst[string] = value """
        self.root = self._insert(string,value,0,self.root)
        assert self[string] == value

    def _insert(self,string,value,index,node):
        if node is None:
            return self._new_node(string,value,index)
    
        local_index = 0
        
        # On avance tant qu'il y a �galit�
        diff = 0
        while local_index < len(node.chars) and index<len(string):
            diff = cmp(string[index],node.chars[local_index])
            if diff == 0:
                local_index += 1
                index += 1
            else:
                break
        
        if diff!=0:
            assert local_index < len(node.chars) and index<len(string)
        
            # On a trouv� un point de divergence avant le dernier caract�re du
            # noeud, et de la cl�, il va donc falloir splitter
            if local_index < len(node.chars) - 1:
                node = self._split_node(node,local_index)
            
            # Maintenant que le split est fait, on peut continuer � positionner
            # la nouvelle cl�
            if diff>0:
                node.left = self._insert(string,value,index,node.left)
            else:
                node.right = self._insert(string,value,index,node.right)

            # Puisqu'on vient d'effectuer une op�ration qui a pu perturber
            # l'�quilibre du noeud, on le r�tablit. Bien s�r cela peut changer
            # le noeud devant �tre retourn� � l'appelant.
            node = self._balance_node(node)

            return node

        elif local_index == len(node.chars):
            # On est arriv� au bout des caract�res du noeud
            # sans diff�rence
            
            if index == len(string):
                # On est �galement au bout de la cl�
                # C'est qu'on a de la chance !
                node.data = value
            else:
                # On n'est pas au bout de la cl�
                node.next = self._insert(string,value,index,node.next)
        
            return node

        else:
            # On est arriv� au bout de la cl�, mais pas au bout des caract�res
            # du noeud
            assert index == len(string)
            
            # On est au bout de la cl�, mais avant la fin des caract�res du
            # noeud ; il faut donc splitter, mais au local_index pr�c�dent car
            # on a b�tement avanc� les deux � la fois aux lignes 105 - 106 
            node = self._split_node(node,local_index-1)

            # On stocke ensuite la cl� et la valeur
            node.data = value
            
            return node

    def _split_node(self,node,local_index):
        assert local_index < len(node.chars)
        
        # On cr�e un nouveau noeud
        new_node = tst_node()
        
        # On prend tout le d�but du segment de cl� du noeud y compris
        # le caract�re qui diff�re et on les met dans le nouveau noeud
        new_node.chars = node.chars[0:local_index + 1]

        # La suite de ce nouveau noeud est l'ancien noeud
        new_node.next = node

        # On adapte la cha�ne dans l'ancien noeud, c'est le restant de
        # la cha�ne apr�s le split
        node.chars = node.chars[local_index + 1:]
        
        return new_node

    def _new_node(self,string,value,index):
        new_node = tst_node()

        # On remplit le segment du noeud avec un maximum de caract�res de la cl�
        # en partant de l'index donn�
        length = min(len(string)-index,CHARS_PER_NODE)
        new_node.chars.extend(string[index:index+length])
        
        if index+length<len(string):
            # s'il reste des caract�res dans la cl� apr�s ce segment...
            new_node.next = self._new_node(string,value,index+length)
        else:
            # sinon on met la cl� et la donn�e dans ce noeud
            new_node.data = value
        
        return new_node
    
    def _balance_node(self,node):
        # ici v�rifier le crit�re AVL et faire une rotation LL, RR, RL ou LR
        # si n�cessaire
        return node
    
    def stats(self,node,acc):
        if node == None : return
        
        acc['nodes'] = acc.get('nodes',0) + 1
        key = ('nbchars',len(node.chars)) 
        acc[key] = acc.get(key,0) + 1
        
        links = ((node.left is not None and 1) or 0) + ((node.next is not None and 1) or 0) + ((node.right is not None and 1) or 0) 
        key = ('links',links)
        acc[keys] = acc.get(key,0) + 1
        
        self.stats(node.left,acc)
        self.stats(node.next,acc)
        self.stats(node.right,acc)
        
if __name__ == '__main__':
    urls = compact_tst()
    try:
        chars = 0
        for n, l in enumerate(file('url-list.txt','rb')):
            if n%1000==0 : print n
            key = l.rstrip()
            chars += len(key)
            urls[key] = 0
    finally:
        stats = {}
        urls.stats(urls.root,stats)
        print n+1, "lignes", chars, "caracteres"
        for key in sorted(stats.keys()):
            print "%16s\t%6i\t%6.2f%%"%(
                key,
                stats[key],
                100.0 * stats[key] / stats['nodes']
            ) 

    t = compact_tst()
    t['nicolas'] = 'nicolas'
    t['laurent'] = 'laurent'
    t['nicolas lehuen'] = 'nicolas lehuen'
    t['laurent querel'] = 'laurent querel'
    assert 'nicolas' == t['nicolas']
    assert 'nicolas lehuen' == t['nicolas lehuen']
    assert 'laurent' == t['laurent']
    assert 'laurent querel' == t['laurent querel']

    import random
        
    t = compact_tst()
    
    data = range(100000)
    random.shuffle(data)
    
    for i, d in enumerate(data):
        t[str(d)] = i
    
    for i,d in enumerate(data):
        assert t[str(d)] == i, "%s => %s != %s"%(d,i,t[str(d)]) 
