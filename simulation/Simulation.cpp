//
// Created by benji on 08/11/16.
//

#include "Simulation.h"

EventListener* EventListener::listener = NULL;

void EventListener::init(Window * mainWindow) {
    EventListener::listener = this;

    mainWindow->setKeyCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
        EventListener::listener->glfwKeyCallback(window, key, scancode, action, mods);
    });

    mainWindow->setScrollCallback([](GLFWwindow* window, double xoffset, double yoffset) {
        EventListener::listener->glfwScrollCallback(window, xoffset, yoffset);
    });
}

void EventListener::glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    listener->keyCallback(window, key, scancode, action, mods);
}

void EventListener::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    listener->scrollCallback(window, xoffset, yoffset);
}

void EventListener::setEventListener(EventListener* eventListener) {
    listener = eventListener;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Simulation::Simulation(int ruleID) :
        Simulation(preConfiguredRules[ruleID <= PRE_CONFIGURED_RULES_NUMBER ? ruleID - 1 : 0]) {
    _currentPreConfiguredRules = ruleID <= PRE_CONFIGURED_RULES_NUMBER ? ruleID - 1 : 0;
}

Simulation::Simulation(RuleDefinition rules) {
    // Scene creation
    _rules = std::make_shared<Rules>(rules);
    _grid = std::make_shared<Grid3D>();
    _scene = std::make_unique<Scene>(_grid);

    setEventListener(this);

    createWindow();
}

void Simulation::initialize() {
    std::cout << _BOLD(_MAGENTA("Initialization of the simulation")) << std::endl;
    _count = 0;
    _grid->reset(_rules->getListColors());

    _listAnts.clear();

    for(unsigned int i = 0 ; i < _antsPosition.size() ; i++) {
        Vector3 position = _antsPosition[i];

        _grid->addCube(position, AllColors::NULL_COLOR);

        std::unique_ptr<Ant> ant = std::make_unique<Ant>(position, _rules->getInitOrientation());
        _listAnts.push_back(std::move(ant));
    }

    // Disable the 60 FPS limit
    glfwSwapInterval(0);

    _beginSimulation = glfwGetTime();
}

void Simulation::createRules() {
    pauseSimulation(true);
    RuleDefinition rules;
    std::cout << _UNDL(_BOLD(_BLUE("\nRules sandbox for the simulation :\n")));

    bool continueGetRules = true;

    while(continueGetRules) {
        Color color;

        std::cout << std::endl;
        std::cout << _CYAN("To add a rule, give one of the following color :\n");
        for(int i = 0 ; i < COLORS_NUMBER ; i++) {
            Color c((AllColors)i);
            std::cout << "'" << c.getColorName() << "' ";
        }
        std::cout << _CYAN("\nOr write [end|exit|quit|stop] to quit") << std::endl;

        std::string userEntry;
        getline(cin, userEntry);
        cin.clear();

        if(userEntry == "end" || userEntry == "exit" || userEntry == "quit" || userEntry == "stop" || userEntry == "e") {
            continueGetRules = false;
            emptyBuffer();
        } else if(color.setColor(userEntry)) { // The user gives a correct color
            bool moveCorrect = false;
            int move = -1;
            while (!moveCorrect) {
                std::cout << "\n1 : GO_FRONT  ;  2 : GO_BACK  ;  3 : GO_RIGHT  ;  4 : GO_LEFT  ;  5 : GO_UP  ;  6 : GO_DOWN  ;  7 : DO_NOTHING" << std::endl;
                emptyBuffer();

                cin >> move;
                cin.clear();

                if (move < 1 or move > 7) {
                    std::cerr << _RED("Invalid integer.") << std::endl;
                } else {
                    moveCorrect = true;
                }
                emptyBuffer();
            }
            rules.addRule(color, (Move)(move - 1));
        }
        emptyBuffer();
    }
    std::cout << '\n' << std::endl;

    if(rules.size() > 0) {
        setRules(rules);
        _window->setTitle("Langton 3D - new rules");
    }
    pauseSimulation(false);
}

void Simulation::setRules(int ruleID) {
    if (ruleID <= PRE_CONFIGURED_RULES_NUMBER) {
        _currentPreConfiguredRules = ruleID;
    } else {
        std::cerr << _RED("Unexisting rule. Launching of the rule 1.") << std::endl;
        _currentPreConfiguredRules = 1;
    }
    setRules(preConfiguredRules[_currentPreConfiguredRules - 1]);

    _window->setTitle("Langton 3D - rule n°" + to_string(_currentPreConfiguredRules));
}

void Simulation::setRules(RuleDefinition rules) {
    _rules->loadRule(rules);
    initialize();
}

Simulation::~Simulation() {
    _listAnts.clear();
}

void Simulation::mainLoop() {
    if (!_pauseSimulation) {
        // Update of the simulation
        for (unsigned int i = 0; i < _listAnts.size(); i++) {
            Vector3 pos = _listAnts[i]->getPosition();
            _grid->update(pos, _rules);

            _listAnts[i]->update(_grid->getColor(), _rules);
        }
        _count++;
    }

    for(int step : _checkpointsList) {
        if(_count == step)
            pauseSimulation(true);
    }

    // Update of the display
    _window->setupFrame();

    if (!_pauseDisplaying) {
        if (!_scene->getCamera().isTraveling()) {
            _scene->render(_window->context());
            input();
        }
    }

    _window->finalizeFrame();
}

void Simulation::start() {
    centerCamera();

    while(_window->isWindowOpened()) {
        mainLoop();
    }
    std::cout << _BOLD(_MAGENTA("\nEnd of the simulation")) << std::endl;
}

void Simulation::pauseSimulation(bool desactivate) {
    // pausing
    if(!_pauseSimulation and desactivate) {
        _pauseSimulation = true;
        _beginSimulationPaused = glfwGetTime();
    }
    // already paused : relaunch
    else if(_pauseSimulation and !desactivate) {
        _pauseSimulation = false;
        _beginSimulation += glfwGetTime() - _beginSimulationPaused;
    }
}

void Simulation::addCheckpoints() {
    pauseSimulation(true);
    std::cout << _BOLD(_BLUE("\nCheckpoints sandbox of the simulation\n")) <<
              _CYAN("[init|delete|null]   ") _YELLOW(": ") "Delete existing checkpoints\n"
              _CYAN("[INT|now]            ") _YELLOW(": ") "Add a new checkpoint\n"
              _CYAN("[get]                ") _YELLOW(": ") "Get the checkpoint list\n"
              _CYAN("[end|exit|quit|stop] ") _YELLOW(": ") "Quit\n" << std::endl;

    bool continueGetChekpoints = true;

    while(continueGetChekpoints) {
        std::string userEntry;
        getline(cin, userEntry);
        cin.clear();

        if(userEntry == "end" || userEntry == "exit" || userEntry == "quit" || userEntry == "stop" || userEntry == "e") {
            continueGetChekpoints = false;
            emptyBuffer();
        } else if(userEntry == "init" || userEntry == "delete" || userEntry == "null") {
            _checkpointsList.clear();
            std::cout << _YELLOW("Deletion of all checkpoints\n");
        } else if(userEntry == "get") {
            if(_checkpointsList.size() == 0)
                std::cout << _CYAN("No existing checkpoint\n");
            else {
                std::cout << _CYAN("Checkpoints : ");
                for (int checkpoint : _checkpointsList)
                    std::cout << checkpoint << ' ';
                std::cout << '\n';
            }
        } else if(userEntry == "now") {
            _checkpointsList.push_back(_count);
        } else if(std::atoi(userEntry.c_str()) > 0) {
            _checkpointsList.push_back(std::atoi(userEntry.c_str()));
        } else {
            _RED("Bad entry\n");
        }
        emptyBuffer();
    }

    std::cout << _BOLD(_BLUE("Leaving of the checkpoints sandbox\n")) << std::endl;
    emptyBuffer();
    pauseSimulation(false);
}


void Simulation::input() {
    glm::vec3 cameraUp = _scene->getCamera().getUp();

    createControlKeys();

    if(cameraUp.x == 0 && cameraUp.y == 0) {
        /// Arrow keys : rotation control
        if(_rightKey == GLFW_PRESS && _leftKey != GLFW_PRESS) {
            _scene->getCamera().rotateZ(0.8f);
        } else if(_leftKey == GLFW_PRESS && _rightKey != GLFW_PRESS) {
            _scene->getCamera().rotateZ(-0.8f);
        }

        if(_upKey == GLFW_PRESS && _downKey != GLFW_PRESS) {
            _scene->getCamera().rotateUpDown(0.8f);
        } else if(_downKey == GLFW_PRESS && _upKey != GLFW_PRESS) {
            _scene->getCamera().rotateUpDown(-0.8f);
        }

        /// ZQSD keys : traveling control
        //glm::vec3 eyePos = _scene->getCamera().getEye();
        //float duration = 0.2f;
        if(_keyD == GLFW_PRESS && _keyQ != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x, center.y + 0.5f, center.z);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x, eye.y + 0.5f, eye.z);
        } else if(_keyQ == GLFW_PRESS && _keyD != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x, center.y - 0.5f, center.z);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x, eye.y - 0.5f, eye.z);
        }

        if(_keyZ == GLFW_PRESS && _keyS != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x + 0.5f, center.y, center.z);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x + 0.5f, eye.y, eye.z);
        } else if(_keyS == GLFW_PRESS && _keyZ != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x - 0.5f, center.y, center.z);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x - 0.5f, eye.y, eye.z);
        }

        if(_keyA == GLFW_PRESS && _keyE != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x, center.y, center.z + 0.5f);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x, eye.y, eye.z + 0.5f);
        } else if(_keyE == GLFW_PRESS && _keyA != GLFW_PRESS) {
            glm::vec3 center = _scene->getCamera().getCenter();
            _scene->getCamera().setCenter(center.x, center.y, center.z - 0.5f);
            glm::vec3 eye = _scene->getCamera().getEye();
            _scene->getCamera().setEye(eye.x, eye.y, eye.z - 0.5f);
        }

    } else if(cameraUp.z == 0) {
        // TODO
    }
}

