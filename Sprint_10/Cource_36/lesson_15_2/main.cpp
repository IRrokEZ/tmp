#include "json_reader.h"
#include "request_handler.h"

int main() {
    transport::Catalogue catalogue;
    JsonReader json_doc(std::cin);
    
    json_doc.FillCatalogue(catalogue);
    
    const auto& render_settings = json_doc.GetRenderSettings().AsMap();
    const auto& renderer = json_doc.FillRenderSettings(render_settings);
    
    RequestHandler rh(catalogue, renderer);
    rh.RenderMap().Render(std::cout);
}