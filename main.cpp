#include "common.h"
#include "pcfg.h"
#include "binarytree.h"
int main() {

    //PCFG *ptr_pcfg = new PCFG("ptb-binary.train");
    //ptr_pcfg->Training();
     std::string str = "(ROOT (NP (NNP Ms.)(NNP Haag))(S (VP (VBZ plays)(NNP Elianti))(. .)))";
    //std::string str = "(ROOT (PP (IN In)(NP (NP (NP (DT an)(NP (NNP Oct.)(NP (CD 19)(NN review))))(PP (IN of)(NP (`` ``)(NP (NP (NP (DT The)(NN Misanthrope))('' ''))(PP (IN at)(NP (NP (NNP Chicago)(POS 's))(NP (NNP Goodman)(NNP Theatre))))))))(PRN (PRN (PRN (PRN (PRN (PRN (-LRB- -LRB-)(`` ``))(S (NP (VBN Revitalized)(NNS Classics))(VP (VP (VBP Take)(NP (DT the)(NN Stage)))(PP (IN in)(NP (NNP Windy)(NNP City))))))(, ,))('' ''))(NP (NP (NN Leisure)(CC &))(NNS Arts)))(-RRB- -RRB-))))(S (, ,)(S (NP (NP (NP (NP (NP (DT the)(NN role))(PP (IN of)(NNP Celimene)))(, ,))(VP (VBN played)(PP (IN by)(NP (NNP Kim)(NNP Cattrall)))))(, ,))(S (VP (VBD was)(VP (RB mistakenly)(VP (VBN attributed)(PP (TO to)(NP (NNP Christina)(NNP Haag))))))(. .)))))";
    BinaryTree *ptr_tree = new BinaryTree(str);
    delete(ptr_tree);
    return 0;
}