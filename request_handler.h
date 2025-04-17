#pragma once

#include "transport_catalogue.h"

namespace request_handler {

class RequestHandler {
public:
    explicit RequestHandler(transport_catalogue::TransportCatalogue& catalogue)
        : catalogue_(catalogue) {
    }
    
    // Методы для обработки запросов могут быть добавлены здесь
    
private:
    transport_catalogue::TransportCatalogue& catalogue_;
};

} // namespace request_handler