#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport::Catalogue catalogue;
    JsonReader requests(std::cin);
    requests.FillCatalogue(catalogue);
    RequestHandler(requests, catalogue);
}