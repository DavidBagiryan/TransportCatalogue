#include <iostream>

#include "json_reader.h"

int main() {
    json_reader::JsonReader processing(std::cout);
    processing.Reader();
}