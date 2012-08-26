#include <iostream>
#include <thread>
#include <chrono>

#include <rainbow/rainbow.hpp>

using namespace rb;

struct Shader_entry {
    const char* name;
    const char* vert;
    const char* frag;
    Shader* shader;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << argv[0] << " <model, ...>" << std::endl;
        return 2;
    }

    bool quit = false;

    // Initialize the display
    Display display("Rainbow");
    display.resolution(640, 480);

    // Handle our input binding in its base form
    Input input;
    input.bind('q', [&quit](int event) { quit = true; });

    // Handle some basic shader tests
    Renderer& renderer = *display.renderer;
    Shader_entry shaders[] = {
        {"barebones", "game/shaders/barebones.vert", 
            "game/shaders/barebones.frag", nullptr},
    };

    // Load the shaders
    for (Shader_entry& e: shaders) {
        int n = renderer.add_shader(e.name, e.vert, e.frag, false);
        Shader* shader = renderer.shaders[n].get();
        if (!shader->valid) {
            std::cerr << shader->error << std::endl;
            return 3;
        }
        e.shader = shader;
    }

    for (int i = 1; i < argc; i++) {
        std::cout << "Loading: " << argv[1] << "\n";
        Model_unique obj = load_model(argv[1], Model_format::OBJ);
        renderer.add_static_vertices(obj->verts(),
                                     obj->vert_size(),
                                     obj->elements(),
                                     obj->element_size());
    }

    // render a test square with the 
    Shader* barebones = shaders[0].shader;
    barebones->attrib("position").vec3(11, 0);
    barebones->attrib("color").vec3(11, 3);
    barebones->use();

    glm::mat4 model;
    model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = glm::lookAt(glm::vec3( 1.2f, 1.2f, 1.2f ),
                                 glm::vec3( 0.0f, 0.0f, 0.0f ),
                                 glm::vec3( 0.0f, 0.0f, 1.0f ));
    glm::mat4 proj = glm::perspective( 45.0f, 800.0f / 600.0f, 1.0f, 10.0f );

    Uniform umodel = barebones->uniform("model");
    Uniform uview = barebones->uniform("view");
    Uniform uproj = barebones->uniform("proj");

    uview.mat4(view);
    uproj.mat4(proj);
    
    int rotation = 0;
    input.bind('w', [&rotation](int event) { 
        rotation = (rotation + 1) % 180;
    }).bind('s', [&rotation](int event) {
        rotation = (rotation - 1) % 180;
    });

    // Run the actual engine
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    uint64_t frame = 0;
    start = std::chrono::high_resolution_clock::now();
    do {
        input.run();
        display.clear();
        display.run();
        display.end_frame();
        model = glm::rotate(model, float(rotation), 
                            glm::vec3(0.0f, 0.5f, 1.0f));

        umodel.mat4(model);
        frame++;
    } while(!quit);
    end = std::chrono::high_resolution_clock::now();
    double frame_time = std::chrono::duration_cast<std::chrono::seconds> (end - start).count();
    std::cout << "FPS: " << frame / frame_time << "\n";

    return 0;
}