void Simulation::addAnt(int x, int y, int z) {
    addAnt(Vector3(x, y, z));
}

void Simulation::addAnt(Vector3 position) {
    Vector3 pos = position;
    pos += Vector3(500, 500, 500);
    _antsPosition.push_back(pos);
}

void Simulation::createWindow() {
    _window = std::make_unique<Window>("Langton 3D");

    EventListener::init(_window.get());

    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // white background
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // grey  backgroung
}

void Simulation::createControlKeys() {
    /// Rotation control
    _rightKey = glfwGetKey(_window->window(), GLFW_KEY_RIGHT);
    _leftKey  = glfwGetKey(_window->window(), GLFW_KEY_LEFT);
    _upKey    = glfwGetKey(_window->window(), GLFW_KEY_UP);
    _downKey  = glfwGetKey(_window->window(), GLFW_KEY_DOWN);

    /// Traveling control
    _keyA = glfwGetKey(_window->window(), GLFW_KEY_Q);
    _keyZ = glfwGetKey(_window->window(), GLFW_KEY_W);
    _keyE = glfwGetKey(_window->window(), GLFW_KEY_E);
    _keyQ = glfwGetKey(_window->window(), GLFW_KEY_A);
    _keyS = glfwGetKey(_window->window(), GLFW_KEY_S);
    _keyD = glfwGetKey(_window->window(), GLFW_KEY_D);
}

