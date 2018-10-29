<?php

$words = [
    "лава",
    "лаванда",
    "слом",
];

//create trie
$trie = trie_new();
foreach($words as $i => $w){
    //add word ($w) to the trie ($trie) with data ($i). Trie data is an int32 value
    trie_store($trie, $w, $i);
}

//create trie state
$s = trie_state_new($trie);

//init result array
$res = [];

//keyword to search
$key = "лавандами";

//search trie cycle
for($i = 0; trie_state_is_walkable($s, $key[$i]); ++$i){
    //walk to the next trie state ($s) using char ($key[$i])
    trie_state_walk($s, $key[$i]);

    //if current trie state is a terminal state. A terminal state is a trie
    // state that terminates a key, and stores a value associated with it.
    if(trie_state_is_terminal($s)){
        //write key corresponding to the current state
        $res[] = substr($key, 0, $i+1);
    }
}


//show trie words fully or partially included in the keyword.
print_r($res);


//show all trie words
print_r(trie_enumerate($trie));