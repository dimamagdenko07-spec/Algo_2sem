#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "extra_tasks.h"
#include <assert.h>
int empty_str(){
    /*Пустая строка*/
    char text[2000] = "";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 0);
    vectorFree(test);
    return 0;
}


int one_clean_word(){
    /*Одно чистое слово*/
    char text[2000] = "day";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 1);
    vectorFree(test);
    return 0;
}


int one_dirty_word(){
    /*Одно слово с мусором*/
    char text[2000] = ".:::>?><>?12423432525day142;;%;%;:№::№;";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 1);
    vectorFree(test);
    return 0;
}


int lots_of_similar_words(){
    /*Много одинаковых слов с разным написанием*/
    char text[2000] = ".DAY :::> DaY? ><>?1dAY24 234 DAy3 2525day142;;%;%;: DAY/№::№;";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 6);
    vectorFree(test);
    return 0;
}


int lots_of_diff_words(){
    /*Много разных слов без повторений*/
    char text[2000] = "play 555 banan//// day ,./.,flag..  . ";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 4);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "play") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 1)).word, "banan") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 1)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 2)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 2)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 3)).word, "flag") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 3)).count == 1);
    vectorFree(test);
    return 0;
}

int lots_of_diff_repeated_words(){
    /*Много разных слов с повторениями*/
    char text[2000] = "play   555 banan//   /play /   day ,.   banan/.   ,flag..   day flAG .   ";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 4);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "play") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 2);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 1)).word, "banan") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 1)).count == 2);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 2)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 2)).count == 2);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 3)).word, "flag") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 3)).count == 2);
    vectorFree(test);
    return 0;
}


int starts_with_space(){
    /*Начинается с кучи пробелов*/
    char text[2000] = "       day";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 1);
    vectorFree(test);
    return 0;
}


int end_with_space(){
    /*Заканчивается кучей пробелов*/
    char text[2000] = "day       ";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "day") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 1);
    vectorFree(test);
    return 0;
}


int tests_from_readme(){
    /*Тесты задания*/
    char text[2000] = "The quick brown fox jumps over the lazy dog. The dog was not that lazy.";
    Vector *test = textToBoW(&text[0]);
    assert((*test).size == 11);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 0)).word, "the") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 0)).count == 3);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 1)).word, "quick") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 1)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 2)).word, "brown") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 2)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 3)).word, "fox") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 3)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 4)).word, "jumps") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 4)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 5)).word, "over") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 5)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 6)).word, "lazy") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 6)).count == 2);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 7)).word, "dog") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 7)).count == 2);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 8)).word, "was") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 8)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 9)).word, "not") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 9)).count == 1);
    assert(wordEquals((*(WordFrequency*)getVectorItem(test, 10)).word, "that") == 1);
    assert((*(WordFrequency*)getVectorItem(test, 10)).count == 1);
    vectorFree(test);
    return 0;
}
int bag_of_words(){
    /*Тесты системы Bag-of-Words*/
    empty_str();
    one_clean_word();
    one_dirty_word();
    lots_of_similar_words();
    lots_of_diff_words();
    lots_of_diff_repeated_words();
    starts_with_space();
    end_with_space();
    tests_from_readme();
    return 0;
    /*Конец тестов*/
}


int main(){
    bag_of_words();
    return 0;
}