void Simulation::centerCamera() {
    if(_antsPosition.size() > 0) {
        Vector3 posAnt = _antsPosition[0];
        _scene->getCamera().moveCameraByCenterPoint(posAnt.x, posAnt.y, posAnt.z);
        _scene->getCamera().setCenter(posAnt.x * 2, posAnt.y * 2, posAnt.z * 2);
        _scene->getCamera().setEye(posAnt.x * 5, posAnt.y * 5, posAnt.z * 5);
        _scene->getCamera().zoom(0.01f);
    }
}

/*
 * Callbacks definition
 */
void Simulation::keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) {
    if(action == GLFW_PRESS) {
        // Exit the simulation
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
            // Ants pause
        else if (key == GLFW_KEY_SPACE)
            pauseSimulation(!_pauseSimulation);
            // Displaying pause
        else if (key == GLFW_KEY_TAB)
            _pauseDisplaying = !_pauseDisplaying;
            // Get simulation time (in ants steps)
        else if (key == GLFW_KEY_ENTER) {
            double timeSimulation = glfwGetTime() - _beginSimulation;
            std::cout << _CYAN("Time of simulation :   ") << _count << " steps\n                       " << timeSimulation << " seconds\n"
                         _CYAN("Frequency of updates : ") <<_count / timeSimulation << "\n" <<
                         _YELLOW("--------------------------------------") << std::endl;
        }
            // Access to the previous rule
        else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_PAGE_DOWN) {
            if (_currentPreConfiguredRules > 1)
                setRules(_currentPreConfiguredRules - 1);
            else
                setRules(PRE_CONFIGURED_RULES_NUMBER);
        }
            // Access to the next rule
        else if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_PAGE_UP) {
            if (_currentPreConfiguredRules < PRE_CONFIGURED_RULES_NUMBER)
                setRules(_currentPreConfiguredRules + 1);
            else
                setRules(1);
        }
            // User creates its own rules
        else if (key == GLFW_KEY_KP_ENTER) {
            createRules();
        } // display the current rules
        else if (key == GLFW_KEY_R) {
            std::cout << _BOLD(_BLUE("List of all the rules :\n")) << _rules->getRules() << std::endl;
        }
            // Get the composition of the grid TODO : display the number of each color cubes
        else if (key == GLFW_KEY_N) {
            std::cout << _BOLD(_BLUE("Composition of the grid : ")) << _grid->getSize() << _BOLD(_BLUE(" cubes.\n")) << std::endl;
        }
            // Reset the simulation
        else if (key == GLFW_KEY_BACKSPACE) {
            initialize();
        } else if (key == GLFW_KEY_C) {
            centerCamera();
        } else if (key == GLFW_KEY_P) {
            addCheckpoints();
        }
            // Display an help message
        else if (key == GLFW_KEY_H) {
            printHelp();
        }
    }
}

void Simulation::printHelp() {
    std::cout << _BOLD(_BLUE("\nCommands to control the 3D Langton's Ant simulation :\n"))
            _CYAN("[Arrows]    ")                       _YELLOW(": ") "Orientate the camera\n"
            _CYAN("A           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("+Y\n")
            _CYAN("Z           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("+Z\n")
            _CYAN("E           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("-Y\n")
            _CYAN("Q           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("-X\n")
            _CYAN("S           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("-Z\n")
            _CYAN("D           ")                       _YELLOW(": ") "Moving of the camera : " _GREEN("+X\n\n")

            _CYAN("[BACKSPACE] ")                       _YELLOW(": ") "Reset the simulation\n"
            _CYAN("[ENTER]     ")                       _YELLOW(": ") "Get the time of the simulation (on ant steps)\n"
            _CYAN("[ENTER-KP]  ")                       _YELLOW(": ") "Create our own rules\n"
            _CYAN("[Escape]    ")                       _YELLOW(": ") "Quit\n"
            _CYAN("[SCROLL]    ")                       _YELLOW(": ") "Zoom\n"
            _CYAN("[SPACE]     ")                       _YELLOW(": ") "Pause the simulation\n"
            _CYAN("[TAB]       ")                       _YELLOW(": ") "Pause the displaying\n\n"

            _CYAN("PageUp  ") _YELLOW("| ") _CYAN("+ ") _YELLOW(": ") "Load next pre-configured rule\n"
            _CYAN("PageDwn ") _YELLOW("| ") _CYAN("- ") _YELLOW(": ") "Load previous pre-configured rule\n\n"

            _CYAN("C           ")                       _YELLOW(": ") "Center the camera on the ant\n"
            _CYAN("H           ")                       _YELLOW(": ") "Help message\n"
            _CYAN("N           ")                       _YELLOW(": ") "Get the number of cubes\n"
            _CYAN("P           ")                       _YELLOW(": ") "Add checkpoints in the simulation\n"
            _CYAN("R           ")                       _YELLOW(": ") "Display the current rules\n" << std::endl;
}

void Simulation::scrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    _scene->getCamera().zoom((float) pow(1.2, - yoffset));
}